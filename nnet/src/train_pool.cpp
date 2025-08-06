#include "train_pool.h"
#include <numeric>
#include <random>
#include <ranges>

namespace nnet {

void thread(TrainState& state, int id, Receiver<int> start, Sender<int> done) {
    std::vector<MatrixXd> batch_inputs(state.batch_size);
    std::vector<MatrixXd> batch_targets(state.batch_size);

    while (auto batch_start_idx = start.recv()) {
        int start_idx = *batch_start_idx;

        for (int i = 0; i < state.batch_size; ++i) {
            int idx = start_idx + i;
            batch_inputs[i] = state.inputs[state.batch_idxs[idx]];
            batch_targets[i] = state.targets[state.batch_idxs[idx]];
        }

        auto gradients = state.nn.backprop_batch(batch_inputs, batch_targets);
        state.gradients[id] = gradients;

        done.send(id);
    }
}

TrainPool::TrainPool(
    NN& nn, const std::vector<MatrixXd>& inputs, const std::vector<MatrixXd>& targets, int threads,
    int batch_size
)
: m_state{ nn, inputs, targets } {
    m_state.batch_idxs = std::vector<int>(inputs.size());
    std::iota(m_state.batch_idxs.begin(), m_state.batch_idxs.end(), 0);
    m_state.gradients = std::vector<std::vector<std::pair<MatrixXd, MatrixXd>>>(threads);
    m_state.batch_size = batch_size;

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

void TrainPool::train(double learning_rate) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::shuffle(m_state.batch_idxs.begin(), m_state.batch_idxs.end(), gen);
    for (int k = 0; k < m_state.batch_idxs.size() / m_state.batch_size; k += m_threads.size()) {
        // Start processing batches in parallel
        for (int i = 0; i < m_threads.size(); ++i) {
            m_send_start[i].send((k + i) * m_state.batch_size);
        }

        // Wait for all threads to finish processing the batch
        for (int i = 0; i < m_threads.size(); ++i) {
            int thread_id = m_recv_done[i].recv().value();
        }

        // Aggregate gradients from all threads
        auto gradients = std::ranges::fold_left(
            m_state.gradients, std::vector<std::pair<MatrixXd, MatrixXd>>{},
            [](auto acc, const auto& grad) {
                if (acc.empty())
                    return grad;
                for (size_t i = 0; i < acc.size(); ++i) {
                    acc[i].first += grad[i].first;
                    acc[i].second += grad[i].second;
                }
                return acc;
            }
        );

        m_state.nn.apply_gradients(gradients, learning_rate);
    }
}

}
