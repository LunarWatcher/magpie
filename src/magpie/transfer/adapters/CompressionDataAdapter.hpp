#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <nghttp2/nghttp2.h>
#include <stdexcept>
#include <vector>
#include <zlib-ng.h>
#include "DataAdapter.hpp"

namespace magpie {

class CompressionDataAdapter : public DataAdapter {
protected:
    DataAdapter* readSource;

    zng_stream stream;
public:
    static inline int DEFLATE_HEADER = 0;
    static inline int GZIP_HEADER = 16;

    /**
     * Initialises the compression adapter with another DataAdapter.
     *
     * This signature takes ownership of the readSource; do not free it after initialization.
     */
    CompressionDataAdapter(
        DataAdapter* readSource,
        bool gzip = true
    ) : readSource(readSource) {
        stream.zfree = nullptr;
        stream.zalloc = nullptr;
        stream.opaque = nullptr;

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
    virtual ~CompressionDataAdapter();

    CompressionDataAdapter(CompressionDataAdapter&) = delete;
    CompressionDataAdapter(CompressionDataAdapter&&) = delete;

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
        // less than we need to ensure that we have enough room for the headers.
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
