#include "db.h"
#include <sqlite3.h>
#include <asio.hpp>
#include <expected>

DB::DB(std::string_view path) : m_pool(1) {
    auto res = sqlite3_open(path.data(), &m_db);
    if (res != SQLITE_OK) {
        throw std::runtime_error("Failed to open database");
    }

    setup_db();
    setup_statements();
}

DB::~DB() {
    for (auto& [key, stmt] : m_statements) {
        sqlite3_finalize(stmt);
    }
    sqlite3_close(m_db);
}

asio::awaitable<DBResult<void>> DB::add_network(AddNetwork&& network) {
    co_return co_await run<void>(
        [this, network = std::move(network)]() -> std::expected<void, DBError> {
            auto stmt = m_statements["add_network"];

            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);

            sqlite3_bind_text(stmt, 1, network.name.c_str(), -1, SQLITE_STATIC);

            int shape_size = static_cast<int>(network.shape.size());
            sqlite3_bind_blob(
                stmt, 2, network.shape.data(), shape_size * sizeof(int), SQLITE_STATIC
            );

            int weights_size = static_cast<int>(network.weights.size());
            sqlite3_bind_blob(
                stmt, 3, network.weights.data(), weights_size * sizeof(double), SQLITE_STATIC
            );

            int biases_size = static_cast<int>(network.biases.size());
            sqlite3_bind_blob(
                stmt, 4, network.biases.data(), biases_size * sizeof(double), SQLITE_STATIC
            );

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                return std::unexpected(
                    DBError{ sqlite3_extended_errcode(m_db), sqlite3_errmsg(m_db) }
                );
            }
            return {};
        }
    );
}

asio::awaitable<DBResult<bool>> DB::delete_network(std::string_view name) {
    co_return co_await run<bool>([this, name]() -> std::expected<bool, DBError> {
        auto stmt = m_statements["delete_network"];

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);

        sqlite3_bind_text(stmt, 1, name.data(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            return std::unexpected(DBError{ sqlite3_errcode(m_db), sqlite3_errmsg(m_db) });
        }

        int changes = sqlite3_changes(m_db);
        return changes > 0;
    });
}

asio::awaitable<DBResult<std::vector<GetNetworks>>> DB::get_networks() {
    co_return co_await run<std::vector<GetNetworks>>(
        [this]() -> std::expected<std::vector<GetNetworks>, DBError> {
            auto stmt = m_statements["get_networks"];

            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);

            std::vector<GetNetworks> networks;

            while (true) {
                int rc = sqlite3_step(stmt);
                if (rc == SQLITE_ROW) {
                    GetNetworks network;

                    network.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

                    const void* shape_blob = sqlite3_column_blob(stmt, 1);
                    int shape_size = sqlite3_column_bytes(stmt, 1) / sizeof(int);
                    network.shape.resize(shape_size);
                    std::memcpy(network.shape.data(), shape_blob, shape_size * sizeof(int));

                    network.training_iterations = sqlite3_column_int(stmt, 2);
                    network.cost = sqlite3_column_double(stmt, 3);

                    networks.push_back(std::move(network));
                } else if (rc == SQLITE_DONE) {
                    break;
                } else {
                    return std::unexpected(DBError{ sqlite3_errcode(m_db), sqlite3_errmsg(m_db) });
                }
            }

            return networks;
        }
    );
}

asio::awaitable<DBResult<std::optional<GetNetwork>>> DB::get_network(std::string_view name) {
    co_return co_await run<std::optional<GetNetwork>>(
        [this, name]() -> std::expected<std::optional<GetNetwork>, DBError> {
            auto stmt = m_statements["get_network"];

            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);

            sqlite3_bind_text(stmt, 1, name.data(), -1, SQLITE_STATIC);

            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                GetNetwork network;

                network.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

                const void* shape_blob = sqlite3_column_blob(stmt, 1);
                int shape_size = sqlite3_column_bytes(stmt, 1) / sizeof(int);
                network.shape.resize(shape_size);
                std::memcpy(network.shape.data(), shape_blob, shape_size * sizeof(int));

                network.training_iterations = sqlite3_column_int(stmt, 2);
                network.cost = sqlite3_column_double(stmt, 3);

                const void* weights_blob = sqlite3_column_blob(stmt, 4);
                int weights_size = sqlite3_column_bytes(stmt, 4) / sizeof(double);
                network.weights.resize(weights_size);
                std::memcpy(network.weights.data(), weights_blob, weights_size * sizeof(double));

                const void* biases_blob = sqlite3_column_blob(stmt, 5);
                int biases_size = sqlite3_column_bytes(stmt, 5) / sizeof(double);
                network.biases.resize(biases_size);
                std::memcpy(network.biases.data(), biases_blob, biases_size * sizeof(double));

                return network;
            } else if (rc == SQLITE_DONE) {
                return std::nullopt;
            } else {
                return std::unexpected(DBError{ sqlite3_errcode(m_db), sqlite3_errmsg(m_db) });
            }
        }
    );
}

void DB::setup_db() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS networks (
            name TEXT PRIMARY KEY,
            shape BLOB NOT NULL,
            training_iterations INTEGER NOT NULL,
            cost REAL NOT NULL,

            weights BLOB NOT NULL,
            biases BLOB NOT NULL
        );
    )";

    auto res = sqlite3_exec(m_db, sql, nullptr, nullptr, nullptr);
    if (res != SQLITE_OK) {
        throw std::runtime_error("Failed to initialize database");
    }
}

void DB::setup_statements() {
    const char* sql_add_network = R"(
        INSERT INTO networks (name, shape, training_iterations, cost, weights, biases)
        VALUES (?, ?, 0, 0.0, ?, ?);
    )";

    sqlite3_stmt* stmt;
    auto res = sqlite3_prepare_v2(m_db, sql_add_network, -1, &stmt, nullptr);
    if (res != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }
    m_statements["add_network"] = stmt;

    const char* sql_delete_network = R"(
        DELETE FROM networks WHERE name = ?;
    )";

    res = sqlite3_prepare_v2(m_db, sql_delete_network, -1, &stmt, nullptr);
    if (res != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }
    m_statements["delete_network"] = stmt;

    const char* sql_get_networks = R"(
        SELECT name, shape, training_iterations, cost FROM networks;
    )";

    res = sqlite3_prepare_v2(m_db, sql_get_networks, -1, &stmt, nullptr);
    if (res != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }
    m_statements["get_networks"] = stmt;

    const char* sql_get_network = R"(
        SELECT name, shape, training_iterations, cost, weights, biases
        FROM networks WHERE name = ?;
    )";

    res = sqlite3_prepare_v2(m_db, sql_get_network, -1, &stmt, nullptr);
    if (res != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement");
    }
    m_statements["get_network"] = stmt;
}

template<typename T>
asio::awaitable<std::expected<T, DBError>>
    DB::run(std::function<std::expected<T, DBError>()> operation) {
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
        init, asio::use_awaitable, std::ref(m_pool), std::move(operation)
    );
}
