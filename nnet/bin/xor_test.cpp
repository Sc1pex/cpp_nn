#include <print>
#include "nn.h"

int main() {
    nnet::NN nn = nnet::NN::init_random({ 2, 2, 1 });

    nnet::MatrixXd inputs(2, 4);
    inputs << 0, 0, 1, 1, 0, 1, 0, 1;
    nnet::MatrixXd targets(1, 4);
    targets << 0, 1, 1, 0;

    const double learning_rate = 0.05;

    auto cost = nn.cost(inputs, targets);
    int epoch = 1;
    while (cost > 0.01) {
        auto grads = nn.backprop_batch_matrix(inputs, targets);
        nn.apply_gradients(grads, learning_rate);

        cost = nn.cost(inputs, targets);
        if (epoch % 50 == 0) {
            std::println("Epoch {}. Cost {}", epoch, cost);
        }
        epoch++;
    }

    for (int i = 0; i < inputs.cols(); ++i) {
        auto output = nn.feed_forward(inputs.col(i));
        std::println("Output: {}x{}", output.rows(), output.cols());
        std::println("{} ^ {} = {}", inputs(0, i), inputs(1, i), output(0, 0));
    }

    return 0;
}
