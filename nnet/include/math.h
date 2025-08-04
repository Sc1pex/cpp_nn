#pragma once

#include <cmath>

inline double sigmoid(double z) {
    return 1.0 / (1.0 + exp(-z));
}

inline double sigmoid_derivative(double z) {
    double sig = sigmoid(z);
    return sig * (1.0 - sig);
}
