#include "app.h"
#include <cstdlib>
#include <print>

App::App() {
    m_loop = uvw::loop::get_default();
    m_quit = m_loop->resource<uvw::async_handle>();
    m_quit->on<uvw::async_event>([](uvw::async_event& e, uvw::async_handle& h) {
        std::exit(0);
    });

    m_cli = std::make_unique<CliRead>(m_loop, m_quit);
    m_cli->on_input([](uvw::async_event& ev, uvw::async_handle& h) {
        auto inp = h.data<std::string>();
        std::print("Got line {}\r\n", *inp);
    });
}

void App::run() {
    m_cli->start();

    m_loop->run();
}
