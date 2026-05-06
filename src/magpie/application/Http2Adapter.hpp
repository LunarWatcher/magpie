#pragma once

#include "magpie/application/Adapter.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
#include <nghttp2/nghttp2.h>

#include <openssl/ssl.h>

namespace magpie { class BaseApp; }

namespace magpie::transport {
class BaseConnection;
}

namespace magpie::application {

struct UserData {
    raven::Connection* conn;
    BaseApp* app;

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
    UserData data;
public:
    Http2Adapter(
        raven::Connection* conn,
        BaseApp* app
    );
    ~Http2Adapter();

    virtual bool parse(
        const raven::Buffer& buff,
        std::size_t readBytes
    ) override;

    virtual bool onWriteReady(
        raven::Connection* conn,
        raven::Buffer&
    ) override;
};

}
