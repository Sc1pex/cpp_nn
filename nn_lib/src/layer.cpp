#include "nn/layer.h"
#include <print>
#include <random>
#include "nn/math.h"

Layer::Layer(MatrixXd weights, MatrixXd biases)
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

MatrixXd Layer::feed_forward(const MatrixXd& input) {
    auto m = input * m_weights + m_biases;
    return m.unaryExpr([](double v) {
        return sigmoid(v);
    });
}

Layer Layer::random(int in, int out) {
    std::random_device rd;
    std::mt19937 gen(rd());

    double eps = sqrt(6. / (in + out));
    std::uniform_real_distribution<> w_distrib(-eps, eps);
    std::uniform_real_distribution<> b_distrib(-1, 1);

    // Initialize weights with random values
    MatrixXd weights{ in, out }, biases{ 1, out };
    for (int i = 0; i < in; i++) {
        for (int j = 0; j < out; j++) {
            weights(i, j) = w_distrib(gen);
        }
    }
    for (int i = 0; i < out; i++) {
        biases(0, i) = b_distrib(gen);
    }

    return Layer{ std::move(weights), std::move(biases) };
}

int Layer::in() {
    return m_in;
}
int Layer::out() {
    return m_out;
};

const MatrixXd& Layer::weights() const {
    return m_weights;
}

const MatrixXd& Layer::biases() const {
    return m_biases;
}

MatrixXd& Layer::weights() {
    return m_weights;
}

MatrixXd& Layer::biases() {
    return m_biases;
}
