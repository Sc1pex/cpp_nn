module;

#include <algorithm>
#include <print>
#include <ranges>
#include <vector>

export module nn:nn;

import :layer;
import :matrix;
import :math;

export class NN {
public:
    template<std::ranges::range T>
    NN(T shape);

    // Feed forward a row matrix
    Matrix feed_forward(const Matrix& input);

    // Calculate the cost of a matrix of row inputs and a matrix of row expected
    // values
    f64 cost(const Matrix& inputs, const Matrix& expected);

    void backprop(const Matrix& inputs, const Matrix& expected, f64 learn_rate);
    std::pair<std::vector<Matrix>, std::vector<Matrix>>
        compute_gradients(const Matrix& inputs, const Matrix& expected);

private:
    void apply_gradients(
        const std::vector<Matrix>& weight_grads, const std::vector<Matrix>& bias_grads,
        f64 learn_rate, f64 batch_size
    );

private:
    std::vector<Layer> m_layers;
};

Matrix NN::feed_forward(const Matrix& input) {
    return std::ranges::fold_left(m_layers, input.clone(), [](const Matrix& m, auto& l) {
        return l.feed_forward(m);
    });
}

template<std::ranges::range T>
NN::NN(T shape) {
    auto layers = shape | std::views::slide(2) | std::views::transform([](auto r) {
                      return Layer::random(r[0], r[1]);
                  });
    m_layers = std::ranges::to<std::vector<Layer>>(layers);
}

f64 NN::cost(const Matrix& inputs, const Matrix& expected) {
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

    f64 cost = 0.;

    for (u32 r = 0; r < inputs.rows(); r++) {
        Matrix inp{ 1, inputs.cols(), inputs.row(r) };
        Matrix out = feed_forward(std::move(inp));

        for (u32 c = 0; c < out.cols(); c++) {
            f64 diff = out.element(0, c) - expected.element(r, c);
            cost += diff * diff;
        }
    }

    return cost / inputs.rows();
}

void NN::backprop(const Matrix& inputs, const Matrix& expected, f64 learn_rate) {
    auto [weight_grads, bias_grads] = compute_gradients(inputs, expected);
    apply_gradients(weight_grads, bias_grads, learn_rate, inputs.rows());
}

std::pair<std::vector<Matrix>, std::vector<Matrix>>
    NN::compute_gradients(const Matrix& inputs, const Matrix& expected) {
    std::vector<Matrix> weight_gradients, bias_gradients;

    for (auto& l : m_layers) {
        weight_gradients.emplace_back(l.weights().rows(), l.weights().cols());
        bias_gradients.emplace_back(1, l.biases().cols());
    }

    for (u32 sample = 0; sample < inputs.rows(); sample++) {
        const auto* inp_data = inputs.row(sample);
        Matrix activation{ 1, inputs.cols(), inp_data };

        std::vector<Matrix> activations, zs;
        activations.push_back(activation.clone());

        // Feed forward
        for (auto& l : m_layers) {
            activation = activation.mult(l.weights());
            activation = activation.add(l.biases());
            zs.push_back(activation.clone());

            activation.map(sigmoid);
            activations.push_back(activation.clone());
        }

        // Calculate output error
        Matrix error{ 1, expected.cols() };
        error.map([&](u32, u32 j, f64 v) {
            f64 diff = activation.element(0, j) - expected.element(sample, j);
            return diff * sigmoid_deriv(activation.element(0, j));
        });

        // Backward pass
        for (int layer_idx = m_layers.size() - 1; layer_idx >= 0; layer_idx--) {
            // Calculate weight gradients
            auto& prev_activ = activations[layer_idx];
            weight_gradients[layer_idx].map([&](u32 i, u32 j, f64 wg) {
                f64 grad = prev_activ.element(0, i) * error.element(0, j);
                return wg + grad;
            });

            // Calculate bias gradients
            bias_gradients[layer_idx].map([&](u32, u32 j, f64 wg) {
                f64 grad = error.element(0, j);
                return wg + grad;
            });

            // Propagate error
            if (layer_idx > 0) {
                Matrix new_error{ 1, m_layers[layer_idx].weights().rows() };
                for (u32 i = 0; i < new_error.cols(); i++) {
                    f64 error_sum = 0.;
                    for (u32 j = 0; j < error.cols(); j++) {
                        error_sum +=
                            m_layers[layer_idx].weights().element(i, j) * error.element(0, j);
                    }
                    f64 err = error_sum * sigmoid_deriv(activations[layer_idx].element(0, i));
                    new_error.set_element(0, i, err);
                }
                error = std::move(new_error);
            }
        }
    }

    return std::make_pair(std::move(weight_gradients), std::move(bias_gradients));
}

void NN::apply_gradients(
    const std::vector<Matrix>& weight_grads, const std::vector<Matrix>& bias_grads, f64 learn_rate,
    f64 batch_size
) {
    for (int layer_idx = 0; layer_idx < m_layers.size(); layer_idx++) {
        // Update weights
        m_layers[layer_idx].weights().map([&](u32 i, u32 j, f64 w) {
            f64 avg_grad = weight_grads[layer_idx].element(i, j) / batch_size;
            return w - learn_rate * avg_grad;
        });

        // Update biases
        m_layers[layer_idx].biases().map([&](u32 i, u32 j, f64 b) {
            f64 avg_grad = bias_grads[layer_idx].element(i, j) / batch_size;
            return b - learn_rate * avg_grad;
        });
    }
}
