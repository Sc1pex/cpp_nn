#include "train_pool.h"
#include <numeric>
#include <random>
#include <ranges>

namespace nnet {

void thread(TrainState& state, Receiver<int> start, Sender<int> done) {
    while (auto chan_thread_idx = start.recv()) {
        int thread_idx = *chan_thread_idx;

        int start_idx = thread_idx * state.batch_size;
        while (start_idx < state.batch_idxs.size()) {
            int current_batch_size =
                std::min(state.batch_size, static_cast<int>(state.batch_idxs.size()) - start_idx);

            MatrixXd batch_inputs{ state.inputs[0].rows(), current_batch_size };
            MatrixXd batch_targets{ state.targets[0].rows(), current_batch_size };

            // Fill the batch with inputs and targets
            for (int i = 0; i < current_batch_size; ++i) {
                int idx = state.batch_idxs[start_idx + i];
                batch_inputs.col(i) = state.inputs[idx];
                batch_targets.col(i) = state.targets[idx];
            }

            auto gradients = state.nn.backprop_batch_matrix(batch_inputs, batch_targets);

            {
                std::lock_guard<std::mutex> lock(state.nn_mutex);
                state.nn.apply_gradients(gradients, state.learning_rate);
            }

            start_idx += state.batch_stride;
        }

        done.send(thread_idx);
    }
}

TrainPool::TrainPool(
    NN& nn, const std::vector<MatrixXd>& inputs, const std::vector<MatrixXd>& targets, int threads,
    int batch_size
)
: m_state{ nn, inputs, targets } {
    m_state.batch_idxs = std::vector<int>(inputs.size());
    std::iota(m_state.batch_idxs.begin(), m_state.batch_idxs.end(), 0);

    m_state.batch_size = batch_size;
    m_state.batch_stride = threads * batch_size;

    for (int i = 0; i < threads; ++i) {
        auto [tx_start, rx_start] = channel<int>();
        auto [tx_done, rx_done] = channel<int>();
        m_send_start.push_back(std::move(tx_start));
        m_recv_done.push_back(std::move(rx_done));

        m_threads.emplace_back([this, rx_start, tx_done]() {
            thread(m_state, std::move(rx_start), std::move(tx_done));
        });
    }
}

void TrainPool::train(double learning_rate) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(m_state.batch_idxs.begin(), m_state.batch_idxs.end(), gen);

    m_state.learning_rate = learning_rate;

    // Start processing batches in parallel
    for (int i = 0; i < m_threads.size(); ++i) {
        m_send_start[i].send(i);
    }

    // Wait for all threads to finish processing the batch
    for (int i = 0; i < m_threads.size(); ++i) {
        int thread_id = m_recv_done[i].recv().value();
    }
}
}
