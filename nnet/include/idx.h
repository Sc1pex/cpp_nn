#pragma once

#include <Eigen/Core>
#include <string>
#include <vector>

using Eigen::MatrixXd;

std::vector<MatrixXd> idx3_read_file(const std::string& file_path);
std::vector<double> idx1_read_file(const std::string& file_path);
