#pragma once

#include <sqlite3.h>
#include <string_view>
#include "asio/awaitable.hpp"
#include "asio/thread_pool.hpp"

struct AddNetwork {};

struct GetNetwork {};

struct GetNetworks {};

class Db {
public:
    Db(std::string_view db_file_path, std::string_view train_input_path,
       std::string_view train_output_path, std::string_view test_input_path,
       std::string_view test_output_path);

    ~Db();

    asio::awaitable<void> add_network(const AddNetwork&& network);
    asio::awaitable<GetNetwork> get_network_by_id(const int id);
    asio::awaitable<std::vector<GetNetworks>> get_networks();

private:
    bool check_data_exists();
    void add_test_train_data(
        std::string_view train_input_path, std::string_view train_output_path,
        std::string_view test_input_path, std::string_view test_output_path
    );

    void create_tables();

    sqlite3* m_db;
    asio::thread_pool m_pool;
};
