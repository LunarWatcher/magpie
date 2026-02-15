#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <magpie/routing/Compile.hpp>
#include "StatusCode.hpp"
#include "magpie/transfer/adapters/DataAdapter.hpp"

namespace magpie {

struct CompressedResponse;
struct Response {
    /**
     * The response headers.
     */
    std::unordered_map<std::string, std::string> headers;

    const StatusCode* code;

    std::shared_ptr<
        DataAdapter
    > body;
    // TODO: does it really make sense for the content-type to be stored like this?
    std::string contentType = "text/plain";

    Response();
    Response(
        const StatusCode& code,
        std::string&& body,
        std::string&& contentType = "text/plain"
    );
    Response(
        const StatusCode& code,
        std::shared_ptr<DataAdapter>&& bodyAdapter,
        std::string&& contentType
    );

    Response(Response&&) = delete;
    Response(Response&) = delete;

    virtual ~Response() = default;

    void setBody(std::string&& body);

    Response& operator=(Response&& other) = default;
    Response& operator=(CompressedResponse&& other);
};

}
