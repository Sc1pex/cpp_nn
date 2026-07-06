#include "api_response.hpp"

using json = nlohmann::json;

void ApiResponse::to_response(httc::Response& res) {
    res.status = status;
    if (!resp.has_value()) {
        return;
    }
    res.headers.set("Content-Type", "application/json");
    res.set_body(resp->dump());
}

void to_json(json& j, const FieldError& v) {
    j = json{
        { "field", v.field },
        { "error", v.error },
    };
}
