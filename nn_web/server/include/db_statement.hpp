#pragma once

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include "db_error.hpp"
#include "sqlite3.h"

struct ResetGuard {
    explicit ResetGuard(sqlite3_stmt& s) : s(s) {
    }
    ~ResetGuard() {
        sqlite3_reset(&s);
    }
    ResetGuard(const ResetGuard&) = delete;
    ResetGuard& operator=(const ResetGuard&) = delete;
    ResetGuard(ResetGuard&&) = delete;
    ResetGuard& operator=(ResetGuard&&) = delete;

    sqlite3_stmt& s;
};

struct Blob {
    const void* data;
    int bytes;
};

// Must not be used after the next call to Extractor::next_row() or after the
class Row {
public:
    explicit Row(sqlite3_stmt& stmt) : m_stmt(stmt) {
    }

    [[nodiscard]] int next_int();
    [[nodiscard]] double next_double();
    [[nodiscard]] std::string next_text();
    [[nodiscard]] Blob next_blob();

private:
    sqlite3_stmt& m_stmt;
    int m_col_idx = 0;
};

class Extractor {
public:
    explicit Extractor(sqlite3_stmt& stmt) : m_guard(stmt) {
    }

    // Steps a non-row statement (INSERT, UPDATE, DELETE).
    // Returns an error if the result is anything other than SQLITE_DONE.
    [[nodiscard]] std::expected<void, DBError> next();

    // Steps the statement. Returns the next Row if one is available,
    // nullopt when done, or an error on failure.
    [[nodiscard]] std::expected<std::optional<Row>, DBError> next_row();

private:
    DBError get_db_error();

    ResetGuard m_guard;
};

class Binder {
public:
    explicit Binder(sqlite3_stmt& stmt) : m_stmt(stmt) {
    }

    [[nodiscard]] Binder&& bind_int(int value);
    [[nodiscard]] Binder&& bind_double(double value);
    [[nodiscard]] Binder&& bind_text(const char* text);
    [[nodiscard]] Binder&& bind_blob(const void* data, int bytes);

    [[nodiscard]] Extractor execute() &&;

private:
    sqlite3_stmt& m_stmt;
    int m_bind_idx = 1;
};

class DbStatement {
public:
    DbStatement(const char* sql, sqlite3* db);

    [[nodiscard]] Binder binder();

private:
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> m_stmt;
};
