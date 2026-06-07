#include "nn_lib/network.hpp"
#include <numeric>
#include <ranges>
#include "nn_lib/activation.hpp"
#include "nn_lib/loss.hpp"

namespace nn {

std::optional<Network> Network::new_random(
    const std::vector<int>& layer_sizes, const std::vector<HiddenActivation>& hidden_activations,
    const OutputActivation& output_activation, const Loss& loss
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
    return Network(weights, biases, hidden_activations, output_activation, loss);
}

std::optional<Network> Network::from_data(
    const std::vector<int>& layer_sizes, const std::vector<double>& weights,
    const std::vector<double>& biases, const std::vector<HiddenActivation>& hidden_activations,
    const OutputActivation& output_activation, const Loss& loss
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

    return Network(weight_matrices, bias_vectors, hidden_activations, output_activation, loss);
}

Network::Network(
    const std::vector<MatrixXd>& weights, const std::vector<VectorXd>& biases,
    const std::vector<HiddenActivation>& hidden_activations,
    const OutputActivation& output_activation, const Loss& loss
)
: m_weights(weights), m_biases(biases), m_hidden_activations(hidden_activations),
  m_output_activation(output_activation), m_loss(loss) {
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
        out = apply_activation(act, (W * out).colwise() + b);
    }

    // Feed forward the output layer
    auto out_w = m_weights.back();
    auto out_b = m_biases.back();
    out = apply_activation(m_output_activation, (out_w * out).colwise() + out_b);

    return out;
}

Prediction Network::predict(const MatrixXd& x, const MatrixXd& y) const {
    auto a = feed_forward(x);
    double loss = std::visit([&](auto&& l) {
        return l.function(a, y);
    }, m_loss);
    return { a, loss };
}

double Network::calculate_loss(const MatrixXd& x, const MatrixXd& y) const {
    auto a = feed_forward(x);
    return std::visit([&](auto&& l) {
        return l.function(a, y);
    }, m_loss);
}

Network::FFResults Network::feed_forward_train(const MatrixXd& input) const {
    FFResults res;

    res.as.reserve(m_weights.size() + 1);
    res.zs.reserve(m_weights.size());

    MatrixXd out = input;
    res.as.push_back(out);

    // Feed forward the hidden layers
    for (auto [W, b, act] : std::views::zip(m_weights, m_biases, m_hidden_activations)) {
        auto z = (W * out).colwise() + b;
        res.zs.push_back(z);

        out = apply_activation(act, z);
        res.as.push_back(out);
    }

    // Feed forward the output layer
    auto out_w = m_weights.back();
    auto out_b = m_biases.back();
    auto z = (out_w * out).colwise() + out_b;
    res.zs.push_back(z);

    out = apply_activation(m_output_activation, z);
    res.as.push_back(out);

    return res;
}

MatrixXd Network::output_delta(const MatrixXd& z, const MatrixXd& a, const MatrixXd& y) const {
    return std::visit([&](const auto& act, const auto& loss) -> MatrixXd {
        using A = std::decay_t<decltype(act)>;
        using L = std::decay_t<decltype(loss)>;

        if constexpr (std::is_same_v<A, activation::SoftMax>
                      && std::is_same_v<L, loss::CrossEntropy>) {
            return a - y;
        } else if constexpr (std::is_same_v<A, activation::Sigmoid>
                             && std::is_same_v<L, loss::CrossEntropy>) {
            return y.array() * (a.array() - 1.);
        } else if constexpr (std::is_same_v<A, activation::SoftMax>) {
            auto dc_da = loss.dc_da(a, y);
            Eigen::MatrixXd delta(a.rows(), a.cols());
            for (int i = 0; i < a.cols(); ++i) {
                delta.col(i) = act.jacobian(a.col(i)) * dc_da.col(i);
            }
            return delta;
        } else {
            auto dc_da = loss.dc_da(a, y);
            auto da_dz = act.derivative(z);
            return dc_da.array() * da_dz.array();
        }
    }, m_output_activation, m_loss);
}

Network::Gradients Network::compute_gradients(const MatrixXd& input, const MatrixXd& y) const {
    auto ff = feed_forward_train(input);
    Gradients grads;

    grads.dw.resize(m_weights.size());
    grads.db.resize(m_biases.size());

    auto dz = output_delta(ff.zs.back(), ff.as.back(), y);

    for (ssize_t n = m_weights.size() - 1; n >= 0; n--) {
        grads.dw[n] = (dz * ff.as[n].transpose()) / input.cols();
        grads.db[n] = dz.rowwise().mean();

        if (n) {
            std::visit([&](const auto& act) {
                auto dwl = m_weights[n].transpose() * dz;
                dz = dwl.array() * act.derivative(ff.zs[n - 1]).array();
            }, m_hidden_activations[n - 1]);
        }
    }

    return grads;
}

void Network::learn(const MatrixXd& input, const MatrixXd& y, double learning_rate) {
    auto grads = compute_gradients(input, y);

    for (size_t i = 0; i < m_weights.size(); ++i) {
        m_weights[i] -= learning_rate * grads.dw[i];
        m_biases[i] -= learning_rate * grads.db[i];
    }
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
