#include <httc/request.h>
#include <httc/response.h>
#include <httc/router.h>
#include <httc/server.h>
#include <spdlog/spdlog.h>
#include <asio.hpp>
#include "db.h"

int main() {
    Db db(
        "nn_web.db", "data/train-images-idx3-ubyte", "data/train-labels-idx1-ubyte",
        "data/t10k-images-idx3-ubyte", "data/t10k-labels-idx1-ubyte"
    );

    auto router = std::make_shared<httc::Router>();
    router->wrap(
        [](const httc::Request& req, httc::Response& res, auto next) -> asio::awaitable<void> {
            spdlog::info("Incoming request: {} {}", req.method, req.uri.path());
            co_await next(req, res);
            spdlog::info("Response status: {}", res.status.code);
        }
    );
    router->route("/", [](const httc::Request&, httc::Response& res) {
        res.set_body("Hello, world");
    });

    asio::io_context io_ctx;
    int port = 8080;

    spdlog::info("Listening on port {}", port);
    httc::bind_and_listen("0.0.0.0", port, router, io_ctx);

    io_ctx.run();
    return 0;
}
