#include "NetUtils.h"
#include "Utils.h"

#include <iostream>
#include <cassert>

namespace {
    constexpr char GET_COMMAND[] = "get";
    constexpr char SET_COMMAND[] = "set";
}

namespace Utils::Net {
    const std::string& getDelimiter() noexcept {
        static std::string delimiter = "\n\r\n\r";
        return delimiter;
    }

    Command::Command(std::string commandStr) noexcept {
        parse(std::move(commandStr));
    }

    Command::Command(Command::Type commandType, std::string key, std::string value) noexcept
        : m_commandType(commandType)
        , m_key(std::move(key))
        , m_value(std::move(value)) {
        assert(m_commandType != Command::Type::UNKNOWN);
    }

    bool Command::parse(std::string command) noexcept {
        m_commandType = Command::Type::UNKNOWN;
        m_value.clear();

        if (command.size() < 6) {
            std::cerr << "Command parse error: unrecognized command" << std::endl;
            return false;
        }

        if (command[0] != '$') {
            std::cerr << "Command parse error: unrecognized command" << std::endl;
            return false;
        }

        if (command[2] != 'e' || command[3] != 't' || command[4] != ' ') { // now it is "$_et=" string, check below is it "set" or "get"
            std::cerr << "Command parse error: unrecognized command" << std::endl;
             return false;
        }

        if (command[1] == 'g') {
            m_commandType = Command::Type::GET;
            m_key = command.substr(5);
        } else if (command[1] == 's') {
            auto keyVal = Utils::getKeyValuePair(command, 5);
            if (!keyVal) {
                std::cerr << "Command parse error: invalid input" << std::endl;
                return false;
            }

            m_commandType = Command::Type::SET;
            m_key = std::move(keyVal->first);
            m_value = std::move(keyVal->second);
        }

        return true;
    }

    std::string Command::createCommandString() const noexcept {
        if (!isValid()) {
            return {};
        }

        std::string res;
        res.reserve(m_key.size() + m_value.size() + 6);
        res += '$';

        static auto fillKey = [] (std::string& res, const std::string& key) {
            res += ' ';
            res += key;
        };

        switch(m_commandType) {
            case Command::Type::GET:
                res += GET_COMMAND;
                fillKey(res, m_key);
                break;
            case Command::Type::SET:
                res += SET_COMMAND;
                fillKey(res, m_key);
                res += '=';
                res += m_value;
                break;
            default:
                assert("unknown command type");
        }

        return res;
    }

    bool Command::isValid() const noexcept {
        return m_commandType != Command::Type::UNKNOWN;
    }

    Command::Type Command::getCommandType() const noexcept {
        return m_commandType;
    }

    const std::string& Command::getKey() const noexcept {
        return m_key;
    }

    const std::string& Command::getValue() const noexcept {
        return m_value;
    }
}