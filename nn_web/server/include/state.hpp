#pragma once

#include "db.hpp"

struct State {
    State()
    : db("nn_web.db", "mnist_data/train-images-idx3-ubyte", "mnist_data/train-labels-idx1-ubyte",
         "mnist_data/t10k-images-idx3-ubyte", "mnist_data/t10k-labels-idx1-ubyte") {
    }

    Db db;
};
