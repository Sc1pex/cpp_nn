#include "app.h"
#include <httc/request.h>
#include <httc/response.h>
#include <httc/router.h>
#include <httc/server.h>
#include <sqlite3.h>
#include <asio.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include "api.h"

App::App() {
    m_state = std::make_shared<State>("mnist.db");
    m_router = std::make_shared<httc::Router>();

    m_router->wrap(
        [](const httc::Request& req, httc::Response& res, auto next) -> asio::awaitable<void> {
            // CORS middleware
            res.headers.set("Access-Control-Allow-Origin", "*");
            res.headers.set("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            co_return co_await next(req, res);
        }
    );
    network_api_routes(m_router, m_state);
}

void App::run() {
    asio::io_context io_ctx;
    httc::bind_and_listen("127.0.0.1", 6969, m_router, io_ctx);
    io_ctx.run();
}
