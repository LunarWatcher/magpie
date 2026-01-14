#pragma once

#include "magpie/application/Adapter.hpp"
#include <nghttp2/nghttp2.h>

namespace magpie::transport {
class Connection;
}

namespace magpie::application {

namespace _detail {

extern ssize_t onSend(
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

}

class Http2Adapter : public Adapter {
private:
    nghttp2_session* sess;
    nghttp2_session_callbacks* callbacks;

    transport::Connection* conn;
public:
    Http2Adapter(
        transport::Connection* conn
    );
    ~Http2Adapter();

    virtual void parse(
        const std::array<char, 4096>& buff,
        std::size_t readBytes
    ) override;
};

}
