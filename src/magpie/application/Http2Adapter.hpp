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

namespace _detail {

extern nghttp2_ssize onSend(
    nghttp2_session* sess,
    const uint8_t* data, 
    size_t length,
    int, void*
);

extern int onChunkRecv(
    nghttp2_session *session,
    uint8_t flags,
    int32_t streamId,
    const uint8_t *data,
    size_t len,
    void *user_data
);

extern int onHeaders(
    nghttp2_session *session,
    const nghttp2_frame *frame,
    const uint8_t *name, size_t namelen,
    const uint8_t *value, size_t valuelen,
    uint8_t flags, void *
);

extern int onFrame(
    nghttp2_session* sess,
    const nghttp2_frame* frame,
    void*
);

extern int onAlpnSelectProto(
    SSL* ssl, const unsigned char** out,
    unsigned char* outLen, const unsigned char* in, 
    unsigned int inLen, void* arg
);

extern int onStreamClose(
    nghttp2_session *session,
    int32_t stream_id,
    uint32_t error_code,
    void *user_data
);

}

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

    virtual void parse(
        const ReadBuffer& buff,
        std::size_t readBytes
    ) override;
};

}
