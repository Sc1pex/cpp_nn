#pragma once

#include <ranges>
#include <vector>
#include "nn/layer.h"

class NN {
public:
    template<std::ranges::range T>
    NN(T shape) {
        auto layers = shape | std::views::slide(2) | std::views::transform([](auto r) {
                          return Layer::random(r[0], r[1]);
                      });
        m_layers = std::ranges::to<std::vector<Layer>>(layers);
    }

    // Feed forward a row matrix
    Matrix feed_forward(const Matrix& input);

    // Calculate the cost of a matrix of row inputs and a matrix of row expected
    // values
    f64 cost(const Matrix& inputs, const Matrix& expected);

    void backprop(const Matrix& inputs, const Matrix& expected, f64 learn_rate);
    std::pair<std::vector<Matrix>, std::vector<Matrix>>
        compute_gradients(const Matrix& inputs, const Matrix& expected);

private:
    void apply_gradients(
        const std::vector<Matrix>& weight_grads, const std::vector<Matrix>& bias_grads,
        f64 learn_rate, f64 batch_size
    );

private:
    std::vector<Layer> m_layers;
};
