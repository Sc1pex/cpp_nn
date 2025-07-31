#include <nn/nn.h>
#include <algorithm>
#include <print>
#include <random>
#include <ranges>
#include <vector>
#include "idx.h"

struct Digit {
    Matrix m;
    int label;
};

int main() {
    auto data1 = idx3_read_file("data/train-images.idx3-ubyte");
    auto data2 = idx3_read_file("data/t10k-images.idx3-ubyte");
    auto label1 = idx1_read_file("data/train-labels.idx1-ubyte");
    auto label2 = idx1_read_file("data/t10k-labels.idx1-ubyte");
    const uint32_t data_dim = data1[0].rows() * data1[0].cols();

    std::println("Read idx data");

    Matrix all_inputs{ (uint32_t) (data1.size() + data2.size()), data_dim };
    Matrix all_labels{ (uint32_t) (data1.size() + data2.size()), 10 };
    for (auto [i, v] : std::views::enumerate(data1)) {
        for (int k = 0; k < v.cols() * v.rows(); k++) {
            all_labels.set_element(i, k, v.element(k));
        }
        all_labels.set_element(i, label1[i], 1);
    }
    for (auto [i, v] : std::views::enumerate(data2)) {
        for (int k = 0; k < v.cols() * v.rows(); k++) {
            all_labels.set_element(i, k, v.element(k));
        }
        all_labels.set_element(i, label2[i], 1);
    }

    std::println("Created matrices");

    std::vector shape = { 28 * 28, 16, 16, 10 };
    NN nn{ shape };

    std::vector<int> batch_idxs(all_inputs.rows());
    std::iota(batch_idxs.begin(), batch_idxs.end(), 0);

    std::random_device rd;
    std::mt19937 gen(rd());
    const int batch_size = 10;
    int training_loops = 1000;

    while (training_loops--) {
        std::shuffle(batch_idxs.begin(), batch_idxs.end(), gen);
        int backprop_iterations = 0;
        for (int k = 0; k < batch_idxs.size() / batch_size; k++) {
            Matrix batch_inp{ batch_size, data_dim };
            for (int b = 0; b < batch_size; b++) {
                for (int i = 0; i < data_dim; i++) {
                    batch_inp.set_element(
                        b, i, all_inputs.element(batch_idxs[k * batch_size + b], i)
                    );
                }
            }

            Matrix batch_labels{ batch_size, 10 };
            for (int b = 0; b < batch_size; b++) {
                for (int i = 0; i < 10; i++) {
                    batch_labels.set_element(
                        b, i, all_labels.element(batch_idxs[k * batch_size + b], i)
                    );
                }
            }

            nn.backprop(batch_inp, batch_labels, 0.05);
            backprop_iterations++;
            if (backprop_iterations % 100 == 0) {
                f64 cost = nn.cost(batch_inp, batch_labels);
                std::println("Backprop iteration: {}, batch cost: {}", backprop_iterations, cost);
            }
        }
        std::println("Training loop done");
        std::println("Total cost: {}", nn.cost(all_inputs, all_labels));
    }
}
