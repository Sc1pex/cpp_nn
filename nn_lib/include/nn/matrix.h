#pragma once

#include <functional>
#include <memory>
#include "nn/types.h"

class Matrix {
public:
    // Create a zero initialized matrix
    Matrix(u32 rows, u32 cols);
    Matrix(u32 rows, u32 cols, const f64 data[]);

    Matrix clone() const;

    u32 rows() const;
    u32 cols() const;

    // Returns a new matrix, or panics if there is a size missmatch
    Matrix add(const Matrix& other) const;

    // Returns a new matrix, or panics if there is a size missmatch
    Matrix mult(const Matrix& other) const;

    void map(std::function<f64(f64 value)> f);
    void map(std::function<f64(u32, u32, f64)> f);

    double element(u32 row, u32 col) const;
    double element(u32 idx) const;

    void set_element(u32 row, u32 col, f64 value);
    void set_element(u32 idx, f64 value);

    // Returns a non owning pointer to the first element of the row
    // I don't like returning a raw pointer here but i also don't want to use a
    // shared_ptr or write my own
    double* row(u32 row);
    const double* row(u32 row) const;

private:
    u32 m_rows;
    u32 m_cols;
    std::unique_ptr<f64[]> m_data;
};
