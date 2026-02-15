#include "Http2Adapter.hpp"

#include <nghttp2/nghttp2.h>
#include "magpie/application/Methods.hpp"
#include "magpie/transfer/StatusCode.hpp"
#include "magpie/transfer/adapters/DataAdapter.hpp"
#include "magpie/transport/Connection.hpp"
#include "magpie/transport/BaseConnection.hpp"
#include "magpie/App.hpp"
#include "magpie/utility/ErrorHandler.hpp"
#include <openssl/comp.h>
#include <stdexcept>
#include <string>
#include <zlib-ng.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

namespace magpie::application {

Http2Adapter::Http2Adapter(transport::BaseConnection* conn) :
    app(conn->app),
    conn(conn)
{

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
    logger::info("Killing connection");
}

void Http2Adapter::parse(
    const std::array<char, 4096>& buff,
    std::size_t readBytes
) {
    nghttp2_session_mem_recv2(
        this->sess,
        (const uint8_t*) buff.data(),
        readBytes
    );
    nghttp2_session_send(
        this->sess
    );
}

nghttp2_ssize _detail::onSend(
    nghttp2_session*,
    const uint8_t* data,
    size_t length,
    int,
    void* userData
) {
    auto* conn = static_cast<UserData*>(userData)->conn;
    return conn->write(
        asio::buffer(data, length)
    );

}

int _detail::onFrame(
    nghttp2_session* sess,
    const nghttp2_frame* frame,
    void* userData
) {
    // These variables are used to manage the lifecycle of the strings we hard-code.
    // Not sure if these are actually necessary, but they need to be constants anyway.
    static std::string HTTP2_STATUS_HEADER = ":status";
    static std::string CONTENT_TYPE_HEADER = "content-type";
    static std::string CONTENT_ENCODING = "content-encoding";

    auto& ud = *static_cast<UserData*>(userData);
    auto* conn = ud.conn;
    if (frame->hd.type == NGHTTP2_HEADERS &&
        frame->hd.flags & NGHTTP2_FLAG_END_HEADERS) {
        logger::debug("End of header stream");
        // TODO: doing it this way enables us to stream bodies, but hooking up this particular state machine would be
        // involved. Not sure if this is desirable. `onFrame` would have to return, so the control flow works entirely
        // differently, or would spawn a sub-task.
    }

    if (
        (
            frame->hd.type == NGHTTP2_DATA
            || frame->hd.type == NGHTTP2_HEADERS
        )
        && frame->hd.flags & NGHTTP2_FLAG_END_STREAM
    ) {
        logger::debug("Starting response");

        int32_t streamId = frame->hd.stream_id;
        std::vector<nghttp2_nv> nva;

        // TODO: this is dumb and you should feel bad
        auto makeNv = [](const std::string &name, const std::string &value) {
            nghttp2_nv nv;
            nv.name = (uint8_t*) name.c_str();
            nv.value = (uint8_t*) value.c_str();
            nv.namelen = name.size();
            nv.valuelen = value.size();
            nv.flags = NGHTTP2_NV_FLAG_NONE;
            return nv;
        };

        auto app = conn->app;
        if (app == nullptr) {
            [[unlikely]]
            throw std::runtime_error("Critical developer error");
        }
        const auto& router = app->getRouter();
        auto& request = ud.requests.at(streamId);
        auto& response = ud.responses.at(streamId);
        if (request == nullptr) {
            logger::critical("Failure: request is nullptr");
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        } else if (response == nullptr) {
            logger::critical("Failure: response is nullptr");
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        // 3 * 2: :status, content-type, and content-encoding, * 2 for buffer
        // 
        // Doing this avoids copies, which is necessary to avoid UB
        // 
        nva.reserve(6 + response->headers.size());

        auto& headers = request->headers;
        auto& destination = headers.at(":path");

        const auto& config = app->getConfig();

        if (!config.trustXRealIp) {
            request->ipAddr = conn->getIPAddr();
        } else {
            auto header = headers.find("x-real-ip");
            if (header == headers.end()) {
                request->ipAddr = conn->getIPAddr();
            } else {
                request->ipAddr = header->second;
            }
        }

        utility::runWithErrorLogging([&]() {
            router.invokeRoute(
                destination,
                app->getContext(),
                *request,
                *response
            );
        }, response.get());

        // Pseudo-headers {{{
        // > ... and must place pseudo-headers before regular header fields.
        // https://nghttp2.org/documentation/nghttp2_submit_response2.html#c.nghttp2_submit_response2
        //
        // Pretty sure the :status header is the only pseudo-header the server needs to care about. The client can send
        // three more: https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages#pseudo-headers
        // But this is a server, so don't need to care.
        std::string codeAsStr = *response->code;
        nva.push_back(makeNv(HTTP2_STATUS_HEADER, codeAsStr));
        // }}}

        // Not a pseudo-header
        nva.push_back(makeNv(CONTENT_TYPE_HEADER, response->contentType));

        for (auto& [header, value] : response->headers) {
            nva.push_back(makeNv(header, value));
        }

        if (response->code == nullptr) {
            [[unlikely]]
            logger::critical(
                "res.code is nullptr. There's physically no way for this to happen unless you've done something "
                "deeply fucking cursed that you should undo right now."
            );
            abort();
        }

        nghttp2_data_provider2 dp;
        dp.source.ptr = response->body.get();
        dp.read_callback = [](
            nghttp2_session*,
            int32_t,
            uint8_t* buf, size_t length,
            uint32_t* dataFlags,
            nghttp2_data_source* src,
            void*
        ) -> nghttp2_ssize {
            // auto& ud = *static_cast<UserData*>(userData);

            auto adapter = (DataAdapter*) src->ptr;
            if (adapter == nullptr) {
                *dataFlags |= NGHTTP2_DATA_FLAG_EOF;
                return 0;
            }
            // logger::debug("Length/body length: {}/{}", length, res->body.size());
            nghttp2_ssize returnLen = (nghttp2_ssize) adapter->getChunk(
                length, buf, dataFlags
            );

            return returnLen;
        };
        int rv = nghttp2_submit_response2(
            sess,
            streamId,
            nva.data(),
            nva.size(),
            &dp
        );
        if (rv != 0) {
            logger::error(
                "Failed to submit: {}", nghttp2_strerror(rv)
            );
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        rv = nghttp2_session_send(sess);
        logger::debug("Data sent!");
        if (rv != 0) {
            logger::error(
                "Failed to send: {}", nghttp2_strerror(rv)
            );
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
    auto& request = ud.requests[frame->hd.stream_id];
    auto& response = ud.responses[frame->hd.stream_id];

    // Headers are always sent first, so if the pointer is null, it'll be here
    if (request == nullptr) {
        request = std::make_shared<Request>();
    }
    if (response == nullptr) {
        response = std::make_shared<Response>();
    }
    if (
        frame->hd.type == NGHTTP2_HEADERS
        && frame->headers.cat == NGHTTP2_HCAT_REQUEST
    ) {
        // TODO: string_view? We probably want to copy everything into std::strings for ownership though
        std::string n((const char*) name, namelen);
        std::string v((const char*) value, valuelen);


        if (n == ":method") {
            // TODO: this doesn't really make sense for error handling, but I don't care for now
            request->method = Method::_detail::strToMethod.at(v);
        } else if (n == "x-real-ip") {
            // We do not check if we trust x-real-ip here because it doesn't matter
            // If x-real-ip is maliciously set, we assume a malicious payload and reject
            // It could be argued that this is unnecessary compute, but /shrug
            try {
                asio::ip::make_address(v);
            } catch (const std::exception& e) {
                // TODO: it would be nice if there was a safe way to log the cibtebts if the header here, but I don't
                // think the underlying logger implementation can be assumed to be secure enough to deal with it, much
                // less the terminal or whatever displays the message.
                logger::critical("Invalid X-Real-IP. Asio error: {}", e.what());
                return NGHTTP2_ERR_INVALID_HEADER_BLOCK;
            }
        }

        request->headers[n] = std::move(v);
    }
    return 0;
}


int _detail::onChunkRecv(
    nghttp2_session*,
    uint8_t,
    int32_t streamId,
    const uint8_t* data,
    size_t len,
    void* userData
) {
    // std::cout << "Chunk size " << len << " with content "
    //     << std::string(
    //         (const char*) data,
    //         len
    //     )
    //     << std::endl;
    auto& ud = *static_cast<UserData*>(userData);
    // TODO: This is horrible for performance
    auto& request = ud.requests[streamId];
    request->body += std::string((const char*) data, len);
    return 0;
}

int _detail::onAlpnSelectProto(
    SSL*, const unsigned char** out,
    unsigned char* outLen, const unsigned char* in, 
    unsigned int inLen, void*
) {
    if (auto err = nghttp2_select_alpn(out, outLen, in, inLen); err != 1) {
        return SSL_TLSEXT_ERR_NOACK;
    }
    return SSL_TLSEXT_ERR_OK;
}

int _detail::onStreamClose(
    nghttp2_session*,
    int32_t streamId,
    uint32_t,
    void *userData
) {
    auto& ud = *static_cast<UserData*>(userData);
    {
        auto it = ud.requests.find(streamId);
        if (it != ud.requests.end()) {
            ud.requests.erase(it);
        }
    }
    {
        auto it = ud.responses.find(streamId);
        if (it != ud.responses.end()) {
            ud.responses.erase(it);
        }
    }
    return 0;
}

}
