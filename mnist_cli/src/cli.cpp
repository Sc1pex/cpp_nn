#include "cli.h"
#include <memory>
#include "key.h"

template<typename... Args>
void write_fmt(
    std::shared_ptr<uvw::tty_handle> tty, std::format_string<Args...> fmt, Args&&... args
) {
    auto s = std::format(fmt, std::forward<Args>(args)...);
    tty->write(s.data(), s.size());
}

CliRead::CliRead(std::shared_ptr<uvw::loop>& loop, std::shared_ptr<uvw::async_handle> quit) {
    m_in = loop->resource<uvw::tty_handle>(0, true);
    m_out = loop->resource<uvw::tty_handle>(1, false);
    m_send = loop->resource<uvw::async_handle>();

    m_in->on<uvw::data_event>([this, quit](const uvw::data_event& event, uvw::tty_handle&) {
        handle_data(event, quit);
    });

    current_line = "";
    cursor_pos = 0;
}

void CliRead::start() {
    m_in->mode(uvw::tty_handle::tty_mode::RAW);
    m_in->read();

    write_fmt(m_out, "> ");
}

void CliRead::on_input(std::function<void(uvw::async_event& ev, uvw::async_handle& h)> handle) {
    m_send->on<uvw::async_event>(handle);
}

void CliRead::handle_data(const uvw::data_event& event, std::shared_ptr<uvw::async_handle> quit) {
    auto key_opt = Key::from_byes(event.data.get(), event.length);
    if (!key_opt) {
        return;
    }
    auto key = *key_opt;

    if (key == Key::Special::CtrlC || key == Key::Special::CtrlD) {
        quit->send();
        return;
    } else if (!key.is_special()) {
        current_line.insert(cursor_pos, 1, key.get_char());
        cursor_pos++;
    } else if (key == Key::Special::Backspace) {
        if (cursor_pos > 0) {
            current_line.erase(cursor_pos - 1, 1);
            cursor_pos--;
        }
    } else if (key == Key::Special::Delete) {
        if (cursor_pos < current_line.size()) {
            current_line.erase(cursor_pos, 1);
        }
    } else if (key == Key::Special::LeftArrow && cursor_pos > 0) {
        cursor_pos--;
    } else if (key == Key::Special::RightArrow && cursor_pos < current_line.size()) {
        cursor_pos++;
    } else if (key == Key::Special::Home) {
        cursor_pos = 0;
    } else if (key == Key::Special::End) {
        cursor_pos = current_line.size();
    } else if (key == Key::Special::Enter) {
        write_fmt(m_out, "\r\n");

        m_send->data(std::make_shared<std::string>(current_line));
        m_send->send();
        current_line.clear();
        cursor_pos = 0;

        return;
    }

    if (cursor_pos != current_line.size()) {
        write_fmt(m_out, "\r\033[K> {}\033[{}D", current_line, current_line.size() - cursor_pos);
    } else {
        write_fmt(m_out, "\r\033[K> {}", current_line);
    }
}
