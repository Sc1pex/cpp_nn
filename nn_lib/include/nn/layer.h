#pragma once

#include "nn/matrix.h"

class Layer {
public:
    Layer(Matrix weights, Matrix biases);
    static Layer random(u32 in, u32 out);

    Matrix feed_forward(const Matrix& input);

    u32 in();
    u32 out();

    const Matrix& weights() const;
    Matrix& weights();
    const Matrix& biases() const;
    Matrix& biases();

private:
    u32 m_in;
    u32 m_out;
    Matrix m_weights;
    Matrix m_biases;
};
