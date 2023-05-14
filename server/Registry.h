#pragma once

#include "utils/FileReaderWriter.h"

#include <boost/asio/steady_timer.hpp>

#include <chrono>
#include <deque>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <iostream>

class Stats {
    using Timestamp = std::chrono::steady_clock::time_point;

public:
    void add(const Timestamp& now = std::chrono::steady_clock::now()) noexcept;

    [[nodiscard]] uint32_t getAllSum() const noexcept;
    [[nodiscard]] uint32_t getLastIntervalSum(const Timestamp& now = std::chrono::steady_clock::now()) const noexcept;

private:
    void eraseOutdated(const Timestamp& now) noexcept;

    uint32_t m_allSum = 0;
    std::deque<Timestamp> m_lastTimestamps;
};

class Value {
public:
    explicit Value(std::string str) noexcept;
    Value(const Value& value) noexcept = default;
    Value(Value&& value) noexcept = default;

    [[nodiscard]] const std::string& getValue() const noexcept;

    [[nodiscard]] const Stats& getReadStats() const noexcept;
    [[nodiscard]] const Stats& getWriteStats() const noexcept;

private:
    friend class Registry; // to manipulate stats

    std::string m_value;
    Stats m_readStats;
    Stats m_writeStats;
};

class Registry {
public:
    template<typename Handler>
    static Registry& create(Handler& context);
    static Registry& instance();

    ~Registry();

    bool init(std::string filename) noexcept;

    void stop() noexcept;

    bool readConfig() noexcept;
    bool writeConfig() noexcept;

    std::optional<Value> getValue(const std::string& key) noexcept;
    void setValue(const std::string& key, const std::string& value) noexcept;

private:
    template<typename Handler>
    explicit Registry(Handler& context) noexcept;

    void onTimer(const boost::system::error_code& ec) noexcept;

    static std::unique_ptr<Registry> REGISTRY;

    mutable std::shared_mutex m_mutex;
    Utils::FileReaderWriter m_fileRW;
    std::unordered_map<std::string, Value> m_registry;
    boost::asio::steady_timer m_timer;
    bool m_dirty{false};
};

template<typename Handler>
Registry::Registry(Handler& context) noexcept
    : m_timer(context) {
    m_timer.expires_from_now(std::chrono::milliseconds(500));
    m_timer.async_wait([this] (const boost::system::error_code& ec) {
        onTimer(ec);
    });
}

template<typename Handler>
Registry& Registry::create(Handler& context) {
    assert(!REGISTRY);
    REGISTRY = std::unique_ptr<Registry>(new Registry(context));
    return *REGISTRY;
}

