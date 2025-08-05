#include "train_pool.h"
#include <numeric>
#include <random>
#include <ranges>

namespace nnet {

void thread(TrainState& state, int id, Receiver<int> start, Sender<int> done) {
    while (auto batch_start_idx = start.recv()) {
        int start_idx = *batch_start_idx;
        auto batch_inputs = state.batch_idxs | std::views::drop(start_idx)
                            | std::views::take(state.batch_size)
                            | std::views::transform([&](int idx) {
                                  return state.inputs[idx];
                              });
        auto batch_targets = state.batch_idxs | std::views::drop(start_idx)
                             | std::views::take(state.batch_size)
                             | std::views::transform([&](int idx) {
                                   return state.targets[idx];
                               });

        auto gradients = state.nn.backprop_batch(batch_inputs, batch_targets);
        state.gradients[id] = gradients;

        done.send(id);
    }
}

TrainPool::TrainPool(
    NN& nn, const std::vector<MatrixXd>& inputs, const std::vector<MatrixXd>& targets, int threads
)
: m_state{ nn, inputs, targets } {
    m_state.batch_idxs = std::vector<int>(inputs.size());
    std::iota(m_state.batch_idxs.begin(), m_state.batch_idxs.end(), 0);
    m_state.gradients = std::vector<std::vector<std::pair<MatrixXd, MatrixXd>>>(threads);

    for (int i = 0; i < threads; ++i) {
        auto [tx_start, rx_start] = channel<int>();
        auto [tx_done, rx_done] = channel<int>();
        m_send_start.push_back(std::move(tx_start));
        m_recv_done.push_back(std::move(rx_done));

        m_threads.emplace_back([this, rx_start, tx_done, i]() {
            thread(m_state, i, std::move(rx_start), std::move(tx_done));
        });
    }
}

void TrainPool::train(
    int batch_size, double learning_rate, std::function<void(int batch_idx)> on_complete
) {
    m_state.batch_size = batch_size;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::shuffle(m_state.batch_idxs.begin(), m_state.batch_idxs.end(), gen);
    for (int k = 0; k < m_state.batch_idxs.size() / batch_size; k += m_threads.size()) {
        // Start processing batches in parallel
        for (int i = 0; i < m_threads.size(); ++i) {
            m_send_start[i].send((k + i) * batch_size);
        }

        // Wait for all threads to finish processing the batch
        for (int i = 0; i < m_threads.size(); ++i) {
            int thread_id = m_recv_done[i].recv().value();
        }

        // Aggregate gradients from all threads
        for (auto& gradients : m_state.gradients) {
            m_state.nn.apply_gradients(gradients, learning_rate);
            on_complete(++m_state.backprop_iterations);
        }
    }
}

}
