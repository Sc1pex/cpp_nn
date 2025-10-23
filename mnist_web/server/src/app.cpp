#include "app.h"
#include <httc/router.h>
#include <httc/server.h>
#include <sqlite3.h>
#include <asio.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

struct FieldError {
    std::string field;
    std::string message;
};

App::App() : m_db("mnist.db") {
    m_router = std::make_shared<httc::Router>();
    m_router->route(
        "/api/networks",
        httc::MethodWrapper<"GET">(
            [this](const httc::Request& req, httc::Response& res) -> asio::awaitable<void> {
                auto networks_res = co_await m_db.get_networks();
                if (!networks_res) {
                    throw std::runtime_error("Failed to get networks from db");
                }
                auto networks = networks_res.value();

                json body = json::array();
                res.headers.set("Content-Type", "application/json");

                for (size_t i = 0; i < networks.size(); ++i) {
                    json net_json;
                    net_json["name"] = networks[i].name;
                    net_json["shape"] = networks[i].shape;
                    net_json["training_iterations"] = networks[i].training_iterations;
                    net_json["cost"] = networks[i].cost;
                    body.push_back(net_json);
                }

                res.set_body(body.dump());
                co_return;
            }
        )
    );
    m_router->route(
        "/api/networks",
        httc::MethodWrapper<"POST">(
            [this](const httc::Request& req, httc::Response& res) -> asio::awaitable<void> {
                auto body_json = json::parse(req.body);
                std::string name = body_json["name"];
                std::vector<int> shape = body_json["shape"].get<std::vector<int>>();

                AddNetwork network{ .name = name,
                                    .shape = shape,
                                    .weights = { 0 },
                                    .biases = { 0 } };

                std::optional<FieldError> validation_error = std::nullopt;

                auto db_res = co_await m_db.add_network(std::move(network));
                if (!db_res) {
                    if (db_res.error().code == SQLITE_CONSTRAINT_PRIMARYKEY) {
                        validation_error =
                            FieldError{ .field = "name",
                                        .message = "Network with this name already exists" };
                    } else {
                        throw std::runtime_error("Failed to add network to db");
                    }
                }

                res.status.code = 201;
                res.headers.set("Content-Type", "application/json");
                res.set_body(R"({"status":"Network added"})");
                co_return;
            }
        )
    );
}

void App::run() {
    asio::io_context io_ctx;
    httc::bind_and_listen("127.0.0.1", 6969, m_router, io_ctx);
    io_ctx.run();
}
