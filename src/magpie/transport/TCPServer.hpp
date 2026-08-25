#pragma once

#include "magpie/application/Http2Adapter.hpp"
#include "raven/config/SSLConfig.hpp"
#include <raven/SocketServer.hpp>

#include <cstdint>
#include <string>
#include <optional>

namespace magpie { class BaseApp; }
namespace magpie::transport {

class TCPServer {
private:
    unsigned int concurrency;
    BaseApp* app;

    bool die = false;
    bool hasSSL = false;
    raven::SocketServer serv;
public:
    TCPServer(
        BaseApp* app,
        uint16_t port,
        unsigned int concurrency,
        const std::string& bindAddr = "127.0.0.1",
        std::optional<raven::SSLConfig>&& sslConfig = std::nullopt
    );
    ~TCPServer();

    void start();
    void stop();

    uint16_t getPort();

    void sync() {
        this->serv.waitForStarted();
    }
};

}
