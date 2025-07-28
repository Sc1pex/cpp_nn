#include <print>
#include "idx.h"

int main() {
    auto data1 = idx3_read_file("data/train-images.idx3-ubyte");
    auto data2 = idx3_read_file("data/t10k-images.idx3-ubyte");
    auto label1 = idx1_read_file("data/train-labels.idx1-ubyte");
    auto label2 = idx1_read_file("data/t10k-labels.idx1-ubyte");

    std::println("{} {}", data1.size(), data2.size());
    std::println("{} {}", label1.size(), label2.size());
}
