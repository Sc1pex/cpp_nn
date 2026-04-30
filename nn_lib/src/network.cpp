#include "nn_lib/network.hpp"
#include <numeric>
#include <ranges>

namespace nn {

std::optional<Network> Network::new_random(
    const std::vector<int>& layer_sizes, const std::vector<HiddenActivation>& hidden_activations,
    const OutputActivation& output_activation
) {
    if (!validate_inputs(layer_sizes, hidden_activations)) {
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
    return Network(weights, biases, hidden_activations, output_activation);
}

std::optional<Network> Network::from_data(
    const std::vector<int>& layer_sizes, const std::vector<double>& weights,
    const std::vector<double>& biases, const std::vector<HiddenActivation>& hidden_activations,
    const OutputActivation& output_activation
) {
    if (!validate_inputs(layer_sizes, weights, biases, hidden_activations)) {
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

    return Network(weight_matrices, bias_vectors, hidden_activations, output_activation);
}

Network::Network(
    const std::vector<MatrixXd>& weights, const std::vector<VectorXd>& biases,
    const std::vector<HiddenActivation>& hidden_activations,
    const OutputActivation& output_activation
)
: m_weights(weights), m_biases(biases), m_hidden_activations(hidden_activations),
  m_output_activation(output_activation) {
}

bool Network::validate_inputs(
    const std::vector<int>& layer_sizes, const std::vector<double>& weights,
    const std::vector<double>& biases, const std::vector<HiddenActivation>& hidden_activations
) {
    if (!validate_inputs(layer_sizes, hidden_activations)) {
        return false;
    }

    auto expected_biases = 0;
    for (int i : layer_sizes | std::views::drop(1)) {
        expected_biases += i;
    }
    auto expected_weights = 0;
    for (const auto& zip : layer_sizes | std::views::slide(2)) {
        expected_weights += zip[0] * zip[1];
    }

    if (weights.size() != expected_weights) {
        return false;
    }
    if (biases.size() != expected_biases) {
        return false;
    }
    return true;
}

bool Network::validate_inputs(
    const std::vector<int>& layer_sizes, const std::vector<HiddenActivation>& hidden_activations
) {
    if (layer_sizes.size() < 2) {
        return false;
    }
    if (layer_sizes.size() - 2 != hidden_activations.size()) {
        return false;
    }

    return true;
}

MatrixXd Network::feed_forward(const MatrixXd& input) const {
    MatrixXd out = input;

    // Feed forward the hidden layers
    for (auto [W, b, act] : std::views::zip(m_weights, m_biases, m_hidden_activations)) {
        out = (W * out).colwise() + b;
        std::visit(
            [&out](auto&& activation) {
                out = activation.function(out);
            },
            act
        );
    }

    // Feed forward the output layer
    auto out_w = m_weights.end() - 1;
    auto out_b = m_biases.end() - 1;
    out = (*out_w * out).colwise() + *out_b;
    std::visit(
        [&out](auto&& activation) {
            out = activation.function(out);
        },
        m_output_activation
    );

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
