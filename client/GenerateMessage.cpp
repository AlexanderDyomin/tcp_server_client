#include "GenerateMessage.h"
#include "utils/NetUtils.h"

#include <chrono>
#include <vector>
#include <random>

namespace {
    const std::vector<std::string> KEYS = {
            "one",
            "two",
            "three",
            "four",
            "five",
            "six",
            "seven",
            "eight",
            "nine",
            "ten"
    };

    std::mt19937 GEN(std::chrono::steady_clock::now().time_since_epoch().count());

    std::string generateKey() noexcept {
        static std::uniform_int_distribution<uint8_t> uniformDist(0, KEYS.size() - 1);
        return KEYS[uniformDist(GEN)];
    }

    std::string generateRandomString(size_t len) noexcept {
        constexpr char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        static std::uniform_int_distribution<uint8_t> uniformDist(0, sizeof(charset) - 2); // because of null terminator that counts to size
        std::string res;
        res.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            res+= charset[uniformDist(GEN)];
        }

        return res;
    }

    size_t randomLength() noexcept {
        static std::uniform_int_distribution<uint8_t> uniformDist(5, 20);
        return uniformDist(GEN);
    }
}

std::string generateRandomCommand() noexcept {
    std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    static std::uniform_int_distribution<uint8_t> uniformDist(0, 99); // call $SET only in 1% of cases
    auto val = uniformDist(gen);
    auto key = generateKey();
    Utils::Net::Command cmd;
    if (val == 0) {
        cmd = Utils::Net::Command{Utils::Net::Command::Type::SET, std::move(key), generateRandomString(randomLength())};
    } else {
        cmd = Utils::Net::Command{Utils::Net::Command::Type::GET, std::move(key)};
    }

    return cmd.createCommandString();
}