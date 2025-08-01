#include "nn/nn.h"
#include <algorithm>
#include <print>
#include <vector>
#include "nn/math.h"

MatrixXd NN::feed_forward(const MatrixXd& input) {
    return std::ranges::fold_left(m_layers, input, [](const MatrixXd& m, auto& l) {
        return l.feed_forward(m);
    });
}

double NN::cost(const MatrixXd& inputs, const MatrixXd& expected) {
    if (inputs.rows() != expected.rows()) {
        std::println(
            "Invalid call to cost with inputs of size ({}, {}) and expected of size ({}, {})",
            inputs.rows(), inputs.cols(), expected.rows(), expected.cols()
        );
        std::abort();
    }
    if (inputs.cols() != m_layers[0].in()) {
        std::println(
            "Invalid call to cost with inputs of size ({}, {}) and first layer input size {}",
            inputs.rows(), inputs.cols(), m_layers[0].in()
        );
        std::abort();
    }
    if (expected.cols() != m_layers.back().out()) {
        std::println(
            "Invalid call to cost with expected of size ({}, {}) and last layer output size {}",
            expected.rows(), expected.cols(), m_layers.back().out()
        );
        std::abort();
    }

    double cost = 0.;

    for (int r = 0; r < inputs.rows(); r++) {
        MatrixXd inp = inputs.row(r);
        MatrixXd out = feed_forward(inp);

        MatrixXd diff = out.row(0) - expected.row(r);
        // std::println("Cost for sample {}: {}", r, diff.squaredNorm());
        cost += diff.squaredNorm();
    }

    return cost / inputs.rows();
}

void NN::backprop(const MatrixXd& inputs, const MatrixXd& expected, double learn_rate) {
    auto [weight_grads, bias_grads] = compute_gradients(inputs, expected);
    apply_gradients(weight_grads, bias_grads, learn_rate, inputs.rows());
}

std::pair<std::vector<MatrixXd>, std::vector<MatrixXd>>
    NN::compute_gradients(const MatrixXd& inputs, const MatrixXd& expected) {
    std::vector<MatrixXd> weight_gradients, bias_gradients;

    for (auto& l : m_layers) {
        weight_gradients.emplace_back(l.weights().rows(), l.weights().cols());
        bias_gradients.emplace_back(1, l.biases().cols());
    }

    for (int sample = 0; sample < inputs.rows(); sample++) {
        MatrixXd activation = inputs.row(sample);

        std::vector<MatrixXd> activations, zs;
        activations.push_back(activation);

        // Feed forward
        for (auto& l : m_layers) {
            MatrixXd z = activation * l.weights() + l.biases();
            zs.push_back(z);
            activation = z.unaryExpr([](double v) {
                return sigmoid(v);
            });
            activations.push_back(activation);
        }

        // Calculate output error
        MatrixXd output = activations.back();
        MatrixXd expected_output = expected.row(sample);
        MatrixXd output_error = output - expected_output;
        output_error = output_error.cwiseProduct(output.unaryExpr([](double v) {
            return sigmoid_deriv(v);
        }));

        // Backward pass
        for (int layer_idx = m_layers.size() - 1; layer_idx >= 0; layer_idx--) {
            weight_gradients[layer_idx] += activations[layer_idx].transpose() * output_error;
            bias_gradients[layer_idx] += output_error;

            // Propagate error
            if (layer_idx > 0) {
                MatrixXd weights = m_layers[layer_idx].weights();
                output_error = output_error * weights.transpose();
                output_error =
                    output_error.cwiseProduct(activations[layer_idx].unaryExpr([](double v) {
                        return sigmoid_deriv(v);
                    }));
            }
        }
    }

    return std::make_pair(std::move(weight_gradients), std::move(bias_gradients));
}

void NN::apply_gradients(
    const std::vector<MatrixXd>& weight_grads, const std::vector<MatrixXd>& bias_grads,
    double learn_rate, double batch_size
) {
    for (int layer_idx = 0; layer_idx < m_layers.size(); layer_idx++) {
        m_layers[layer_idx].weights() -= (learn_rate / batch_size) * weight_grads[layer_idx];
        m_layers[layer_idx].biases() -= (learn_rate / batch_size) * bias_grads[layer_idx];
    }
}
