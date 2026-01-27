#pragma once

#include "magpie/application/Methods.hpp"
#include <string>
#include <unordered_map>
namespace magpie {

struct Request {
    std::unordered_map<
        std::string, std::string
    > headers;

    // TODO: not sure how requests should deal with streamed input, largely because there's no way to signal which to
    // use in the current setup
    std::string body;
    Method::HttpMethod method;
};

}
