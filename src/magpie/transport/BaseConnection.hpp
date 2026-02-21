#pragma once

#include "magpie/application/Adapter.hpp"
#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

#include "magpie/application/Http2Adapter.hpp"
#include "magpie/logger/Logger.hpp"
#include <array>
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

    virtual std::string getIPAddr() = 0;

};

template <typename SocketType, typename NativeType = SocketType>
class CommonConnection
    : public BaseConnection,
      public std::enable_shared_from_this<CommonConnection<SocketType, NativeType>>
{
public:

    CommonConnection(
        BaseApp* app
    ) : BaseConnection(app) {}

    virtual ~CommonConnection() = default;

    virtual NativeType& getRawSocket() = 0; 
    virtual SocketType& getSocket() = 0; 

    size_t write(
        const asio::const_buffer& buff
    ) override {
        return asio::write(
            getSocket(),
            buff
        );
    }

    void doRead() override {
        auto self = this->shared_from_this();
        getSocket().async_read_some(
            asio::buffer(recv),
            [self](const asio::error_code& ec, size_t bytes) {
                if (!ec && bytes > 0) {
                    self->adapter.parse(
                        self->recv,
                        bytes
                    );

                    self->doRead();
                }
            }
        );
    }

    void start() override {
        doRead();
    }

    std::string getIPAddr() override {
        return getRawSocket().remote_endpoint().address().to_string();
    }
};

}
