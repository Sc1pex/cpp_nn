#include <print>
#include "idx.h"

void print_block(double v) {
    v *= 24.;
    int c = (int) v + 232;
    if (c > 255) {
        c = 255;
    }
    std::print("\x1b[38;5;{}m██\x1b[0m", c);
}

int main() {
    auto data = idx3_read_file("data/train-images.idx3-ubyte");
    auto data2 = idx3_read_file("data/t10k-images.idx3-ubyte");
    auto label = idx1_read_file("data/train-labels.idx1-ubyte");
    auto label2 = idx1_read_file("data/t10k-labels.idx1-ubyte");

    data.insert(
        data.end(), std::make_move_iterator(data2.begin()), std::make_move_iterator(data2.end())
    );
    label.insert(
        label.end(), std::make_move_iterator(label2.begin()), std::make_move_iterator(label2.end())
    );

    // select a random image
    std::srand(std::time(nullptr));
    int idx = std::rand() % data.size();

    auto& i = data[idx];
    std::println("Label: {}", (int) label[idx]);
    for (int r = 0; r < i.rows(); r++) {
        for (int c = 0; c < i.cols(); c++) {
            print_block(i.element(r, c));
        }
        std::println();
    }
}
