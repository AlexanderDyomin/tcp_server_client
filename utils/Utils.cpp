#include <iostream>
#include "Utils.h"

namespace Utils {
    std::optional<std::pair<std::string, std::string>> getKeyValuePair(const std::string &str, size_t pos) noexcept {
        if (str.size() < 3) {
            return {};
        }

        auto foundPos = str.find('=', pos);
        if (foundPos == std::string::npos) {
            return {};
        }

        return {{str.substr(pos, foundPos - pos), (foundPos == str.size() - 1 ? std::string{} : str.substr(foundPos + 1))}};
    }
}
