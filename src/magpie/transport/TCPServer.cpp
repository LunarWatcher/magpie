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
    asio::error_code err;
    ipv4Acceptor.listen(
        asio::ip::tcp::acceptor::max_listen_connections,
        err
    );

    // Technically, this pattern means the minimum concurrency is 2
    for (size_t i = 0; i < concurrency; ++i) {
        this->workerContexts.push_back(
            std::make_unique<internals::Worker>(app->getConfig())
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
    internals::Worker* min = nullptr;

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
        [[unlikely]]
        throw std::runtime_error("Panic: Failed to resolve worker");
    }

    return min;
}

void TCPServer::doAccept() {
    // TODO: I hate this pattern
    internals::Worker* worker = getWorker();
    std::shared_ptr<BaseConnection> conn;
    if (!worker->sslContext.has_value()) {
        conn = std::make_shared<Connection>(
            this->app,
            worker
        );
    } else {
        conn = std::make_shared<SSLConnection>(
            this->app,
            worker,
            worker->sslContext.value()
        );
    }


    conn->asyncAccept(
        ipv4Acceptor,
        // TODO: asio has built-in C++20 coroutine support. Figure out how to shoehorn it in here
        // (or figure out how to add C++20 coroutines some other way)
        [worker, conn, this](const asio::error_code& err) {
            worker->workload.fetch_add(1);
            if (!err) {
                asio::post(worker->ioContext, [conn]() {
                    conn->handshake();
                    utility::runWithErrorLogging([&]() {
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
        this->app->getConfig().ssl.has_value() ? "https" : "http",
        this->ipv4Acceptor.local_endpoint().address().to_string(),
        this->ipv4Acceptor.local_endpoint().port()
    );
    std::vector<std::thread> threads;
    threads.reserve(this->concurrency);
    for (unsigned int i = 0; i < this->concurrency; ++i) {
        threads.push_back(std::thread([this, i]() {
            while (true) {
                try {
                    if (this->workerContexts.at(i)->ioContext.run() == 0) {
                        break;
                    }
                } catch (std::exception& e) {
                    logger::error("Exception in worker: {}", e.what());
                }
            }
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
        auto& context = worker->ioContext;

        if (!context.stopped()) {
            context.stop();
        }
    }
}

uint16_t TCPServer::getPort() {
    return this->ipv4Acceptor.local_endpoint().port();
}

}
