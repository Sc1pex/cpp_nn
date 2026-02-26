#include "nn_lib/activation.hpp"

namespace nn {

std::optional<HiddenActivation> str_to_hidden_activation(std::string_view s) {
    if (s == "relu") {
        return hidden_activation::ReLU{};
    } else if (s == "sigmoid") {
        return hidden_activation::Sigmoid{};
    } else if (s == "none") {
        return hidden_activation::None{};
    } else {
        return std::nullopt;
    }
}

std::optional<std::vector<HiddenActivation>>
    strs_to_hidden_activation(const std::vector<std::string>& v) {
    std::vector<HiddenActivation> activations;
    for (const auto& s : v) {
        auto act_opt = str_to_hidden_activation(s);
        if (!act_opt) {
            return std::nullopt;
        }
        activations.push_back(act_opt.value());
    }
    return activations;
}

}
