#pragma once
#include <httc/router.h>

class App {
public:
    App();

    void run();

private:
    std::shared_ptr<httc::Router> m_router;
};
