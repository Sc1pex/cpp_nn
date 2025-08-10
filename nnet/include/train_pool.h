#pragma once

#include <Eigen/Core>
#include <mutex>
#include <thread>
#include "channel.h"
#include "nn.h"

namespace nnet {

struct TrainState {
    NN& nn;
    const std::vector<MatrixXd>& inputs;
    const std::vector<MatrixXd>& targets;

    std::vector<int> batch_idxs;
    double learning_rate;
    int batch_size;
    int batch_stride;

    std::mutex nn_mutex;
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
