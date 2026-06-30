#pragma once

#include "db.hpp"
#include "memmat.hpp"
#include "nlohmann/json.hpp"

const int TRAIN_SIZE = 60000;
const int TEST_SIZE = 10000;
const int IMAGE_SIZE = 28 * 28;
const int LABEL_SIZE = 10;

struct Sample {
    Eigen::MatrixXd input;
    Eigen::MatrixXd label;
};

struct State {
    State()
    : db("nn_web.db"), train_inputs("mnist_data/train_inputs.bin", TRAIN_SIZE, IMAGE_SIZE),
      train_labels("mnist_data/train_labels.bin", TRAIN_SIZE, LABEL_SIZE),
      test_inputs("mnist_data/test_inputs.bin", TEST_SIZE, IMAGE_SIZE),
      test_labels("mnist_data/test_labels.bin", TEST_SIZE, LABEL_SIZE) {
    }

    std::optional<Sample> get_test_sample(int index) const;
    std::optional<Sample> get_train_sample(int index) const;

    Db db;
    MemMatrix train_inputs;
    MemMatrix train_labels;
    MemMatrix test_inputs;
    MemMatrix test_labels;
};

void to_json(json& j, const Sample& v);
