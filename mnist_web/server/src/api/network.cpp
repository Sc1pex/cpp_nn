#include <httc/request.h>
#include <httc/response.h>
#include <httc/router.h>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include "api.h"

using json = nlohmann::json;

struct FieldError {
    std::string field;
    std::string error;
};

void to_json(json& j, const FieldError& fe) {
    j = json{ { "field", fe.field }, { "error", fe.error } };
}

std::optional<FieldError> validate_network(AddNetwork& net) {
    if (net.shape.size() < 2) {
        return FieldError{ .field = "shape", .error = "Network shape must have at least 2 layers" };
    }
    if (net.shape[0] != 784) {
        return FieldError{ .field = "shape", .error = "Input layer must have 784 neurons" };
    }
    if (net.shape.back() != 10) {
        return FieldError{ .field = "shape", .error = "Output layer must have 10 neurons" };
    }

    return std::nullopt;
}

void network_api_routes(std::shared_ptr<httc::Router> router, std::shared_ptr<State> state) {
    router->route(
        "/api/networks",
        httc::MethodWrapper<"GET">(
            [state](const httc::Request& req, httc::Response& res) -> asio::awaitable<void> {
                auto networks_res = co_await state->db.get_networks();
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
    router->route(
        "/api/addNetwork",
        httc::MethodWrapper<"POST">(
            [state](const httc::Request& req, httc::Response& res) -> asio::awaitable<void> {
                auto body_json = json::parse(req.body);
                std::string name = body_json["name"];
                std::vector<int> shape = body_json["shape"].get<std::vector<int>>();

                AddNetwork network{ .name = name,
                                    .shape = shape,
                                    .weights = { 0 },
                                    .biases = { 0 } };

                auto validation_error = validate_network(network);
                if (validation_error) {
                    res.status.code = 400;
                    res.headers.set("Content-Type", "application/json");
                    json error_json = *validation_error;
                    res.set_body(error_json.dump());
                    co_return;
                }

                auto db_res = co_await state->db.add_network(std::move(network));
                if (!db_res) {
                    if (db_res.error().code == SQLITE_CONSTRAINT_PRIMARYKEY) {
                        auto error_json =
                            json{ { "field", "name" },
                                  { "error", "Network with this name already exists" } };
                        res.status.code = 400;
                        res.headers.set("Content-Type", "application/json");
                        res.set_body(error_json.dump());
                        co_return;
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
    router->route(
        "/api/deleteNetwork",
        httc::MethodWrapper<"POST">(
            [state](const httc::Request& req, httc::Response& res) -> asio::awaitable<void> {
                auto body_json = json::parse(req.body);
                std::string name = body_json["name"];

                auto db_res = co_await state->db.delete_network(name);
                if (!db_res.has_value()) {
                    throw std::runtime_error("Failed to delete network from db");
                }

                auto deleted = db_res.value();
                if (!deleted) {
                    res.status.code = 404;
                    res.headers.set("Content-Type", "application/json");
                    res.set_body(R"({"error":"Network not found"})");
                } else {
                    res.status.code = 200;
                    res.headers.set("Content-Type", "application/json");
                    res.set_body(R"({"status":"Network deleted"})");
                }

                co_return;
            }
        )
    );
}
