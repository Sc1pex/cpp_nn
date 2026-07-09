#pragma once

#include <sqlite3.h>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include "asio/awaitable.hpp"
#include "asio/thread_pool.hpp"
#include "db_error.hpp"
#include "db_statement.hpp"
#include "nlohmann/json.hpp"

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
    int loss_points;
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

void to_json(nlohmann::json& j, const NetworkFull& v);
void to_json(nlohmann::json& j, const NetworkInfo& v);

class Db {
public:
    static Db create(const std::string& db_file_path);

    asio::awaitable<DBResult<void>> add_network(AddNetwork network);
    asio::awaitable<DBResult<std::optional<NetworkFull>>> get_full_network_by_id(int id);
    asio::awaitable<DBResult<std::optional<NetworkInfo>>> get_network_by_id(int id);
    asio::awaitable<DBResult<std::vector<NetworkInfo>>> get_networks();
    asio::awaitable<DBResult<bool>> delete_network_by_id(int id);

    asio::awaitable<DBResult<void>> update_network_weights(
        int id, std::vector<double> weights, std::vector<double> biases, int epochs_added,
        int loss_points_added
    );

    asio::awaitable<DBResult<void>>
        insert_training_session(int network_id, int epoch, int test_correct);
    asio::awaitable<DBResult<std::vector<int>>> get_test_points(int network_id);

    asio::awaitable<DBResult<void>> insert_loss_point(int network_id, int step, double loss);
    asio::awaitable<DBResult<std::vector<double>>> get_loss_points(int network_id);

private:
    struct Statements {
        DbStatement add_network;
        DbStatement get_network_by_id;
        DbStatement get_full_network_by_id;
        DbStatement get_networks;
        DbStatement update_network_params;
        DbStatement delete_network_by_id;

        DbStatement insert_training_session;
        DbStatement get_training_sessions;

        DbStatement insert_loss_point;
        DbStatement get_loss_points;
    };
    static Statements make_statements(sqlite3* db);

    static void enable_foreign_keys(sqlite3* db);
    static void create_tables(sqlite3* db);

    Db(std::unique_ptr<sqlite3, decltype(&sqlite3_close_v2)> db, Statements statements);

    // Destruction order matters (first the pool, then the statements and last the db)
    // so the fields must be in this order
    std::unique_ptr<sqlite3, decltype(&sqlite3_close_v2)> m_db;
    Statements m_statements;
    asio::thread_pool m_pool;
};
