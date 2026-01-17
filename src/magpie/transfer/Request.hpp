#pragma once

#include <string>
#include <unordered_map>
namespace magpie {

/**
 * TODO: this should not be an enum. Initially thought it was a good idea, but I forgot about webdav, which uses
 * non-standard protocols.
 */
enum class HttpMethod {
    GET,
    HEAD,
    OPTIONS,
    TRACE,
    PUT,
    DELETE,
    POST,
    PATCH,
    CONNECT
};

struct Request {
    std::string body;

    std::unordered_map<
        std::string, std::string
    > headers;

    // Raw, mainly used for routing and other internal shit, but can be used by consumers as well
    std::string rawRequestPath;
    HttpMethod method;
};

}
