#include <print>
#include "nn/matrix.h"

int main() {
    // Create a 2x3 zero-initialized matrix
    Matrix mat1(2, 3);

    // Set some values
    mat1.set_element(0, 0, 1.0);
    mat1.set_element(0, 1, 2.0);
    mat1.set_element(0, 2, 3.0);
    mat1.set_element(1, 0, 4.0);
    mat1.set_element(1, 1, 5.0);
    mat1.set_element(1, 2, 6.0);

    // Create another 2x3 matrix with initial data
    double data[] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
    Matrix mat2(2, 3, data);

    // Matrix addition
    Matrix sum = mat1.add(mat2);

    // Print the result
    std::println("Matrix addition result:");
    for (u32 i = 0; i < sum.rows(); ++i) {
        for (u32 j = 0; j < sum.cols(); ++j) {
            std::print("{} ", sum.element(i, j));
        }
        std::println("");
    }

    // Create matrices for multiplication (3x2 and 2x3)
    Matrix mat3(3, 2);
    mat3.set_element(0, 0, 1.0);
    mat3.set_element(0, 1, 2.0);
    mat3.set_element(1, 0, 3.0);
    mat3.set_element(1, 1, 4.0);
    mat3.set_element(2, 0, 5.0);
    mat3.set_element(2, 1, 6.0);

    Matrix mat4(2, 3);
    mat4.set_element(0, 0, 1.0);
    mat4.set_element(0, 1, 0.0);
    mat4.set_element(0, 2, 1.0);
    mat4.set_element(1, 0, 0.0);
    mat4.set_element(1, 1, 1.0);
    mat4.set_element(1, 2, 0.0);

    // Matrix multiplication (3x2) * (2x3) = (3x3)
    Matrix product = mat3.mult(mat4);

    std::println("\nMatrix multiplication result:");
    for (u32 i = 0; i < product.rows(); ++i) {
        for (u32 j = 0; j < product.cols(); ++j) {
            std::print("{} ", product.element(i, j));
        }
        std::println("");
    }

    // Demonstrate linear indexing
    std::println("\nUsing linear indexing on first matrix:");
    for (u32 i = 0; i < mat1.rows() * mat1.cols(); ++i) {
        std::println("Index {}: {}", i, mat1.element(i));
    }

    return 0;
}
