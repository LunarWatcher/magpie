#pragma once

#include "magpie/application/Adapter.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
#include "magpie/transport/TCPServer.hpp"
#include <nghttp2/nghttp2.h>

#include <openssl/ssl.h>

namespace magpie { class BaseApp; }

namespace magpie::transport {
class BaseConnection;
}

namespace magpie::application {

struct UserData {
    transport::BaseConnection* conn;

    /**
     * Maps stream IDs to requests.
     */
    std::unordered_map<int, std::shared_ptr<Request>> requests;
    std::unordered_map<int, std::shared_ptr<Response>> responses;
    std::unordered_map<int, size_t> writeOffsets;
};

class Http2Adapter : public Adapter {
private:
    nghttp2_session* sess;
    nghttp2_session_callbacks* callbacks;

    BaseApp* app;
    transport::BaseConnection* conn;
    UserData data;

    void createSslContext();
public:
    Http2Adapter(
        transport::BaseConnection* conn
    );
    ~Http2Adapter();

    virtual bool parse(
        const ReadBuffer& buff,
        std::size_t readBytes
    ) override;
};

}
