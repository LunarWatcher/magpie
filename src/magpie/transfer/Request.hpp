#pragma once

#include "magpie/application/Methods.hpp"
#include <string>
#include <unordered_map>
namespace magpie {

struct Request {
    std::unordered_map<
        std::string, std::string
    > headers;

    /**
     * Contains the body for non-streamed endpoints.
     */
    std::string body;

    /**
     * Contains the IP address associated with this request. Depending on AppConfig options, this value is one of:
     *
     * 1. The IP address associated with the connection
     * 2. The IP addres specified by X-Real-IP (requires AppConfig::trustXRealIp = true)
     */
    std::string ipAddr;

    /**
     * The HTTP method for this request. This is typically inferred by the endpoint, but if you use the same handler for
     * multiple methods, you may need to use this to differentiate between methods.
     *
     * Using the multiple handler for different methods is discouraged, however.
     */
    Method::HttpMethod method;
};

}
