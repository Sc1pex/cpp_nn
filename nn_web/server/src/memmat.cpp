#include "memmat.hpp"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

MemMatrix::MemMatrix(const std::string& path, int rows, int cols) : m_rows(rows), m_cols(cols) {
    m_fd = open(path.c_str(), O_RDONLY);
    if (m_fd == -1) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    struct stat sb;
    if (fstat(m_fd, &sb) == -1) {
        close(m_fd);
        throw std::runtime_error("Failed to get file size: " + path);
    }
    m_size = sb.st_size;

    int expected_size = rows * cols * sizeof(double);
    if (m_size != expected_size) {
        close(m_fd);
        throw std::runtime_error("File size does not match expected matrix size");
    }

    void* m_addr = mmap(nullptr, m_size, PROT_READ, MAP_SHARED, m_fd, 0);
    if (m_addr == MAP_FAILED) {
        close(m_fd);
        throw std::runtime_error("mmap failed");
    }

    madvise(m_addr, m_size, MADV_SEQUENTIAL);
    m_data = reinterpret_cast<const double*>(m_addr);
}

MemMatrix::~MemMatrix() {
    if (m_data) {
        munmap(const_cast<double*>(m_data), m_size);
    }
    if (m_fd != -1) {
        close(m_fd);
    }
}

Eigen::Map<const Eigen::MatrixXd> MemMatrix::mat() const {
    return Eigen::Map<const Eigen::MatrixXd>(m_data, m_rows, m_cols);
}
