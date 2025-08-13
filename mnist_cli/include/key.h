#pragma once

#include <format>
#include <optional>
#include <string>

class Key {
public:
    enum class Special {
        UpArrow,
        DownArrow,
        LeftArrow,
        RightArrow,

        Backspace,
        Enter,
        Delete,
        Tab,

        Home,
        End,
    };

public:
    static std::optional<Key> from_byes(const char* bytes, int length);

    bool operator==(const char c) const;
    bool operator==(const Special s) const;

    bool is_special() const;

    // Can only be called if the key is not special
    char get_char() const;

private:
    Key(char c);
    Key(Special s);

    bool m_is_special;
    union {
        char m_char;
        Special m_special;
    };

    friend struct std::formatter<Key>;
};

template<>
struct std::formatter<Key> : std::formatter<std::string> {
    auto format(const Key& key, std::format_context& ctx) const {
        if (key.m_is_special) {
            if (key.m_special == Key::Special::UpArrow) {
                return std::format_to(ctx.out(), "Key({})", "UpArrow");
            } else if (key.m_special == Key::Special::DownArrow) {
                return std::format_to(ctx.out(), "Key({})", "DownArrow");
            } else if (key.m_special == Key::Special::LeftArrow) {
                return std::format_to(ctx.out(), "Key({})", "LeftArrow");
            } else if (key.m_special == Key::Special::RightArrow) {
                return std::format_to(ctx.out(), "Key({})", "RightArrow");
            } else if (key.m_special == Key::Special::Backspace) {
                return std::format_to(ctx.out(), "Key({})", "Backspace");
            } else if (key.m_special == Key::Special::Enter) {
                return std::format_to(ctx.out(), "Key({})", "Enter");
            } else if (key.m_special == Key::Special::Delete) {
                return std::format_to(ctx.out(), "Key({})", "Delete");
            } else if (key.m_special == Key::Special::Tab) {
                return std::format_to(ctx.out(), "Key({})", "Tab");
            } else if (key.m_special == Key::Special::Home) {
                return std::format_to(ctx.out(), "Key({})", "Home");
            } else if (key.m_special == Key::Special::End) {
                return std::format_to(ctx.out(), "Key({})", "End");
            } else {
                return std::format_to(ctx.out(), "Invalid Key");
            }
        } else {
            return std::format_to(ctx.out(), "Key('{}')", key.m_char);
        }
    }
};
