#pragma once

#include <string>
#include <unordered_map>
namespace magpie {

/**
 * The supported HTTP methods. Custom HTTP methods are not supported, because it mostly doesn't make sense. Design
 * better APIs instead.
 *
 * @see https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Methods#safe_idempotent_and_cacheable_request_methods
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
