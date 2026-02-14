#pragma once

#include <Eigen/Core>
#include <optional>
#include <vector>
#include "nn_lib/activation.hpp"

namespace nn {

using Eigen::MatrixXd;
using Eigen::VectorXd;

class Network {
public:
    static std::optional<Network> from_data(
        const std::vector<int>& layer_sizes, const std::vector<double>& weights,
        const std::vector<double>& biases, const std::vector<Activation>& activations
    );

    static std::optional<Network>
        new_random(const std::vector<int>& layer_sizes, const std::vector<Activation>& activations);

    MatrixXd feed_forward(const MatrixXd& input) const;

    std::vector<double> dump_weights();
    std::vector<double> dump_biases();

private:
    Network(
        const std::vector<MatrixXd>& weights, const std::vector<VectorXd>& biases,
        const std::vector<Activation>& activations
    );

    std::vector<MatrixXd> m_weights;
    std::vector<VectorXd> m_biases;
    std::vector<Activation> m_activations;
};

}
