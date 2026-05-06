#pragma once

#include <Eigen/Core>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace nn {

namespace activation {

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

struct Linear {
    MatrixXd function(const MatrixXd& x) const {
        return x;
    }
    MatrixXd derivative(const MatrixXd& x) const {
        return MatrixXd::Ones(x.rows(), x.cols());
    }
};

struct SoftMax {
    MatrixXd function(const MatrixXd& x) const {
        auto max_per_col = x.colwise().maxCoeff();
        auto x_shifted = x.rowwise() - max_per_col;

        auto exp_x = x_shifted.array().exp();

        auto sum_per_col = exp_x.colwise().sum();
        return exp_x.array().rowwise() / sum_per_col.array();
    }
};

}

using HiddenActivation = std::variant<activation::ReLU, activation::Sigmoid, activation::Linear>;
using OutputActivation = std::variant<activation::SoftMax, activation::Sigmoid, activation::Linear>;

Eigen::MatrixXd apply_activation(const HiddenActivation& activation, const Eigen::MatrixXd& m);
Eigen::MatrixXd apply_activation(const OutputActivation& activation, const Eigen::MatrixXd& m);

std::optional<HiddenActivation> str_to_hidden_activation(std::string_view s);
std::optional<OutputActivation> str_to_output_activation(std::string_view s);

std::optional<std::vector<HiddenActivation>>
    strs_to_hidden_activation(std::ranges::input_range auto&& r)
    requires std::convertible_to<std::ranges::range_reference_t<decltype(r)>, std::string_view>
{
    std::vector<HiddenActivation> activations;
    for (const auto& s : r) {
        auto act_opt = str_to_hidden_activation(s);
        if (!act_opt) {
            return std::nullopt;
        }
        activations.push_back(act_opt.value());
    }
    return activations;
}

}
