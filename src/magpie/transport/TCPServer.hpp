#pragma once

#include <asio.hpp>
#include <asio/io_context.hpp>


namespace magpie { class BaseApp; }
namespace magpie::transport {

class TCPServer {
private:
    asio::io_context ctx;
    asio::ip::tcp::acceptor ipv4Acceptor;
    unsigned int concurrency;
    BaseApp* app;

    bool die = false;

    void doAccept();
public:
    TCPServer(
        BaseApp* app,
        short port,
        unsigned int concurrency
    );
    ~TCPServer();

    void start();
    void stop();
};

}
