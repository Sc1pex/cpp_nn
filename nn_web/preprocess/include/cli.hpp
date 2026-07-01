#pragma once

#include <string>

struct CliArgs {
    static CliArgs parse(int argc, char** argv);

    std::string train_inputs_in() const {
        return input_dir + "/train-images-idx3-ubyte";
    }
    std::string train_inputs_out() const {
        return output_dir + "/train_inputs.bin";
    }
    std::string train_labels_in() const {
        return input_dir + "/train-labels-idx1-ubyte";
    }
    std::string train_labels_out() const {
        return output_dir + "/train_labels.bin";
    }
    std::string test_inputs_in() const {
        return input_dir + "/t10k-images-idx3-ubyte";
    }
    std::string test_inputs_out() const {
        return output_dir + "/test_inputs.bin";
    }
    std::string test_labels_in() const {
        return input_dir + "/t10k-labels-idx1-ubyte";
    }
    std::string test_labels_out() const {
        return output_dir + "/test_labels.bin";
    }

    std::string input_dir;
    std::string output_dir;
};
