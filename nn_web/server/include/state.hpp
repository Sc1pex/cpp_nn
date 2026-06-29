#pragma once

#include "db.hpp"
#include "memmat.hpp"

const int TRAIN_SIZE = 60000;
const int TEST_SIZE = 10000;
const int IMAGE_SIZE = 28 * 28;
const int LABEL_SIZE = 10;

struct State {
    State()
    : db("nn_web.db"), train_inputs("mnist_data/train_inputs.bin", TRAIN_SIZE, IMAGE_SIZE),
      train_labels("mnist_data/train_labels.bin", TRAIN_SIZE, LABEL_SIZE),
      test_inputs("mnist_data/test_inputs.bin", TEST_SIZE, IMAGE_SIZE),
      test_labels("mnist_data/test_labels.bin", TEST_SIZE, LABEL_SIZE) {
    }

    Db db;
    MemMatrix train_inputs;
    MemMatrix train_labels;
    MemMatrix test_inputs;
    MemMatrix test_labels;
};
