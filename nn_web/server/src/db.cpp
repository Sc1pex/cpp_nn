#include "db.hpp"
#include <spdlog/spdlog.h>
#include <asio.hpp>
#include <expected>
#include <functional>
#include "nlohmann/json.hpp"

Db::Db(std::string_view db_file_path) : m_pool(1) {
    spdlog::info("Opening database at {}", db_file_path);
    if (sqlite3_open(db_file_path.data(), &m_db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database");
    }

    create_tables();
    create_statements();
}

Db::~Db() {
    m_pool.join();

    for (auto& [name, stmt] : m_stmts) {
        sqlite3_finalize(stmt);
    }

    if (m_db) {
        spdlog::info("Closing database");
        sqlite3_close(m_db);
    }
}

void Db::create_tables() {
    auto create_table = [this](const char* sql) {
        char* err_msg = nullptr;
        if (sqlite3_exec(this->m_db, sql, 0, 0, &err_msg) != SQLITE_OK) {
            std::string error = "Failed to create table: ";
            error += err_msg;
            sqlite3_free(err_msg);
            throw std::runtime_error(error);
        }
    };

    const char* create_networks_table_sql = R"(
    CREATE TABLE IF NOT EXISTS networks (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL UNIQUE,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        
        layer_sizes TEXT NOT NULL,  -- JSON: [784, 128, 64, 10]
        
        correct_predictions INTEGER NOT NULL DEFAULT 0,
        training_epochs INTEGER NOT NULL DEFAULT 0,
        cost REAL NOT NULL DEFAULT 0.0,
        
        weights BLOB NOT NULL,      -- serialized vector<double>
        biases BLOB NOT NULL,       -- serialized vector<double>
        activations TEXT NOT NULL,  -- "relu", "softmax", "sigmoid" or "linear"
        loss TEXT NOT NULL          -- "cross_entropy" or "mse"
    );)";
    create_table(create_networks_table_sql);

    const char* create_training_sessions_table_sql = R"(
    CREATE TABLE IF NOT EXISTS training_sessions (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        network_id INTEGER NOT NULL,
        epoch INTEGER NOT NULL,
        train_loss REAL NOT NULL,
        train_correct INTEGER NOT NULL,
        test_correct INTEGER NOT NULL,
        FOREIGN KEY(network_id) REFERENCES networks(id)
    );)";
    create_table(create_training_sessions_table_sql);
}

template<typename F>
asio::awaitable<std::invoke_result_t<F>> run_on_pool(F&& operation, asio::thread_pool& pool) {
    using ReturnType = std::invoke_result_t<F>;
    co_return co_await asio::co_spawn(
        pool.executor(),
        [](op = std::forward<F>(operation)) mutable -> asio::awaitable<ReturnType> {
        co_return op();
    }, asio::use_awaitable
    );
}

asio::awaitable<DBResult<void>> Db::add_network(const AddNetwork&& network) {
    co_return co_await run_on_pool([this, network]() -> std::expected<void, DBError> {
        sqlite3_stmt* stmt = m_stmts["add_network"];

        sqlite3_bind_text(stmt, 1, network.name.c_str(), -1, SQLITE_STATIC);

        nlohmann::json layer_sizes_json = network.layer_sizes;
        std::string layer_sizes_str = layer_sizes_json.dump();
        sqlite3_bind_text(stmt, 2, layer_sizes_str.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_blob(
            stmt, 3, network.weights.data(),
            static_cast<int>(network.weights.size() * sizeof(double)), SQLITE_STATIC
        );
        sqlite3_bind_blob(
            stmt, 4, network.biases.data(),
            static_cast<int>(network.biases.size() * sizeof(double)), SQLITE_STATIC
        );

        nlohmann::json activations_json = network.activations;
        std::string activations_str = activations_json.dump();
        sqlite3_bind_text(stmt, 5, activations_str.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(stmt, 6, network.loss.c_str(), -1, SQLITE_STATIC);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            rc = sqlite3_extended_errcode(m_db);

            sqlite3_reset(stmt);
            return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
        }

        sqlite3_reset(stmt);
        return {};
    }, m_pool);
}

asio::awaitable<DBResult<std::optional<NetworkInfo>>> Db::get_network_by_id(const int id) {
    co_return co_await run_on_pool(
        [this, id]() -> std::expected<std::optional<NetworkInfo>, DBError> {
        sqlite3_stmt* stmt = m_stmts["get_network_by_id"];

        sqlite3_bind_int(stmt, 1, id);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            rc = sqlite3_extended_errcode(m_db);
            sqlite3_reset(stmt);
            if (rc == SQLITE_DONE) {
                return std::nullopt;
            } else {
                return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
            }
        }

        NetworkInfo network;
        network.id = sqlite3_column_int(stmt, 0);
        network.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        network.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        std::string layer_sizes_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        nlohmann::json layer_sizes_json = nlohmann::json::parse(layer_sizes_str);
        network.layer_sizes = layer_sizes_json.get<std::vector<int>>();

        network.correct_predictions = sqlite3_column_int(stmt, 4);
        network.cost = sqlite3_column_double(stmt, 5);
        network.training_epochs = sqlite3_column_int(stmt, 6);

        std::string activations_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        nlohmann::json activations_json = nlohmann::json::parse(activations_str);
        network.activations = activations_json.get<std::vector<std::string>>();

        network.loss = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

        sqlite3_reset(stmt);
        return network;
    }, m_pool
    );
}

asio::awaitable<DBResult<std::optional<NetworkFull>>> Db::get_full_network_by_id(const int id) {
    co_return co_await run_on_pool(
        [this, id]() -> std::expected<std::optional<NetworkFull>, DBError> {
        sqlite3_stmt* stmt = m_stmts["get_full_network_by_id"];

        sqlite3_bind_int(stmt, 1, id);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            rc = sqlite3_extended_errcode(m_db);
            sqlite3_reset(stmt);
            if (rc == SQLITE_DONE) {
                return std::nullopt;
            } else {
                return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
            }
        }

        NetworkFull network;
        network.id = sqlite3_column_int(stmt, 0);
        network.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        network.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        std::string layer_sizes_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        nlohmann::json layer_sizes_json = nlohmann::json::parse(layer_sizes_str);
        network.layer_sizes = layer_sizes_json.get<std::vector<int>>();

        network.correct_predictions = sqlite3_column_int(stmt, 4);
        network.cost = sqlite3_column_double(stmt, 5);
        network.training_epochs = sqlite3_column_int(stmt, 6);

        const void* weights_blob = sqlite3_column_blob(stmt, 7);
        int weights_size = sqlite3_column_bytes(stmt, 7);
        network.weights.resize(weights_size / sizeof(double));
        std::memcpy(network.weights.data(), weights_blob, static_cast<size_t>(weights_size));

        const void* biases_blob = sqlite3_column_blob(stmt, 8);
        int biases_size = sqlite3_column_bytes(stmt, 8);
        network.biases.resize(biases_size / sizeof(double));
        std::memcpy(network.biases.data(), biases_blob, static_cast<size_t>(biases_size));

        std::string activations_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        nlohmann::json activations_json = nlohmann::json::parse(activations_str);
        network.activations = activations_json.get<std::vector<std::string>>();

        network.loss = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));

        sqlite3_reset(stmt);
        return network;
    }, m_pool
    );
}

asio::awaitable<DBResult<std::vector<NetworkInfo>>> Db::get_networks() {
    co_return co_await run_on_pool([this]() -> std::expected<std::vector<NetworkInfo>, DBError> {
        sqlite3_stmt* stmt = m_stmts["get_networks"];

        std::vector<NetworkInfo> networks;

        int rc;
        while (true) {
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_ROW) {
                break;
            }

            NetworkInfo network;
            network.id = sqlite3_column_int(stmt, 0);
            network.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            network.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

            std::string layer_sizes_str =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            nlohmann::json layer_sizes_json = nlohmann::json::parse(layer_sizes_str);
            network.layer_sizes = layer_sizes_json.get<std::vector<int>>();

            network.correct_predictions = sqlite3_column_int(stmt, 4);
            network.cost = sqlite3_column_double(stmt, 5);
            network.training_epochs = sqlite3_column_int(stmt, 6);

            std::string activations_str =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            nlohmann::json activations_json = nlohmann::json::parse(activations_str);
            network.activations = activations_json.get<std::vector<std::string>>();

            network.loss = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

            networks.push_back(network);
        }

        if (rc != SQLITE_DONE) {
            rc = sqlite3_extended_errcode(m_db);
            sqlite3_reset(stmt);
            return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
        }

        sqlite3_reset(stmt);
        return networks;
    }, m_pool);
}

asio::awaitable<DBResult<bool>> Db::delete_network_by_id(const int id) {
    co_return co_await run_on_pool([this, id]() -> std::expected<bool, DBError> {
        sqlite3_stmt* stmt = m_stmts["delete_network_by_id"];

        sqlite3_bind_int(stmt, 1, id);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_reset(stmt);
            return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
        }

        int rows_affected = sqlite3_changes(m_db);
        sqlite3_reset(stmt);

        return rows_affected > 0;
    }, m_pool);
}

asio::awaitable<DBResult<void>> Db::update_network_weights(
    int id, const std::vector<double>& weights, const std::vector<double>& biases, int epochs_added
) {
    co_return co_await run_on_pool(
        [this, id, weights, biases, epochs_added]() -> std::expected<void, DBError> {
        sqlite3_stmt* stmt = m_stmts["update_network_weights"];

        sqlite3_bind_blob(
            stmt, 1, weights.data(), static_cast<int>(weights.size() * sizeof(double)),
            SQLITE_STATIC
        );
        sqlite3_bind_blob(
            stmt, 2, biases.data(), static_cast<int>(biases.size() * sizeof(double)), SQLITE_STATIC
        );
        sqlite3_bind_int(stmt, 3, epochs_added);
        sqlite3_bind_int(stmt, 4, id);

        int rc = sqlite3_step(stmt);
        sqlite3_reset(stmt);

        if (rc != SQLITE_DONE) {
            return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
        }
        return {};
    }, m_pool
    );
}

void Db::create_statements() {
    auto add_stmt = [this](const char* sql, const char* name) {
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            // Get error message
            const char* err_msg = sqlite3_errmsg(m_db);
            spdlog::error("Failed to prepare {} statement: {}", name, err_msg);
            throw std::runtime_error(
                "Failed to prepare " + std::string(name) + " statement" + err_msg
            );
        }
        m_stmts[name] = stmt;
    };

    const char* add_network_sql = R"(
    INSERT INTO networks (
        name, layer_sizes, weights, biases, activations, loss
    ) VALUES (?, ?, ?, ?, ?, ?);)";
    add_stmt(add_network_sql, "add_network");

    const char* get_network_by_id_sql = R"(
    SELECT id, name, created_at, layer_sizes, correct_predictions, cost, training_epochs, activations, loss
    FROM networks WHERE id = ?;)";
    add_stmt(get_network_by_id_sql, "get_network_by_id");

    const char* get_full_network_by_id_sql = R"(
    SELECT id, name, created_at, layer_sizes, correct_predictions, cost, training_epochs, weights, biases, activations, loss
    FROM networks WHERE id = ?;)";
    add_stmt(get_full_network_by_id_sql, "get_full_network_by_id");

    const char* get_networks_sql = R"(
    SELECT id, name, created_at, layer_sizes, correct_predictions, cost, training_epochs, activations, loss FROM networks;)";
    add_stmt(get_networks_sql, "get_networks");

    const char* delete_network_by_id_sql = R"(
    DELETE FROM networks WHERE id = ?;)";
    add_stmt(delete_network_by_id_sql, "delete_network_by_id");

    const char* update_network_weights_sql = R"(
    UPDATE networks SET weights = ?, biases = ?, training_epochs = training_epochs + ? WHERE id = ?;)";
    add_stmt(update_network_weights_sql, "update_network_weights");
}

void to_json(json& j, const NetworkFull& v) {
    j = json{
        { "id", v.id },
        { "name", v.name },
        { "created_at", v.created_at },
        { "layer_sizes", v.layer_sizes },
        { "correct_predictions", v.correct_predictions },
        { "training_epochs", v.training_epochs },
        { "cost", v.cost },
        { "weights", v.weights },
        { "biases", v.biases },
        { "activations", v.activations },
        { "loss", v.loss },
    };
}

void to_json(json& j, const NetworkInfo& v) {
    j = json{
        { "id", v.id },
        { "name", v.name },
        { "created_at", v.created_at },
        { "layer_sizes", v.layer_sizes },
        { "correct_predictions", v.correct_predictions },
        { "training_epochs", v.training_epochs },
        { "cost", v.cost },
        { "activations", v.activations },
        { "loss", v.loss },
    };
}
