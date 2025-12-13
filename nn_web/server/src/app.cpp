#include "app.h"
#include <httc/request.h>
#include <httc/response.h>
#include <httc/router.h>
#include <httc/server.h>
#include <nn_lib/activation.h>
#include <nn_lib/network.h>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <asio.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include "httc/status.h"

using json = nlohmann::json;

struct AddNetworkRequest {
    std::string name;
    std::vector<int> layer_sizes;
    std::vector<std::string> activations;
};

void from_json(const json& j, AddNetworkRequest& req) {
    j.at("name").get_to(req.name);
    j.at("layer_sizes").get_to(req.layer_sizes);
    j.at("activations").get_to(req.activations);
}

void App::add_network_routes() {
    m_router->route(
        "/api/networks",
        httc::MethodWrapper<"GET">{
            [this](const httc::Request& req, httc::Response& res) -> asio::awaitable<void> {
                auto networks_res = co_await m_state->db.get_networks();
                if (!networks_res) {
                    spdlog::error(
                        "Failed to retrieve networks: {} {}", networks_res.error().message,
                        networks_res.error().code
                    );
                    res.status = httc::StatusCode::INTERNAL_SERVER_ERROR;
                    res.set_body("Failed to retrieve networks");
                    co_return;
                }

                json j;
                j["networks"] = networks_res.value();
                res.headers.set("Content-Type", "application/json");
                res.set_body(j.dump());

                co_return;
            } }
    );
    m_router->route(
        "/api/networks/:id",
        httc::MethodWrapper<"GET">{
            [this](const httc::Request& req, httc::Response& res) -> asio::awaitable<void> {
                int network_id;
                try {
                    auto id_str = req.path_params.at("id");
                    network_id = std::stoi(id_str);
                } catch (const std::exception&) {
                    res.status = httc::StatusCode::BAD_REQUEST;
                    res.set_body("Invalid network ID");
                    co_return;
                }

                auto network_res = co_await m_state->db.get_network_by_id(network_id);
                if (!network_res) {
                    res.status = httc::StatusCode::INTERNAL_SERVER_ERROR;
                    res.set_body("Failed to retrieve network");
                    co_return;
                }

                if (!network_res.value()) {
                    res.status = httc::StatusCode::NOT_FOUND;
                    res.set_body("Network not found");
                    co_return;
                }

                json j = network_res.value().value();
                res.headers.set("Content-Type", "application/json");
                res.set_body(j.dump());

                co_return;
            } }
    );
    m_router->route(
        "/api/networks",
        httc::MethodWrapper<"POST">{
            [this](const httc::Request& req, httc::Response& res) -> asio::awaitable<void> {
                json j = json::parse(req.body);
                AddNetworkRequest add_req = j.get<AddNetworkRequest>();

                auto activations_opt = nn::strs_to_activation(add_req.activations);
                if (!activations_opt) {
                    res.status = httc::StatusCode::BAD_REQUEST;
                    res.set_body("Invalid activation functions");
                    co_return;
                }

                nn::Network network =
                    nn::Network::new_random(add_req.layer_sizes, activations_opt.value());

                AddNetwork db_network;
                db_network.name = add_req.name;
                db_network.layer_sizes = add_req.layer_sizes;
                db_network.activations = add_req.activations;
                db_network.weights = network.dump_weights();
                db_network.biases = network.dump_biases();

                auto add_res = co_await m_state->db.add_network(std::move(db_network));
                if (!add_res) {
                    res.status = httc::StatusCode::INTERNAL_SERVER_ERROR;
                    res.set_body("Failed to add network");
                    co_return;
                }

                res.status = httc::StatusCode::CREATED;
                res.set_body("Network added successfully");
                co_return;
            } }
    );
}

App::App() {
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

    add_network_routes();
}

void App::run() {
    asio::io_context io_ctx;
    int port = 8080;

    spdlog::info("Listening on port {}", port);
    httc::bind_and_listen("0.0.0.0", port, m_router, io_ctx);

    io_ctx.run();
}
