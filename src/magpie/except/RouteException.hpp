#pragma once

#include <exception>
#include <string>

namespace magpie {

class RouteException : public std::exception {
public:
    std::string message;
    RouteException(const std::string& message) : message(message) {

    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};

}
