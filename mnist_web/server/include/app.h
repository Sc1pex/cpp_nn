#pragma once

#include <httc/router.h>
#include "state.h"

class App {
public:
    App();

    void run();

private:
    std::shared_ptr<httc::Router> m_router;
    std::shared_ptr<State> m_state;
};
