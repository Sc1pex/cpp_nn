#pragma once

#include <memory>
#include "types.h"

class Matrix {
public:
    // Create a zero initialized matrix
    Matrix(u32 rows, u32 cols);
    Matrix(u32 rows, u32 cols, double data[]);

    u32 rows();
    u32 cols();

    // Returns a new matrix, or panics if there is a size missmatch
    Matrix add(const Matrix& other);

    // Returns a new matrix, or panics if there is a size missmatch
    Matrix mult(const Matrix& other);

    double element(u32 row, u32 col) const;
    double element(u32 idx) const;

    void set_element(u32 row, u32 col, f64 value);
    void set_element(u32 idx, f64 value);

private:
    u32 m_rows;
    u32 m_cols;
    std::unique_ptr<f64[]> m_data;
};
