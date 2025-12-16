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
};

struct NetworkInfo {
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
};

struct NetworkSummary {
    int id;
    std::string name;
    std::string created_at;
    std::vector<int> layer_sizes;
    int correct_predictions;
    int training_epochs;
    double cost;
};

void to_json(json& j, const NetworkInfo& v);
void to_json(json& j, const NetworkSummary& v);

class Db {
public:
    Db(std::string_view db_file_path, std::string_view train_input_path,
       std::string_view train_output_path, std::string_view test_input_path,
       std::string_view test_output_path);

    ~Db();

    asio::awaitable<DBResult<void>> add_network(const AddNetwork&& network);
    asio::awaitable<DBResult<std::optional<NetworkInfo>>> get_network_by_id(const int id);
    asio::awaitable<DBResult<std::vector<NetworkSummary>>> get_networks();
    asio::awaitable<DBResult<bool>> delete_network_by_id(const int id);

private:
    bool check_data_exists();
    void add_test_train_data(
        std::string_view train_input_path, std::string_view train_output_path,
        std::string_view test_input_path, std::string_view test_output_path
    );

    void create_statements();
    void create_tables();

    sqlite3* m_db;
    asio::thread_pool m_pool;

    std::unordered_map<const char*, sqlite3_stmt*> m_stmts;
};
