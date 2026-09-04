#pragma once
#include <string>
#include <cstdint>
#include <cctype>

namespace libsorter::common {

// UTF-8 lowercase для ASCII + кириллицы (достаточно для MVP)
inline std::string utf8_tolower(const std::string& s) {
    std::string r;
    r.reserve(s.size());
    size_t i = 0;
    while (i < s.size()) {
        uint8_t c = static_cast<uint8_t>(s[i]);
        if (c < 0x80) {
            r += static_cast<char>(std::tolower(c));
            i += 1;
        } else if (c == 0xD0 && i + 1 < s.size()) {
            uint8_t c2 = static_cast<uint8_t>(s[i + 1]);
            if (c2 >= 0x90 && c2 <= 0x9F) {
                r += static_cast<char>(0xD0);
                r += static_cast<char>(c2 + 0x20);
                i += 2;
            } else if (c2 >= 0xA0 && c2 <= 0xAF) {
                r += static_cast<char>(0xD1);
                r += static_cast<char>(c2 - 0x20);
                i += 2;
            } else if (c2 == 0x81) {
                r += static_cast<char>(0xD1);
                r += static_cast<char>(0x91);
                i += 2;
            } else {
                r += s[i]; r += s[i + 1]; i += 2;
            }
        } else if (c >= 0xC0 && i + 1 < s.size()) {
            r += s[i]; r += s[i + 1]; i += 2;
        } else if (c >= 0xE0 && i + 2 < s.size()) {
            r += s[i]; r += s[i+1]; r += s[i+2]; i += 3;
        } else if (c >= 0xF0 && i + 3 < s.size()) {
            r += s[i]; r += s[i+1]; r += s[i+2]; r += s[i+3]; i += 4;
        } else {
            r += s[i]; i += 1;
        }
    }
    return r;
}

} // namespace libsorter::common
