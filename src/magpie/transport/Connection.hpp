#pragma once

#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

#include "magpie/transport/BaseConnection.hpp"
#include "magpie/application/Http2Adapter.hpp"
#include <asio.hpp>

namespace magpie { class BaseApp; }
namespace magpie::transport {

class Connection : public CommonConnection<asio::ip::tcp::socket> {
private:
    asio::ip::tcp::socket socket;
public:

    Connection(
        BaseApp* app,
        internals::Worker* worker
    ) :
        CommonConnection(
            app, worker
        ),
        socket(worker->ioContext)
    {
    }
    ~Connection() = default;

    decltype(socket)& getSocket() override { return socket; }
    decltype(socket)& getRawSocket() override { return socket; }
};

}
