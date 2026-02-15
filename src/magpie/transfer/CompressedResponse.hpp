#pragma once

#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"

namespace magpie {

enum class Encoding {
    GZIP,
    DEFLATE,
    IDENTITY
};

/**
 * Used for a response that should be compressed.
 *
 * Do not use for binary data, or data that is otherwise already compressed. Compressing compressed data is just a waste
 * of compute for no compression ratio. Zlib-ng should avoid the response being significantly bigger (aside a <1%
 * overhead due to zlib header bytes), but there is still extra overhead.
 */
struct CompressedResponse : public Response {
    CompressedResponse(
        const Request& req,
        const StatusCode& code,
        std::string&& body,
        std::string&& contentType = "text/plain"
    );
};

}
