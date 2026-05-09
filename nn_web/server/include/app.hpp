#pragma once

#include <httc/router.hpp>
#include <httc/status.hpp>
#include <nlohmann/json.hpp>
#include "state.hpp"

struct FieldError {
    std::string field;
    std::string error;
};
void to_json(json& j, const FieldError& v);

struct ApiResponse {
    httc::StatusCode status;
    json resp;

    static ApiResponse ok(json resp) {
        return ApiResponse{ httc::StatusCode::OK, resp };
    }

    static ApiResponse error(httc::StatusCode status, std::string_view message) {
        return ApiResponse{ status, json{ { "error", message } } };
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

class App {
public:
    App(std::optional<std::string_view> static_assets_path);

    void run();

private:
    asio::awaitable<ApiResponse> get_networks(const httc::Request& req);
    asio::awaitable<ApiResponse> get_network_by_id(const httc::Request& req);
    asio::awaitable<ApiResponse> create_network(const httc::Request& req);
    asio::awaitable<ApiResponse> remove_network(const httc::Request& req);

    asio::awaitable<ApiResponse> predict(const httc::Request& req);
    asio::awaitable<ApiResponse> predict_custom(const httc::Request& req);

    asio::awaitable<ApiResponse> get_data(const httc::Request& req);

private:
    std::optional<std::string_view> m_static_assets_path;

    std::shared_ptr<httc::Router> m_router;
    std::shared_ptr<State> m_state;
};
