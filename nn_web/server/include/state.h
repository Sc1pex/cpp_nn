#pragma once

#include "db.h"

struct State {
    State()
    : db("nn_web.db", "data/train-images-idx3-ubyte", "data/train-labels-idx1-ubyte",
         "data/t10k-images-idx3-ubyte", "data/t10k-labels-idx1-ubyte") {
    }

    Db db;
};
