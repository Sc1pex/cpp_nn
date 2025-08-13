#include "cli.h"
#include <print>
#include "key.h"

CliRead::CliRead(std::shared_ptr<uvw::loop>& loop) {
    m_in = loop->resource<uvw::tty_handle>(0, true);
    m_send = loop->resource<uvw::async_handle>();

    m_in->on<uvw::data_event>([this](const uvw::data_event& event, uvw::tty_handle&) {
        handle_data(event);
    });
}

void CliRead::start() {
    m_in->mode(uvw::tty_handle::tty_mode::RAW);
    m_in->read();
}

void CliRead::handle_data(const uvw::data_event& event) {
    auto key = Key::from_byes(event.data.get(), event.length);
    if (key) {
        std::println("Got {}", *key);
    }
}
