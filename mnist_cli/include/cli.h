#pragma once

#include <memory>
#include <uvw.hpp>

class CliRead {
public:
    CliRead(std::shared_ptr<uvw::loop>& loop);

    void start();

private:
    void handle_data(const uvw::data_event& event);

private:
    std::shared_ptr<uvw::tty_handle> m_in;
    std::shared_ptr<uvw::async_handle> m_send;

    std::string current_line;
};
