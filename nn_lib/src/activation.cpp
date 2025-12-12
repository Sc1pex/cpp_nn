#include "activation.h"

namespace nn {

std::optional<Activation> str_to_activation(std::string_view s) {
    if (s == "relu") {
        return activation::ReLU{};
    } else if (s == "sigmoid") {
        return activation::Sigmoid{};
    } else if (s == "none") {
        return activation::None{};
    } else {
        return std::nullopt;
    }
}

std::optional<std::vector<Activation>> strs_to_activation(const std::vector<std::string>& v) {
    std::vector<Activation> activations;
    for (const auto& s : v) {
        auto act_opt = str_to_activation(s);
        if (!act_opt) {
            return std::nullopt;
        }
        activations.push_back(act_opt.value());
    }
    return activations;
}

}
