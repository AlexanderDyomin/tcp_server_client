#include "Registry.h"

#include "utils/Utils.h"

#include <fstream>
#include <numeric>
#include <iostream>
#include <filesystem>

using namespace std::chrono_literals;

namespace {
    constexpr auto DURATION = 5s;
}

void Stats::add(const Stats::Timestamp& now) noexcept {
    ++m_allSum;

    eraseOutdated(now);
    m_lastTimestamps.emplace_back(now);
}
uint32_t Stats::getAllSum() const noexcept {
    return m_allSum;
}

uint32_t Stats::getLastIntervalSum(const Stats::Timestamp& now) const noexcept {
    auto upper = std::upper_bound(m_lastTimestamps.begin(), m_lastTimestamps.end(), now - DURATION);
    return std::distance(upper, m_lastTimestamps.end());
}

void Stats::eraseOutdated(const Stats::Timestamp& now) noexcept {
    auto upper = std::upper_bound(m_lastTimestamps.begin(), m_lastTimestamps.end(), now - DURATION);
    m_lastTimestamps.erase(m_lastTimestamps.begin(), upper);
}

Value::Value(std::string str) noexcept
    : m_value(std::move(str)) {
}

const std::string& Value::getValue() const noexcept {
    return m_value;
}

const Stats& Value::getReadStats() const noexcept {
    return m_readStats;
}

const Stats& Value::getWriteStats() const noexcept {
    return m_writeStats;
}

std::unique_ptr<Registry> Registry::REGISTRY{nullptr};

Registry& Registry::instance() {
    assert(REGISTRY);
    return *REGISTRY;
}

bool Registry::init(std::string filename) noexcept {
    return m_fileRW.init(std::move(filename));
}

void Registry::stop() noexcept {
    m_timer.cancel();
    if (m_dirty) {
        writeConfig();
    }
}

Registry::~Registry() {
    stop();
}

void Registry::onTimer(const boost::system::error_code& ec) noexcept {
    if (ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }
        std::cerr << "Error on timer timeout: " << ec.message() << std::endl;
    }

    if (m_dirty) {
        m_dirty = false;
        if (!writeConfig()) {
            std::cerr << "Failed write config on timeout" << std::endl;
        }
    }

    boost::system::error_code restartEc;
    m_timer.expires_at(m_timer.expires_at() + std::chrono::milliseconds(500), restartEc);
    if (restartEc) {
        std::cerr << "Failed restart write config timer: " << restartEc.message() << std::endl;
        std::terminate(); // non-recoverable situation
    }
    m_timer.async_wait([this](const boost::system::error_code& ec) {
        onTimer(ec);
    });
}

bool Registry::readConfig() noexcept {
    std::unique_lock l(m_mutex);
    auto readFromFile = [this](std::ifstream& input) {
        for (std::string line; std::getline(input, line);) {
            auto keyValue = Utils::getKeyValuePair(line);
            if (!keyValue) {
                m_registry.clear();
                return false;
            }
            m_registry.emplace(std::move(keyValue->first), Value(std::move(keyValue->second)));
        }
        return true;
    };

    return m_fileRW.read(readFromFile);
}

bool Registry::writeConfig() noexcept {
    std::stringstream ss;
    {
        // it is faster to write string to memory and only then to disk
        std::unique_lock l(m_mutex);
        for (const auto& entry : m_registry) {
            ss << entry.first << '=' << entry.second.m_value << std::endl;
        }
    }
    auto writeToFile = [&ss] (std::ofstream& out) {
        out << ss.rdbuf();
    };

    return m_fileRW.write(writeToFile);
}

std::optional<Value> Registry::getValue(const std::string& key) noexcept {
    std::shared_lock l(m_mutex);
    auto it = m_registry.find(key);
    if (it == m_registry.end()) {
        return {};
    }
    it->second.m_readStats.add();
    return it->second;
}

void Registry::setValue(const std::string& key, const std::string& value) noexcept {
    {
        std::unique_lock l(m_mutex);
        auto [it, inserted] = m_registry.try_emplace(key, value);
        if (!inserted) {
            it->second.m_value = value;
        }

        it->second.m_writeStats.add();
    }

    m_dirty = true;
}