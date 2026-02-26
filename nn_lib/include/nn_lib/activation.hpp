#pragma once

#include <Eigen/Core>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace nn {

using Eigen::MatrixXd;

namespace activation {

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

using Activation = std::variant<activation::ReLU, activation::Sigmoid, activation::None>;

std::optional<Activation> str_to_activation(std::string_view s);
std::optional<std::vector<Activation>> strs_to_activation(const std::vector<std::string>& v);

}
