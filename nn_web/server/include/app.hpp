#pragma once

#include <httc/router.hpp>
#include "state.hpp"

class App {
public:
    App(std::optional<std::string_view> static_assets_path);

    void run();

private:
    asio::awaitable<void> get_networks(const httc::Request& req, httc::Response& res);
    asio::awaitable<void> get_network_by_id(const httc::Request& req, httc::Response& res);
    asio::awaitable<void> create_network(const httc::Request& req, httc::Response& res);
    asio::awaitable<void> remove_network(const httc::Request& req, httc::Response& res);

    asio::awaitable<void> predict(const httc::Request& req, httc::Response& res);
    asio::awaitable<void> predict_custom(const httc::Request& req, httc::Response& res);

    asio::awaitable<void> get_data(const httc::Request& req, httc::Response& res);

private:
    std::optional<std::string_view> m_static_assets_path;

    std::shared_ptr<httc::Router> m_router;
    std::shared_ptr<State> m_state;
};
