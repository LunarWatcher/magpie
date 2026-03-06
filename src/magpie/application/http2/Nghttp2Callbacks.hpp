#pragma once

#include "magpie/config/SSLConfig.hpp"
#include <nghttp2/nghttp2.h>
namespace magpie::application::_detail {

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

extern int onClientHello(SSL* ssl, int* al, void* arg);

extern int onStreamClose(
    nghttp2_session *session,
    int32_t stream_id,
    uint32_t error_code,
    void *user_data
);

}

