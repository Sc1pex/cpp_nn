#pragma once

#include <Eigen/Core>
#include <ranges>
#include <vector>

namespace nnet {

using Eigen::MatrixXd;

class NN {
public:
    static NN init_random(const std::vector<int>& shape);

    MatrixXd feed_forward(const MatrixXd& input);

    // Retunrs the gradients of the weights and biases for a single input-target pair
    std::vector<std::pair<MatrixXd, MatrixXd>>
        backprop(const MatrixXd& input, const MatrixXd& target);

    // Returns the gradients of the weights and biases for a batch of inputs and targets
    template<std::ranges::range Iter>
    std::vector<std::pair<MatrixXd, MatrixXd>>
        backprop_batch(const Iter& inputs, const Iter& targets) {
        std::vector<std::pair<MatrixXd, MatrixXd>> gradients(m_weights.size());
        for (auto [i, grad] : std::views::enumerate(gradients)) {
            grad.first = MatrixXd::Zero(m_weights[i].rows(), m_weights[i].cols());
            grad.second = MatrixXd::Zero(m_biases[i].rows(), m_biases[i].cols());
        }

        for (const auto& [input, target] : std::views::zip(inputs, targets)) {
            auto batch_gradients = backprop(input, target);

            for (size_t i = 0; i < m_weights.size(); ++i) {
                gradients[i].first += batch_gradients[i].first;
                gradients[i].second += batch_gradients[i].second;
            }
        }

        // Average the gradients over the batch
        for (auto& grad : gradients) {
            grad.first /= inputs.size();
            grad.second /= inputs.size();
        }

        return gradients;
    }

    void apply_gradients(
        const std::vector<std::pair<MatrixXd, MatrixXd>>& gradients, double learning_rate
    );

    template<std::ranges::range Iter>
    double cost(const Iter& inputs, const Iter& targets) {
        double total_cost = 0.0;
        for (const auto& [input, target] : std::views::zip(inputs, targets)) {
            MatrixXd output = feed_forward(input);
            total_cost += (output - target).squaredNorm();
        }
        return total_cost / inputs.size();
    }

private:
    std::vector<MatrixXd> m_weights;
    std::vector<MatrixXd> m_biases;
};

}
