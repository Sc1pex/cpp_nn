#include <httc/server.h>
#include <memory>
#include "httc/request.h"
#include "httc/response.h"
#include "httc/router.h"

int main() {
    auto r = std::make_shared<httc::Router>();
    r->route("/", [](const httc::Request& req, httc::Response& res) {
        res.set_body("hello there");
    });

    httc::bind_and_listen("127.0.0.1", 6969, r);
}
