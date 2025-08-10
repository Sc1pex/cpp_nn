#pragma once

#include <Eigen/Core>
#include <ranges>
#include <vector>
#include "Eigen/src/Core/Matrix.h"

namespace nnet {

using Eigen::MatrixXd;

using Gradients = std::vector<std::pair<MatrixXd, MatrixXd>>;

class NN {
public:
    static NN init_random(const std::vector<int>& shape);

    MatrixXd feed_forward(const MatrixXd& input);

    int num_layers() const;

    // Returns the gradients of the weights and biases for multiple inputs and targets
    Gradients backprop_batch_matrix(const MatrixXd& inputs, const MatrixXd& targets);

    void apply_gradients(const Gradients& gradients, double learning_rate);

    template<std::ranges::input_range InputIter, std::ranges::input_range TargetIter>
        requires std::ranges::sized_range<InputIter>
    double cost(const InputIter& inputs, const TargetIter& targets) {
        double total_cost = 0.0;
        for (const auto& [input, target] : std::views::zip(inputs, targets)) {
            MatrixXd output = feed_forward(input);
            total_cost += (output - target).squaredNorm();
        }
        return total_cost / std::ranges::size(inputs);
    }

    double cost(const MatrixXd& inputs, const MatrixXd& targets);

private:
    std::vector<MatrixXd> m_weights;
    std::vector<MatrixXd> m_biases;
};

}
