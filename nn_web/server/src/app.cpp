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
            auto resp = co_await handler(req); \
            resp.to_response(res); \
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
        spdlog::error(
            "Failed to retrieve networks: {} {}", networks_res.error().message,
            networks_res.error().code
        );
        co_return ApiResponse::internal_error("Failed to retrieve networks");
    }

    co_return ApiResponse::ok(networks_res.value());
}

asio::awaitable<ApiResponse> App::get_network_by_id(const httc::Request& req) {
    int network_id;
    try {
        auto id_str = req.path_params.at("id");
        network_id = std::stoi(id_str);
    } catch (const std::exception&) {
        co_return ApiResponse::bad_request("Invalid network ID");
    }

    auto network_res = co_await m_state->db.get_network_by_id(network_id);
    if (!network_res) {
        spdlog::error(
            "Failed to retrieve network with id {}: {} {}", network_id, network_res.error().message,
            network_res.error().code
        );
        co_return ApiResponse::internal_error("Failed to retrieve network");
    }
    if (!network_res.value()) {
        spdlog::info("Network with id {} not found", network_id);
        co_return ApiResponse::not_found("Network not found");
    }

    co_return ApiResponse::ok(network_res.value().value());
}

asio::awaitable<ApiResponse> App::create_network(const httc::Request& req) {
    json j = json::parse(req.body);
    AddNetworkRequest add_req = j.get<AddNetworkRequest>();

    if (add_req.name.length() < 2) {
        co_return ApiResponse::field_error(
            { .field = "name", .error = "Name must be at least 2 characters long" }
        );
    }

    if (add_req.activations.size() + 1 != add_req.layer_sizes.size()) {
        co_return ApiResponse::field_error(
            { .field = "activations",
              .error = "Number of activations must be one less than number of layers" }
        );
    }

    auto hidden_activations_view =
        add_req.activations | std::views::take(add_req.activations.size() - 1);
    auto hidden_activations_opt = nn::strs_to_hidden_activation(hidden_activations_view);
    if (!hidden_activations_opt) {
        co_return ApiResponse::field_error(
            { .field = "activations", .error = "Invalid activation functions in hidden layers" }
        );
    }

    auto output_activation_req = add_req.activations.back();
    auto output_activation_opt = nn::str_to_output_activation(output_activation_req);
    if (!output_activation_opt) {
        co_return ApiResponse::field_error(
            { .field = "activations", .error = "Invalid activation function in output layer" }
        );
    }

    auto layer_error = validate_network_layers(add_req.layer_sizes);
    if (layer_error) {
        co_return ApiResponse::field_error(*layer_error);
    }

    auto output_activation = output_activation_opt.value();

    auto loss_opt = nn::str_to_loss(add_req.loss);
    if (!loss_opt) {
        co_return ApiResponse::field_error({ .field = "loss", .error = "Invalid loss function" });
    }
    auto loss = loss_opt.value();

    auto network_opt = nn::Network::new_random(
        add_req.layer_sizes, hidden_activations_opt.value(), output_activation, loss
    );
    if (!network_opt.has_value()) {
        spdlog::error("Invlid network configuration");
        co_return ApiResponse::internal_error("Failed to create network");
    }
    auto network = network_opt.value();

    AddNetwork db_network;
    db_network.name = add_req.name;
    db_network.layer_sizes = add_req.layer_sizes;
    db_network.activations = add_req.activations;
    db_network.loss = add_req.loss;
    db_network.weights = network.dump_weights();
    db_network.biases = network.dump_biases();

    auto add_res = co_await m_state->db.add_network(std::move(db_network));
    if (!add_res) {
        if (add_res.error().code == SQLITE_CONSTRAINT_UNIQUE) {
            spdlog::info("Network with name '{}' already exists", add_req.name);
            co_return ApiResponse::field_error(
                { .field = "name", .error = "Network with this name already exists" }
            );
        } else {
            spdlog::error(
                "Failed to add network: {} {}", add_res.error().message, add_res.error().code
            );
            co_return ApiResponse::internal_error("Failed to add network");
        }
    }

    co_return ApiResponse{ httc::StatusCode::CREATED,
                           json{ { "message", "Network created successfully" } } };
}

asio::awaitable<ApiResponse> App::remove_network(const httc::Request& req) {
    int network_id;
    try {
        auto id_str = req.path_params.at("id");
        network_id = std::stoi(id_str);
    } catch (const std::exception&) {
        co_return ApiResponse::bad_request("Invalid network ID");
    }

    auto delete_res = co_await m_state->db.delete_network_by_id(network_id);
    if (!delete_res) {
        spdlog::error(
            "Failed to delete network with id {}: {} {}", network_id, delete_res.error().message,
            delete_res.error().code
        );
        co_return ApiResponse::internal_error("Failed to delete network");
    }
    if (!delete_res.value()) {
        spdlog::info("Network with id {} not found", network_id);
        co_return ApiResponse::not_found("Network not found");
    }

    co_return ApiResponse::ok(json{ { "message", "Network deleted successfully" } });
}

asio::awaitable<ApiResponse> App::predict(const httc::Request& req) {
    int index;
    try {
        auto idx_str = req.path_params.at("idx");
        index = std::stoi(idx_str);
    } catch (const std::exception&) {
        co_return ApiResponse::bad_request("Invalid index");
    }

    int network_id;
    try {
        auto id_str = req.path_params.at("id");
        network_id = std::stoi(id_str);
    } catch (const std::exception&) {
        co_return ApiResponse::bad_request("Invalid network ID");
    }

    std::expected<std::optional<Sample>, DBError> sample_res;
    auto source = req.path_params.at("source");
    if (source == "test") {
        sample_res = co_await m_state->db.get_test_sample_by_index(index);
    } else if (source == "train") {
        sample_res = co_await m_state->db.get_train_sample_by_index(index);
    } else {
        co_return ApiResponse::bad_request("Invalid source");
    }

    if (!sample_res) {
        spdlog::error(
            "Failed to retrieve sample from source {} with index {}: {} {}", source, index,
            sample_res.error().message, sample_res.error().code
        );
        co_return ApiResponse::internal_error("Failed to retrieve training sample");
    }
    if (!sample_res.value()) {
        spdlog::info("Sample from source {} with index {} not found", source, index);
        co_return ApiResponse::not_found("Training sample not found");
    }

    auto network_res = co_await m_state->db.get_full_network_by_id(network_id);
    if (!network_res) {
        spdlog::error(
            "Failed to retrieve network with id {}: {} {}", network_id, network_res.error().message,
            network_res.error().code
        );
        co_return ApiResponse::internal_error("Failed to retrieve network");
    }
    if (!network_res.value()) {
        spdlog::info("Network with id {} not found", network_id);
        co_return ApiResponse::not_found("Network not found");
    }
    auto network_info = network_res.value().value();

    auto hidden_activations_view =
        network_info.activations | std::views::take(network_info.activations.size() - 1);
    auto output_activation_str = network_info.activations.back();

    // Should never be invalid since it was validated on insertion
    auto hidden_activations = nn::strs_to_hidden_activation(hidden_activations_view).value();
    auto output_activation = nn::str_to_output_activation(output_activation_str).value();
    auto loss = nn::str_to_loss(network_info.loss).value();

    auto network_opt = nn::Network::from_data(
        network_info.layer_sizes, network_info.weights, network_info.biases, hidden_activations,
        output_activation, loss
    );
    auto network = network_opt.value();

    auto sample = sample_res.value().value();
    auto input_matrix =
        Eigen::Map<Eigen::Matrix<uint8_t, 784, 1>>(sample.input.data()).cast<double>();
    Eigen::MatrixXd expected_output = Eigen::MatrixXd::Zero(10, 1);
    expected_output(sample.expected_output, 0) = 1.;
    auto prediction = network.predict(input_matrix, expected_output);

    co_return ApiResponse::ok(
        json{ { "output",
                std::vector<double>(
                    prediction.output.data(), prediction.output.data() + prediction.output.size()
                ) },
              { "loss", prediction.loss } }
    );
}

asio::awaitable<ApiResponse> App::predict_custom(const httc::Request& req) {
    int network_id;
    try {
        auto id_str = req.path_params.at("id");
        network_id = std::stoi(id_str);
    } catch (const std::exception&) {
        co_return ApiResponse::bad_request("Invalid network ID");
    }

    json j = json::parse(req.body);
    std::vector<double> input = j.at("input").get<std::vector<double>>();
    if (input.size() != 784) {
        co_return ApiResponse::bad_request("Input size must be 784");
    }

    auto network_res = co_await m_state->db.get_full_network_by_id(network_id);
    if (!network_res) {
        spdlog::error(
            "Failed to retrieve network with id {}: {} {}", network_id, network_res.error().message,
            network_res.error().code
        );
        co_return ApiResponse::internal_error("Failed to retrieve network");
    }
    if (!network_res.value()) {
        spdlog::info("Network with id {} not found", network_id);
        co_return ApiResponse::not_found("Network not found");
    }
    auto network_info = network_res.value().value();

    auto hidden_activations_view =
        network_info.activations | std::views::take(network_info.activations.size() - 1);
    auto output_activation_str = network_info.activations.back();

    // Should never be invalid since it was validated on insertion
    auto hidden_activations = nn::strs_to_hidden_activation(hidden_activations_view).value();
    auto output_activation = nn::str_to_output_activation(output_activation_str).value();
    auto loss = nn::str_to_loss(network_info.loss).value();

    auto network_opt = nn::Network::from_data(
        network_info.layer_sizes, network_info.weights, network_info.biases, hidden_activations,
        output_activation, loss
    );
    auto network = network_opt.value();

    auto input_matrix = Eigen::Map<Eigen::Matrix<double, 784, 1>>(input.data());
    auto output = network.feed_forward(input_matrix);

    co_return ApiResponse::ok(
        json{ { "output", std::vector<double>(output.data(), output.data() + output.size()) } }
    );
}

asio::awaitable<ApiResponse> App::get_data(const httc::Request& req) {
    int index;
    try {
        auto idx_str = req.path_params.at("idx");
        index = std::stoi(idx_str);
    } catch (const std::exception&) {
        co_return ApiResponse::bad_request("Invalid index");
    }

    std::expected<std::optional<Sample>, DBError> sample_res;

    auto source = req.path_params.at("source");
    if (source == "test") {
        sample_res = co_await m_state->db.get_test_sample_by_index(index);
    } else if (source == "train") {
        sample_res = co_await m_state->db.get_train_sample_by_index(index);
    } else {
        co_return ApiResponse::bad_request("Invalid source");
    }

    if (!sample_res) {
        spdlog::error(
            "Failed to retrieve sample from source {} with index {}: {} {}", source, index,
            sample_res.error().message, sample_res.error().code
        );
        co_return ApiResponse::internal_error("Failed to retrieve training sample");
    }
    if (!sample_res.value()) {
        spdlog::info("Sample from source {} with index {} not found", source, index);
        co_return ApiResponse::not_found("Training sample not found");
    }

    co_return ApiResponse::ok(sample_res.value().value());
}

void ApiResponse::to_response(httc::Response& res) {
    res.status = status;
    res.headers.set("Content-Type", "application/json");
    res.set_body(resp.dump());
}

void to_json(json& j, const FieldError& v) {
    j = json{
        { "field", v.field },
        { "error", v.error },
    };
}
