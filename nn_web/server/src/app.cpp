#include "app.hpp"
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <asio.hpp>
#include <httc/request.hpp>
#include <httc/response.hpp>
#include <httc/router.hpp>
#include <httc/server.hpp>
#include <httc/status.hpp>
#include <httc/utils/file_handlers.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <nn_lib/activation.hpp>
#include <nn_lib/network.hpp>
#include "utils.hpp"

using json = nlohmann::json;

struct AddNetworkRequest {
    std::string name;
    std::vector<int> layer_sizes;
    std::vector<std::string> activations;
    std::string loss;
};

std::optional<FieldError> validate_network_layers(const std::vector<int>& layers) {
    if (layers.size() < 2) {
        return FieldError{ .field = "shape", .error = "Network must have at least 2 layers" };
    }
    if (layers.front() != 784) {
        return FieldError{ .field = "shape", .error = "Input layer must have 784 neurons" };
    }
    if (layers.back() != 10) {
        return FieldError{ .field = "shape", .error = "Output layer must have 10 neurons" };
    }

    return std::nullopt;
}

void from_json(const json& j, AddNetworkRequest& req) {
    j.at("name").get_to(req.name);
    j.at("layer_sizes").get_to(req.layer_sizes);
    j.at("activations").get_to(req.activations);
    j.at("loss").get_to(req.loss);
}

#define MAKE_API_ROUTE(method, handler) \
    method { \
        [this](auto& req, auto& res) -> asio::awaitable<void> { \
            if constexpr (requires { this->handler(req, res); }) { \
                auto resp = co_await this->handler(req, res); \
                resp.to_response(res); \
            } else { \
                auto resp = co_await this->handler(req); \
                resp.to_response(res); \
            } \
            co_return; \
        } \
    }

App::App(std::optional<std::string_view> static_assets_path)
: m_static_assets_path(static_assets_path) {
    m_state = std::make_shared<State>();
    m_router = std::make_shared<httc::Router>();

    // Logging middleware
    m_router->wrap(
        [](const httc::Request& req, httc::Response& res, auto next) -> asio::awaitable<void> {
        spdlog::info("Incoming request: {} {}", req.method, req.uri.path());
        co_await next(req, res);
        spdlog::info("Response status: {}", res.status.code);

        co_return;
    }
    );

    // CORS middleware
    m_router->wrap(
        [](const httc::Request& req, httc::Response& res, auto next) -> asio::awaitable<void> {
        res.headers.set("Access-Control-Allow-Origin", "*");
        res.headers.set("Access-Control-Allow-Methods", "GET, POST, OPTIONS, DELETE");
        res.headers.set(
            "Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With"
        );
        co_return co_await next(req, res);
    }
    );

    // Add routes
    using namespace httc::methods;

    if (m_static_assets_path) {
        m_router->route(
            "/", httc::utils::FileHandler(std::format("{}/index.html", *m_static_assets_path))
        );
        m_router->route(
            "/assets/*",
            httc::utils::DirectoryHandler(std::format("{}/assets", *m_static_assets_path))
        );
    }

    m_router->route("/api/networks", MAKE_API_ROUTE(get, get_networks));
    m_router->route("/api/networks/:id", MAKE_API_ROUTE(get, get_network_by_id));
    m_router->route("/api/networks", MAKE_API_ROUTE(post, create_network));
    m_router->route("/api/networks/:id", MAKE_API_ROUTE(del, remove_network));
    m_router->route("/api/networks/:id/predict/:source/:idx", MAKE_API_ROUTE(post, predict));
    m_router->route("/api/networks/:id/predict", MAKE_API_ROUTE(post, predict_custom));
    m_router->route("/api/networks/:id/train", MAKE_API_ROUTE(post, train_network));
    m_router->route("/api/networks/:id/test-points", MAKE_API_ROUTE(get, get_test_points));
    m_router->route("/api/networks/:id/loss-points", MAKE_API_ROUTE(get, get_loss_points));

    m_router->route("/api/data/:source/:idx", MAKE_API_ROUTE(get, get_data));
}

void App::run() {
    asio::io_context io_ctx;
    int port = 8080;

    spdlog::info("Listening on port {}", port);
    httc::bind_and_listen("0.0.0.0", port, m_router, io_ctx);

    io_ctx.run();
}

asio::awaitable<ApiResponse> App::get_networks(const httc::Request& req) {
    auto networks_res = co_await m_state->db.get_networks();
    if (!networks_res) {
        co_return map_db_error(
            networks_res.error(), "Failed to retrieve networks", "Failed to retrieve networks"
        );
    }

    co_return ApiResponse::ok(networks_res.value());
}

asio::awaitable<ApiResponse> App::get_network_by_id(const httc::Request& req) {
    auto network_id = parse_int_param(req, "id", "Invalid network ID");
    if (!network_id) {
        co_return network_id.error();
    }

    auto network_res = co_await m_state->db.get_network_by_id(*network_id);
    if (!network_res) {
        co_return map_db_error(
            network_res.error(), "Failed to retrieve network",
            "Failed to retrieve network with id {}", *network_id
        );
    }
    if (!network_res.value()) {
        co_return ApiResponse::not_found("Network not found");
    }

    co_return ApiResponse::ok(network_res.value().value());
}

asio::awaitable<ApiResponse> App::create_network(const httc::Request& req) {
    auto add_req = parse_json_body<AddNetworkRequest>(req);
    if (!add_req) {
        co_return add_req.error();
    }

    if (add_req->name.length() < 2) {
        co_return ApiResponse::field_error(
            { .field = "name", .error = "Name must be at least 2 characters long" }
        );
    }
    if (add_req->name.length() > 32) {
        co_return ApiResponse::field_error(
            { .field = "name", .error = "Name must be less than 32 characters long" }
        );
    }

    if (add_req->activations.size() + 1 != add_req->layer_sizes.size()) {
        co_return ApiResponse::field_error(
            { .field = "activations",
              .error = "Number of activations must be one less than number of layers" }
        );
    }

    auto hidden_activations_view =
        add_req->activations | std::views::take(add_req->activations.size() - 1);
    auto hidden_activations_opt = nn::strs_to_hidden_activation(hidden_activations_view);
    if (!hidden_activations_opt) {
        co_return ApiResponse::field_error(
            { .field = "activations", .error = "Invalid activation functions in hidden layers" }
        );
    }

    auto output_activation_req = add_req->activations.back();
    auto output_activation_opt = nn::str_to_output_activation(output_activation_req);
    if (!output_activation_opt) {
        co_return ApiResponse::field_error(
            { .field = "activations", .error = "Invalid activation function in output layer" }
        );
    }

    auto layer_error = validate_network_layers(add_req->layer_sizes);
    if (layer_error) {
        co_return ApiResponse::field_error(*layer_error);
    }

    auto output_activation = output_activation_opt.value();

    auto loss_opt = nn::str_to_loss(add_req->loss);
    if (!loss_opt) {
        co_return ApiResponse::field_error({ .field = "loss", .error = "Invalid loss function" });
    }
    auto loss = loss_opt.value();

    auto network_opt = nn::Network::new_random(
        add_req->layer_sizes, hidden_activations_opt.value(), output_activation, loss
    );
    if (!network_opt.has_value()) {
        spdlog::error("Invlid network configuration");
        co_return ApiResponse::internal_error("Failed to create network");
    }
    auto network = network_opt.value();

    AddNetwork db_network;
    db_network.name = add_req->name;
    db_network.layer_sizes = add_req->layer_sizes;
    db_network.activations = add_req->activations;
    db_network.loss = add_req->loss;
    db_network.weights = network.dump_weights();
    db_network.biases = network.dump_biases();

    auto add_res = co_await m_state->db.add_network(std::move(db_network));
    if (!add_res) {
        if (add_res.error().code == SQLITE_CONSTRAINT_UNIQUE) {
            spdlog::info("Network with name '{}' already exists", add_req->name);
            co_return ApiResponse::field_error(
                { .field = "name", .error = "Network with this name already exists" }
            );
        } else {
            co_return map_db_error(
                add_res.error(), "Failed to add network", "Failed to add network"
            );
        }
    }

    co_return ApiResponse{ httc::StatusCode::CREATED,
                           json{ { "message", "Network created successfully" } } };
}

asio::awaitable<ApiResponse> App::remove_network(const httc::Request& req) {
    auto network_id = parse_int_param(req, "id", "Invalid network ID");
    if (!network_id) {
        co_return network_id.error();
    }

    auto delete_res = co_await m_state->db.delete_network_by_id(*network_id);
    if (!delete_res) {
        co_return map_db_error(
            delete_res.error(), "Failed to delete network", "Failed to delete network with id {}",
            *network_id
        );
    }
    if (!delete_res.value()) {
        spdlog::info("Network with id {} not found", *network_id);
        co_return ApiResponse::not_found("Network not found");
    }

    co_return ApiResponse::ok(json{ { "message", "Network deleted successfully" } });
}

asio::awaitable<ApiResponse> App::predict(const httc::Request& req) {
    auto index = parse_int_param(req, "idx", "Invalid index");
    if (!index) {
        co_return index.error();
    }

    auto network_id = parse_int_param(req, "id", "Invalid network ID");
    if (!network_id) {
        co_return network_id.error();
    }

    auto sample = load_sample(*m_state, req.path_params.at("source"), *index);
    if (!sample) {
        co_return sample.error();
    }

    auto network_res = co_await load_network(m_state->db, *network_id);
    if (!network_res) {
        co_return network_res.error();
    }
    auto network = network_res->second;

    auto prediction = network.predict(sample->input, sample->label);

    co_return ApiResponse::ok(
        json{ { "output",
                std::vector<double>(
                    prediction.output.data(), prediction.output.data() + prediction.output.size()
                ) },
              { "loss", prediction.loss } }
    );
}

struct PredictRequest {
    std::vector<double> input;
};

void from_json(const json& j, PredictRequest& req) {
    j.at("input").get_to(req.input);
}

asio::awaitable<ApiResponse> App::predict_custom(const httc::Request& req) {
    auto network_id = parse_int_param(req, "id", "Invalid network ID");
    if (!network_id) {
        co_return network_id.error();
    }

    auto predict_req = parse_json_body<PredictRequest>(req);
    if (!predict_req) {
        co_return predict_req.error();
    }

    auto input = predict_req->input;
    if (input.size() != 784) {
        co_return ApiResponse::bad_request("Input size must be 784");
    }

    auto network_res = co_await load_network(m_state->db, *network_id);
    if (!network_res) {
        co_return network_res.error();
    }
    auto network = network_res->second;

    auto input_matrix = Eigen::Map<Eigen::Matrix<double, 784, 1>>(input.data());
    auto output = network.feed_forward(input_matrix);

    co_return ApiResponse::ok(
        json{ { "output", std::vector<double>(output.data(), output.data() + output.size()) } }
    );
}

struct TrainRequest {
    int epochs = 1;
    int batch_size = 32;
    double learning_rate = 0.01;
};

void from_json(const json& j, TrainRequest& req) {
    if (j.contains("epochs"))
        j.at("epochs").get_to(req.epochs);
    if (j.contains("batch_size"))
        j.at("batch_size").get_to(req.batch_size);
    if (j.contains("learning_rate"))
        j.at("learning_rate").get_to(req.learning_rate);
}

asio::awaitable<ApiResponse> App::train_network(const httc::Request& req, httc::Response& res) {
    auto network_id = parse_int_param(req, "id", "Invalid network ID");
    if (!network_id) {
        co_return network_id.error();
    }

    auto train_req = parse_json_body<TrainRequest>(req);
    if (!train_req) {
        co_return train_req.error();
    }

    auto network_res = co_await load_network(m_state->db, *network_id);
    if (!network_res) {
        co_return network_res.error();
    }
    auto [network_info, network] = network_res.value();

    auto inputs = m_state->train_inputs.mat();
    auto labels = m_state->train_labels.mat();
    auto test_inputs = m_state->test_inputs.mat();
    auto test_labels = m_state->test_labels.mat();

    auto stream = co_await ApiResponse::stream(res);

    // ~10 loss reports per epoch regardless of batch size
    int num_batches =
        (static_cast<int>(inputs.cols()) + train_req->batch_size - 1) / train_req->batch_size;
    int log_interval = std::max(1, num_batches / 10);

    nn::SGDHyperparams hyperparams{ train_req->learning_rate, 1, train_req->batch_size,
                                    log_interval };

    int step = 0;
    for (int epoch = 0; epoch < train_req->epochs; epoch++) {
        auto epoch_losses = co_await asio::co_spawn(
            req.thread_pool_executor(),
            [&]() -> asio::awaitable<std::optional<std::vector<double>>> {
            co_return network.train_sgd(inputs, labels, hyperparams);
        }, asio::use_awaitable
        );

        double sum_loss = 0.0;
        if (epoch_losses) {
            for (double loss_val : *epoch_losses) {
                sum_loss += loss_val;

                auto global_step = step + network_info.loss_points;

                auto res =
                    co_await m_state->db.insert_loss_point(*network_id, global_step, loss_val);
                if (!res) {
                    spdlog::error(
                        "Failed to insert loss point: {} {}", res.error().message, res.error().code
                    );
                    co_await stream.write("error: Failed to persist training data\n");
                    co_return ApiResponse::stream_end();
                }

                // Stream one loss event per log-interval window
                auto msg =
                    json{
                        { "type", "loss" },
                        { "step", global_step },
                        { "epoch", epoch + 1 },
                        { "loss", loss_val },
                    }
                        .dump();
                co_await stream.write(msg + "\n");

                step++;
            }
        }

        int test_correct =
            co_await asio::co_spawn(req.thread_pool_executor(), [&]() -> asio::awaitable<int> {
            co_return network.evaluate_onehot(test_inputs, test_labels);
        }, asio::use_awaitable);

        int absolute_epoch = network_info.training_epochs + epoch + 1;
        auto res =
            co_await m_state->db.insert_training_session(*network_id, absolute_epoch, test_correct);
        if (!res) {
            spdlog::error(
                "Failed to insert training session: {} {}", res.error().message, res.error().code
            );
            co_await stream.write("error: Failed to persist training session\n");
            co_return ApiResponse::stream_end();
        }

        // Stream epoch summary event
        auto epoch_msg =
            json{
                { "type", "epoch" },
                { "epoch", epoch + 1 },
                { "test_correct", test_correct },
            }
                .dump();
        co_await stream.write(epoch_msg + "\n");
    }

    auto update_res = co_await m_state->db.update_network_weights(
        *network_id, std::move(network.dump_weights()), std::move(network.dump_biases()),
        train_req->epochs, step
    );
    if (!update_res) {
        spdlog::error(
            "Failed to update network weights: {} {}", update_res.error().message,
            update_res.error().code
        );
        co_await stream.write("error: Failed to save trained network\n");
    }

    co_return ApiResponse::stream_end();
}

asio::awaitable<ApiResponse> App::get_data(const httc::Request& req) {
    auto index = parse_int_param(req, "idx", "Invalid index");
    if (!index) {
        co_return index.error();
    }

    auto sample = load_sample(*m_state, req.path_params.at("source"), *index);
    if (!sample) {
        co_return sample.error();
    }
    co_return ApiResponse::ok(*sample);
}

asio::awaitable<ApiResponse> App::get_test_points(const httc::Request& req) {
    auto network_id = parse_int_param(req, "id", "Invalid network ID");
    if (!network_id) {
        co_return network_id.error();
    }

    auto sessions_res = co_await m_state->db.get_test_points(*network_id);
    if (!sessions_res) {
        co_return map_db_error(
            sessions_res.error(), "Failed to retrieve training sessions",
            "Failed to retrieve training sessions for network {}", *network_id
        );
    }

    co_return ApiResponse::ok(sessions_res.value());
}

asio::awaitable<ApiResponse> App::get_loss_points(const httc::Request& req) {
    auto network_id = parse_int_param(req, "id", "Invalid network ID");
    if (!network_id) {
        co_return network_id.error();
    }

    auto points_res = co_await m_state->db.get_loss_points(*network_id);
    if (!points_res) {
        co_return map_db_error(
            points_res.error(), "Failed to retrieve loss points",
            "Failed to retrieve loss points for network {}", *network_id
        );
    }

    co_return ApiResponse::ok(points_res.value());
}
