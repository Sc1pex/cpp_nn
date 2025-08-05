#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>

namespace nnet {

template<typename T>
struct Inner {
    int m_sender_count = 0;
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable available;
};

template<typename T>
class Sender {
public:
    Sender<T>(std::shared_ptr<Inner<T>> inner) : m_inner(inner) {
        std::lock_guard<std::mutex> lock(m_inner->m_mutex);
        m_inner->m_sender_count++;
    }
    Sender<T>(const Sender<T>& other) : m_inner(other.m_inner) {
        std::lock_guard<std::mutex> lock(m_inner->m_mutex);
        m_inner->m_sender_count++;
    }
    Sender<T>(Sender<T>&& other) {
        m_inner = std::move(other.m_inner);
    }

    ~Sender<T>() {
        if (m_inner == nullptr) {
            return;
        }

        bool last_sender = false;
        {
            std::lock_guard<std::mutex> lock(m_inner->m_mutex);
            m_inner->m_sender_count--;
            last_sender = m_inner->m_sender_count == 0;
        }
        if (last_sender) {
            m_inner->available.notify_one();
        }
    }

    void send(T data) {
        {
            std::lock_guard<std::mutex> lock(m_inner->m_mutex);
            m_inner->m_queue.push(std::move(data));
        }
        m_inner->available.notify_one();
    }

private:
    std::shared_ptr<Inner<T>> m_inner;
};

template<typename T>
class Receiver {
public:
    Receiver<T>(std::shared_ptr<Inner<T>> inner) {
        m_inner = inner;
    }

    std::optional<T> recv() {
        std::unique_lock<std::mutex> lock(m_inner->m_mutex);
        if (m_inner->m_queue.empty()) {
            m_inner->available.wait(lock, [this] {
                return !m_inner->m_queue.empty() || m_inner->m_sender_count == 0;
            });
        }

        if (m_inner->m_queue.empty() && m_inner->m_sender_count == 0) {
            // Channel is closed and empty
            return std::nullopt;
        }

        T data = std::move(m_inner->m_queue.front());
        m_inner->m_queue.pop();
        return data;
    }

private:
    std::shared_ptr<Inner<T>> m_inner;
};

template<typename T>
std::pair<Sender<T>, Receiver<T>> channel() {
    std::shared_ptr<Inner<T>> inner = std::make_shared<Inner<T>>();
    return std::make_pair(Sender{ inner }, Receiver{ inner });
}

}
