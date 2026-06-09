#pragma once

#include <Eigen/Core>
#include <optional>
#include <vector>
#include "nn_lib/activation.hpp"
#include "nn_lib/loss.hpp"

namespace nn {

using Eigen::MatrixXd;
using Eigen::VectorXd;

struct Prediction {
    MatrixXd output;
    double loss;
};

struct SGDHyperparams {
    double learning_rate;
    int epochs;
    int batch_size;
    // Window width for reporting average loss (in batches)
    std::optional<int> log_interval;
};

class Network {
public:
    static std::optional<Network> from_data(
        const std::vector<int>& layer_sizes, const std::vector<double>& weights,
        const std::vector<double>& biases, const std::vector<HiddenActivation>& hidden_activations,
        const OutputActivation& output_activation, const Loss& loss
    );

    static std::optional<Network> new_random(
        const std::vector<int>& layer_sizes,
        const std::vector<HiddenActivation>& hidden_activations,
        const OutputActivation& output_activation, const Loss& loss
    );

    MatrixXd feed_forward(const MatrixXd& input) const;
    Prediction predict(const MatrixXd& x, const MatrixXd& y) const;
    double calculate_loss(const MatrixXd& x, const MatrixXd& y) const;

    double learn(const MatrixXd& input, const MatrixXd& y, double learning_rate);
    std::optional<std::vector<double>> train_sgd(
        const MatrixXd& inputs, const MatrixXd& targets, const SGDHyperparams& hyperparams
    );

    std::vector<double> dump_weights();
    std::vector<double> dump_biases();

private:
    Network(
        const std::vector<MatrixXd>& weights, const std::vector<VectorXd>& biases,
        const std::vector<HiddenActivation>& hidden_activations,
        const OutputActivation& output_activation, const Loss& loss
    );

    struct FFResults {
        std::vector<MatrixXd> zs;
        std::vector<MatrixXd> as;
    };
    struct Gradients {
        std::vector<MatrixXd> dw;
        std::vector<VectorXd> db;
        double loss;
    };

    FFResults feed_forward_train(const MatrixXd& input) const;
    Gradients compute_gradients(const MatrixXd& input, const MatrixXd& y) const;
    MatrixXd output_delta(const MatrixXd& z, const MatrixXd& a, const MatrixXd& y) const;

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
    Loss m_loss;
};

}
