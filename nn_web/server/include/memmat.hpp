#pragma once

#include <string>
#include "Eigen/Core"

struct MemMatrix {
    MemMatrix(const std::string& path, int rows, int cols);

    Eigen::Map<const Eigen::MatrixXd> mat() const;

private:
    struct MunmapDeleter {
        size_t size;
        void operator()(const double* p) const;
    };

    size_t m_size;
    std::unique_ptr<const double, MunmapDeleter> m_data;
    int m_rows;
    int m_cols;
};
