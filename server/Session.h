#pragma once

#include "Session.h"
#include "utils/NetUtils.h"

#include <boost/asio.hpp>

#include <memory>
#include <functional>
#include <iostream>

template<typename Handler>
class Session
  : public std::enable_shared_from_this<Session<Handler>> {
public:
    explicit Session(boost::asio::ip::tcp::socket socket, std::function<void(const std::shared_ptr<Session>&)> closeCallback) noexcept;
    ~Session();
    void start() noexcept;
    void write(std::string message) noexcept;
    void stop() noexcept;

private:
    void onRead() noexcept;

private:
    Handler m_handler;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::streambuf m_streambuf;
    std::function<void(const std::shared_ptr<Session>&)> m_closeCallback;
};

template<typename Handler>
Session<Handler>::Session(boost::asio::ip::tcp::socket socket, std::function<void(const std::shared_ptr<Session>& /*self*/)> closeCallback) noexcept
        : m_handler(std::bind(&Session<Handler>::write, this, std::placeholders::_1))
        , m_socket(std::move(socket))
        , m_streambuf(2048)
        , m_closeCallback(std::move(closeCallback)) {
}

template<typename Handler>
Session<Handler>::~Session() {
    stop();
}

template<typename Handler>
void Session<Handler>::start() noexcept {
    onRead();
}

template<typename Handler>
void Session<Handler>::onRead() noexcept {
    boost::asio::async_read_until(m_socket, m_streambuf, Utils::Net::getDelimiter(),
          [weakSelf = this->weak_from_this()] (boost::system::error_code ec, size_t length) {
              auto self = weakSelf.lock();
              if (!self) {
                  return;
              }
              if (ec) {
                  if (ec == boost::asio::error::eof) { // eof means socket closed
                      self->m_closeCallback(self);
                      return;
                  } else if (ec == boost::system::errc::operation_canceled) {
                      return;
                  }

                  std::cerr << "Session read error occurred: " << ec.message() << " " << ec.value() << std::endl;
                  return;
              }

              self->m_handler.execute({boost::asio::buffers_begin(self->m_streambuf.data()),
                                       boost::asio::buffers_begin(self->m_streambuf.data()) + length - Utils::Net::getDelimiter().size()});
              self->m_streambuf.consume(length);
              self->onRead();
          });
}

template<typename Handler>
void Session<Handler>::write(std::string message) noexcept {
    message.append(Utils::Net::getDelimiter());
    boost::asio::async_write(m_socket, boost::asio::buffer(message), [] (boost::system::error_code ec, size_t /*sizeWritten*/) {
        if (ec) {
            if (ec == boost::system::errc::operation_canceled) {
                return;
            }

            std::cerr << "Session write failed: " << ec.message() << std::endl;
            return;
        }
    });
}
template<typename Handler>
void Session<Handler>::stop() noexcept {
    boost::system::error_code ec;
    m_socket.close(ec);
}

