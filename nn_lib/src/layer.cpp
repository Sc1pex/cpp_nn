#include "nn/layer.h"
#include <print>
#include <random>
#include "nn/math.h"

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

Matrix Layer::feed_forward(const Matrix& input) {
    auto m = input.mult(m_weights);
    m = m.add(m_biases);
    m.map(sigmoid);
    return m;
}

Layer Layer::random(u32 in, u32 out) {
    std::random_device rd;
    std::mt19937 gen(rd());

    double eps = sqrt(6. / (in + out));
    std::uniform_real_distribution<> w_distrib(-eps, eps);
    std::uniform_real_distribution<> b_distrib(-1, 1);

    // Initialize weights with random values
    Matrix weights{ in, out }, biases{ 1, out };
    for (u32 i = 0; i < in * out; i++) {
        weights.set_element(i, w_distrib(gen));
    }
    for (u32 i = 0; i < out; i++) {
        biases.set_element(i, b_distrib(gen));
    }

    return Layer{ std::move(weights), std::move(biases) };
}

u32 Layer::in() {
    return m_in;
}
u32 Layer::out() {
    return m_out;
};

const Matrix& Layer::weights() const {
    return m_weights;
}

const Matrix& Layer::biases() const {
    return m_biases;
}

Matrix& Layer::weights() {
    return m_weights;
}

Matrix& Layer::biases() {
    return m_biases;
}
