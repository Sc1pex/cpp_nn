#pragma once

#include <nn/matrix.h>
#include <string>
#include <vector>

std::vector<Matrix> idx3_read_file(const std::string& file_path);
std::vector<double> idx1_read_file(const std::string& file_path);
