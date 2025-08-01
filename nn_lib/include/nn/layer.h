#pragma once

// #include "nn/matrix.h"
#include <Eigen/Core>

using Eigen::MatrixXd;

class Layer {
public:
    Layer(MatrixXd weights, MatrixXd biases);
    static Layer random(int in, int out);

    MatrixXd feed_forward(const MatrixXd& input);

    int in();
    int out();

    const MatrixXd& weights() const;
    MatrixXd& weights();
    const MatrixXd& biases() const;
    MatrixXd& biases();

private:
    int m_in;
    int m_out;
    MatrixXd m_weights;
    MatrixXd m_biases;
};
