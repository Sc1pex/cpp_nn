module;

#include <algorithm>
#include <cmath>
#include <random>
#include <ranges>

export module nn:nn;

import :matrix;

class Layer {
public:
    Layer(Matrix weights, Matrix biases);
    static Layer random(u32 in, u32 out);

    Matrix feed_forward(Matrix input);

private:
    Matrix m_weights;
    Matrix m_biases;
};

export class NN {
public:
    template<std::ranges::range T>
    NN(T shape);

    // Feed forward a column matrix
    Matrix feed_forward(Matrix input);

private:
    std::vector<Layer> m_layers;
};

Layer::Layer(Matrix weights, Matrix biases)
: m_weights(std::move(weights)), m_biases(std::move(biases)) {
}

Matrix Layer::feed_forward(Matrix input) {
    auto m = m_weights.mult(input);
    m = m.add(m_biases);
    // Apply sigmoid
    for (u32 i = 0; i < m.cols() * m.rows(); i++) {
        f64 sig = 1 / (1 + exp(m.element(i)));
        m.set_element(i, sig);
    }
    return m;
}

Layer Layer::random(u32 in, u32 out) {
    std::random_device rd;
    std::mt19937 gen(rd());

    double eps = sqrt(6. / (in + out));
    std::uniform_real_distribution<> distrib(-eps, eps);

    // Initialize weights with random values
    // and biases with zeros
    Matrix weights{ out, in }, biases{ out, 1 };
    for (u32 i = 0; i < in * out; i++) {
        weights.set_element(i, distrib(gen));
    }

    return Layer{ std::move(weights), std::move(biases) };
}

Matrix NN::feed_forward(Matrix input) {
    return std::ranges::fold_left(
        m_layers, std::move(input), [](auto m, auto& l) {
            return l.feed_forward(std::move(m));
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
