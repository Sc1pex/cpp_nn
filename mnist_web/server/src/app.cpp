#include "app.h"
#include <httc/server.h>
#include <asio.hpp>
#include <memory>

App::App() {
    using namespace std::chrono_literals;

    m_router = std::make_shared<httc::Router>();
    m_router->route(
        "/", [](const httc::Request& req, httc::Response& res) -> asio::awaitable<void> {
            auto ctx = co_await asio::this_coro::executor;

            // Wait for 10 seconds
            auto timer = asio::steady_timer(ctx, 10s);
            co_await timer.async_wait(asio::use_awaitable);

            res.set_body("hello there");

            co_return;
        }
    );
}

void App::run() {
    httc::bind_and_listen("127.0.0.1", 6969, m_router);
}
