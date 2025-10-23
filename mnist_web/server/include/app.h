#pragma once
#include <httc/router.h>
#include "db.h"

class App {
public:
    App();

    void run();

private:
    std::shared_ptr<httc::Router> m_router;
    DB m_db;
};
