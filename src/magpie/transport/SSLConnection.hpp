#pragma once

#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

#include "magpie/transport/BaseConnection.hpp"
#include "magpie/application/Http2Adapter.hpp"
#include <asio.hpp>

namespace magpie { class BaseApp; }
namespace magpie::transport {

using TCPSocket = asio::ip::tcp::socket;
using SSLSocketWrapper = asio::ssl::stream<TCPSocket>;

class SSLConnection 
    : public CommonConnection<SSLSocketWrapper, SSLSocketWrapper::lowest_layer_type>
{
private:
    SSLSocketWrapper socket;
public:

    SSLConnection(
        BaseApp* app,
        asio::io_context& ctx,
        asio::ssl::context& sslContext
    ) :
        CommonConnection(
            app
        ),
        socket(ctx, sslContext)
    {
    }

    ~SSLConnection() = default;

    SSLSocketWrapper& getSocket() override {
        return socket;
    }

    SSLSocketWrapper::lowest_layer_type& getRawSocket() override {
        return socket.next_layer(); 
    }
};

}
