#include "app.h"
#include <httc/server.h>
#include <memory>

App::App() {
    m_router = std::make_shared<httc::Router>();
    m_router->route("/", [](const httc::Request& req, httc::Response& res) {
        res.set_body("hello there");
    });
}

void App::run() {
    httc::bind_and_listen("127.0.0.1", 6969, m_router);
}
