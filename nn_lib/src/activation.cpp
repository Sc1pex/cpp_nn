#include "nn_lib/activation.hpp"

namespace nn {

Eigen::MatrixXd apply_activation(const HiddenActivation& activation, const Eigen::MatrixXd& m) {
    return std::visit(
        [&m](auto&& activation) {
            return activation.function(m);
        },
        activation
    );
}
Eigen::MatrixXd apply_activation(const OutputActivation& activation, const Eigen::MatrixXd& m) {
    return std::visit(
        [&m](auto&& activation) {
            return activation.function(m);
        },
        activation
    );
}

std::optional<HiddenActivation> str_to_hidden_activation(std::string_view s) {
    if (s == "relu") {
        return activation::ReLU{};
    } else if (s == "sigmoid") {
        return activation::Sigmoid{};
    } else if (s == "linear") {
        return activation::Linear{};
    } else {
        return std::nullopt;
    }
}

std::optional<OutputActivation> str_to_output_activation(std::string_view s) {
    if (s == "softmax") {
        return activation::SoftMax{};
    } else if (s == "sigmoid") {
        return activation::Sigmoid{};
    } else if (s == "linear") {
        return activation::Linear{};
    } else {
        return std::nullopt;
    }
}

}
