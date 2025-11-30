#pragma once

#include <cmath>
#include <variant>

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

}
