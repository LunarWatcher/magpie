#pragma once

#include <string>
#include <unordered_map>
#include <magpie/routing/Compile.hpp>
#include "StatusCode.hpp"

namespace magpie {

struct Response {
    /**
     * The response headers.
     */
    std::unordered_map<std::string, std::string> headers;

    const StatusCode& code;
    // TODO: this could probably be converted to a variant<string, function> to allow streamed data
    std::string body;
    std::string contentType = "text/plain";

    Response(const StatusCode& code, std::string&& body) 
        : code(code), body(std::move(body)) {}
    Response(const StatusCode& code, std::string&& body, std::string&& contentType) 
        : code(code), body(std::move(body)), contentType(std::move(contentType)) {}

    Response(Response&& other) 
        : headers(std::move(other.headers)),
          code(other.code),
          body(std::move(other.body)),
          contentType(std::move(other.contentType)) {}

    virtual ~Response() = default;
};

}
