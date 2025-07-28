#pragma once

#include <nn/matrix.h>
#include <string_view>
#include <vector>

std::vector<Matrix> read_file(const std::string_view& file_path);
