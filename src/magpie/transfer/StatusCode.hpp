#pragma once

#include <string_view>
#include <string>

namespace magpie {

struct StatusCode {
    unsigned short statusCode;
    std::string_view statusLine;

    constexpr StatusCode(
        unsigned short statusCode,
        std::string_view statusLine
    ) : statusCode(statusCode),
        statusLine(statusLine) {
    }

    StatusCode(StatusCode&) = delete;
    StatusCode(StatusCode&&) = delete;

    bool operator==(const StatusCode& other) {
        return statusCode == other.statusCode;
    }
};

namespace Status {
    constexpr StatusCode CONTINUE(100, "Continue");
    constexpr StatusCode SWITCHING_PROTOCOLS(101, "Switching Protocols");

    constexpr StatusCode OK(200, "OK");
    constexpr StatusCode CREATED(201, "Created");
    constexpr StatusCode ACCEPTED(202, "Accepted");
    constexpr StatusCode NON_AUTHORITATIVE_INFORMATION(203, "Non-Authoritative Information");
    constexpr StatusCode NO_CONTENT(204, "No Content");
    constexpr StatusCode RESET_CONTENT(205, "Reset Content");
    constexpr StatusCode PARTIAL_CONTENT(206, "Partial Content");

    constexpr StatusCode MULTIPLE_CHOICES(300, "Multiple Choices");
    constexpr StatusCode MOVED_PERMANENTLY(301, "Moved Permanently");
    constexpr StatusCode FOUND(302, "Found");
    constexpr StatusCode SEE_OTHER(303, "See Other");
    constexpr StatusCode NOT_MODIFIED(304, "Not Modified");
    constexpr StatusCode TEMPORARY_REDIRECT(307, "Temporary Redirect");
    constexpr StatusCode PERMANENT_REDIRECT(308, "Permanent Redirect");

    constexpr StatusCode BAD_REQUEST(400, "Bad Request");
    constexpr StatusCode UNAUTHORIZED(401, "Unauthorized");
    constexpr StatusCode FORBIDDEN(403, "Forbidden");
    constexpr StatusCode NOT_FOUND(404, "Not Found");
    constexpr StatusCode METHOD_NOT_ALLOWED(405, "Method Not Allowed");
    constexpr StatusCode NOT_ACCEPTABLE(406, "Not Acceptable");
    constexpr StatusCode PROXY_AUTHENTICATION_REQUIRED(407, "Proxy Authentication Required");
    constexpr StatusCode CONFLICT(409, "Conflict");
    constexpr StatusCode GONE(410, "Gone");
    constexpr StatusCode PAYLOAD_TOO_LARGE(413, "Payload Too Large");
    constexpr StatusCode UNSUPPORTED_MEDIA_TYPE(415, "Unsupported Media Type");
    constexpr StatusCode RANGE_NOT_SATISFIABLE(416, "Range Not Satisfiable");
    constexpr StatusCode EXPECTATION_FAILED(417, "Expectation Failed");
    constexpr StatusCode IM_A_TEAPOT(418, "I'm a teapot");
    constexpr StatusCode PRECONDITION_REQUIRED(428, "Precondition Required");
    constexpr StatusCode TOO_MANY_REQUESTS(429, "Too Many Requests");
    constexpr StatusCode UNAVAILABLE_FOR_LEGAL_REASONS(451, "Unavailable For Legal Reasons");

    constexpr StatusCode INTERNAL_SERVER_ERROR(500, "Internal Server Error");
    constexpr StatusCode NOT_IMPLEMENTED(501, "Not Implemented");
    constexpr StatusCode BAD_GATEWAY(502, "Bad Gateway");
    constexpr StatusCode SERVICE_UNAVAILABLE(503, "Service Unavailable");
    constexpr StatusCode GATEWAY_TIMEOUT(504, "Gateway Timeout");
    constexpr StatusCode VARIANT_ALSO_NEGOTIATES(506, "Variant Also Negotiates");

}

}
