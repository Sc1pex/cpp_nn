#pragma once

#include <string>

class CliArgs {
public:
    static CliArgs parse(int argc, char** argv);

    std::string train_inputs_in() {
        return input_dir + "/train-images-idx3-ubyte";
    }
    std::string train_inputs_out() {
        return output_dir + "/train_inputs.bin";
    }
    std::string train_labels_in() {
        return input_dir + "/train-labels-idx1-ubyte";
    }
    std::string train_labels_out() {
        return output_dir + "/train_labels.bin";
    }
    std::string test_inputs_in() {
        return input_dir + "/t10k-images-idx3-ubyte";
    }
    std::string test_inputs_out() {
        return output_dir + "/test_inputs.bin";
    }
    std::string test_labels_in() {
        return input_dir + "/t10k-labels-idx1-ubyte";
    }
    std::string test_labels_out() {
        return output_dir + "/test_labels.bin";
    }

private:
    std::string input_dir;
    std::string output_dir;
};
