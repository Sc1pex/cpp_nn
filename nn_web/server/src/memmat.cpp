#include "memmat.hpp"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

MemMatrix::MemMatrix(const std::string& path, int rows, int cols) : m_rows(rows), m_cols(cols) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        close(fd);
        throw std::runtime_error("Failed to get file size: " + path);
    }
    m_size = sb.st_size;

    int expected_size = rows * cols * sizeof(double);
    if (m_size != expected_size) {
        close(fd);
        throw std::runtime_error("File size does not match expected matrix size");
    }

    void* m_addr = mmap(nullptr, m_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if (m_addr == MAP_FAILED) {
        throw std::runtime_error("mmap failed");
    }

    madvise(m_addr, m_size, MADV_SEQUENTIAL);
    m_data = std::unique_ptr<const double, MunmapDeleter>{ static_cast<const double*>(m_addr),
                                                           MunmapDeleter{ m_size } };
}

Eigen::Map<const Eigen::MatrixXd> MemMatrix::mat() const {
    return Eigen::Map<const Eigen::MatrixXd>(m_data.get(), m_rows, m_cols);
}

void MemMatrix::MunmapDeleter::operator()(const double* p) const {
    if (p) {
        munmap(const_cast<double*>(p), size);
    }
}
