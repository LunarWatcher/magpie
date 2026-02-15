#pragma once

#include <string_view>
#include <string>

namespace magpie {

/**
 * Contains data for a status code. 
 *
 * Status codes are singletons, and cannot be generated on the fly. If you need additional status codes, create globals
 * for them, and then use them. Creating them within a function is either a memory leak (`new`) or a segfault (returning
 * a pointer to a temporary object). 
 *
 * This class has conversion semantics to int and strings. Magpie's built-in status objects can therefore be used to
 * compare status codes of other libraries, provided the status code is stored as an int, or a string only containing
 * the code number. For example, with libcpr, this code is perfectly valid:
 * ```cpp
 * cpr::Get(cpr::Url{ ... }).status_code == magpie::Status::OK
 * ```
 * libcpr has no relation to this library (other than this library using it in tests), but because `status_code` is a
 * number, the conversions makes it easier to use the same status codes with other unrelated libraries for
 * standardisation. Probably only useful if you're writing tests for a magpie server, but ✨ the more you know ✨
 */
struct StatusCode {
    unsigned short statusCode;
    std::string_view statusLine;

    /**
     * \param statusCode    The HTTP status code
     * \param statusLine    The HTTP status line. Only matters for HTTP/1.1, as HTTP/2 and newer do not use this field.
     */
    constexpr StatusCode(
        unsigned short statusCode,
        std::string_view statusLine
    ) : statusCode(statusCode),
        statusLine(statusLine) {
    }

    StatusCode(StatusCode&) = delete;
    StatusCode(StatusCode&&) = delete;

    operator int() const { return statusCode; }
    operator short() const { return statusCode; }
    operator std::string() const { return std::to_string(statusCode); }

    bool operator==(const StatusCode& other) const {
        return statusCode == other.statusCode;
    }

    bool operator==(unsigned short code) const {
        return statusCode == code;
    }

    // required by catch2 when operator int() exists for whatever reason
    bool operator==(long int code) const {
        return statusCode == (unsigned short) code;
    }
};

namespace Status {
    constexpr StatusCode Continue(100, "Continue");
    constexpr StatusCode SwitchingProtocols(101, "Switching Protocols");

    constexpr StatusCode OK(200, "OK");
    constexpr StatusCode Created(201, "Created");
    constexpr StatusCode Accepted(202, "Accepted");
    constexpr StatusCode NonAuthoritativeInformation(203, "Non-Authoritative Information");
    constexpr StatusCode NoContent(204, "No Content");
    constexpr StatusCode ResetContent(205, "Reset Content");
    constexpr StatusCode PartialContent(206, "Partial Content");

    constexpr StatusCode MultipleChoices(300, "Multiple Choices");
    constexpr StatusCode MovedPermanently(301, "Moved Permanently");
    constexpr StatusCode Found(302, "Found");
    constexpr StatusCode SeeOther(303, "See Other");
    constexpr StatusCode NotModified(304, "Not Modified");
    constexpr StatusCode TemporaryRedirect(307, "Temporary Redirect");
    constexpr StatusCode PermanentRedirect(308, "Permanent Redirect");

    constexpr StatusCode BadRequest(400, "Bad Request");
    constexpr StatusCode Unauthorized(401, "Unauthorized");
    constexpr StatusCode Forbidden(403, "Forbidden");
    constexpr StatusCode NotFound(404, "Not Found");
    constexpr StatusCode MethodNotAllowed(405, "Method Not Allowed");
    constexpr StatusCode NotAcceptable(406, "Not Acceptable");
    constexpr StatusCode ProxyAuthenticationRequired(407, "Proxy Authentication Required");
    constexpr StatusCode Conflict(409, "Conflict");
    constexpr StatusCode Gone(410, "Gone");
    constexpr StatusCode PayloadTooLarge(413, "Payload Too Large");
    constexpr StatusCode URITooLong(414, "URI Too Long");
    constexpr StatusCode UnsupportedMediaType(415, "Unsupported Media Type");
    constexpr StatusCode RangeNotSatisfiable(416, "Range Not Satisfiable");
    constexpr StatusCode ExpectationFailed(417, "Expectation Failed");
    constexpr StatusCode ImATeapot(418, "I'm a teapot");
    constexpr StatusCode PreconditionRequired(428, "Precondition Required");
    constexpr StatusCode TooManyRequests(429, "Too Many Requests");
    /**
     * Only respected by nginx
     */
    constexpr StatusCode NginxNoResponse(444, "No Response");
    constexpr StatusCode UnavailableForLegalReasons(451, "Unavailable For Legal Reasons");

    constexpr StatusCode InternalServerError(500, "Internal Server Error");
    constexpr StatusCode NotImplemented(501, "Not Implemented");
    constexpr StatusCode BadGateway(502, "Bad Gateway");
    constexpr StatusCode ServiceUnavailable(503, "Service Unavailable");
    constexpr StatusCode GatewayTimeout(504, "Gateway Timeout");
    constexpr StatusCode HttpVersionNotSupported(505, "HTTP Version Not Supported");
    constexpr StatusCode VariantAlsoNegotiates(506, "Variant Also Negotiates");

}

}
