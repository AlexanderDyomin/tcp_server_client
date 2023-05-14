#pragma once

#include "Session.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include <string>
#include <iostream>
#include <unordered_set>

template<typename Handler>
class Server {
public:
    template<typename ExecutionContext>
    explicit Server(ExecutionContext& io_context) noexcept;
    ~Server();
    bool start(const std::string& host, uint16_t port) noexcept;
    void stop() noexcept;

private:
    void doAccept();

    boost::asio::ip::tcp::acceptor m_acceptor;
    std::unordered_set<std::shared_ptr<Session<Handler>>> m_sessionSet;
};

template<typename Handler>
template<typename ExecutionContext>
Server<Handler>::Server(ExecutionContext& io_context) noexcept
    : m_acceptor(io_context) {
}

template<typename Handler>
Server<Handler>::~Server() {
    stop();
}

template<typename Handler>
bool Server<Handler>::start(const std::string& host, uint16_t port) noexcept {
    boost::system::error_code ec;
    auto v4Address = boost::asio::ip::address_v4::from_string(host, ec);
    if (ec) {
        std::cerr << "Convert v4 address failed: string: \"" << host << "\" error: " << ec.message() << std::endl;
        return false;
    }

    boost::asio::ip::tcp::endpoint endpoint(v4Address, port);
    m_acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        std::cerr << "Acceptor open error: " << ec.message() << std::endl;
        return false;
    }

    m_acceptor.set_option(boost::asio::socket_base::reuse_address(true));
    m_acceptor.bind(endpoint, ec);
    if (ec) {
        std::cerr << "Acceptor bind error: " << ec.message() << std::endl;
        return false;
    }

    m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
        std::cerr << "Acceptor listen error: " << ec.message() << std::endl;
        return false;
    }

    doAccept();
    return true;
}

template<typename Handler>
void Server<Handler>::stop() noexcept {
    boost::system::error_code ec;
    m_acceptor.close(ec);
    for (auto& entry : m_sessionSet) {
        entry->stop();
    }
    m_sessionSet.clear();
}

template<typename Handler>
void Server<Handler>::doAccept() {
    m_acceptor.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (!ec) {
            std::cout << "Accepted new connection!" << std::endl;
            auto session = std::make_shared<Session<Handler>>(std::move(socket), [this] (const std::shared_ptr<Session<Handler>>& session) {
                m_sessionSet.erase(session);
            });
            session->start();
            m_sessionSet.emplace(session);
            doAccept();
        }
    });
}