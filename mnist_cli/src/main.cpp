#include <memory>
#include <print>
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

    m_cli->on_input([](uvw::async_event& ev, uvw::async_handle& h) {
        auto inp = h.data<std::string>();
        std::print("Got line {}\r\n", *inp);
    });
}

void App::run() {
    m_cli->start();

    m_loop->run();
}

int main() {
    App app;
    app.run();
}
