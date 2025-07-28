#include <print>
#include <vector>
#include "nn/nn.h"

int main() {
    std::vector<uint32_t> shape = { 2, 5, 5, 8, 10 };
    NN network(shape);

    double input_data[] = { 0.5, -0.3 };
    Matrix input(1, 2, input_data);

    Matrix output = network.feed_forward(std::move(input));

    for (u32 i = 0; i < output.rows(); ++i) {
        for (u32 j = 0; j < output.cols(); ++j) {
            std::print("Output[{}, {}]: {}\n", i, j, output.element(i, j));
        }
    }
    return 0;
}
