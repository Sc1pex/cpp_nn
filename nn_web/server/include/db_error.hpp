#pragma once

#include <expected>
#include <string_view>

struct DBError {
    int code;
    std::string_view message;
};
template<typename T>
using DBResult = std::expected<T, DBError>;
