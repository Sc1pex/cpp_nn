#include <memory>
#include <uvw.hpp>
#include "cli.h"

class App {
public:
    App();

    void run();

private:
    std::unique_ptr<CliRead> m_cli;
    std::shared_ptr<uvw::loop> m_loop;
};

App::App() {
    m_loop = uvw::loop::get_default();
    m_cli = std::make_unique<CliRead>(m_loop);
}

void App::run() {
    m_cli->start();

    m_loop->run();
}

int main() {
    App app;
    app.run();
}
