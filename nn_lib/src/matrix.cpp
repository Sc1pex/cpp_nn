#include "nn/matrix.h"
#include <print>

Matrix::Matrix(u32 rows, u32 cols) {
    m_rows = rows;
    m_cols = cols;
    m_data = std::make_unique<double[]>(m_cols * m_rows);
}

Matrix::Matrix(u32 rows, u32 cols, const f64 data[]) {
    m_rows = rows;
    m_cols = cols;
    m_data = std::make_unique<double[]>(m_cols * m_rows);
    for (u32 i = 0; i < rows * cols; i++) {
        m_data[i] = data[i];
    }
}

u32 Matrix::rows() const {
    return m_rows;
}
u32 Matrix::cols() const {
    return m_cols;
}

Matrix Matrix::add(const Matrix& other) const {
    if (this->m_cols != other.m_cols || this->m_rows != other.m_rows) {
        std::println(
            "Called add on incompatible matricies: ({} {}) and ({} {})", this->m_rows, this->m_cols,
            other.m_rows, other.m_cols
        );
        std::abort();
    }

    Matrix res = Matrix{ this->m_rows, this->m_cols };

    for (u32 i = 0; i < res.m_rows; i++) {
        for (u32 j = 0; j < res.m_cols; j++) {
            u32 idx = i * res.m_cols + j;
            res.m_data[idx] = this->m_data[idx] + other.m_data[idx];
        }
    }

    return res;
}

Matrix Matrix::mult(const Matrix& other) const {
    if (this->m_cols != other.m_rows) {
        std::println(
            "Called mult on incompatible matricies: ({} {}) and ({} {})", this->m_rows,
            this->m_cols, other.m_rows, other.m_cols
        );
        std::abort();
    }

    Matrix res = Matrix{ this->m_rows, other.m_cols };

    for (u32 i = 0; i < res.m_rows; i++) {
        for (u32 j = 0; j < res.m_cols; j++) {
            u32 res_idx = i * res.m_cols + j;

            f64 value = 0.;
            for (u32 k = 0; k < this->m_cols; k++) {
                u32 this_idx = i * this->m_cols + k;
                u32 other_idx = k * other.m_cols + j;
                value += this->m_data[this_idx] * other.m_data[other_idx];
            }

            res.m_data[res_idx] = value;
        }
    }

    return res;
}

double Matrix::element(u32 row, u32 col) const {
    return element(row * m_cols + col);
}

double Matrix::element(u32 idx) const {
    if (idx >= m_rows * m_cols) {
        std::println(
            "Indexed matrix of size ({} {}) with index {}", this->m_rows, this->m_cols, idx
        );
        std::abort();
    }

    return m_data[idx];
}

void Matrix::set_element(u32 row, u32 col, f64 value) {
    set_element(row * m_cols + col, value);
}

void Matrix::set_element(u32 idx, f64 value) {
    if (idx >= m_rows * m_cols) {
        return;
    }

    m_data[idx] = value;
}

double* Matrix::row(u32 row) {
    if (row > m_rows) {
        return nullptr;
    }

    u32 start_idx = row * m_cols;
    return &m_data[start_idx];
}

const double* Matrix::row(u32 row) const {
    if (row > m_rows) {
        return nullptr;
    }

    u32 start_idx = row * m_cols;
    return &m_data[start_idx];
}

Matrix Matrix::clone() const {
    return Matrix{ m_rows, m_cols, m_data.get() };
}

void Matrix::map(std::function<f64(f64)> f) {
    for (u32 i = 0; i < m_cols * m_rows; i++) {
        m_data[i] = f(m_data[i]);
    }
}

void Matrix::map(std::function<f64(u32, u32, f64)> f) {
    for (u32 i = 0; i < m_rows; i++) {
        for (u32 j = 0; j < m_cols; j++) {
            u32 idx = i * m_cols + j;
            m_data[idx] = f(i, j, m_data[idx]);
        }
    }
}
