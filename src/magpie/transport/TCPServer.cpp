#include "TCPServer.hpp"
#include "magpie/config/SSLConfig.hpp"
#include "magpie/logger/Logger.hpp"
#include "magpie/transport/Connection.hpp"
#include "magpie/App.hpp"
#include "magpie/transport/SSLConnection.hpp"
#include "magpie/utility/ErrorHandler.hpp"
#include <asio/error_code.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/address_v4.hpp>
#include <asio/post.hpp>

namespace magpie::transport {

TCPServer::TCPServer(
    BaseApp* app,
    uint16_t port,
    unsigned int concurrency,
    std::string_view bindAddr
): 
    ctx(concurrency),
    ipv4Acceptor(
        ctx,
        asio::ip::tcp::endpoint(
            asio::ip::make_address(bindAddr),
            port
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
        SSL_CTX_set_alpn_protos(
            ctx.native_handle(),
            (const unsigned char*)"\x02h2",
            3
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
    this->ctx.stop();
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
                utility::runWithErrorLogging([&]() {
                    if (!err) {
                        conn->start();
                    } else {
                        logger::error(
                            "Connection error: {}",
                            err.message()
                        );
                    }
                });
                this->doAccept();
            }
        );
    } else {
        // TODO: I do not like this pattern. Fix
        auto conn = std::make_shared<SSLConnection>(
            this->app,
            ctx,
            this->sslCtx.value()
        );
        // TODO: std::make_shared eradicates type hinting, which makes this signature better:
        // auto conn = std::shared_ptr<SSLConnection>(
        //     new SSLConnection(this->app, ctx, this->sslCtx.value())
        // );
        // But I vaguely remember there being downsides. Probably worth figuring it out and writing notes on it
        ipv4Acceptor.async_accept(
            conn->getRawSocket(),
            // TODO: asio has built-in C++20 coroutine support. Figure out how to shoehorn it in here
            // (or figure out how to add C++20 coroutines some other way)
            [conn, this](const asio::error_code& err) {
                utility::runWithErrorLogging([&]() {
                    if (!err) {
                        conn->getSocket()
                            .handshake(SSLSocketWrapper::server);
                        conn->start();
                    } else {
                        logger::error(
                            "Connection error: {}",
                            err.message()
                        );
                    }
                });
                this->doAccept();
            }
        );
    }
}

void TCPServer::start() {
    // Pretty sure this is at least partly wrong. Looks like ctx::run has to be called multiple times to create a thread
    // pool. But then that should correspond to the number of handelrs available, which suggests this is necessary?
    // Asio is fucking weird
    for (unsigned int i = 0; i < this->concurrency; ++i) {
        doAccept();
    }

    logger::info(
        "TCPServer listening on {}://{}:{}",
        this->sslCtx.has_value() ? "https" : "http",
        this->ipv4Acceptor.local_endpoint().address().to_string(),
        this->ipv4Acceptor.local_endpoint().port()
    );
    // TODO: is this actually multithreaded?
    this->ctx.run();
}

void TCPServer::stop() {
    this->ctx.stop();
}

uint16_t TCPServer::getPort() {
    return this->ipv4Acceptor.local_endpoint().port();
}

}
