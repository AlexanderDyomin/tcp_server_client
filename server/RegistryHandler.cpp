#include "RegistryHandler.h"

#include "Registry.h"
#include "utils/NetUtils.h"

namespace {
    std::string formGetResponse(const std::string& key, const Value& value) {
        std::string res;
        res.reserve(32);
        res.append(key)
           .append(1, '=')
           .append(value.getValue())
           .append(1,'\n')
           .append("all_reads=")
           .append(std::to_string(value.getReadStats().getAllSum()))
           .append(1,'\n')
           .append("last_reads=")
           .append(std::to_string(value.getReadStats().getLastIntervalSum()))
           .append(1,'\n')
           .append("all_writes=")
           .append(std::to_string(value.getWriteStats().getAllSum()))
           .append(1,'\n')
           .append("last_writes=")
           .append(std::to_string(value.getWriteStats().getLastIntervalSum()));

       return res;
    }

    std::string formSetResponse(const std::string& key, const std::string& value) {
        std::string res;
        res.append(key).append(1, '=').append(value);
        return res;
    }
}

RegistryHandler::RegistryHandler(std::function<void(std::string)> responseCallback) noexcept
    : m_responseCallback(std::move(responseCallback)) {
}

void RegistryHandler::execute(std::string commandStr) noexcept {Utils::Net::Command cmd(std::move(commandStr));
    if (!cmd.isValid()) {
        m_responseCallback("error: unrecognized command");
        return;
    }

    switch (cmd.getCommandType()) {
        case Utils::Net::Command::Type::GET: {
            auto value = Registry::instance().getValue(cmd.getKey());
            if (!value) {
                m_responseCallback("error: value is not set");
                return;
            }

            m_responseCallback(formGetResponse(cmd.getKey(), *value));
            break;
        }
        case Utils::Net::Command::Type::SET: {
            Registry::instance().setValue(cmd.getKey(), cmd.getValue());
            m_responseCallback(formSetResponse(cmd.getKey(), cmd.getValue()));
        }
        default:
            assert("error: unprocessed command type");
    }
}