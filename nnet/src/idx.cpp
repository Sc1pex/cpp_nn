#include "idx.h"
#include <endian.h>
#include <fstream>
#include <ios>
#include <print>

std::vector<MatrixXd> idx3_read_file(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        std::println("Failed to open file: {}", file_path);
        std::abort();
    }

    char buf[4];
    // Read the magic number
    file.read(buf, 4);
    if (buf[0] != 0 || buf[1] != 0 || buf[2] != 8 || buf[3] != 3) {
        std::println("Invalid IDX3 file: {}", file_path);
        std::abort();
    }

    int32_t num_images, rows, cols;
    file.read(reinterpret_cast<char*>(&num_images), 4);
    file.read(reinterpret_cast<char*>(&rows), 4);
    file.read(reinterpret_cast<char*>(&cols), 4);
    num_images = be32toh(num_images);
    rows = be32toh(rows);
    cols = be32toh(cols);

    std::vector<MatrixXd> data;
    data.reserve(num_images);

    for (int i = 0; i < num_images; i++) {
        MatrixXd m(rows, cols);
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                file.read(buf, 1);
                if (!file) {
                    std::println("Error reading data in file {}", file_path);
                    std::abort();
                }
                m(r, c) = (unsigned char) buf[0] / 255.0;
            }
        }
        data.push_back(std::move(m));
    }

    return data;
}

std::vector<double> idx1_read_file(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        std::println("Failed to open file: {}", file_path);
        std::abort();
    }

    char buf[4];
    // Read the magic number
    file.read(buf, 4);
    if (buf[0] != 0 || buf[1] != 0 || buf[2] != 8 || buf[3] != 1) {
        std::println("Invalid IDX1 file: {}", file_path);
        std::abort();
    }

    int32_t length;
    file.read(reinterpret_cast<char*>(&length), 4);
    length = be32toh(length);

    std::vector<double> data(length);

    for (int i = 0; i < length; i++) {
        file.read(buf, 1);
        if (!file) {
            std::println("Error reading data in file {}", file_path);
            std::abort();
        }
        data[i] = (unsigned char) buf[0];
    }

    return data;
}
