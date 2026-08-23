#include "FixedDataAdapter.hpp"
#include "magpie/transfer/adapters/FlagCompat.hpp"
#include <nghttp2/nghttp2.h>

namespace magpie {

FixedDataAdapter::FixedDataAdapter(std::string&& data)
        : data(std::move(data)) {}

size_t FixedDataAdapter::getChunk(
    size_t outLen,
    uint8_t* buf,
    uint32_t* dataFlags
) {
    size_t len = std::min(
        outLen,
        data.size() - readOffset
    );
    std::memcpy(
        buf,
        data.c_str() + readOffset,
        len
    );

    if (len + readOffset == data.size()) {
        *dataFlags = transfer::Flags::FlagEOF;
    } else {
        readOffset += len;
    }
    return len;
}

size_t FixedDataAdapter::getContentLength() {
    return this->data.size();
}

}
