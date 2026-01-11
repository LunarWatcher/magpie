#include "TCPServer.hpp"
#include "magpie/transport/Connection.hpp"
#include <asio/error_code.hpp>
#include <asio/post.hpp>
#include <future>
#include <iostream>

namespace magpie::transport {

TCPServer::TCPServer(
    short port,
    unsigned int concurrency
): 
    ctx(concurrency),
    ipv4Acceptor(
        ctx,
        asio::ip::tcp::endpoint(
            asio::ip::tcp::v4(), port
        )
    ),
    concurrency(concurrency)
{
    asio::error_code err;
    if (ipv4Acceptor.listen(
        asio::ip::tcp::acceptor::max_listen_connections,
        err
    )) {
        throw std::runtime_error(
            "Failed to listen on port: " + err.message()
        );
    }
}

TCPServer::~TCPServer() {
    ipv4Acceptor.close();
}

void TCPServer::doAccept() {
    // TODO: I do not like this pattern. Fix
    auto conn = std::make_shared<Connection>(ctx);
    ipv4Acceptor.async_accept(
        conn->socket,
        // TODO: asio has built-in C++20 coroutine support. Figure out how to shoehorn it in here
        // (or figure out how to add C++20 coroutines some other way)
        [conn, this](const asio::error_code& err) {
            if (!err) {
                conn->start();
            } else {
                std::cout << "Connection error: " << err.message() << "\n";
            }

            this->doAccept();
        }
    );
}

void TCPServer::start(
    const std::function<
        void(const Request&)
    >&
) {
    std::vector<std::future<void>> threads;
    for (unsigned int i = 0; i < this->concurrency; ++i) {
        threads.push_back(
            std::async(
                std::bind(&TCPServer::doAccept, this)
            )
        );
    }

    this->ctx.run();
}

}
