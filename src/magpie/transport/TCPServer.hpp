#pragma once

#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

#include <asio.hpp>
#include <asio/io_context.hpp>
#include <asio/ssl.hpp>


namespace magpie { class BaseApp; }
namespace magpie::transport {

class TCPServer {
private:
    asio::io_context ctx;
    asio::ip::tcp::acceptor ipv4Acceptor;
    std::optional<asio::ssl::context> sslCtx;

    unsigned int concurrency;
    BaseApp* app;

    bool die = false;

    void doAccept();
public:
    TCPServer(
        BaseApp* app,
        uint16_t port,
        unsigned int concurrency,
        std::string_view bindAddr = "127.0.0.1"
    );
    ~TCPServer();

    void start();
    void stop();

    uint16_t getPort();
};

}
