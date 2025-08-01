#include <print>
#include <vector>
#include "nn/nn.h"

int main() {
    std::vector<uint32_t> shape = { 2, 5, 5, 8, 10 };
    NN network(shape);

    double input_data[] = { 0.5, -0.3 };
    MatrixXd input{ { 0.5, -0.3 } };

    MatrixXd output = network.feed_forward(std::move(input));

    for (int i = 0; i < output.rows(); ++i) {
        for (int j = 0; j < output.cols(); ++j) {
            std::print("Output[{}, {}]: {}\n", i, j, output(i, j));
        }
    }

    return 0;
}
