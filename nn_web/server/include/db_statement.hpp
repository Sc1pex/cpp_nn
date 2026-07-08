#pragma once

#include <format>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>
#include "db_error.hpp"
#include "sqlite3.h"

template<typename T>
concept BindArg = std::is_same_v<int, T> or std::is_same_v<double, T>;

template<typename T>
concept ColumnArg = std::is_same_v<int, T> or std::is_same_v<double, T>;

struct ResetGuard {
    ResetGuard(sqlite3_stmt& s) : s(s) {
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

template<ColumnArg... Args>
class Extractor {
public:
    Extractor(sqlite3_stmt& stmt) : m_guard(stmt) {
    }

    std::expected<void, DBError> next()
        requires(sizeof...(Args) == 0)
    {
        int rc = sqlite3_step(&m_guard.s);
        if (rc != SQLITE_DONE) {
            return std::unexpected(get_db_error());
        }
        return {};
    }

    std::expected<std::optional<std::tuple<Args...>>, DBError> next()
        requires(sizeof...(Args) > 0)
    {
        int rc = sqlite3_step(&m_guard.s);
        if (rc == SQLITE_ROW) {
            return std::optional(get_row(std::index_sequence_for<Args...>{}));
        } else if (rc == SQLITE_DONE) {
            return std::nullopt;
        } else {
            return std::unexpected(get_db_error());
        }
    }

    template<typename Row>
        requires(sizeof...(Args) > 0)
    std::expected<std::optional<Row>, DBError> next_as() {
        auto result = next();
        if (!result)
            return std::unexpected(result.error());
        if (!*result)
            return std::nullopt;
        return std::apply([](auto&&... vals) {
            return Row{ std::forward<decltype(vals)>(vals)... };
        }, std::move(**result));
    }

private:
    DBError get_db_error() {
        sqlite3* db = sqlite3_db_handle(&m_guard.s);
        int rc = sqlite3_extended_errcode(db);
        return DBError{ rc, sqlite3_errmsg(db) };
    }

    template<size_t... Is>
    std::tuple<Args...> get_row(std::index_sequence<Is...>) {
        return { column<Args>(Is)... };
    }

    template<ColumnArg Arg>
    Arg column(size_t idx) {
        if constexpr (std::is_same_v<Arg, int>) {
            return sqlite3_column_int(&m_guard.s, idx);
        } else if constexpr (std::is_same_v<Arg, double>) {
            return sqlite3_column_double(&m_guard.s, idx);
        } else {
            static_assert(false, "Unsupported column type");
        }
    }

    ResetGuard m_guard;
};

template<BindArg... Args>
struct BindList;

template<ColumnArg... Args>
struct ColumnList;

template<typename BindListType, typename ColumnListType>
class DbStatement;

template<BindArg... Binds, ColumnArg... Cols>
class DbStatement<BindList<Binds...>, ColumnList<Cols...>> {
public:
    DbStatement(const char* stmt, sqlite3* db) : m_stmt(nullptr, sqlite3_finalize) {
        sqlite3_stmt* s;
        int rc = sqlite3_prepare_v2(db, stmt, -1, &s, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errstr(rc));
        }
        m_stmt.reset(s);
    }

    Extractor<Cols...> bind(Binds... binds) {
        if constexpr (sizeof...(Binds) > 0) {
            size_t idx = 1;
            (bind(idx++, std::forward<Binds>(binds)), ...);
        }
        return Extractor<Cols...>(*m_stmt);
    }

private:
    template<BindArg Arg>
    void bind(size_t idx, Arg a) {
        if constexpr (std::is_same_v<Arg, int>) {
            sqlite3_bind_int(m_stmt.get(), idx, a);
        } else if constexpr (std::is_same_v<Arg, double>) {
            sqlite3_bind_double(m_stmt.get(), idx, a);
        } else {
            static_assert(false, "Unsupported bind type");
        }
    }

    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> m_stmt;
};
