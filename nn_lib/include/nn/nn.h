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
    MatrixXd feed_forward(const MatrixXd& input);

    // Calculate the cost of a matrix of row inputs and a matrix of row expected
    // values
    double cost(const MatrixXd& inputs, const MatrixXd& expected);

    void backprop(const MatrixXd& inputs, const MatrixXd& expected, double learn_rate);
    std::pair<std::vector<MatrixXd>, std::vector<MatrixXd>>
        compute_gradients(const MatrixXd& inputs, const MatrixXd& expected);

private:
    void apply_gradients(
        const std::vector<MatrixXd>& weight_grads, const std::vector<MatrixXd>& bias_grads,
        double learn_rate, double batch_size
    );

private:
    std::vector<Layer> m_layers;
};
