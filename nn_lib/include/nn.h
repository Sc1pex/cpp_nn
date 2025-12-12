#pragma once

#include <Eigen/Core>
#include <vector>
#include "activation.h"

namespace nn {

using Eigen::MatrixXd;
using Eigen::VectorXd;

class NN {
public:
    NN(const std::vector<MatrixXd>& weights, const std::vector<VectorXd>& biases,
       const std::vector<Activation>& activations);

    static NN
        new_random(const std::vector<int>& layer_sizes, const std::vector<Activation>& activations);

    MatrixXd feed_forward(const MatrixXd& input) const;

    std::vector<double> dump_weights();
    std::vector<double> dump_biases();

private:
    std::vector<MatrixXd> m_weights;
    std::vector<VectorXd> m_biases;
    std::vector<Activation> m_activations;
};

}
