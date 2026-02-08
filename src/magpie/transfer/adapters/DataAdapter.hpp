#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <nghttp2/nghttp2.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <zlib-ng.h>

namespace magpie {

/**
 * Base class for data adapters.
 *
 * Data adapters are responsible for mediating between some form of input data, and the output at the HttpAdapter level.
 * The Http2Adapter, for example, is event-driven, and therefore accepts writes in blocks rather than in full chunks of
 * text. Therefore, both streamed and in-memory data being sent must be modified before it actually sets sent to the
 * client. This class defines the standard interface for this process.
 */
class DataAdapter {
public:
    virtual size_t getChunk(
        size_t len,
        uint8_t* buf,
        uint32_t* dataFlags
    ) = 0;

    virtual ~DataAdapter() = default;
};

// TODO: this should be split into a general compression adapter so there can be specific implementations
// (maybe, I don't know how the other compression libs are set up)
class CompressionAdapter : public DataAdapter {
protected:
    DataAdapter* readSource;

    zng_stream stream;
public:
    static inline int DEFLATE_HEADER = 0;
    static inline int GZIP_HEADER = 16;

    CompressionAdapter(
        DataAdapter* readSource,
        bool gzip = true
    ) : readSource(readSource) {
        stream.zfree = nullptr;
        stream.zalloc = nullptr;
        stream.opaque = nullptr;
    // int32_t zng_deflateInit2(zng_stream *strm, int32_t level, int32_t method, int32_t windowBits, int32_t memLevel, int32_t strategy);
        if (zng_deflateInit2(
                &stream,
                6,
                Z_DEFLATED,
                15 | (gzip ? GZIP_HEADER : DEFLATE_HEADER), 
                8,
                Z_DEFAULT_STRATEGY
        ) != Z_OK) {
            throw std::runtime_error("Critical zlib init error");
        }
    }
    virtual ~CompressionAdapter() = default;

    size_t getChunk(
        size_t len,
        uint8_t* buf,
        uint32_t* dataFlags
    ) override {
        size_t blocks = (size_t) std::ceil(
            (double) len / (double) 16384
        );
        // Per https://www.zlib.net/zlib_tech.html, there's 5 bytes of overhead per block. 
        // This means that as long as we reserve enough bytes for the headers, we don't need to cache anything. We read
        // less than we need to ensure that we have neough room for the headers.
        size_t overheadBytes = 5 * (blocks + 1);

        std::vector<uint8_t> intermediateBuf;
        // The intermediateBuf is adjusted for the size of gzip overhead
        intermediateBuf.resize(len - overheadBytes);
        size_t readLen = readSource->getChunk(
            intermediateBuf.size(),
            intermediateBuf.data(),
            dataFlags
        );

        stream.next_in = intermediateBuf.data();
        stream.avail_in = readLen;
        stream.next_out = buf;
        stream.avail_out = len;

        if (
            zng_deflate(&stream, Z_FULL_FLUSH) != Z_OK
        ) {
            throw std::runtime_error("Failed to deflate");
        }
        size_t outputSize = len - stream.avail_out;
        return outputSize;
    }
};

}
