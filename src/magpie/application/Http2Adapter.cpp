#include "Http2Adapter.hpp"

#include <nghttp2/nghttp2.h>
#include "magpie/transport/Connection.hpp"
#include "magpie/App.hpp"
#include <string>

namespace magpie::application {

Http2Adapter::Http2Adapter(transport::Connection* conn): conn(conn) {

    if (auto result = nghttp2_session_callbacks_new(
        &callbacks
    ); result != 0) {
        std::cerr << "Failed to create callback object: " << result << std::endl;
    }
    // TODO: deprecated, but the deprecation has not hit mint yet
    // Looks like the fix is just adding a 2 at the end and nothing else, but I'm too eepy to figure out how to macro
    // that right now. Especially because that involves ✨ testing ✨ with stuff I don't have access to, and I'm busy
    // enough with cursed testing setups at work
    nghttp2_session_callbacks_set_send_callback(
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

    data.conn = conn;
    if (auto result = nghttp2_session_server_new(
        &sess,
        callbacks,
        &data
    ); result != 0) {
        std::cerr << "Failed to create session: " << result << std::endl;
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

void Http2Adapter::parse(
    const std::array<char, 4096>& buff,
    std::size_t readBytes
) {
    nghttp2_session_mem_recv(
        this->sess,
        (const uint8_t*) buff.data(),
        readBytes
    );
    nghttp2_session_send(
        this->sess
    );
}

ssize_t _detail::onSend(
    nghttp2_session*,
    const uint8_t* data,
    size_t length,
    int,
    void* userData
) {
    auto* conn = static_cast<UserData*>(userData)->conn;
    return asio::write(
        conn->socket,
        asio::buffer(data, length)
    );

}

int _detail::onFrame(
    nghttp2_session* sess,
    const nghttp2_frame* frame,
    void* userData
) {
    auto& ud = *static_cast<UserData*>(userData);
    auto* conn = ud.conn;
    if (frame->hd.type == NGHTTP2_HEADERS &&
        frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
        std::cout << "End of stream" << std::endl;
    }
    if (
        frame->hd.type == NGHTTP2_HEADERS
        && frame->headers.cat == NGHTTP2_HCAT_REQUEST
    ) {
        std::cout << "Sending response" << std::endl;

        int32_t stream_id = frame->hd.stream_id;
        std::vector<nghttp2_nv> nva;
        // TODO: this is dumb and you should feel bad
        auto makeNv = [](const std::string &name, const std::string &value) {
            nghttp2_nv nv;
            nv.name = (uint8_t*)name.c_str();
            nv.value = (uint8_t*)value.c_str();
            nv.namelen = name.size();
            nv.valuelen = value.size();
            nv.flags = NGHTTP2_NV_FLAG_NONE;
            return nv;
        };
        // These need to be cached as strings because lifecycle, and nghttp2 doesn't seem to copy
        std::string a = ":status", b = "200",
            c = "content-type", d = "text/plain",
            e = "server", f = "trans-rights-are-human-rights";
        nva.push_back(makeNv(a, b));
        nva.push_back(makeNv(c, d));
        nva.push_back(makeNv(e, f));
        // Looks like this is where we'd feed in arbitrary data from the server
        auto app = conn->app;
        if (app == nullptr) {
            [[unlikely]]
            throw std::runtime_error("Critical developer error");
        }
        const auto& router = app->getRouter();
        auto& headers = ud.headers[frame->hd.stream_id];
        auto& destination = headers.at(":path");

        router.invokeRoute(destination);

        nghttp2_data_provider dp;
        dp.read_callback = [](
            nghttp2_session *,
            int32_t,
            uint8_t *buf, size_t length,
            uint32_t *data_flags,
            nghttp2_data_source*,
            void *
        ) -> ssize_t {
            std::string response = "Hewwo world :3";
            size_t len = std::min(length, response.size());
            std::memcpy(buf, response.c_str(), len);
            *data_flags = NGHTTP2_DATA_FLAG_EOF;
            return (ssize_t) len;
        };
        int rv = nghttp2_submit_response(
            sess,
            stream_id,
            nva.data(),
            nva.size(),
            &dp
        );
        if (rv != 0) {
            std::cerr << "Failed to submit: " << nghttp2_strerror(rv) << "\n";
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        rv = nghttp2_session_send(sess);
        if (rv != 0) {
            std::cerr << "Failed to send: " << nghttp2_strerror(rv) << "\n";
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
    }
    return 0;
}

int _detail::onHeaders(
    nghttp2_session*,
    const nghttp2_frame *frame,
    const uint8_t* name, size_t namelen,
    const uint8_t* value, size_t valuelen,
    uint8_t, void* userData
) {
    auto& ud = *static_cast<UserData*>(userData);
    if (
        frame->hd.type == NGHTTP2_HEADERS
        && frame->headers.cat == NGHTTP2_HCAT_REQUEST
    ) {
        // TODO: string_view? We probably want to copy everything into std::strings for ownership though
        std::string n((const char*) name, namelen);
        std::string v((const char*) value, valuelen);
        std::cout << "Header: " << n << " = " << v << "\n";

        ud.headers[frame->hd.stream_id][n] = v;

        if (n == "content-length") {
            // TODO: reserve size in request object
        }
    }
    return 0;
}


int _detail::onChunkRecv(
    nghttp2_session*,
    uint8_t,
    int32_t,
    const uint8_t* data,
    size_t len,
    void*
) {
    std::cout << "Chunk size " << len << " with content "
        << std::string(
            (const char*) data,
            len
        )
        << std::endl;
    return 0;
}

}
