#pragma once

#include <cmath>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace nn {

namespace activation {

struct ReLU {
    double function(double x) const {
        return x > 0 ? x : 0;
    }
    double derivative(double x) const {
        return x > 0 ? 1.0 : 0.0;
    }
};

struct Sigmoid {
    double function(double x) const {
        return 1.0 / (1.0 + std::exp(-x));
    }
    double derivative(double x) const {
        double s = function(x);
        return s * (1.0 - s);
    }
};

struct None {
    double function(double x) const {
        return x;
    }
    double derivative(double) const {
        return 1.0;
    }
};

}

using Activation = std::variant<activation::ReLU, activation::Sigmoid, activation::None>;

std::optional<Activation> str_to_activation(std::string_view s);
std::optional<std::vector<Activation>> strs_to_activation(const std::vector<std::string>& v);

}
