#include <print>
#include "nn.h"

int main() {
    nnet::NN nn = nnet::NN::init_random({ 2, 2, 1 });

    std::vector<nnet::MatrixXd> inputs = { nnet::MatrixXd{ { 0 }, { 0 } },
                                           nnet::MatrixXd{ { 0 }, { 1 } },
                                           nnet::MatrixXd{ { 1 }, { 0 } },
                                           nnet::MatrixXd{ { 1 }, { 1 } } };
    std::vector<nnet::MatrixXd> targets = { nnet::MatrixXd::Constant(1, 1, 0),
                                            nnet::MatrixXd::Constant(1, 1, 1),
                                            nnet::MatrixXd::Constant(1, 1, 1),
                                            nnet::MatrixXd::Constant(1, 1, 0) };

    const double learning_rate = 0.05;

    auto cost = nn.cost(inputs, targets);
    int epoch = 1;
    while (cost > 0.01) {
        auto grads = nn.backprop_batch(inputs, targets);
        nn.apply_gradients(grads, learning_rate);

        cost = nn.cost(inputs, targets);
        if (epoch % 50 == 0) {
            std::println("Epoch {}. Cost {}", epoch, cost);
        }
        epoch++;
    }

    for (const auto& input : inputs) {
        auto output = nn.feed_forward(input);
        std::println("Output: {}x{}", output.rows(), output.cols());
        std::println("{} ^ {} = {}", input(0, 0), input(1, 0), output(0, 0));
    }

    return 0;
}
