#include "Registry.h"
#include "Server.h"
#include "RegistryHandler.h"

#include <boost/asio/thread_pool.hpp>

#include <string>
#include <iostream>

int main(int argc, char** argv) {
    std::string configFilename = "config.txt";
    if (argc > 1) {
        configFilename = argv[1];
    }

    boost::asio::io_context utilityContext;
    auto& registry = Registry::create(utilityContext);
    if (!registry.init(configFilename)) {
        std::cerr << "Error init registry with config path \"" << configFilename << "\"" << std::endl;
        return -1;
    }
    if (!registry.readConfig()) {
        std::cerr << "Error reading config file \"" << configFilename << "\"" << std::endl;
        return -2;
    }

    auto utilityGuard = boost::asio::make_work_guard(utilityContext);
    auto utilityThread = std::thread([&utilityContext] {
        utilityContext.run();
    });

    boost::asio::thread_pool threadPool(std::thread::hardware_concurrency() - 1); // one is reserved for utilityThread
    auto guard = boost::asio::make_work_guard(threadPool);
    Server<RegistryHandler> srv(threadPool);
    srv.start("127.0.0.1", 8888);

    boost::asio::signal_set signals(threadPool, SIGINT, SIGTERM);
    signals.async_wait([&] (const boost::system::error_code& ec, int signalNumber) {
        if (ec) {
            std::cerr << "Error occurred on signal handler: " << ec.message() << std::endl;
            return;
        }

        std::cout << "Interrupted by signal " << signalNumber << std::endl;

        guard.reset();
        srv.stop();
        utilityGuard.reset();
        registry.stop();
    });

    threadPool.join();
    utilityThread.join();

    return 0;
}