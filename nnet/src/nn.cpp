#include "nn.h"
#include <random>
#include <ranges>
#include "math.h"

namespace nnet {

NN NN::init_random(const std::vector<int>& shape) {
    NN nn;

    std::random_device rd;
    std::mt19937 gen(rd());

    // Distribution for biases is the same for all layers
    std::uniform_real_distribution<> b_distrib(-1, 1);

    for (auto s : shape | std::views::slide(2)) {
        auto in = s[0], out = s[1];

        double eps = sqrt(6. / (in + out));
        // Distribution for weights depends on the layer size
        std::uniform_real_distribution<> w_distrib(-eps, eps);

        nn.m_weights.emplace_back(MatrixXd::NullaryExpr(out, in, [&]() {
            return w_distrib(gen);
        }));
        nn.m_biases.emplace_back(MatrixXd::NullaryExpr(out, 1, [&]() {
            return b_distrib(gen);
        }));
    }

    return nn;
}

MatrixXd NN::feed_forward(const MatrixXd& input) {
    MatrixXd output = input;

    for (auto [w, b] : std::views::zip(m_weights, m_biases)) {
        output = w * output + b;
        output = output.unaryExpr(std::function(sigmoid));
    }

    return output;
}

void NN::apply_gradients(const Gradients& gradients, double learning_rate) {
    for (size_t i = 0; i < m_weights.size(); ++i) {
        m_weights[i] -= learning_rate * gradients[i].first;
        m_biases[i] -= learning_rate * gradients[i].second;
    }
}

int NN::num_layers() const {
    return m_weights.size();
}

Gradients NN::backprop_batch_matrix(const MatrixXd& inputs, const MatrixXd& targets) {
    int batch_size = inputs.cols();
    std::vector<MatrixXd> activations;
    std::vector<MatrixXd> zs;

    MatrixXd activation = inputs;
    activations.push_back(activation);

    // Forward pass
    for (auto [w, b] : std::views::zip(m_weights, m_biases)) {
        MatrixXd z = w * activation + b.replicate(1, batch_size);
        zs.push_back(z);

        activation = z.unaryExpr(std::function(sigmoid));
        activations.push_back(activation);
    }

    // Backward pass
    MatrixXd delta = (activations.back() - targets);
    delta = delta.cwiseProduct(zs.back().unaryExpr(std::function(sigmoid_derivative)));

    Gradients gradients(m_weights.size());

    gradients.back().first = delta * activations[activations.size() - 2].transpose() / batch_size;
    gradients.back().second = delta.rowwise().mean();

    for (int i = m_weights.size() - 2; i >= 0; --i) {
        delta = m_weights[i + 1].transpose() * delta;
        delta = delta.cwiseProduct(zs[i].unaryExpr(std::function(sigmoid_derivative)));

        gradients[i].first = delta * activations[i].transpose() / batch_size;
        gradients[i].second = delta.rowwise().mean();
    }

    return gradients;
}

double NN::cost(const MatrixXd& inputs, const MatrixXd& targets) {
    double total_cost = 0.0;
    for (int i = 0; i < inputs.cols(); ++i) {
        MatrixXd output = feed_forward(inputs.col(i));
        total_cost += (output - targets.col(i)).squaredNorm();
    }
    return total_cost / inputs.cols();
}

}
