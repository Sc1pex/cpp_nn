#pragma once

#include <memory>
#include <uvw.hpp>
#include "cli.h"

class App {
public:
    App();

    void run();

private:
    std::unique_ptr<CliRead> m_cli;
    std::shared_ptr<uvw::async_handle> m_quit;
    std::shared_ptr<uvw::loop> m_loop;
};
