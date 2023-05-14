#pragma once

#include <optional>
#include <string>

namespace Utils {
    std::optional<std::pair<std::string, std::string>> getKeyValuePair(const std::string& str, size_t pos = 0) noexcept;
}
