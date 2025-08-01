#include <print>
#include <vector>
#include "nn/nn.h"

double xor_propagate(double x, double y, NN& nn) {
    MatrixXd m{ { x, y } };

    auto out = nn.feed_forward(m);
    return out(0);
}

int main() {
    std::vector shape = { 2, 2, 1 };
    NN nn{ shape };

    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            double value = xor_propagate(x, y, nn);
            std::println("{} ^ {} = {}", x, y, value);
        }
    }

    MatrixXd inputs{ { 0., 0. }, { 0., 1. }, { 1., 0. }, { 1., 1. } };
    MatrixXd expected{ { 0. }, { 1. }, { 1. }, { 0. } };
    double cost = nn.cost(inputs, expected);
    std::println("cost: {}", cost);

    for (int i = 0; i < 200000; i++) {
        nn.backprop(inputs, expected, 0.05);
        cost = nn.cost(inputs, expected);
        if (i % 100 == 99) {
            std::println("cost: {}", cost);
        }
    }

    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            double value = xor_propagate(x, y, nn);
            std::println("{} ^ {} = {}", x, y, value);
        }
    }
}
