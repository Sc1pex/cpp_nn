#include <httc/request.h>
#include <httc/response.h>
#include <httc/router.h>
#include <httc/server.h>
#include <asio.hpp>
#include <print>

int main() {
    auto router = std::make_shared<httc::Router>();
    router->route("/", [](const httc::Request&, httc::Response& res) {
        res.set_body("Hello, world");
    });

    asio::io_context io_ctx;
    int port = 8080;

    std::println("Listening on port {}", port);
    httc::bind_and_listen("0.0.0.0", port, router, io_ctx);

    io_ctx.run();
    return 0;
}
