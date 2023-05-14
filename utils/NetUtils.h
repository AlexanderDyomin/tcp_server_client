#include <string>

namespace Utils::Net {
    const std::string& getDelimiter() noexcept;

    class Command {
    public:
        enum class Type : uint8_t {
            UNKNOWN,
            GET,
            SET
        };

        Command() noexcept = default;
        explicit Command(std::string commandStr) noexcept;
        explicit Command(Type commandType, std::string key, std::string value = {}) noexcept;
        Command(const Command&) noexcept = default;
        Command(Command&&) noexcept = default;

        Command& operator=(const Command&) noexcept = default;
        Command& operator=(Command&&) noexcept = default;

        bool parse(std::string command) noexcept;
        std::string createCommandString() const noexcept;

        bool isValid() const noexcept;

        Type getCommandType() const noexcept;
        const std::string& getKey() const noexcept;
        const std::string& getValue() const noexcept;
    private:
        Type m_commandType{Type::UNKNOWN};
        std::string m_key;
        std::string m_value;
    };
}