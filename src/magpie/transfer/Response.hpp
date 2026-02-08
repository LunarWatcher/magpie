#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <magpie/routing/Compile.hpp>
#include <variant>
#include "StatusCode.hpp"
#include "magpie/transfer/adapters/DataAdapter.hpp"

namespace magpie {

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

    Response() : code(&Status::OK), body(nullptr) {}

    Response(const StatusCode& code, std::string&& body) 
        : code(&code), body(std::make_shared<FixedAdapter>(std::move(body))) {}
    Response(const StatusCode& code, std::string&& body, std::string&& contentType) 
        : 
        code(&code),
        body(std::make_shared<FixedAdapter>(std::move(body))),
        contentType(std::move(contentType)) 
    {}


    Response(Response&&) = delete;
    Response(Response&) = delete;

    void setBody(std::string&& body) {
        this->body = std::make_shared<FixedAdapter>(std::move(body));
    }

    virtual ~Response() = default;

    Response& operator=(Response&& other) = default;
};

}
