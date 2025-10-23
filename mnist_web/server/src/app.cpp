#include "app.h"
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

    network_api_routes(m_router, m_state);
}

void App::run() {
    asio::io_context io_ctx;
    httc::bind_and_listen("127.0.0.1", 6969, m_router, io_ctx);
    io_ctx.run();
}
