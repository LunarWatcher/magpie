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

    const StatusCode* code;
    // TODO: this could probably be converted to a variant<string, function> to allow streamed data
    std::string body;
    // TODO: does it really make sense for the content-type to be stored like this?
    std::string contentType = "text/plain";

    Response() : code(&Status::OK) {}

    Response(const StatusCode& code, std::string&& body) 
        : code(&code), body(std::move(body)) {}
    Response(const StatusCode& code, std::string&& body, std::string&& contentType) 
        : code(&code), body(std::move(body)), contentType(std::move(contentType)) {}


    Response(Response&&) = delete;
    Response(Response&) = delete;

    virtual ~Response() = default;

    Response& operator=(Response&& other) = default;
};

}
