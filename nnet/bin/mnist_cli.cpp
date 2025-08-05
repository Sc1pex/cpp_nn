#include <print>
#include "idx.h"
#include "nn.h"
#include "train_pool.h"

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

    nnet::TrainPool pool(nn, data.train_inputs, data.train_labels, 20);

    const int batch_size = 100;
    const double learning_rate = 0.01;
    int epochs = 50;

    double cost = nn.cost(data.test_inputs, data.test_labels);

    while (epochs--) {
        pool.train(batch_size, learning_rate, [&](int backprop_iterations) {
            if (backprop_iterations % 100 == 0) {
                double cost = nn.cost(data.test_inputs, data.test_labels);
                std::println("Backprop iteration: {}, batch cost: {}", backprop_iterations, cost);
            }
        });
        std::println("Epoch completed");
    }
}
