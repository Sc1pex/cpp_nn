#include "db.hpp"
#include <spdlog/spdlog.h>
#include <asio.hpp>
#include <expected>

using json = nlohmann::json;

Db Db::create(const std::string& db_file_path) {
    spdlog::info("Opening database at {}", db_file_path);
    sqlite3* raw_db = nullptr;
    int rc = sqlite3_open(db_file_path.data(), &raw_db);

    std::unique_ptr<sqlite3, decltype(&sqlite3_close_v2)> db(raw_db, sqlite3_close_v2);

    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to open database");
    }

    enable_foreign_keys(db.get());
    create_tables(db.get());
    auto stmts_map = create_statements(db.get());
    auto statements = make_statements(db.get());

    return Db(std::move(db), std::move(statements), std::move(stmts_map));
}

Db::Db(
    std::unique_ptr<sqlite3, decltype(&sqlite3_close_v2)> db, Statements statements, StmtsMap stmts
)
: m_db(std::move(db)), m_pool(1), m_statements(std::move(statements)), m_stmts(std::move(stmts)) {
}

void Db::enable_foreign_keys(sqlite3* db) {
    char* err_msg = nullptr;
    if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, &err_msg) != SQLITE_OK) {
        std::string error = "Failed to enable foreign keys: ";
        error += err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(error);
    }
}

void Db::create_tables(sqlite3* db) {
    auto create_table = [db](const char* sql) {
        char* err_msg = nullptr;
        if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK) {
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
        loss_points INTEGER NOT NULL DEFAULT 0,
        cost REAL NOT NULL DEFAULT 0.0,
        
        weights BLOB NOT NULL,      -- serialized vector<double>
        biases BLOB NOT NULL,       -- serialized vector<double>
        activations TEXT NOT NULL,  -- "relu", "softmax", "sigmoid" or "linear"
        loss TEXT NOT NULL          -- "cross_entropy" or "mse"
    );)";
    create_table(create_networks_table_sql);

    const char* create_training_sessions_table_sql = R"(
    CREATE TABLE IF NOT EXISTS training_sessions (
        network_id INTEGER NOT NULL,
        epoch INTEGER NOT NULL,
        test_correct INTEGER NOT NULL,
        PRIMARY KEY(network_id, epoch),
        FOREIGN KEY(network_id) REFERENCES networks(id) ON DELETE CASCADE
    );)";
    create_table(create_training_sessions_table_sql);

    const char* create_training_loss_points_table_sql = R"(
    CREATE TABLE IF NOT EXISTS training_loss_points (
        network_id INTEGER NOT NULL,
        step INTEGER NOT NULL,
        loss REAL NOT NULL,
        PRIMARY KEY(network_id, step),
        FOREIGN KEY(network_id) REFERENCES networks(id) ON DELETE CASCADE
    );)";
    create_table(create_training_loss_points_table_sql);
}

template<typename F>
asio::awaitable<std::invoke_result_t<F>> run_on_pool(F&& operation, asio::thread_pool& pool) {
    using ReturnType = std::invoke_result_t<F>;
    co_return co_await asio::co_spawn(pool.executor(), [&]() -> asio::awaitable<ReturnType> {
        co_return operation();
    }, asio::use_awaitable);
}

asio::awaitable<DBResult<void>> Db::add_network(AddNetwork network) {
    co_return co_await run_on_pool(
        [this, network = std::move(network)]() -> std::expected<void, DBError> {
        auto stmt = m_stmts.at("add_network").get();

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
            rc = sqlite3_extended_errcode(m_db.get());

            sqlite3_reset(stmt);
            return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
        }

        sqlite3_reset(stmt);
        return {};
    }, m_pool
    );
}

asio::awaitable<DBResult<std::optional<NetworkInfo>>> Db::get_network_by_id(int id) {
    co_return co_await run_on_pool(
        [this, id]() -> std::expected<std::optional<NetworkInfo>, DBError> {
        auto stmt = m_stmts.at("get_network_by_id").get();

        sqlite3_bind_int(stmt, 1, id);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            rc = sqlite3_extended_errcode(m_db.get());
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

asio::awaitable<DBResult<std::optional<NetworkFull>>> Db::get_full_network_by_id(int id) {
    co_return co_await run_on_pool(
        [this, id]() -> std::expected<std::optional<NetworkFull>, DBError> {
        auto stmt = m_stmts.at("get_full_network_by_id").get();

        sqlite3_bind_int(stmt, 1, id);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            rc = sqlite3_extended_errcode(m_db.get());
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

        network.loss_points = sqlite3_column_int(stmt, 11);

        sqlite3_reset(stmt);
        return network;
    }, m_pool
    );
}

asio::awaitable<DBResult<std::vector<NetworkInfo>>> Db::get_networks() {
    co_return co_await run_on_pool([this]() -> std::expected<std::vector<NetworkInfo>, DBError> {
        auto stmt = m_stmts.at("get_networks").get();

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
            rc = sqlite3_extended_errcode(m_db.get());
            sqlite3_reset(stmt);
            return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
        }

        sqlite3_reset(stmt);
        return networks;
    }, m_pool);
}

asio::awaitable<DBResult<bool>> Db::delete_network_by_id(int id) {
    co_return co_await run_on_pool([this, id]() -> std::expected<bool, DBError> {
        auto ex = m_statements.delete_network_by_id.bind(id);

        auto res = ex.next();
        if (!res) {
            return std::unexpected(res.error());
        }

        int rows_affected = sqlite3_changes(m_db.get());
        return rows_affected > 0;
    }, m_pool);
}

asio::awaitable<DBResult<void>> Db::update_network_weights(
    int id, std::vector<double> weights, std::vector<double> biases, int epochs_added,
    int loss_points_added
) {
    co_return co_await run_on_pool(
        [this, id, weights = std::move(weights), biases = std::move(biases), epochs_added,
         loss_points_added]() -> std::expected<void, DBError> {
        auto stmt = m_stmts.at("update_network_weights").get();

        sqlite3_bind_blob(
            stmt, 1, weights.data(), static_cast<int>(weights.size() * sizeof(double)),
            SQLITE_STATIC
        );
        sqlite3_bind_blob(
            stmt, 2, biases.data(), static_cast<int>(biases.size() * sizeof(double)), SQLITE_STATIC
        );
        sqlite3_bind_int(stmt, 3, epochs_added);
        sqlite3_bind_int(stmt, 4, loss_points_added);
        sqlite3_bind_int(stmt, 5, id);

        int rc = sqlite3_step(stmt);
        sqlite3_reset(stmt);

        if (rc != SQLITE_DONE) {
            return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
        }
        return {};
    },
        m_pool
    );
}

Db::StmtsMap Db::create_statements(sqlite3* db) {
    StmtsMap stmts;

    auto add_stmt = [&](const char* sql, const char* name) {
        sqlite3_stmt* raw_stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &raw_stmt, nullptr) != SQLITE_OK) {
            const char* err_msg = sqlite3_errmsg(db);
            spdlog::error("Failed to prepare {} statement: {}", name, err_msg);
            throw std::runtime_error(
                "Failed to prepare " + std::string(name) + " statement" + err_msg
            );
        }
        stmts.emplace(name, StmtPtr{ raw_stmt, sqlite3_finalize });
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
    SELECT id, name, created_at, layer_sizes, correct_predictions, cost, training_epochs, weights, biases, activations, loss, loss_points
    FROM networks WHERE id = ?;)";
    add_stmt(get_full_network_by_id_sql, "get_full_network_by_id");

    const char* get_networks_sql = R"(
    SELECT id, name, created_at, layer_sizes, correct_predictions, cost, training_epochs, activations, loss FROM networks;)";
    add_stmt(get_networks_sql, "get_networks");

    const char* update_network_weights_sql = R"(
    UPDATE networks SET weights = ?, biases = ?, training_epochs = training_epochs + ?, loss_points = loss_points + ? WHERE id = ?;)";
    add_stmt(update_network_weights_sql, "update_network_weights");

    const char* insert_training_session_sql = R"(
    INSERT INTO training_sessions (network_id, epoch, test_correct)
    VALUES (?, ?, ?);)";
    add_stmt(insert_training_session_sql, "insert_training_session");

    const char* get_training_sessions_sql = R"(
    SELECT test_correct
    FROM training_sessions WHERE network_id = ? ORDER BY epoch ASC;)";
    add_stmt(get_training_sessions_sql, "get_training_sessions");

    const char* insert_loss_point_sql = R"(
    INSERT INTO training_loss_points (network_id, step, loss)
    VALUES (?, ?, ?);)";
    add_stmt(insert_loss_point_sql, "insert_loss_point");

    const char* get_loss_points_sql = R"(
    SELECT loss
    FROM training_loss_points WHERE network_id = ? ORDER BY step ASC;)";
    add_stmt(get_loss_points_sql, "get_loss_points");

    return stmts;
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

asio::awaitable<DBResult<void>>
    Db::insert_training_session(int network_id, int epoch, int test_correct) {
    co_return co_await run_on_pool(
        [this, network_id, epoch, test_correct]() -> std::expected<void, DBError> {
        auto stmt = m_stmts.at("insert_training_session").get();

        sqlite3_bind_int(stmt, 1, network_id);
        sqlite3_bind_int(stmt, 2, epoch);
        sqlite3_bind_int(stmt, 3, test_correct);

        int rc = sqlite3_step(stmt);
        sqlite3_reset(stmt);

        if (rc != SQLITE_DONE) {
            rc = sqlite3_extended_errcode(m_db.get());
            return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
        }
        return {};
    }, m_pool
    );
}

asio::awaitable<DBResult<std::vector<int>>> Db::get_test_points(int network_id) {
    co_return co_await run_on_pool(
        [this, network_id]() -> std::expected<std::vector<int>, DBError> {
        auto stmt = m_stmts.at("get_training_sessions").get();
        sqlite3_bind_int(stmt, 1, network_id);

        std::vector<int> sessions;
        int rc;
        while (true) {
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_ROW) {
                break;
            }

            sessions.push_back(sqlite3_column_int(stmt, 0));
        }

        if (rc != SQLITE_DONE) {
            rc = sqlite3_extended_errcode(m_db.get());
            sqlite3_reset(stmt);
            return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
        }
        sqlite3_reset(stmt);
        return sessions;
    }, m_pool
    );
}

asio::awaitable<DBResult<void>> Db::insert_loss_point(int network_id, int step, double loss) {
    co_return co_await run_on_pool(
        [this, network_id, step, loss]() -> std::expected<void, DBError> {
        auto stmt = m_stmts.at("insert_loss_point").get();

        sqlite3_bind_int(stmt, 1, network_id);
        sqlite3_bind_int(stmt, 2, step);
        sqlite3_bind_double(stmt, 3, loss);

        int rc = sqlite3_step(stmt);
        sqlite3_reset(stmt);

        if (rc != SQLITE_DONE) {
            rc = sqlite3_extended_errcode(m_db.get());
            return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
        }
        return {};
    }, m_pool
    );
}

asio::awaitable<DBResult<std::vector<double>>> Db::get_loss_points(int network_id) {
    co_return co_await run_on_pool(
        [this, network_id]() -> std::expected<std::vector<double>, DBError> {
        auto stmt = m_stmts.at("get_loss_points").get();
        sqlite3_bind_int(stmt, 1, network_id);

        std::vector<double> points;
        int rc;
        while (true) {
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_ROW) {
                break;
            }

            points.push_back(sqlite3_column_double(stmt, 0));
        }

        if (rc != SQLITE_DONE) {
            rc = sqlite3_extended_errcode(m_db.get());
            sqlite3_reset(stmt);
            return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
        }
        sqlite3_reset(stmt);
        return points;
    }, m_pool
    );
}

Db::Statements Db::make_statements(sqlite3* db) {
    return Statements{
        .delete_network_by_id = { R"(DELETE FROM networks WHERE id = ?;)", db },
    };
}
