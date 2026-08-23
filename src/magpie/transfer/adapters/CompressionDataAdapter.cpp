#include "CompressionDataAdapter.hpp"

namespace magpie {

CompressionDataAdapter::~CompressionDataAdapter() {
    if (this->readSource != nullptr) {
        delete readSource;
        readSource = nullptr;
    }

    // This is apparently zlib's free function, because the words "free" and "close" were too difficult
    deflateEnd(&this->stream);
}

size_t CompressionDataAdapter::getContentLength() {
    // TODO: how do we know what size the data is?
    return 0;
}

}
