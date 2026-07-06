#pragma once

#include <asio/awaitable.hpp>
#include <httc/response.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <string_view>

struct FieldError {
    std::string field;
    std::string error;
};
void to_json(nlohmann::json& j, const FieldError& v);

struct ApiResponse {
    httc::StatusCode status;
    std::optional<nlohmann::json> resp;

    // Static helper to initialize streaming on a response
    static asio::awaitable<httc::Response::ChunkedStream> stream(
        httc::Response& res, httc::StatusCode status = httc::StatusCode::OK,
        std::string content_type = "text/plain"
    ) {
        res.status = status;
        res.headers.set("Content-Type", std::move(content_type));
        co_return co_await res.send_chunked();
    }

    // A sentinel response that indicates the handler already streamed everything
    static ApiResponse stream_end() {
        return ApiResponse{ httc::StatusCode::OK, std::nullopt };
    }

    static ApiResponse ok(nlohmann::json resp) {
        return ApiResponse{ httc::StatusCode::OK, resp };
    }

    static ApiResponse error(httc::StatusCode status, std::string_view message) {
        return ApiResponse{ status, nlohmann::json{ { "error", message } } };
    }

    static ApiResponse internal_error(std::string_view message) {
        return error(httc::StatusCode::INTERNAL_SERVER_ERROR, message);
    }

    static ApiResponse bad_request(std::string_view message) {
        return error(httc::StatusCode::BAD_REQUEST, message);
    }

    static ApiResponse not_found(std::string_view message) {
        return error(httc::StatusCode::NOT_FOUND, message);
    }

    static ApiResponse field_error(const FieldError& err) {
        return ApiResponse{ httc::StatusCode::BAD_REQUEST, err };
    }

    void to_response(httc::Response& res);
};
