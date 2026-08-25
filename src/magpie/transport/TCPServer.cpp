#include "TCPServer.hpp"
#include "magpie/application/Http2Adapter.hpp"
#include "magpie/application/http2/Nghttp2Callbacks.hpp"
#include "magpie/logger/Logger.hpp"
#include "raven/SocketServer.hpp"
#include <magpie/App.hpp>
#include <stdexcept>
#include <openssl/ssl.h>

namespace magpie::transport {

TCPServer::TCPServer(
    BaseApp* app,
    uint16_t port,
    unsigned int concurrency,
    const std::string& bindAddr,
    std::optional<raven::SSLConfig>&& sslConfig
) : concurrency(concurrency),
    app(app),
    hasSSL(sslConfig.has_value()),
    serv(
        raven::SocketConfig {
            .type = raven::SocketType::Stream,
            .port = port,
            .ip = bindAddr,
            .sslConfig = std::move(
                this->app->getConfig().adapterFactory->withSslConfigExtras(std::move(sslConfig))
            ),
        },
        raven::ServerConfig {
            .threads = concurrency,
        },
        raven::ConnPoolConfig {
            .onRecv = [this](
                raven::Connection* conn,
                const raven::Buffer& buff,
                size_t availableBytes
            ) {
                if (conn->userData == nullptr) {
                    conn->userData = this->app->getConfig().adapterFactory->get(conn, this->app);
                }
                static_pointer_cast<application::Adapter>(
                    conn->userData
                )->parse(buff, availableBytes);
            },
            .onWriteReady = [](auto* conn, auto& buff) {
                if (conn->userData == nullptr) {
                    return;
                }
                static_pointer_cast<application::Adapter>(
                    conn->userData
                )->onWriteReady(conn, buff);
            },
            .onWriteComplete = [](auto*) {},
        }
    )
{
    if (concurrency < 1) {
        throw std::runtime_error("You're trying to run nothing");
    }
}

TCPServer::~TCPServer() {
    stop();
}

void TCPServer::start() {

    logger::info(
        "TCPServer listening on {}://{}:{}",
        this->hasSSL ? "https" : "http",
        this->serv.getAssignedAddr(),
        this->serv.getPort()
    );
    this->serv.start();

    this->serv.waitForDone();

    logger::info("Shutting down...");
}

void TCPServer::stop() {
    this->serv.close();
}

uint16_t TCPServer::getPort() {
    return this->serv.getPort();
}

}
