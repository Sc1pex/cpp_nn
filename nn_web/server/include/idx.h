#pragma once

#include <Eigen/Core>
#include <expected>
#include <string>
#include <vector>

using Eigen::MatrixXd;

std::expected<std::vector<MatrixXd>, std::string> idx3_read_file(const std::string& file_path);
std::expected<std::vector<double>, std::string> idx1_read_file(const std::string& file_path);
