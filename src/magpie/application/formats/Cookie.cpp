#include "Cookie.hpp"
#include <stc/StringUtil.hpp>

namespace magpie {

Cookie::Cookie(
    std::string&& name,
    std::string&& value
): name(std::move(name)), value(std::move(value)) {}

const std::string& Cookie::getName() const { return name; }
const std::string& Cookie::getValue() const { return value; }
const std::optional<size_t>& Cookie::getMaxAge() const { return maxAge; }

bool Cookie::isHttpOnly() const { return httpOnly; }
bool Cookie::isSecure() const { return secure; }

/**
 * Call this to set the `HttpOnly` flag in the cookie.
 */
Cookie& Cookie::setHttpOnly() {
    httpOnly = true;
    return *this;
}

/**
 * Call this to set the `Secure` flag in the cookie.
 */
Cookie& Cookie::setSecure() {
    secure = true;
    return *this;
}

Cookie& Cookie::setPath(std::string&& path) {
    this->path = std::move(path);
    return *this;
}

Cookie& Cookie::setDomain(const std::string& domain) {
    this->domain = domain;
    return *this;
}

}
