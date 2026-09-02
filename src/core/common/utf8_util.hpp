#pragma once
#include <string>
#include <algorithm>
#include <cctype>

namespace libsorter::common {

// Простая реализация tolower для ASCII/UTF-8 (для MVP достаточно)
inline std::string utf8_tolower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return result;
}

} // namespace libsorter::common
