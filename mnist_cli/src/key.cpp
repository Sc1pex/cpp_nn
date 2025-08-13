#include "key.h"

std::optional<Key> Key::from_byes(const char* bytes, int length) {
    if (length == 1) {
        if (bytes[0] == 127) {
            // Who could have guessed the backspace key actually means delete
            return Key(Special::Backspace);
        } else if (bytes[0] == 13) {
            return Key(Special::Enter);
        } else if (bytes[0] == 9) {
            return Key(Special::Tab);
        } else if (32 <= bytes[0] && bytes[0] <= 126) {
            return Key(bytes[0]);
        } else if (bytes[0] == 3) {
            return Key(Special::CtrlC);
        } else if (bytes[0] == 4) {
            return Key(Special::CtrlD);
        }
    } else if (length == 3) {
        if (bytes[0] == 27 && bytes[1] == 91) {
            if (bytes[2] == 65) {
                return Key(Special::UpArrow);
            } else if (bytes[2] == 66) {
                return Key(Special::DownArrow);
            } else if (bytes[2] == 67) {
                return Key(Special::RightArrow);
            } else if (bytes[2] == 68) {
                return Key(Special::LeftArrow);
            }
        }
    } else if (length == 4) {
        if (bytes[0] == 27 && bytes[1] == 91 && bytes[3] == 126) {
            if (bytes[2] == 49) {
                return Key(Special::Home);
            } else if (bytes[2] == 52) {
                return Key(Special::End);
            } else if (bytes[2] == 51) {
                return Key(Special::Delete);
            }
        }
    }

    return std::nullopt;
}

Key::Key(char c) {
    m_is_special = false;
    m_char = c;
}

Key::Key(Special s) {
    m_is_special = true;
    m_special = s;
}

bool Key::operator==(const char c) const {
    return !m_is_special && m_char == c;
}

bool Key::operator==(const Special s) const {
    return m_is_special && m_special == s;
}

bool Key::is_special() const {
    return m_is_special;
}

char Key::get_char() const {
    return m_char;
}
