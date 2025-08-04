#include <numeric>
#include <print>
#include <random>
#include <ranges>
#include "idx.h"
#include "nn.h"

struct MnistData {
    std::vector<nnet::MatrixXd> train_inputs;
    std::vector<nnet::MatrixXd> test_inputs;

    std::vector<nnet::MatrixXd> train_labels;
    std::vector<nnet::MatrixXd> test_labels;
};

MnistData read_mnist_data() {
    MnistData data;

    auto data1 = idx3_read_file("data/train-images-idx3-ubyte");
    auto data2 = idx3_read_file("data/t10k-images-idx3-ubyte");

    // Reshape the data to 1x(28*28) matrices
    for (const auto& image : data1) {
        nnet::MatrixXd input = nnet::MatrixXd::Zero(28 * 28, 1);
        for (int i = 0; i < 28; ++i) {
            for (int j = 0; j < 28; ++j) {
                input(i * 28 + j, 0) = image(i, j);
            }
        }
        data.train_inputs.push_back(input);
    }
    for (const auto& image : data2) {
        nnet::MatrixXd input = nnet::MatrixXd::Zero(28 * 28, 1);
        for (int i = 0; i < 28; ++i) {
            for (int j = 0; j < 28; ++j) {
                input(i * 28 + j, 0) = image(i, j);
            }
        }
        data.test_inputs.push_back(input);
    }

    auto label1 = idx1_read_file("data/train-labels-idx1-ubyte");
    auto label2 = idx1_read_file("data/t10k-labels-idx1-ubyte");

    // Convert labels to one-hot encoded matrices
    for (const auto& label : label1) {
        MatrixXd label_matrix = MatrixXd::Zero(10, 1);
        label_matrix((int) label, 0) = 1.0;
        data.train_labels.push_back(label_matrix);
    }
    for (const auto& label : label2) {
        MatrixXd label_matrix = MatrixXd::Zero(10, 1);
        label_matrix((int) label, 0) = 1.0;
        data.test_labels.push_back(label_matrix);
    }

    return data;
}

int main() {
    auto data = read_mnist_data();
    std::println(
        "Read {} training inputs, {} test inputs", data.train_inputs.size(), data.test_inputs.size()
    );

    std::vector shape = { 28 * 28, 16, 16, 10 };
    nnet::NN nn = nnet::NN::init_random(shape);

    std::vector<int> batch_idxs(data.train_inputs.size());
    std::iota(batch_idxs.begin(), batch_idxs.end(), 0);

    std::random_device rd;
    std::mt19937 gen(rd());
    const int batch_size = 50;
    const double learning_rate = 0.01;
    int epochs = 30;

    while (epochs--) {
        std::shuffle(batch_idxs.begin(), batch_idxs.end(), gen);
        int backprop_iterations = 0;
        for (int k = 0; k < batch_idxs.size() / batch_size; k++) {
            auto batch_inputs = batch_idxs | std::views::drop(k * batch_size)
                                | std::views::take(batch_size)
                                | std::views::transform([&](int idx) {
                                      return data.train_inputs[idx];
                                  });
            auto batch_labels = batch_idxs | std::views::drop(k * batch_size)
                                | std::views::take(batch_size)
                                | std::views::transform([&](int idx) {
                                      return data.train_labels[idx];
                                  });

            auto gradients = nn.backprop_batch(batch_inputs, batch_labels);
            nn.apply_gradients(gradients, learning_rate);

            backprop_iterations++;
            if (backprop_iterations % 100 == 0) {
                double cost = nn.cost(batch_inputs, batch_labels);
                std::println("Backprop iteration: {}, batch cost: {}", backprop_iterations, cost);
            }
        }
        std::println("Training loop done");
        std::println("Total cost: {}", nn.cost(data.test_inputs, data.test_labels));
    }
}
