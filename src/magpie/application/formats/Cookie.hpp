#pragma once

#include <optional>
#include <string>

namespace magpie {

enum class SameSite {
    Strict,
    Lax,
    None,
    /**
     * Undefined is not a valid SameSite value, but this is used to indicate that SameSite is not to be sent.
     */
    Undefined
};

enum class CookieParseError {
    MissingKeyValuePair,
    NoCookies,
};

/**
 * Used to represent both inbound and outbound cookies in a parsed format.
 * When constructing a cookie in code, there's several builder functions that can be used to set other options.
 *
 * \note `Expires` is not supported at all even though it is a technically valid cookie field. This is because the field
 *          is considered bad practice by some accounts, and meeting the date format required for it in a sensible way
 *          is annoying.
 *          If you need expires, have you considered not needing it? You can get the, for all intents and purposes,
 *          exact same result with `Max-Age`.
 * 
 * \see https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Set-Cookie
 */
class Cookie {
private:
    std::string name;
    std::string value;
    std::optional<size_t> maxAge;
    std::optional<std::string> path;
    std::optional<std::string> domain;

    bool httpOnly = false;
    bool secure = false;
public:

    /**
     * Constructs a cookie.
     *
     * \param name      The name of the cookie
     * \param value     The value stored in the cookie
     * \param maxAge    The max age of the cookie represented as seconds. If nullopt, no MaxAge is set.
     */
    Cookie(
        std::string&& name,
        std::string&& value
    );

    const std::string& getName() const;
    const std::string& getValue() const;
    const std::optional<size_t>& getMaxAge() const;

    bool isHttpOnly() const;
    bool isSecure() const;

    /**
     * Call this to set the `HttpOnly` flag in the cookie.
     */
    Cookie& setHttpOnly();

    /**
     * Call this to set the `Secure` flag in the cookie.
     */
    Cookie& setSecure();

    Cookie& setPath(std::string&& path);
    Cookie& setDomain(const std::string& domain);

};

}
