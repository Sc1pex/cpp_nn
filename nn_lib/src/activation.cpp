#include "nn_lib/activation.hpp"

namespace nn {

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
