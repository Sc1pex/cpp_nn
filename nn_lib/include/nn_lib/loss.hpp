#pragma once

#include <Eigen/Core>
#include <variant>

namespace nn {

namespace loss {

using Eigen::MatrixXd;

struct MSE {
    double function(const MatrixXd& a, const MatrixXd& y) const {
        return (a - y).array().square().sum() / (2.0 * a.cols());
    }
    MatrixXd dc_da(const MatrixXd& a, const MatrixXd& y) const {
        return a - y;
    }
};

struct CrossEntropy {
    double function(const MatrixXd& a, const MatrixXd& y) const {
        return -(y.array() * a.array().log()).sum() / a.cols();
    }
    MatrixXd dc_da(const MatrixXd& a, const MatrixXd& y) const {
        return -y.array() / a.array();
    }
};

}

using Loss = std::variant<loss::MSE, loss::CrossEntropy>;

std::optional<Loss> str_to_loss(std::string_view s);

}
