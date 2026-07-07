#pragma once

#include <format>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>
#include "db_error.hpp"
#include "sqlite3.h"

template<typename T>
concept BindArg = std::is_same_v<int, T>;

template<typename T>
concept ColumnArg = std::is_same_v<int, T>;

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

    using NextReturnType = std::conditional_t<
        sizeof...(Args) == 0, std::expected<void, DBError>,
        std::expected<std::optional<std::tuple<Args...>>, DBError>>;

    NextReturnType next() {
        int rc = sqlite3_step(&m_guard.s);

        if constexpr (sizeof...(Args) == 0) {
            if (rc != SQLITE_DONE) {
                return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
            } else {
                return {};
            }
        } else {
            if (rc == SQLITE_ROW) {
                return std::optional(get_row(std::index_sequence_for<Args...>{}));
            } else if (rc == SQLITE_DONE) {
                return std::nullopt;
            } else {
                return std::unexpected(DBError{ rc, sqlite3_errstr(rc) });
            }
        }
    }

private:
    template<size_t... Is>
    std::tuple<Args...> get_row(std::index_sequence<Is...>) const {
        return { column<Args>(Is)... };
    }

    template<ColumnArg Arg>
    Arg column(size_t idx) {
        if constexpr (std::is_same_v<Arg, int>) {
            return sqlite3_column_int(&m_guard.s, idx);
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
        }
    }

    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> m_stmt;
};
