#pragma once

#include <Eigen/Core>
#include <functional>
#include <thread>
#include "channel.h"
#include "nn.h"

namespace nnet {

struct TrainState {
    NN& nn;
    const std::vector<MatrixXd>& inputs;
    const std::vector<MatrixXd>& targets;

    int batch_size = 0;
    int backprop_iterations = 0;
    std::vector<int> batch_idxs;

    std::vector<std::vector<std::pair<MatrixXd, MatrixXd>>> gradients;
};

class TrainPool {
public:
    TrainPool(
        NN& nn, const std::vector<MatrixXd>& inputs, const std::vector<MatrixXd>& targets,
        int threads
    );

    void
        train(int batch_size, double learning_rate, std::function<void(int batch_idx)> on_comlpete);

private:
    TrainState m_state;

    std::vector<std::jthread> m_threads;
    std::vector<Sender<int>> m_send_start;
    std::vector<Receiver<int>> m_recv_done;
};

}
