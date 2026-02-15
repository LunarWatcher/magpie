#include "CompressedResponse.hpp"
#include "magpie/transfer/adapters/CompressionDataAdapter.hpp"
#include "magpie/transfer/adapters/FixedDataAdapter.hpp"

namespace magpie {

CompressedResponse::CompressedResponse(
    const Request& req,
    const StatusCode& code,
    std::string&& body,
    std::string&& contentType
) : Response(
    code,
    (std::shared_ptr<DataAdapter>) nullptr,
    std::move(contentType)
) {
    auto acceptEncoding = req.headers.find("accept-encoding");

    auto responseEncoding = Encoding::IDENTITY;
    if (acceptEncoding != req.headers.end()) {
        auto& encoding = acceptEncoding->second;

        if (encoding.contains("gzip")) {
            responseEncoding = Encoding::GZIP;
        } else if (encoding.contains("deflate")) {
            responseEncoding = Encoding::DEFLATE;
        }
    }

    switch (responseEncoding) {
        case Encoding::IDENTITY:
            this->body = std::make_shared<FixedDataAdapter>(
                std::move(body)
            );
            this->headers["content-encoding"] = "identity";
            break;
        case Encoding::GZIP:
            [[fallthrough]];
        case Encoding::DEFLATE:
            this->body = std::make_shared<CompressionDataAdapter>(
                new FixedDataAdapter(
                    std::move(body)
                ),
                responseEncoding == Encoding::GZIP
            );
            this->headers["content-encoding"] = responseEncoding == Encoding::GZIP ? "gzip" : "deflate";
            break;
    }
}

}
