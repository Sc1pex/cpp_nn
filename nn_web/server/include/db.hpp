#pragma once

#include <sqlite3.h>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include "asio/awaitable.hpp"
#include "asio/thread_pool.hpp"
#include "nlohmann/json.hpp"

using nlohmann::json;

struct DBError {
    int code;
    std::string_view message;
};
template<typename T>
using DBResult = std::expected<T, DBError>;

struct AddNetwork {
    std::string name;
    std::vector<int> layer_sizes;
    std::vector<double> weights;
    std::vector<double> biases;
    std::vector<std::string> activations;
    std::string loss;
};

struct NetworkFull {
    int id;
    std::string name;
    std::string created_at;
    std::vector<int> layer_sizes;
    int correct_predictions;
    int training_epochs;
    double cost;
    std::vector<double> weights;
    std::vector<double> biases;
    std::vector<std::string> activations;
    std::string loss;
};

struct NetworkInfo {
    int id;
    std::string name;
    std::string created_at;
    std::vector<int> layer_sizes;
    int correct_predictions;
    int training_epochs;
    double cost;
    std::vector<std::string> activations;
    std::string loss;
};

void to_json(json& j, const NetworkFull& v);
void to_json(json& j, const NetworkInfo& v);

class Db {
public:
    Db(std::string_view db_file_path);

    ~Db();

    asio::awaitable<DBResult<void>> add_network(const AddNetwork&& network);
    asio::awaitable<DBResult<std::optional<NetworkFull>>> get_full_network_by_id(const int id);
    asio::awaitable<DBResult<std::optional<NetworkInfo>>> get_network_by_id(const int id);
    asio::awaitable<DBResult<std::vector<NetworkInfo>>> get_networks();
    asio::awaitable<DBResult<bool>> delete_network_by_id(const int id);

    asio::awaitable<DBResult<void>> update_network_weights(
        int id, const std::vector<double>& weights, const std::vector<double>& biases,
        int epochs_added
    );

private:
    void create_statements();
    void create_tables();

    sqlite3* m_db;
    asio::thread_pool m_pool;

    std::unordered_map<const char*, sqlite3_stmt*> m_stmts;
};
