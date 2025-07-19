module;

#include <cmath>
#include <print>
#include <random>

export module nn:layer;

import :matrix;

class Layer {
public:
    Layer(Matrix weights, Matrix biases);
    static Layer random(u32 in, u32 out);

    Matrix feed_forward(Matrix input);

    u32 in();
    u32 out();

private:
    u32 m_in;
    u32 m_out;
    Matrix m_weights;
    Matrix m_biases;
};

Layer::Layer(Matrix weights, Matrix biases)
: m_weights(std::move(weights)), m_biases(std::move(biases)) {
    if (m_biases.rows() != 1 || m_weights.cols() != m_biases.cols()) {
        std::println(
            "Invalid layer initialization with weights of size ({}, {}) and biases of size ({}, {})",
            m_weights.rows(), m_weights.cols(), m_biases.rows(), m_biases.cols()
        );
        std::abort();
    }

    m_in = m_weights.rows();
    m_out = m_weights.cols();
}

Matrix Layer::feed_forward(Matrix input) {
    auto m = input.mult(m_weights);
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
    Matrix weights{ in, out }, biases{ 1, out };
    for (u32 i = 0; i < in * out; i++) {
        weights.set_element(i, distrib(gen));
    }

    return Layer{ std::move(weights), std::move(biases) };
}

u32 Layer::in() {
    return m_in;
}
u32 Layer::out() {
    return m_out;
};
