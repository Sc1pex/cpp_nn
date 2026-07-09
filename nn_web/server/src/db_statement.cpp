#include "db_statement.hpp"

int Row::next_int() {
    return sqlite3_column_int(&m_stmt, m_col_idx++);
}

double Row::next_double() {
    return sqlite3_column_double(&m_stmt, m_col_idx++);
}

std::string Row::next_text() {
    const auto* text = reinterpret_cast<const char*>(sqlite3_column_text(&m_stmt, m_col_idx++));
    if (text == nullptr) {
        return {};
    }
    return std::string{ text };
}

Blob Row::next_blob() {
    const void* data = sqlite3_column_blob(&m_stmt, m_col_idx);
    int bytes = sqlite3_column_bytes(&m_stmt, m_col_idx);
    m_col_idx++;
    return Blob{ data, bytes };
}

std::expected<std::optional<Row>, DBError> Extractor::next_row() {
    int rc = sqlite3_step(&m_guard.s);
    if (rc == SQLITE_ROW) {
        return std::optional<Row>{ Row{ m_guard.s } };
    } else if (rc == SQLITE_DONE) {
        return std::nullopt;
    } else {
        return std::unexpected(get_db_error());
    }
}

std::expected<void, DBError> Extractor::next() {
    int rc = sqlite3_step(&m_guard.s);
    if (rc == SQLITE_DONE) {
        return {};
    } else {
        return std::unexpected(get_db_error());
    }
}

DBError Extractor::get_db_error() {
    sqlite3* db = sqlite3_db_handle(&m_guard.s);
    int rc = sqlite3_extended_errcode(db);
    return DBError{ rc, sqlite3_errmsg(db) };
}

void check_bind_rc(int rc) {
    if (rc == SQLITE_RANGE) {
        throw std::logic_error("bind: index out of range");
    }
}

Binder&& Binder::bind_int(int value) {
    int rc = sqlite3_bind_int(&m_stmt, m_bind_idx++, value);
    check_bind_rc(rc);
    return std::move(*this);
}

Binder&& Binder::bind_double(double value) {
    int rc = sqlite3_bind_double(&m_stmt, m_bind_idx++, value);
    check_bind_rc(rc);
    return std::move(*this);
}

Binder&& Binder::bind_text(const char* text) {
    int rc = sqlite3_bind_text(&m_stmt, m_bind_idx++, text, -1, SQLITE_STATIC);
    check_bind_rc(rc);
    return std::move(*this);
}

Binder&& Binder::bind_blob(const void* data, int bytes) {
    int rc = sqlite3_bind_blob(&m_stmt, m_bind_idx++, data, bytes, SQLITE_STATIC);
    check_bind_rc(rc);
    return std::move(*this);
}

Extractor Binder::execute() && {
    return Extractor{ m_stmt };
}

DbStatement::DbStatement(const char* sql, sqlite3* db) : m_stmt(nullptr, sqlite3_finalize) {
    sqlite3_stmt* s;
    int rc = sqlite3_prepare_v2(db, sql, -1, &s, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errstr(rc));
    }
    m_stmt.reset(s);
}

Binder DbStatement::binder() {
    return Binder{ *m_stmt };
}
