#pragma once

#include "magpie/transfer/Request.hpp"
#include <asio.hpp>
#include <asio/io_service.hpp>
#include <functional>

namespace magpie::transport {

class TCPServer {
private:
    asio::io_context ctx;
    asio::ip::tcp::acceptor ipv4Acceptor;
    unsigned int concurrency;

    bool die = false;

    void doAccept();
public:
    TCPServer(
        short port,
        unsigned int concurrency
    );
    ~TCPServer();

    void start(
        const std::function<
            void(const Request&)
        >& onRequest
    );
    void stop();
};

}
