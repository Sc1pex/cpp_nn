#pragma once

#include <Eigen/Core>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace nn {

namespace hidden_activation {

using Eigen::MatrixXd;

struct ReLU {
    MatrixXd function(const MatrixXd& x) const {
        return x.cwiseMax(0.0);
    }
    MatrixXd derivative(const MatrixXd& x) const {
        return (x.array() > 0).cast<double>();
    }
};

struct Sigmoid {
    MatrixXd function(const MatrixXd& x) const {
        return 1.0 / (1.0 + (-x.array()).exp());
    }
    MatrixXd derivative(const MatrixXd& x) const {
        MatrixXd s = function(x);
        return s.array() * (1.0 - s.array());
    }
};

struct None {
    MatrixXd function(const MatrixXd& x) const {
        return x;
    }
    MatrixXd derivative(const MatrixXd& x) const {
        return MatrixXd::Ones(x.rows(), x.cols());
    }
};

}

using HiddenActivation =
    std::variant<hidden_activation::ReLU, hidden_activation::Sigmoid, hidden_activation::None>;

std::optional<HiddenActivation> str_to_hidden_activation(std::string_view s);
std::optional<std::vector<HiddenActivation>>
    strs_to_hidden_activation(const std::vector<std::string>& v);

}
