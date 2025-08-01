#pragma once

#include <cmath>

inline double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

inline double sigmoid_deriv(double x) {
    double s = sigmoid(x);
    return s * (1.0 - s);
}
