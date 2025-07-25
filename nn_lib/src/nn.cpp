module;

#include <algorithm>
#include <print>
#include <ranges>
#include <vector>

export module nn:nn;

import :layer;
import :matrix;

export class NN {
public:
    template<std::ranges::range T>
    NN(T shape);

    // Feed forward a row matrix
    Matrix feed_forward(const Matrix& input);

    // Calculate the cost of a matrix of row inputs and a matrix of row expected
    // values
    f64 cost(const Matrix& inputs, const Matrix& expected);

private:
    std::vector<Layer> m_layers;
};

Matrix NN::feed_forward(const Matrix& input) {
    return std::ranges::fold_left(
        m_layers, input.clone(), [](const Matrix& m, auto& l) {
            return l.feed_forward(m);
        }
    );
}

template<std::ranges::range T>
NN::NN(T shape) {
    auto layers =
        shape | std::views::slide(2) | std::views::transform([](auto r) {
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
