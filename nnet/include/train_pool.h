#pragma once

#include <Eigen/Core>
#include <thread>
#include "channel.h"
#include "nn.h"

namespace nnet {

struct TrainState {
    NN& nn;
    const std::vector<MatrixXd>& inputs;
    const std::vector<MatrixXd>& targets;

    int batch_size;
    int backprop_iterations = 0;
    std::vector<int> batch_idxs;

    std::vector<std::vector<std::pair<MatrixXd, MatrixXd>>> gradients;
};

class TrainPool {
public:
    TrainPool(
        NN& nn, const std::vector<MatrixXd>& inputs, const std::vector<MatrixXd>& targets,
        int threads, int batch_size
    );

    void train(double learning_rate);

private:
    TrainState m_state;

    std::vector<std::jthread> m_threads;
    std::vector<Sender<int>> m_send_start;
    std::vector<Receiver<int>> m_recv_done;
};

}
