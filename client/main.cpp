#include "GenerateMessage.h"
#include "utils/NetUtils.h"

#include <boost/asio.hpp>

#include <iostream>
#include <boost/asio/read_until.hpp>

namespace {
    constexpr size_t STEPS_COUNT = 10000;

    bool isPortNumber(const std::string& s) noexcept {
        if (!std::isdigit(s[0])) { // if there is minus it is not a port number
            return false;
        }

        try {
            auto converted = std::stoll(s);
            return converted <= std::numeric_limits<uint16_t>::max(); // max port num is max 16-bit integer
        } catch(...) {
            return false;
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: client <host> <port>" << std::endl;
        return 0;
    }

    if (!isPortNumber(argv[2])) {
        std::cerr << argv[2] << " is not a port number" << std::endl;
        return -1;
    }

    boost::asio::io_context context;
    boost::asio::ip::tcp::resolver resolver(context);
    boost::system::error_code ec;
    auto endpoints = resolver.resolve(argv[1], argv[2], ec);
    if (ec) {
        std::cerr << "Error resolving host \"" << argv[1] << "\" port " << argv[2] << ": " << ec.message() << std::endl;
        return -1;
    }

    boost::asio::ip::tcp::socket socket(context);
    boost::asio::connect(socket, endpoints, ec);
    if (ec) {
        std::cerr << "Error connecting socket: " << ec.message() << std::endl;
        return -1;
    }

    boost::asio::streambuf response(2048);
    for (size_t i = 0; i < STEPS_COUNT; ++i) {
        auto command = generateRandomCommand();
        std::cout << "command: " << command << std::endl;
        command.append(Utils::Net::getDelimiter());
        std::cout << "i = " << i << std::endl;
        boost::asio::write(socket, boost::asio::buffer(command), ec);
        if (ec) {
            std::cerr << "Error on write: " << ec.message() << std::endl;
            return -2;
        }

        auto length = boost::asio::read_until(socket, response, Utils::Net::getDelimiter(), ec);
        if (ec) {
            if (ec == boost::asio::error::eof) {
                //socket closed
                return 0;
            }
            std::cerr << "Error on read: " << ec.message() << std::endl;
            return -2;
        }
        std::string_view respStringView(static_cast<const char*>(response.data().data()), length - Utils::Net::getDelimiter().size());
        std::cout << "Response: " << respStringView << std::endl;
        response.consume(length);
    }

    socket.close();

    return 0;
}