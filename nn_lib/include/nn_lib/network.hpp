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
        const std::vector<double>& biases, const std::vector<HiddenActivation>& hidden_activations,
        const OutputActivation& output_activation
    );

    static std::optional<Network> new_random(
        const std::vector<int>& layer_sizes,
        const std::vector<HiddenActivation>& hidden_activations,
        const OutputActivation& output_activation
    );

    MatrixXd feed_forward(const MatrixXd& input) const;

    std::vector<double> dump_weights();
    std::vector<double> dump_biases();

private:
    Network(
        const std::vector<MatrixXd>& weights, const std::vector<VectorXd>& biases,
        const std::vector<HiddenActivation>& hidden_activations,
        const OutputActivation& output_activation
    );

    static bool validate_inputs(
        const std::vector<int>& layer_sizes, const std::vector<double>& weights,
        const std::vector<double>& biases, const std::vector<HiddenActivation>& hidden_activations
    );
    static bool validate_inputs(
        const std::vector<int>& layer_sizes, const std::vector<HiddenActivation>& hidden_activations
    );

private:
    std::vector<MatrixXd> m_weights;
    std::vector<VectorXd> m_biases;
    std::vector<HiddenActivation> m_hidden_activations;
    OutputActivation m_output_activation;
};

}
