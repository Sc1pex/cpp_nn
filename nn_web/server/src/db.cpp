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
    auto statements = make_statements(db.get());

    return Db(std::move(db), std::move(statements));
}

Db::Db(std::unique_ptr<sqlite3, decltype(&sqlite3_close_v2)> db, Statements statements)
: m_db(std::move(db)), m_pool(1), m_statements(std::move(statements)) {
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
        nlohmann::json layer_sizes_json = network.layer_sizes;
        std::string layer_sizes_str = layer_sizes_json.dump();

        nlohmann::json activations_json = network.activations;
        std::string activations_str = activations_json.dump();

        auto ex =
            m_statements.add_network.binder()
                .bind_text(network.name.c_str())
                .bind_text(layer_sizes_str.c_str())
                .bind_blob(
                    network.weights.data(),
                    static_cast<int>(network.weights.size() * sizeof(double))
                )
                .bind_blob(
                    network.biases.data(), static_cast<int>(network.biases.size() * sizeof(double))
                )
                .bind_text(activations_str.c_str())
                .bind_text(network.loss.c_str())
                .execute();

        auto res = ex.next();
        if (!res) {
            return std::unexpected(res.error());
        }
        return {};
    }, m_pool
    );
}

asio::awaitable<DBResult<std::optional<NetworkInfo>>> Db::get_network_by_id(int id) {
    co_return co_await run_on_pool(
        [this, id]() -> std::expected<std::optional<NetworkInfo>, DBError> {
        auto ex = m_statements.get_network_by_id.binder().bind_int(id).execute();

        auto res = ex.next_row();
        if (!res) {
            return std::unexpected(res.error());
        }
        if (!*res) {
            return std::nullopt;
        }

        auto row = **res;

        NetworkInfo network;
        network.id = row.next_int();
        network.name = row.next_text();
        network.created_at = row.next_text();

        std::string layer_sizes_str = row.next_text();
        nlohmann::json layer_sizes_json = nlohmann::json::parse(layer_sizes_str);
        network.layer_sizes = layer_sizes_json.get<std::vector<int>>();

        network.correct_predictions = row.next_int();
        network.cost = row.next_double();
        network.training_epochs = row.next_int();

        std::string activations_str = row.next_text();
        nlohmann::json activations_json = nlohmann::json::parse(activations_str);
        network.activations = activations_json.get<std::vector<std::string>>();

        network.loss = row.next_text();

        return network;
    }, m_pool
    );
}

asio::awaitable<DBResult<std::optional<NetworkFull>>> Db::get_full_network_by_id(int id) {
    co_return co_await run_on_pool(
        [this, id]() -> std::expected<std::optional<NetworkFull>, DBError> {
        auto ex = m_statements.get_full_network_by_id.binder().bind_int(id).execute();

        auto res = ex.next_row();
        if (!res) {
            return std::unexpected(res.error());
        }
        if (!*res) {
            return std::nullopt;
        }

        auto row = **res;

        NetworkFull network;
        network.id = row.next_int();
        network.name = row.next_text();
        network.created_at = row.next_text();

        std::string layer_sizes_str = row.next_text();
        nlohmann::json layer_sizes_json = nlohmann::json::parse(layer_sizes_str);
        network.layer_sizes = layer_sizes_json.get<std::vector<int>>();

        network.correct_predictions = row.next_int();
        network.cost = row.next_double();
        network.training_epochs = row.next_int();

        auto weights = row.next_blob();
        network.weights.resize(weights.bytes / sizeof(double));
        std::memcpy(network.weights.data(), weights.data, static_cast<size_t>(weights.bytes));

        auto biases = row.next_blob();
        network.biases.resize(biases.bytes / sizeof(double));
        std::memcpy(network.biases.data(), biases.data, static_cast<size_t>(biases.bytes));

        std::string activations_str = row.next_text();
        nlohmann::json activations_json = nlohmann::json::parse(activations_str);
        network.activations = activations_json.get<std::vector<std::string>>();

        network.loss = row.next_text();

        network.loss_points = row.next_int();

        return network;
    }, m_pool
    );
}

asio::awaitable<DBResult<std::vector<NetworkInfo>>> Db::get_networks() {
    co_return co_await run_on_pool([this]() -> std::expected<std::vector<NetworkInfo>, DBError> {
        auto ex = m_statements.get_networks.binder().execute();

        std::vector<NetworkInfo> networks;

        while (true) {
            auto res = ex.next_row();
            if (!res) {
                return std::unexpected(res.error());
            }
            if (!*res) {
                break;
            }
            auto row = **res;

            NetworkInfo network;
            network.id = row.next_int();
            network.name = row.next_text();
            network.created_at = row.next_text();

            std::string layer_sizes_str = row.next_text();
            nlohmann::json layer_sizes_json = nlohmann::json::parse(layer_sizes_str);
            network.layer_sizes = layer_sizes_json.get<std::vector<int>>();

            network.correct_predictions = row.next_int();
            network.cost = row.next_double();
            network.training_epochs = row.next_int();

            std::string activations_str = row.next_text();
            nlohmann::json activations_json = nlohmann::json::parse(activations_str);
            network.activations = activations_json.get<std::vector<std::string>>();

            network.loss = row.next_text();

            networks.push_back(network);
        }

        return networks;
    }, m_pool);
}

asio::awaitable<DBResult<bool>> Db::delete_network_by_id(int id) {
    co_return co_await run_on_pool([this, id]() -> std::expected<bool, DBError> {
        auto ex = m_statements.delete_network_by_id.binder().bind_int(id).execute();

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
        auto ex = m_statements.update_network_params.binder()
                      .bind_blob(weights.data(), static_cast<int>(weights.size() * sizeof(double)))
                      .bind_blob(biases.data(), static_cast<int>(biases.size() * sizeof(double)))
                      .bind_int(epochs_added)
                      .bind_int(loss_points_added)
                      .bind_int(id)
                      .execute();

        auto res = ex.next();
        if (!res) {
            return std::unexpected(res.error());
        }
        return {};
    },
        m_pool
    );
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
        auto ex = m_statements.insert_training_session.binder()
                      .bind_int(network_id)
                      .bind_int(epoch)
                      .bind_int(test_correct)
                      .execute();

        auto res = ex.next();
        if (!res) {
            return std::unexpected(res.error());
        }
        return {};
    }, m_pool
    );
}

asio::awaitable<DBResult<std::vector<int>>> Db::get_test_points(int network_id) {
    co_return co_await run_on_pool(
        [this, network_id]() -> std::expected<std::vector<int>, DBError> {
        auto ex = m_statements.get_training_sessions.binder().bind_int(network_id).execute();

        std::vector<int> sessions;
        while (true) {
            auto res = ex.next_row();
            if (!res) {
                return std::unexpected(res.error());
            }
            if (!*res) {
                break;
            }
            auto row = **res;
            sessions.push_back(row.next_int());
        }
        return sessions;
    }, m_pool
    );
}

asio::awaitable<DBResult<void>> Db::insert_loss_point(int network_id, int step, double loss) {
    co_return co_await run_on_pool(
        [this, network_id, step, loss]() -> std::expected<void, DBError> {
        auto ex = m_statements.insert_loss_point.binder()
                      .bind_int(network_id)
                      .bind_int(step)
                      .bind_double(loss)
                      .execute();

        auto res = ex.next();
        if (!res) {
            return std::unexpected(res.error());
        }
        return {};
    }, m_pool
    );
}

asio::awaitable<DBResult<std::vector<double>>> Db::get_loss_points(int network_id) {
    co_return co_await run_on_pool(
        [this, network_id]() -> std::expected<std::vector<double>, DBError> {
        auto ex = m_statements.get_loss_points.binder().bind_int(network_id).execute();

        std::vector<double> points;
        while (true) {
            auto res = ex.next_row();

            if (!res) {
                return std::unexpected(res.error());
            } else if (!*res) {
                break;
            }
            auto row = **res;
            points.push_back(row.next_double());
        }

        return points;
    }, m_pool
    );
}

Db::Statements Db::make_statements(sqlite3* db) {
    return Statements {
        .add_network = {
            R"(INSERT INTO networks (name, layer_sizes, weights, biases, activations, loss)
               VALUES (?, ?, ?, ?, ?, ?);)",
            db },

        .get_network_by_id = {
            R"(SELECT id, name, created_at, layer_sizes, correct_predictions, cost,
                      training_epochs, activations, loss
               FROM networks WHERE id = ?;)",
            db },

        .get_full_network_by_id = {
            R"(SELECT id, name, created_at, layer_sizes, correct_predictions, cost,
                      training_epochs, weights, biases, activations, loss, loss_points
               FROM networks WHERE id = ?;)",
            db },

        .get_networks = {
            R"(SELECT id, name, created_at, layer_sizes, correct_predictions, cost,
                      training_epochs, activations, loss
               FROM networks;)",
            db },

        .update_network_params = {
            R"(UPDATE networks
               SET weights = ?, biases = ?,
                   training_epochs = training_epochs + ?,
                   loss_points = loss_points + ?
               WHERE id = ?;)",
            db },

        .delete_network_by_id = {
            R"(DELETE FROM networks WHERE id = ?;)",
            db },

        .insert_training_session = {
            R"(INSERT INTO training_sessions (network_id, epoch, test_correct)
               VALUES (?, ?, ?);)",
            db },

        .get_training_sessions = {
            R"(SELECT test_correct
               FROM training_sessions WHERE network_id = ? ORDER BY epoch ASC;)",
            db },

        .insert_loss_point = {
            R"(INSERT INTO training_loss_points (network_id, step, loss)
               VALUES (?, ?, ?);)",
            db },

        .get_loss_points = {
            R"(SELECT loss
               FROM training_loss_points WHERE network_id = ? ORDER BY step ASC;)",
            db },
    };
}
