module;

#include <cmath>

export module nn:math;

import :types;

export f64 sigmoid(f64 x) {
    return 1.0 / (1.0 + exp(-x));
}

export f64 sigmoid_deriv(f64 x) {
    return x * (1.0 - x);
}
