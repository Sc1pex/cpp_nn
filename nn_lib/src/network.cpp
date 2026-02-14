#include "nn_lib/network.hpp"
#include <ranges>

namespace nn {

std::optional<Network> Network::new_random(
    const std::vector<int>& layer_sizes, const std::vector<Activation>& activations
) {
    if (layer_sizes.size() < 2 || layer_sizes.size() - 1 != activations.size()) {
        return std::nullopt;
    }

    std::vector<MatrixXd> weights;
    std::vector<VectorXd> biases;

    for (size_t i = 0; i < layer_sizes.size() - 1; ++i) {
        double limit = std::sqrt(6.0 / layer_sizes[i]);
        MatrixXd W = MatrixXd::Random(layer_sizes[i + 1], layer_sizes[i]) * limit;
        VectorXd b = VectorXd::Zero(layer_sizes[i + 1]);

        weights.push_back(W);
        biases.push_back(b);
    }
    return Network(weights, biases, activations);
}

std::optional<Network> Network::from_data(
    const std::vector<int>& layer_sizes, const std::vector<double>& weights,
    const std::vector<double>& biases, const std::vector<Activation>& activations
) {
    if (layer_sizes.size() < 2 || layer_sizes.size() - 1 != activations.size()) {
        return std::nullopt;
    }

    std::vector<MatrixXd> weight_matrices;
    std::vector<VectorXd> bias_vectors;

    size_t weight_index = 0;
    size_t bias_index = 0;

    for (size_t i = 0; i < layer_sizes.size() - 1; ++i) {
        int rows = layer_sizes[i + 1];
        int cols = layer_sizes[i];
        size_t weight_count = rows * cols;

        if (weight_index + weight_count > weights.size()) {
            return std::nullopt;
        }

        MatrixXd W(rows, cols);
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                W(r, c) = weights[weight_index++];
            }
        }
        weight_matrices.push_back(W);

        if (bias_index + rows > biases.size()) {
            return std::nullopt;
        }

        VectorXd b(rows);
        for (int r = 0; r < rows; ++r) {
            b(r) = biases[bias_index++];
        }
        bias_vectors.push_back(b);
    }

    return Network(weight_matrices, bias_vectors, activations);
}

Network::Network(
    const std::vector<MatrixXd>& weights, const std::vector<VectorXd>& biases,
    const std::vector<Activation>& activations
)
: m_weights(weights), m_biases(biases), m_activations(activations) {
    assert(m_weights.size() >= 1);
    assert(m_weights.size() == m_biases.size());
    assert(m_weights.size() == m_activations.size());
}

MatrixXd Network::feed_forward(const MatrixXd& input) const {
    MatrixXd out = input;

    for (auto [W, b, act] : std::views::zip(m_weights, m_biases, m_activations)) {
        out = (W * out).colwise() + b;
        std::visit(
            [&out](auto&& activation) {
                out = out.unaryExpr([&activation](double x) {
                    return activation.function(x);
                });
            },
            act
        );
    }

    return out;
}

std::vector<double> Network::dump_weights() {
    std::vector<double> dumped;
    for (const auto& W : m_weights) {
        for (int r = 0; r < W.rows(); ++r) {
            for (int c = 0; c < W.cols(); ++c) {
                dumped.push_back(W(r, c));
            }
        }
    }
    return dumped;
}

std::vector<double> Network::dump_biases() {
    std::vector<double> dumped;
    for (const auto& b : m_biases) {
        for (int i = 0; i < b.size(); ++i) {
            dumped.push_back(b(i));
        }
    }
    return dumped;
}

}
