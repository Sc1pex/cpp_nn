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

    // Calculate total data size and read all at once
    const size_t image_size = rows * cols;
    const size_t total_size = num_images * image_size;
    std::vector<unsigned char> raw_data(total_size);

    file.read(reinterpret_cast<char*>(raw_data.data()), total_size);
    if (!file) {
        std::println("Error reading data in file {}", file_path);
        std::abort();
    }

    std::vector<MatrixXd> data;
    data.reserve(num_images);

    // Process the raw data into matrices
    for (int i = 0; i < num_images; i++) {
        MatrixXd m(rows, cols);
        const unsigned char* image_start = raw_data.data() + i * image_size;

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                m(r, c) = image_start[r * cols + c] / 255.0;
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

    // Read all data at once
    std::vector<unsigned char> raw_data(length);
    file.read(reinterpret_cast<char*>(raw_data.data()), length);
    if (!file) {
        std::println("Error reading data in file {}", file_path);
        std::abort();
    }

    // Convert to double vector
    std::vector<double> data(length);
    for (int i = 0; i < length; i++) {
        data[i] = static_cast<double>(raw_data[i]);
    }

    return data;
}
