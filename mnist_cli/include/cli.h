#pragma once

#include <memory>
#include <uvw.hpp>

class CliRead {
public:
    CliRead(std::shared_ptr<uvw::loop>& loop);

    void start();
    void on_input(std::function<void(uvw::async_event& ev, uvw::async_handle& h)> handle);

private:
    void handle_data(const uvw::data_event& event);

private:
    std::shared_ptr<uvw::tty_handle> m_in;
    std::shared_ptr<uvw::tty_handle> m_out;
    std::shared_ptr<uvw::async_handle> m_send;

    std::string current_line;
    int cursor_pos;
};
