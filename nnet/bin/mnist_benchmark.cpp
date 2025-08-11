#include <iostream>
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

    auto train_images = idx3_read_file("data/train-images-idx3-ubyte");
    auto test_images = idx3_read_file("data/t10k-images-idx3-ubyte");
    auto train_labels = idx1_read_file("data/train-labels-idx1-ubyte");
    auto test_labels = idx1_read_file("data/t10k-labels-idx1-ubyte");

    data.train_inputs.reserve(train_images.size());
    data.test_inputs.reserve(test_images.size());
    data.train_labels.reserve(train_labels.size());
    data.test_labels.reserve(test_labels.size());

    auto convert_image = [](const auto& image) {
        nnet::MatrixXd input(28 * 28, 1);
        std::memcpy(input.data(), image.data(), 28 * 28 * sizeof(double));
        return input;
    };
    auto convert_label = [](const auto& label) {
        nnet::MatrixXd label_matrix = nnet::MatrixXd::Zero(10, 1);
        label_matrix(static_cast<int>(label), 0) = 1.0;
        return label_matrix;
    };

    for (const auto& image : train_images) {
        data.train_inputs.emplace_back(convert_image(image));
    }
    for (const auto& image : test_images) {
        data.test_inputs.emplace_back(convert_image(image));
    }
    for (const auto& label : train_labels) {
        data.train_labels.emplace_back(convert_label(label));
    }
    for (const auto& label : test_labels) {
        data.test_labels.emplace_back(convert_label(label));
    }

    return data;
}

struct CliArgs {
    CliArgs(int argc, char** argv) {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "--batch_size" || arg == "-b") && i + 1 < argc) {
                batch_size = std::stoi(argv[++i]);
            } else if ((arg == "--learning_rate" || arg == "-l") && i + 1 < argc) {
                learning_rate = std::stod(argv[++i]);
            } else if ((arg == "--epochs" || arg == "-e") && i + 1 < argc) {
                epochs = std::stoi(argv[++i]);
            } else if ((arg == "--threads" || arg == "-t") && i + 1 < argc) {
                threads = std::stoi(argv[++i]);
            } else {
                std::cerr << "Unknown argument: " << arg << std::endl;
                std::cerr << "Usage: " << argv[0]
                          << " [--batch_size|-b <size>] [--learning_rate|-l <rate>] "
                          << "[--epochs|-e <num>] [--threads|-t <num>]" << std::endl;
                exit(1);
            }
        }
    }

    int batch_size = 32;
    double learning_rate = 0.01;
    int epochs = 1;
    int threads = std::thread::hardware_concurrency();
};

int main(int argc, char** argv) {
    CliArgs args(argc, argv);

    auto data = read_mnist_data();

    std::vector shape = { 28 * 28, 16, 16, 10 };
    nnet::NN nn = nnet::NN::init_random(shape);

    nnet::TrainPool pool(nn, data.train_inputs, data.train_labels, args.threads, args.batch_size);

    double cost = nn.cost(data.test_inputs, data.test_labels);

    while (args.epochs--) {
        pool.train(args.learning_rate);
    }
    std::println("{}", nn.cost(data.test_inputs, data.test_labels));
}
