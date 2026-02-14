#include "db.hpp"
#include <spdlog/spdlog.h>
#include <asio.hpp>
#include <expected>
#include <functional>
#include "idx.hpp"
#include "nlohmann/json.hpp"

Db::Db(
    std::string_view db_file_path, std::string_view train_input_path,
    std::string_view train_output_path, std::string_view test_input_path,
    std::string_view test_output_path
)
: m_pool(1) {
    spdlog::info("Opening database at {}", db_file_path);
    if (sqlite3_open(db_file_path.data(), &m_db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database");
    }

    create_tables();

    if (!check_data_exists()) {
        spdlog::info("Adding training and test data to database");
        add_test_train_data(train_input_path, train_output_path, test_input_path, test_output_path);
    }

    create_statements();
}

Db::~Db() {
    m_pool.join();

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
        activations TEXT NOT NULL   -- JSON: ["relu", "relu", "sigmoid"]
    );)";
    create_table(create_networks_table_sql);

    const char* create_train_data_table_sql = R"(
    CREATE TABLE IF NOT EXISTS train_data (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        input BLOB NOT NULL,   -- serialized vector<uint8_t>
        output INTEGER NOT NULL
    );)";
    create_table(create_train_data_table_sql);

    const char* create_test_data_table_sql = R"(
    CREATE TABLE IF NOT EXISTS test_data (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        input BLOB NOT NULL,   -- serialized vector<uint8_t>
        output INTEGER NOT NULL
    );)";
    create_table(create_test_data_table_sql);

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

bool Db::check_data_exists() {
    const char* count_train_sql = "SELECT COUNT(*) FROM train_data;";

    int train_count = 0;
    char* err_msg = nullptr;
    int result = sqlite3_exec(
        m_db, count_train_sql,
        [](void* count_ptr, int, char** argv, char**) -> int {
            int* count = static_cast<int*>(count_ptr);
            *count = std::stoi(argv[0]);
            return 0;
        },
        &train_count, &err_msg
    );
    if (result != SQLITE_OK) {
        std::string error = "Failed to count training data: ";
        error += err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(error);
    }

    if (train_count != 60000) {
        return false;
    }

    const char* count_test_sql = "SELECT COUNT(*) FROM test_data;";
    int test_count = 0;
    result = sqlite3_exec(
        m_db, count_test_sql,
        [](void* count_ptr, int, char** argv, char**) -> int {
            int* count = static_cast<int*>(count_ptr);
            *count = std::stoi(argv[0]);
            return 0;
        },
        &test_count, &err_msg
    );
    if (result != SQLITE_OK) {
        std::string error = "Failed to count test data: ";
        error += err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(error);
    }

    return test_count == 10000;
}

void Db::add_test_train_data(
    std::string_view train_input_path, std::string_view train_output_path,
    std::string_view test_input_path, std::string_view test_output_path
) {
    try {
        auto train_inputs = idx3_read_file(std::string(train_input_path)).value();
        auto train_outputs = idx1_read_file(std::string(train_output_path)).value();
        spdlog::info(
            "Loaded {} training inputs and {} training outputs", train_inputs.size(),
            train_outputs.size()
        );

        auto test_inputs = idx3_read_file(std::string(test_input_path)).value();
        auto test_outputs = idx1_read_file(std::string(test_output_path)).value();
        spdlog::info(
            "Loaded {} test inputs and {} test outputs", test_inputs.size(), test_outputs.size()
        );

        auto insert_data = [this](const char* sql, const auto& inputs, const auto& outputs) {
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
                sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
                throw std::runtime_error("Failed to prepare insert statement");
            }

            for (size_t i = 0; i < inputs.size(); i++) {
                const auto& input_matrix = inputs[i];
                std::vector<uint8_t> input_vector(input_matrix.size());
                for (int r = 0; r < input_matrix.rows(); r++) {
                    for (int c = 0; c < input_matrix.cols(); c++) {
                        input_vector[r * input_matrix.cols() + c] =
                            static_cast<uint8_t>(input_matrix(r, c));
                    }
                }

                sqlite3_bind_blob(
                    stmt, 1, input_vector.data(), static_cast<int>(input_vector.size()),
                    SQLITE_STATIC
                );
                sqlite3_bind_int(stmt, 2, static_cast<int>(outputs[i]));

                if (sqlite3_step(stmt) != SQLITE_DONE) {
                    sqlite3_finalize(stmt);
                    sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
                    throw std::runtime_error("Failed to insert training data");
                }
                sqlite3_reset(stmt);
            }
            sqlite3_finalize(stmt);
        };

        if (sqlite3_exec(m_db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to begin transaction for data insertion");
        }

        insert_data(
            "INSERT INTO train_data (input, output) VALUES (?, ?);", train_inputs, train_outputs
        );
        insert_data(
            "INSERT INTO test_data (input, output) VALUES (?, ?);", test_inputs, test_outputs
        );

        if (sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to commit transaction for test data insertion");
        }

        spdlog::info("Successfully added training and test data to database");
    } catch (std::bad_expected_access<std::string>& e) {
        spdlog::error("Failed to load IDX data: {}", e.error());
        throw std::runtime_error("Failed to load IDX data: " + e.error());
    }
}

template<typename T>
asio::awaitable<std::expected<T, DBError>>
    run_on_pool(std::function<std::expected<T, DBError>()> operation, asio::thread_pool& pool) {
    auto init = [](auto completion_handler, asio::thread_pool& pool, auto operation) {
        asio::post(
            pool, [completion_handler = std::move(completion_handler),
                   operation = std::move(operation)]() mutable {
                auto result = operation();
                std::move(completion_handler)(result);
            }
        );
    };

    co_return co_await async_initiate<
        decltype(asio::use_awaitable), void(std::expected<T, DBError>)>(
        init, asio::use_awaitable, std::ref(pool), std::move(operation)
    );
}

asio::awaitable<DBResult<void>> Db::add_network(const AddNetwork&& network) {
    co_return co_await run_on_pool<void>(
        [this, network]() -> std::expected<void, DBError> {
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

            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                rc = sqlite3_extended_errcode(m_db);

                sqlite3_reset(stmt);
                return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
            }

            sqlite3_reset(stmt);
            return {};
        },
        m_pool
    );
}

asio::awaitable<DBResult<std::optional<NetworkInfo>>> Db::get_network_by_id(const int id) {
    co_return co_await run_on_pool<std::optional<NetworkInfo>>(
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

            std::string layer_sizes_str =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            nlohmann::json layer_sizes_json = nlohmann::json::parse(layer_sizes_str);
            network.layer_sizes = layer_sizes_json.get<std::vector<int>>();

            network.correct_predictions = sqlite3_column_int(stmt, 4);
            network.training_epochs = sqlite3_column_int(stmt, 5);
            network.cost = sqlite3_column_double(stmt, 6);

            std::string activations_str =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            nlohmann::json activations_json = nlohmann::json::parse(activations_str);
            network.activations = activations_json.get<std::vector<std::string>>();

            sqlite3_reset(stmt);
            return network;
        },
        m_pool
    );
}

asio::awaitable<DBResult<std::optional<NetworkFull>>> Db::get_full_network_by_id(const int id) {
    co_return co_await run_on_pool<std::optional<NetworkFull>>(
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

            std::string layer_sizes_str =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            nlohmann::json layer_sizes_json = nlohmann::json::parse(layer_sizes_str);
            network.layer_sizes = layer_sizes_json.get<std::vector<int>>();

            network.correct_predictions = sqlite3_column_int(stmt, 4);
            network.training_epochs = sqlite3_column_int(stmt, 5);
            network.cost = sqlite3_column_double(stmt, 6);

            const void* weights_blob = sqlite3_column_blob(stmt, 7);
            int weights_size = sqlite3_column_bytes(stmt, 7);
            network.weights.resize(weights_size / sizeof(double));
            std::memcpy(network.weights.data(), weights_blob, static_cast<size_t>(weights_size));

            const void* biases_blob = sqlite3_column_blob(stmt, 8);
            int biases_size = sqlite3_column_bytes(stmt, 8);
            network.biases.resize(biases_size / sizeof(double));
            std::memcpy(network.biases.data(), biases_blob, static_cast<size_t>(biases_size));

            std::string activations_str =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
            nlohmann::json activations_json = nlohmann::json::parse(activations_str);
            network.activations = activations_json.get<std::vector<std::string>>();

            sqlite3_reset(stmt);
            return network;
        },
        m_pool
    );
}

asio::awaitable<DBResult<std::vector<NetworkListItem>>> Db::get_networks() {
    co_return co_await run_on_pool<std::vector<NetworkListItem>>(
        [this]() -> std::expected<std::vector<NetworkListItem>, DBError> {
            sqlite3_stmt* stmt = m_stmts["get_networks"];

            std::vector<NetworkListItem> networks;

            int rc;
            while (true) {
                rc = sqlite3_step(stmt);
                if (rc != SQLITE_ROW) {
                    break;
                }

                NetworkListItem network;
                network.id = sqlite3_column_int(stmt, 0);
                network.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                network.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

                std::string layer_sizes_str =
                    reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                nlohmann::json layer_sizes_json = nlohmann::json::parse(layer_sizes_str);
                network.layer_sizes = layer_sizes_json.get<std::vector<int>>();

                network.correct_predictions = sqlite3_column_int(stmt, 4);
                network.training_epochs = sqlite3_column_int(stmt, 5);
                network.cost = sqlite3_column_double(stmt, 6);

                networks.push_back(network);
            }

            if (rc != SQLITE_DONE) {
                rc = sqlite3_extended_errcode(m_db);
                sqlite3_reset(stmt);
                return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
            }

            sqlite3_reset(stmt);
            return networks;
        },
        m_pool
    );
}

asio::awaitable<DBResult<bool>> Db::delete_network_by_id(const int id) {
    co_return co_await run_on_pool<bool>(
        [this, id]() -> std::expected<bool, DBError> {
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
        },
        m_pool
    );
}

asio::awaitable<DBResult<std::optional<Sample>>> Db::get_train_sample_by_index(const int index) {
    co_return co_await run_on_pool<std::optional<Sample>>(
        [this, index]() -> std::expected<std::optional<Sample>, DBError> {
            sqlite3_stmt* stmt = m_stmts["get_train_sample_by_index"];

            sqlite3_bind_int(stmt, 1, index);

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

            Sample sample;

            const void* input_blob = sqlite3_column_blob(stmt, 0);
            int input_size = sqlite3_column_bytes(stmt, 0);
            sample.input.resize(input_size);
            std::memcpy(sample.input.data(), input_blob, input_size);

            sample.expected_output = sqlite3_column_int(stmt, 1);

            sqlite3_reset(stmt);
            return sample;
        },
        m_pool
    );
}

asio::awaitable<DBResult<std::optional<Sample>>> Db::get_test_sample_by_index(const int index) {
    co_return co_await run_on_pool<std::optional<Sample>>(
        [this, index]() -> std::expected<std::optional<Sample>, DBError> {
            sqlite3_stmt* stmt = m_stmts["get_test_sample_by_index"];

            sqlite3_bind_int(stmt, 1, index);

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

            Sample sample;

            const void* input_blob = sqlite3_column_blob(stmt, 0);
            int input_size = sqlite3_column_bytes(stmt, 0);
            sample.input.resize(input_size);
            std::memcpy(sample.input.data(), input_blob, input_size);

            sample.expected_output = sqlite3_column_int(stmt, 1);

            sqlite3_reset(stmt);
            return sample;
        },
        m_pool
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
        name, layer_sizes, weights, biases, activations
    ) VALUES (?, ?, ?, ?, ?);)";
    add_stmt(add_network_sql, "add_network");

    const char* get_network_by_id_sql = R"(
    SELECT id, name, created_at, layer_sizes, correct_predictions, cost, training_epochs, activations
    FROM networks WHERE id = ?;)";
    add_stmt(get_network_by_id_sql, "get_network_by_id");

    const char* get_full_network_by_id_sql = R"(
    SELECT id, name, created_at, layer_sizes, correct_predictions, cost, training_epochs, weights, biases, activations
    FROM networks WHERE id = ?;)";
    add_stmt(get_full_network_by_id_sql, "get_full_network_by_id");

    const char* get_networks_sql = R"(
    SELECT id, name, created_at, layer_sizes, correct_predictions, cost, training_epochs FROM networks;)";
    add_stmt(get_networks_sql, "get_networks");

    const char* delete_network_by_id_sql = R"(
    DELETE FROM networks WHERE id = ?;)";
    add_stmt(delete_network_by_id_sql, "delete_network_by_id");

    const char* get_train_sample_by_index_sql = R"(
    SELECT input, output FROM train_data WHERE id = ?;)";
    add_stmt(get_train_sample_by_index_sql, "get_train_sample_by_index");

    const char* get_test_sample_by_index_sql = R"(
    SELECT input, output FROM test_data WHERE id = ?;)";
    add_stmt(get_test_sample_by_index_sql, "get_test_sample_by_index");
}

void to_json(json& j, const NetworkFull& v) {
    j = json{
        { "id", v.id },
        { "name", v.name },
        { "created_at", v.created_at },
        { "layet_sizes", v.layer_sizes },
        { "correct_predictions", v.correct_predictions },
        { "training_epochs", v.training_epochs },
        { "cost", v.cost },
        { "weights", v.weights },
        { "biases", v.biases },
        { "activations", v.activations },
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
    };
}

void to_json(json& j, const NetworkListItem& v) {
    j = json{
        { "id", v.id },
        { "name", v.name },
        { "created_at", v.created_at },
        { "layer_sizes", v.layer_sizes },
        { "correct_predictions", v.correct_predictions },
        { "training_epochs", v.training_epochs },
        { "cost", v.cost },
    };
}

void to_json(json& j, const Sample& v) {
    j = json{
        { "input", v.input },
        { "expected_output", v.expected_output },
    };
}
