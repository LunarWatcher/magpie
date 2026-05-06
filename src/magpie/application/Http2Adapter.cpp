#include "Http2Adapter.hpp"

#include "magpie/application/http2/Nghttp2Callbacks.hpp"

#include <nghttp2/nghttp2.h>
#include "magpie/application/Methods.hpp"
#include "magpie/transfer/StatusCode.hpp"
#include "magpie/transfer/adapters/DataAdapter.hpp"
#include "magpie/App.hpp"
#include "magpie/utility/ErrorHandler.hpp"
#include "magpie/logger/Logger.hpp"
#include <openssl/comp.h>
#include <openssl/tls1.h>
#include <stdexcept>

#include <openssl/ssl.h>
#include <openssl/err.h>

namespace magpie::application {

Http2Adapter::Http2Adapter(raven::Connection* conn, BaseApp* app)
    : app(app) {

    if (auto result = nghttp2_session_callbacks_new(
        &callbacks
    ); result != 0) {
        logger::error("Failed to create callback object: {}", result);
        throw std::runtime_error("Critical: callback object creation failed");
    }
    nghttp2_session_callbacks_set_send_callback2(
        callbacks,
        &_detail::onSend
    );
    nghttp2_session_callbacks_set_on_frame_recv_callback(
        callbacks,
        &_detail::onFrame
    );
    nghttp2_session_callbacks_set_on_data_chunk_recv_callback(
        callbacks,
        &_detail::onChunkRecv
    );

    nghttp2_session_callbacks_set_on_header_callback(
        callbacks,
        _detail::onHeaders
    );
    nghttp2_session_callbacks_set_on_stream_close_callback(
        callbacks,
        _detail::onStreamClose
    );

    data.conn = conn;
    data.app = app;
    if (auto result = nghttp2_session_server_new(
        &sess,
        callbacks,
        &data
    ); result != 0) {
        logger::error("Failed to create session: {}", result);
        throw std::runtime_error("Session init error");
    }

    nghttp2_settings_entry settings[] = {
        {NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 1}
    };
    nghttp2_submit_settings(
        sess,
        NGHTTP2_FLAG_NONE,
        settings,
        1
    );
    // nghttp2_session_send(sess);

}

Http2Adapter::~Http2Adapter() {
    if (this->sess != nullptr) {
        nghttp2_session_del(
            this->sess
        );
    }
    if (callbacks != nullptr) {
        nghttp2_session_callbacks_del(
            this->callbacks
        );
    }
}

bool Http2Adapter::parse(
    const raven::Buffer& buff,
    std::size_t readBytes
) {
    if (readBytes > 0) {
        nghttp2_session_mem_recv2(
            this->sess,
            (const uint8_t*) buff.data(),
            readBytes
        );
    }
    nghttp2_session_send(
        this->sess
    );

    return nghttp2_session_want_read(this->sess) == 0
        && nghttp2_session_want_write(this->sess) == 0;
}


bool Http2Adapter::onWriteReady(
    raven::Connection*,
    raven::Buffer&
) {
    nghttp2_session_send(
        this->sess
    );
    return true;
}

}
