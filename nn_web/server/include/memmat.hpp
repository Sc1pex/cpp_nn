#pragma once

#include <string>
#include "Eigen/Core"

struct MemMatrix {
    MemMatrix(const std::string& path, int rows, int cols);
    ~MemMatrix();

    Eigen::Map<const Eigen::MatrixXd> mat() const;

private:
    int m_fd;
    size_t m_size;
    const double* m_data;
    int m_rows;
    int m_cols;
};
