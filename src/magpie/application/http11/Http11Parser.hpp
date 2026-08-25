#pragma once

#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
#include "magpie/transfer/adapters/FixedDataAdapter.hpp"
#include "raven/conn/CommonDefs.hpp"
#include "raven/conn/Connection.hpp"

#include <cmath>
#include <memory>
#include <sstream>

/**
 * Contains a basic HTTP/1.1 implementation.
 *
 * * Chunked encoding is not supported: Use websockets or another specific-purpose streaming standard instead.
 * * Some validation is done on core fields to try to avoid some standard desync attacks
 */
namespace magpie::http11 {


enum class Http11ParserState {
    ReadHeader,
    ReadBody,
    WriteHeaders,
    WriteBody,

    WriteChunkedEnd,
};

enum class ServerAction {
    ContinueRead,
    SetResponse,
    WriteResponse,
    /**
     * Special action that tells the server to (shock) kill the request. This is used when the request is malformed in
     * specific ways that makes it impossible to parse, or incompatible with this parser.
     *
     * This MUST result in the termination of the connection.
     */
    KillRequest
};

struct WriteBuffer {
    raven::Buffer buffer;
    size_t available = 0;
    size_t written = 0;

    void makeAvailable(size_t available) {
        this->available = available;
        this->written = 0;
    }

    raven::Buffer* operator->() { return &buffer; }

    size_t writeableBytes() {
        return available - written;
    }
};

struct Http11State {
private:
    /**
     * Contains the header and the pre-content HTTP text
     */
    std::stringstream headerBuffer;
    /**
     * Buffer for the input body
     *
     * TODO: this is not compatible with streaming input bodies (but then again, neither is the rest of the server
     * design as it currently stands). Should preferably find some way to stream directly to disk, since that'll be the
     * main use-case where streaming is desired
     */
    std::stringstream bodyBuffer;

    std::shared_ptr<FixedDataAdapter> headerOutputAdapter;

    static_assert(raven::WindowSize == 16'384,
        "If the buffer size changes, the logic here may break (at least until C++26)"
    );
    constexpr static inline int contentBuffSize = raven::WindowSize
        - 4 // 2x CRLF
        - 5; // up to 5 digits for the size
        // TODO: log10 is constexpr in C++26
        // TODO: log16*, the string part is base16, not base10
        // - (std::floor(std::log10(raven::WindowSize)) + 1);
    static inline thread_local std::array<char, contentBuffSize> contentBuff;

    WriteBuffer writeBuffer;

    void advance(Http11ParserState nextState);
    void populateWriteBuffer();
    void writeChunk(
        raven::Connection* conn,
        int& writeFlags
    );
public:
    std::shared_ptr<Request> req;
    std::shared_ptr<Response> res;

    size_t currBufferOffset;

    std::optional<size_t> contentLength;

    Http11ParserState state = Http11ParserState::ReadHeader;

    ServerAction read(
        const raven::Buffer& buff,
        std::size_t readBytes
    );

    void write(
        raven::Connection* conn,
        raven::Buffer& buff
    );

    void setResponse(
        const std::shared_ptr<Response>& res
    );
};

}
