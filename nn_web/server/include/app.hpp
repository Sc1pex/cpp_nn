#pragma once

#include <httc/router.hpp>
#include "state.hpp"

class App {
public:
    App();

    void run();

private:
    void add_network_routes();
    void add_data_routes();

private:
    std::shared_ptr<httc::Router> m_router;
    std::shared_ptr<State> m_state;
};
