#include "db.h"
#include <spdlog/spdlog.h>
#include "idx.h"
#include "sqlite3.h"

Db::Db(
    std::string_view db_file_path, std::string_view train_input_path,
    std::string_view train_output_path, std::string_view test_input_path,
    std::string_view test_output_path
) {
    spdlog::info("Opening database at {}", db_file_path);
    if (sqlite3_open(db_file_path.data(), &m_db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database");
    }

    create_tables();

    if (!check_data_exists()) {
        spdlog::info("Adding training and test data to database");
        add_test_train_data(train_input_path, train_output_path, test_input_path, test_output_path);
    }
}

Db::~Db() {
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
        name TEXT NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        
        layer_sizes TEXT NOT NULL,  -- JSON: [784, 128, 64, 10]
        
        accuracy REAL,
        training_epochs INTEGER,
        
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
    auto train_inputs_result = idx3_read_file(std::string(train_input_path)).value();
    auto train_outputs_result = idx1_read_file(std::string(train_output_path)).value();
    spdlog::info(
        "Loaded {} training inputs and {} training outputs", train_inputs_result.size(),
        train_outputs_result.size()
    );

    auto test_inputs_result = idx3_read_file(std::string(test_input_path)).value();
    auto test_outputs_result = idx1_read_file(std::string(test_output_path)).value();
    spdlog::info(
        "Loaded {} test inputs and {} test outputs", test_inputs_result.size(),
        test_outputs_result.size()
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
                stmt, 1, input_vector.data(), static_cast<int>(input_vector.size()), SQLITE_STATIC
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
        "INSERT INTO train_data (input, output) VALUES (?, ?);", train_inputs_result,
        train_outputs_result
    );
    insert_data(
        "INSERT INTO test_data (input, output) VALUES (?, ?);", test_inputs_result,
        test_outputs_result
    );

    if (sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to commit transaction for test data insertion");
    }

    spdlog::info("Successfully added training and test data to database");
}
