#pragma once

#include <httc/router.hpp>
#include <nlohmann/json.hpp>
#include "api_response.hpp"
#include "httc/response.hpp"
#include "state.hpp"

class App {
public:
    App(std::optional<std::string_view> static_assets_path);

    void run();

private:
    asio::awaitable<ApiResponse> get_networks(const httc::Request& req);
    asio::awaitable<ApiResponse> get_network_by_id(const httc::Request& req);
    asio::awaitable<ApiResponse> create_network(const httc::Request& req);
    asio::awaitable<ApiResponse> remove_network(const httc::Request& req);

    asio::awaitable<ApiResponse> predict(const httc::Request& req);
    asio::awaitable<ApiResponse> predict_custom(const httc::Request& req);
    asio::awaitable<ApiResponse> train_network(const httc::Request& req, httc::Response& res);

    asio::awaitable<ApiResponse> get_data(const httc::Request& req);
    asio::awaitable<ApiResponse> get_test_points(const httc::Request& req);
    asio::awaitable<ApiResponse> get_loss_points(const httc::Request& req);

private:
    std::optional<std::string_view> m_static_assets_path;

    std::shared_ptr<httc::Router> m_router;
    std::shared_ptr<State> m_state;
};
