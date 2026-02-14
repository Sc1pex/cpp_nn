#pragma once

#include <httc/router.hpp>
#include "state.hpp"

class App {
public:
    App(std::optional<std::string_view> static_assets_path);

    void run();

private:
    void add_network_routes();
    void add_data_routes();
    void add_static_routes();

private:
    std::optional<std::string_view> m_static_assets_path;

    std::shared_ptr<httc::Router> m_router;
    std::shared_ptr<State> m_state;
};
