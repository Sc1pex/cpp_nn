#include "nn.h"
#include <ranges>

namespace nn {

NN NN::new_random(const std::vector<int>& layer_sizes, const std::vector<Activation>& activations) {
    assert(layer_sizes.size() >= 2);
    assert(layer_sizes.size() - 1 == activations.size());

    std::vector<MatrixXd> weights;
    std::vector<VectorXd> biases;

    for (size_t i = 0; i < layer_sizes.size() - 1; ++i) {
        double limit = std::sqrt(6.0 / layer_sizes[i]);
        MatrixXd W = MatrixXd::Random(layer_sizes[i + 1], layer_sizes[i]) * limit;
        VectorXd b = VectorXd::Zero(layer_sizes[i + 1]);

        weights.push_back(W);
        biases.push_back(b);
    }
    return NN(weights, biases, activations);
}

NN::NN(
    const std::vector<MatrixXd>& weights, const std::vector<VectorXd>& biases,
    const std::vector<Activation>& activations
)
: m_weights(weights), m_biases(biases), m_activations(activations) {
    assert(m_weights.size() >= 1);
    assert(m_weights.size() == m_biases.size());
    assert(m_weights.size() == m_activations.size());
}

MatrixXd NN::feed_forward(const MatrixXd& input) const {
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

std::vector<double> NN::dump_weights() {
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

std::vector<double> NN::dump_biases() {
    std::vector<double> dumped;
    for (const auto& b : m_biases) {
        for (int i = 0; i < b.size(); ++i) {
            dumped.push_back(b(i));
        }
    }
    return dumped;
}

}
