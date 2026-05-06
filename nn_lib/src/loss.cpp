#include "nn_lib/loss.hpp"

namespace nn {

std::optional<Loss> str_to_loss(std::string_view s) {
    if (s == "mse") {
        return loss::MSE{};
    } else if (s == "cross_entropy") {
        return loss::CrossEntropy{};
    } else {
        return std::nullopt;
    }
}

}
