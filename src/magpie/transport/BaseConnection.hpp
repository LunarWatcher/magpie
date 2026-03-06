#pragma once

#include "magpie/application/Adapter.hpp"
#include "magpie/transport/Worker.hpp"
#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

#include "magpie/application/Http2Adapter.hpp"
#include "magpie/logger/Logger.hpp"
#include <memory>

namespace magpie {
class BaseApp;
}

namespace magpie::transport {

class BaseConnection {
public:
    application::ReadBuffer recv;
    // TODO: make the protocol dynamic
    // TODO: Alternatively: Support multiple. ALPN should make this fairly easy to do
    application::Http2Adapter adapter;
    BaseApp* app;

    BaseConnection(BaseApp* app) : recv(), adapter(this), app(app) {}
    virtual ~BaseConnection() = default;

    virtual size_t write(
        const asio::const_buffer& buff
    ) = 0;
    virtual void start() = 0;

    virtual void doRead() = 0;

    /**
     * Runs a handshake. This only needs to be defined for connections that have a handshake process. There is an empty
     * default implementation that does nothing.
     */
    virtual void handshake() {}

    virtual void asyncAccept(
        asio::ip::tcp::acceptor& acceptor,
        const std::function<void(const asio::error_code&)>& callback
    ) = 0;

    virtual std::string getIPAddr() = 0;

};

template <typename SocketType, typename NativeType = SocketType>
class CommonConnection
    : public BaseConnection,
      public std::enable_shared_from_this<CommonConnection<SocketType, NativeType>>
{
private:
    internals::Worker* worker;
public:

    CommonConnection(
        BaseApp* app,
        internals::Worker* worker
    ) : BaseConnection(app), worker(worker) {}

    virtual ~CommonConnection() {
        worker->workload.fetch_sub(1);
    }

    virtual NativeType& getRawSocket() = 0; 
    virtual SocketType& getSocket() = 0; 

    size_t write(
        const asio::const_buffer& buff
    ) override {
        asio::error_code ec;
        size_t written = asio::write(
            getSocket(),
            buff,
            ec
        );

        if (ec) {
            logger::error("{}", ec.message());
            return std::numeric_limits<size_t>::max();
        }
        return written;
    }

    void doRead() override {
        std::shared_ptr<BaseConnection> self = this->shared_from_this();
        getSocket().async_read_some(
            asio::buffer(recv),
            [self](const asio::error_code& ec, size_t bytes) {
                if (!ec && bytes > 0) {
                    if (!self->adapter.parse(
                        self->recv,
                        bytes
                    )) {
                        self->doRead();
                    }
                } else if (ec) {
                    logger::error("Read failed: {}", ec.message());
                }
            }
        );
    }

    virtual void asyncAccept(
        asio::ip::tcp::acceptor& acceptor,
        const std::function<void(const asio::error_code&)>& callback
    ) override {
        acceptor.async_accept(getRawSocket(), callback);
    }

    void start() override {
        doRead();
    }

    std::string getIPAddr() override {
        return getRawSocket().remote_endpoint().address().to_string();
    }
};

}
