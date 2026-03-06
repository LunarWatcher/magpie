#include "TCPServer.hpp"
#include "magpie/config/SSLConfig.hpp"
#include "magpie/logger/Logger.hpp"
#include "magpie/transport/Connection.hpp"
#include "magpie/App.hpp"
#include "magpie/transport/SSLConnection.hpp"
#include "magpie/transport/Worker.hpp"
#include "magpie/utility/ErrorHandler.hpp"
#include <asio/error_code.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/address_v4.hpp>
#include <asio/post.hpp>
#include <limits>
#include <openssl/ssl.h>

namespace magpie::transport {

TCPServer::TCPServer(
    BaseApp* app,
    uint16_t port,
    unsigned int concurrency,
    std::string_view bindAddr
): 
    coreContext(),
    ipv4Acceptor(
        coreContext,
        asio::ip::tcp::endpoint(
            asio::ip::make_address(bindAddr),
            port
        )
    ),
    concurrency(concurrency),
    app(app)
{
    if (concurrency < 1) {
        throw std::runtime_error("You're trying to run nothing");
    }
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
        SSL_CTX_set_client_hello_cb(
            ctx.native_handle(),
            application::_detail::onClientHello,
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

    // Technically, this pattern means the minimum concurrency is 2
    for (size_t i = 0; i < concurrency; ++i) {
        this->workerContexts.push_back(
            std::make_unique<internals::Worker>()
        );
    }

    if (err) {
        throw std::runtime_error(
            "Failed to listen on port: " + err.message()
        );
    }
}

TCPServer::~TCPServer() {
    stop();
    ipv4Acceptor.close();
}

internals::Worker* TCPServer::getWorker() {
    uint32_t minWorkload = std::numeric_limits<uint32_t>::max();
    internals::Worker* min;

    if (this->workerContexts.size() == 0) {
        [[unlikely]]
        throw std::runtime_error("This should never happen");
    }

    for (auto& worker : workerContexts) {
        uint32_t workload = worker->workload.load();
        if (workload < minWorkload) {
            minWorkload = workload;
            min = worker.get();
        }
    }

    if (min == nullptr) {
        throw std::runtime_error("Panic: Failed to resolve worker");
    }

    return min;
}

void TCPServer::doAccept() {
    // TODO: I hate this pattern
    std::shared_ptr<BaseConnection> conn;
    if (!this->sslCtx.has_value()) {
        conn = std::make_shared<Connection>(this->app, coreContext);
    } else {
        conn = std::make_shared<SSLConnection>(
            this->app,
            coreContext,
            this->sslCtx.value()
        );
    }

    internals::Worker* worker = getWorker();

    conn->asyncAccept(
        ipv4Acceptor,
        // TODO: asio has built-in C++20 coroutine support. Figure out how to shoehorn it in here
        // (or figure out how to add C++20 coroutines some other way)
        [worker, conn, this](const asio::error_code& err) {
                if (!err) {
                    asio::post(worker->context, [conn]() {
                        utility::runWithErrorLogging([&]() {
                            conn->handshake();
                            conn->start();
                        });
                    });
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

void TCPServer::start() {
    doAccept();

    logger::info(
        "TCPServer listening on {}://{}:{}",
        this->sslCtx.has_value() ? "https" : "http",
        this->ipv4Acceptor.local_endpoint().address().to_string(),
        this->ipv4Acceptor.local_endpoint().port()
    );
    std::vector<std::thread> threads;
    threads.reserve(this->concurrency);
    for (unsigned int i = 0; i < this->concurrency; ++i) {
        threads.push_back(std::thread([this, i]() {
            while (this->workerContexts.at(i)->context.run() != 0) {}
            logger::debug("Worker thread {} shutting down", i);
        }));
    }
    coreContext.run();

    for (auto& thread : threads) {
        thread.join();
    }

    logger::info("Shutting down...");
}

void TCPServer::stop() {
    if (!coreContext.stopped()) {
        this->coreContext.stop();
    }

    for (auto& worker : workerContexts) {
        auto& context = worker->context;

        if (!context.stopped()) {
            context.stop();
        }
    }
}

uint16_t TCPServer::getPort() {
    return this->ipv4Acceptor.local_endpoint().port();
}

}
