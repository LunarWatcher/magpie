#include "TCPServer.hpp"
#include "magpie/config/SSLConfig.hpp"
#include "magpie/logger/Logger.hpp"
#include "magpie/transport/Connection.hpp"
#include "magpie/App.hpp"
#include "magpie/transport/SSLConnection.hpp"
#include <asio/error_code.hpp>
#include <asio/post.hpp>
#include <future>
#include <iostream>

namespace magpie::transport {

TCPServer::TCPServer(
    BaseApp* app,
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
    concurrency(concurrency),
    app(app)
{
    this->sslCtx = app->getConfig().ssl.and_then([](const SSLConfig& ssl) {
        asio::ssl::context ctx(
            asio::ssl::context::sslv23
        );
        ctx.use_certificate_chain_file(ssl.certFile);
        ctx.use_private_key_file(ssl.keyFile, asio::ssl::context::pem);

        SSL_CTX_set_alpn_select_cb(
            ctx.native_handle(),
            application::_detail::onAlpnSelectProto,
            nullptr
        );
        return std::optional(std::move(ctx));
    }) ;
    asio::error_code err;
    ipv4Acceptor.listen(
        asio::ip::tcp::acceptor::max_listen_connections,
        err
    );
    if (err) {
        throw std::runtime_error(
            "Failed to listen on port: " + err.message()
        );
    }
}

TCPServer::~TCPServer() {
    ipv4Acceptor.close();
}

void TCPServer::doAccept() {
    // TODO: I hate this pattern
    if (!this->sslCtx.has_value()) {
        // TODO: I do not like this pattern. Fix
        auto conn = std::make_shared<Connection>(this->app, ctx);
        ipv4Acceptor.async_accept(
            conn->getRawSocket(),
            // TODO: asio has built-in C++20 coroutine support. Figure out how to shoehorn it in here
            // (or figure out how to add C++20 coroutines some other way)
            [conn, this](const asio::error_code& err) {
                if (!err) {
                    conn->start();
                } else {
                    logger::error(
                        "Connection error: {}",
                        err.message()
                    );
                }

                this->doAccept();
            }
        );
    } else {
        // TODO: I do not like this pattern. Fix
        // auto conn = std::make_shared<SSLConnection>(
        //     this->app,
        //     ctx,
        //     this->sslCtx.value()
        // );
        auto conn = std::shared_ptr<SSLConnection>(
            new SSLConnection(this->app, ctx, this->sslCtx.value())
        );
        ipv4Acceptor.async_accept(
            conn->getRawSocket(),
            // TODO: asio has built-in C++20 coroutine support. Figure out how to shoehorn it in here
            // (or figure out how to add C++20 coroutines some other way)
            [conn, this](const asio::error_code& err) {
                conn->getSocket()
                    .handshake(SSLSocketWrapper::server);

                if (!err) {
                    conn->start();
                } else {
                    logger::error(
                        "Connection error: {}",
                        err.message()
                    );
                }

                this->doAccept();
            }
        );
    }
}

void TCPServer::start() {
    std::vector<std::future<void>> threads;
    for (unsigned int i = 0; i < this->concurrency; ++i) {
        threads.push_back(
            std::async(
                std::bind(&TCPServer::doAccept, this)
            )
        );
    }

    logger::info(
        "TCPServer listening on http://{}:{}",
        this->ipv4Acceptor.local_endpoint().address().to_string(),
        this->ipv4Acceptor.local_endpoint().port()
    );
    this->ctx.run();
}

}
