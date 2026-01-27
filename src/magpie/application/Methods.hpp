#pragma once

#include <string>
#include <unordered_map>

namespace magpie::Method {

/**
 * Contains the supported HTTP methods. Largely based on IANA's HTTP method registry.
 *
 * \see https://www.iana.org/assignments/http-methods/http-methods.xhtml
 */
enum HttpMethod {
    // Default HTTP {{{
    CONNECT,
    DELETE,
    GET,
    HEAD,
    OPTIONS,
    // Patch is _technically_ an extension, but it's so broadly used that it doesn't matter
    PATCH,
    POST,
    PUT,
    TRACE,
    // }}}
    // Various extensions {{{
    ACL,
    BASELINE_CONTROL,
    BIND,
    CHECKIN,
    CHECKOUT,
    COPY,
    LABEL,
    LINK,
    LOCK,
    MERGE,
    MKACTIVITY,
    MKCALENDAR,
    MKCOL,
    MKREDIRECTREF,
    MKWORKSPACE,
    MOVE,
    ORDERPATCH,
    PRI,
    PROPFIND,
    PROPPATCH,
    QUERY,
    REBIND,
    REPORT,
    SEARCH,
    UNBIND,
    UNCHECKOUT,
    UNLINK,
    UNLOCK,
    UPDATE,
    UPDATEDIRECTREF,
    VERSION_CONTROL,
    // }}}
};

namespace _detail {
/**
 * Contains remappings for HTTP strings to HTTPMethod. This is exclusively meant for internal use, and should not be
 * used anywhere else.
 */
static inline const std::unordered_map<std::string, HttpMethod> strToMethod {
    { "CONNECT", CONNECT },
    { "DELETE", DELETE },
    { "GET", GET },
    { "HEAD", HEAD },
    { "OPTIONS", OPTIONS },
    { "PATCH", PATCH },
    { "POST", POST },
    { "PUT", PUT },
    { "TRACE", TRACE },
    { "ACL", ACL },
    { "BASELINE-CONTROL", BASELINE_CONTROL },
    { "BIND", BIND },
    { "CHECKIN", CHECKIN },
    { "CHECKOUT", CHECKOUT },
    { "COPY", COPY },
    { "LABEL", LABEL },
    { "LINK", LINK },
    { "LOCK", LOCK },
    { "MERGE", MERGE },
    { "MKACTIVITY", MKACTIVITY },
    { "MKCALENDAR", MKCALENDAR },
    { "MKCOL", MKCOL },
    { "MKREDIRECTREF", MKREDIRECTREF },
    { "MKWORKSPACE", MKWORKSPACE },
    { "MOVE", MOVE },
    { "ORDERPATCH", ORDERPATCH },
    { "PRI", PRI },
    { "PROPFIND", PROPFIND },
    { "PROPPATCH", PROPPATCH },
    { "QUERY", QUERY },
    { "REBIND", REBIND },
    { "REPORT", REPORT },
    { "SEARCH", SEARCH },
    { "UNBIND", UNBIND },
    { "UNCHECKOUT", UNCHECKOUT },
    { "UNLINK", UNLINK },
    { "UNLOCK", UNLOCK },
    { "UPDATE", UPDATE },
    { "UPDATEDIRECTREF", UPDATEDIRECTREF },
    { "VERSION-CONTROL", VERSION_CONTROL },
};
}

}
