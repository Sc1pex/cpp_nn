#pragma once

#include <Eigen/Core>
#include <ranges>
#include <vector>

namespace nnet {

using Eigen::MatrixXd;

using Gradients = std::vector<std::pair<MatrixXd, MatrixXd>>;

class NN {
public:
    static NN init_random(const std::vector<int>& shape);

    MatrixXd feed_forward(const MatrixXd& input);

    int num_layers() const;

    // Retunrs the gradients of the weights and biases for a single input-target pair
    void backprop(const MatrixXd& input, const MatrixXd& target, Gradients& gradients);

    // Returns the gradients of the weights and biases for a batch of inputs and targets
    template<std::ranges::input_range InputIter, std::ranges::input_range TargetIter>
        requires std::ranges::sized_range<InputIter>
    Gradients backprop_batch(const InputIter& inputs, const TargetIter& targets) {
        Gradients gradients(m_weights.size());
        for (auto [i, grad] : std::views::enumerate(gradients)) {
            grad.first = MatrixXd::Zero(m_weights[i].rows(), m_weights[i].cols());
            grad.second = MatrixXd::Zero(m_biases[i].rows(), m_biases[i].cols());
        }

        for (const auto& [input, target] : std::views::zip(inputs, targets)) {
            backprop(input, target, gradients);
        }

        // Average the gradients over the batch
        for (auto& grad : gradients) {
            grad.first /= std::ranges::size(inputs);
            grad.second /= std::ranges::size(inputs);
        }

        return gradients;
    }

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

private:
    std::vector<MatrixXd> m_weights;
    std::vector<MatrixXd> m_biases;
};

}
