#include <print>
#include <vector>
#include "nn/nn.h"

f64 xor_propagate(f64 x, f64 y, NN& nn) {
    std::vector input = { x, y };
    Matrix m{ 1, 2, input.data() };

    auto out = nn.feed_forward(m);
    return out.element(0);
}

int main() {
    std::vector shape = { 2, 2, 1 };
    NN nn{ shape };

    for (u32 x = 0; x <= 1; x++) {
        for (u32 y = 0; y <= 1; y++) {
            f64 value = xor_propagate(x, y, nn);
            std::println("{} ^ {} = {}", x, y, value);
        }
    }

    std::vector inputs_data = { 0., 0., 0., 1., 1., 0., 1., 1. };
    Matrix inputs{ 4, 2, inputs_data.data() };
    std::vector expected_data = { 0., 1., 1., 0. };
    Matrix expected{ 4, 1, expected_data.data() };
    f64 cost = nn.cost(inputs, expected);
    std::println("cost: {}", cost);

    for (int i = 0; i < 200000; i++) {
        nn.backprop(inputs, expected, 0.01);
        cost = nn.cost(inputs, expected);
        if (i % 100 == 99) {
            std::println("cost: {}", cost);
        }
    }

    for (u32 x = 0; x <= 1; x++) {
        for (u32 y = 0; y <= 1; y++) {
            f64 value = xor_propagate(x, y, nn);
            std::println("{} ^ {} = {}", x, y, value);
        }
    }
}
