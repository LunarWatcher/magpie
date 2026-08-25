#include "Http11Parser.hpp"
#include "magpie/application/Methods.hpp"
#include "magpie/transfer/adapters/FlagCompat.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <string_view>

#include <stc/StringUtil.hpp>

namespace magpie::http11 {

void Http11State::advance(Http11ParserState nextState) {
    state = nextState;

    if (nextState == Http11ParserState::WriteHeaders) {
        headerBuffer = {};
        bodyBuffer = {};
    } else if (nextState == Http11ParserState::WriteBody) {
        headerOutputAdapter = nullptr;
    }
}

ServerAction Http11State::read(
    const raven::Buffer& buff,
    std::size_t readBytes
) {
    std::string_view v{buff.data(), readBytes};
    if (v.at(0) == '\r') {
        return ServerAction::KillRequest;
    }

    if (state != Http11ParserState::ReadBody) {
        size_t headerEnd = v.find("\r\n\r\n");
        if (headerEnd == std::string_view::npos) {
            headerBuffer << v;
        } else {
            std::string_view headerPart = v.substr(0, headerEnd + 2); // Normalize so every line has a CRLF
            // We validate the contentPart after the fact
            std::string_view contentPart = v.substr(headerEnd + 4);
            bodyBuffer << contentPart;
            headerBuffer << headerPart;

            // I do not like this
            auto lines = stc::string::split(headerBuffer.str(), "\r\n");

            std::string& headerLine = lines.at(0);

            size_t methodSep = headerLine.find(' ');
            if (methodSep == 0 || methodSep == std::string::npos) {
                return ServerAction::KillRequest;
            }
            size_t uriSep = headerLine.find(' ', methodSep + 1);
            if (uriSep == methodSep + 1 || methodSep == std::string::npos) {
                return ServerAction::KillRequest;
            }
            if (headerLine.find(' ', uriSep + 1) != std::string::npos) {
                return ServerAction::KillRequest;
            }
            
            std::string method = headerLine.substr(0, methodSep);
            std::string uri = headerLine.substr(methodSep + 1, uriSep - methodSep - 1);
            std::string protoVersion = headerLine.substr(uriSep + 1);
            std::cout << "\"" << uri << "\"" << std::endl;

            if (protoVersion != "HTTP/1.1") {
                return ServerAction::KillRequest;
            }

            req = std::make_shared<Request>();
            auto typedMethod = Method::_detail::strToMethod.at(method);

            // TODO: need URI parsing for this to be proxy-compatible, but for now we assume uri == path
            req->uri = uri;
            req->path = uri;
            req->method = typedMethod;

            for (size_t i = 1; i < lines.size(); ++i) {
                auto& line = lines.at(i);
                auto colon = line.find(':');

                if (line.empty()) {
                    continue;
                }

                if (
                    colon == std::string::npos
                    || colon + 2 >= line.size()
                    || line.at(colon + 1) != ' '
                ) {
                    setResponse(
                        std::make_shared<Response>(
                            Status::BadRequest,
                            "Illegal header",
                            "text/plain"
                        )
                    );
                    return ServerAction::WriteResponse;
                }


                auto key = line.substr(0, colon);
                auto value = line.substr(colon + 2);

                // Normalize all headers to lower-case for consistency with HTTP/2
                std::transform(
                    key.begin(), key.end(),
                    key.begin(),
                    [](const auto& ch) {
                        return std::tolower(ch);
                    }
                );
                if (key == "transfer-encoding") {
                    return ServerAction::KillRequest;
                } else if (key == "content-length") {
                    try {
                        // TODO: use the currently nullptr idx arg to make sure the entire message is consumed
                        this->contentLength = std::stoull(value, nullptr, 10);
                    } catch (...) {
                        // Malformed content-length = kill the request
                        return ServerAction::KillRequest;
                    }
                    req->setHeader(key, std::to_string(*contentLength));
                } else {
                    req->setHeader(key, value);
                }
            }

            // TODO: need to validate the method to see if the method actually requires a body. Wikipedia has a table
            // for whether or not the body is required
            if (contentLength.has_value() && *contentLength != 0) {
                advance(Http11ParserState::ReadBody);
            } else {
                advance(Http11ParserState::WriteHeaders);
                return ServerAction::SetResponse;
            }
        }
    } else if (state == Http11ParserState::ReadBody) {
        bodyBuffer << v;
    }

    if (state == Http11ParserState::ReadBody) {
        // TODO: tellp() apparently returns a signed int and not an unsigned int, because it can in theory fail. Does
        // this only apply to the IO buffers, or can this happen to stringstreams too?
        // I assume this only happens in the event of a segfault or something
        auto offset = bodyBuffer.tellp();
        if ((size_t) offset == *contentLength) {
            req->body = bodyBuffer.str();
            advance(Http11ParserState::WriteHeaders);
            return ServerAction::SetResponse;
        } else if ((size_t) offset > *contentLength) {
            // This will only happen if the connection uses whatever the fuck the extension was for multiple requests in
            // parallel, which is a massive desync attack vector due to ambiguity and client lies
            // I suspect there's still ways to inject stuff here, but I'll have to look at that later.
            return ServerAction::KillRequest;
        }

        return ServerAction::ContinueRead;
    }

    return ServerAction::ContinueRead;
}

void Http11State::writeChunk(
    raven::Connection* conn,
    int& writeFlags
) {
    size_t written = conn->write(
        writeBuffer.buffer.data() + writeBuffer.written,
        writeBuffer.available - writeBuffer.written,
        writeFlags
    );
    writeBuffer.written += written;
}

void Http11State::populateWriteBuffer() {
    const static std::string closeChunkedSegment = "0\r\n\r\n";
    if (writeBuffer.written >= writeBuffer.available) {
        std::shared_ptr<DataAdapter> adapter;
        if (state == Http11ParserState::WriteBody) {
            adapter = res->body;
        } else if (state == Http11ParserState::WriteChunkedEnd) {
            std::copy(closeChunkedSegment.begin(), closeChunkedSegment.end(), writeBuffer.buffer.begin());
            writeBuffer.available = closeChunkedSegment.size();
            writeBuffer.makeAvailable(closeChunkedSegment.size());
            advance(Http11ParserState::ReadHeader);
            return;
        } else {
            adapter = headerOutputAdapter;
        }

        uint32_t chunkFlags = 0;
        auto buffSize = adapter->isStreamedAdapter() ? contentBuff.size() : writeBuffer->size();
        auto* buffPtr = adapter->isStreamedAdapter() ? contentBuff.data() : writeBuffer->data();

        size_t chunkBytes = adapter->getChunk(
            buffSize,
            (uint8_t*) buffPtr,
            &chunkFlags
        );

        if (adapter->isStreamedAdapter()) {
            // Pro tip: it helps if you RTFM and notice that the spec says hex number and not decimal number :facepaw:
            std::string size = std::format("{:x}", chunkBytes);
            std::copy(size.begin(), size.end(), writeBuffer->data());
            writeBuffer.buffer[size.size()] = '\r';
            writeBuffer.buffer[size.size() + 1] = '\n';
            auto dataStart = writeBuffer->data() + size.size() + 2;

            if (chunkBytes != 0) {
                std::copy(contentBuff.begin(), contentBuff.begin() + chunkBytes, dataStart);
            } else {
                advance(Http11ParserState::ReadHeader);
            }

            dataStart += chunkBytes;
            *dataStart = '\r';
            *(dataStart + 1) = '\n';

            chunkBytes += 4 + size.size();
            std::cout << chunkBytes << std::endl;
        }

        if (chunkBytes == 0) {
            return;
        } else if (chunkFlags == transfer::Flags::FlagEOF) {
            if (state == Http11ParserState::WriteHeaders) {
                if (res->body) {
                    advance(Http11ParserState::WriteBody);
                } else {
                    advance(Http11ParserState::ReadHeader);
                }
            } else if (adapter->isStreamedAdapter() && state == Http11ParserState::WriteBody) {
                advance(Http11ParserState::WriteChunkedEnd);
            } else {
                advance(Http11ParserState::ReadHeader);
            }
        }

        writeBuffer.makeAvailable(chunkBytes);
    }
}

void Http11State::write(
    raven::Connection* conn,
    raven::Buffer&
) {
    // TODO: This really should not do IO directly. This should be outsourced similar to the HTTP/2 implementation
    do {
        if (
            state != Http11ParserState::WriteBody
            && state != Http11ParserState::WriteHeaders
            && state != Http11ParserState::WriteChunkedEnd
        ) {
            break;
        }

        int writeFlags = 0;
        this->populateWriteBuffer();
        this->writeChunk(conn, writeFlags);

        if (
            writeFlags == raven::Connection::WriteFlags::BufferFull
        ) {
            return;
        }
    } while (writeBuffer.writeableBytes());
}

void Http11State::setResponse(const std::shared_ptr<Response>& res) {
    assert(res != nullptr);
    this->res = res;

    std::stringstream ss;
    ss << "HTTP/1.1 " << res->code->statusCode << " "
       << res->code->statusLine << "\r\n";

    for (auto& [k, v] : res->headers) {
        if (
            k == "transfer-encoding"
            || k == "content-length"
        ) {
            [[unlikely]]
            continue;
        }
        ss << k << ": " << v << "\r\n";
    }
    if (res->body) {
        if (res->body->isStreamedAdapter()) {
            ss << "Transfer-Encoding: chunked" << "\r\n";
        } else {
            ss << "Content-Length: " << res->body->getContentLength() << "\r\n";
        }
    } else {
        // Required for GETs that return `3xx`es to not break
        ss << "Content-Length: " << 0 << "\r\n";
    }
    ss << "\r\n";

    headerOutputAdapter = std::make_shared<FixedDataAdapter>(
        ss.str()
    );
}

}
