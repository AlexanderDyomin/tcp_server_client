#pragma once

#include <functional>
#include <string>

class RegistryHandler {
public:
    RegistryHandler(std::function<void(std::string)> responseCallback) noexcept;
    void execute(std::string command) noexcept;

private:
    std::function<void(std::string)> m_responseCallback;
};
