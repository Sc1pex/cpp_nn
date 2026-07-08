#pragma once

#include <expected>
#include <string>

struct DBError {
    int code;
    std::string message;
};
template<typename T>
using DBResult = std::expected<T, DBError>;
