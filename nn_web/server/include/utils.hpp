#pragma once

#include <asio/awaitable.hpp>
#include <expected>
#include <format>
#include <httc/request.hpp>
#include "api_response.hpp"
#include "nn_lib/network.hpp"
#include "spdlog/spdlog.h"
#include "state.hpp"

std::expected<int, ApiResponse> parse_int_param(
    const httc::Request& req, std::string_view param_name, std::string_view error_msg
);

std::expected<Sample, ApiResponse>
    load_sample(const State& state, std::string_view source, int idx);

asio::awaitable<std::expected<std::pair<NetworkFull, nn::Network>, ApiResponse>>
    load_network(Db& db, int network_id);

template<typename T>
std::expected<T, ApiResponse> parse_json_body(const httc::Request& req) {
    try {
        if (req.body.empty())
            return std::unexpected(ApiResponse::bad_request("Missing request body"));
        return nlohmann::json::parse(req.body).get<T>();
    } catch (const std::exception& e) {
        return std::unexpected(
            ApiResponse::bad_request(std::string("Invalid JSON body: ") + e.what())
        );
    }
}

template<typename... Args>
ApiResponse map_db_error(
    DBError db_err, std::string_view api_message, std::format_string<Args...> log_fmt,
    Args&&... args
) {
    spdlog::error(
        "{}: {} {}", std::format(log_fmt, std::forward<Args>(args)...), db_err.message, db_err.code
    );
    return ApiResponse::internal_error(std::string(api_message));
}
