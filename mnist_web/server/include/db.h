#pragma once

#include <sqlite3.h>
#include <asio/awaitable.hpp>
#include <asio/thread_pool.hpp>
#include <expected>
#include <optional>
#include <string_view>
#include <vector>

struct DBError {
    int code;
    std::string_view message;
};
template<typename T>
using DBResult = std::expected<T, DBError>;

struct GetNetworks {
    std::string name;
    std::vector<int> shape;
    int training_iterations;
    double cost;
};

struct GetNetwork {
    std::string name;
    std::vector<int> shape;
    int training_iterations;
    double cost;

    std::vector<double> weights;
    std::vector<double> biases;
};

struct AddNetwork {
    std::string name;
    std::vector<int> shape;

    std::vector<double> weights;
    std::vector<double> biases;
};

class DB {
public:
    DB(std::string_view path);
    ~DB();

    asio::awaitable<DBResult<void>> add_network(AddNetwork&& network);
    asio::awaitable<DBResult<bool>> delete_network(std::string_view name);
    asio::awaitable<DBResult<std::vector<GetNetworks>>> get_networks();
    asio::awaitable<DBResult<std::optional<GetNetwork>>> get_network(std::string_view name);

private:
    void setup_db();
    void setup_statements();

    template<typename T>
    asio::awaitable<std::expected<T, DBError>>
        run(std::function<std::expected<T, DBError>()> operation);

private:
    asio::thread_pool m_pool;

    std::unordered_map<const char*, sqlite3_stmt*> m_statements;
    sqlite3* m_db;
};
