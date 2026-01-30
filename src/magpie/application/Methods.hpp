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
    Connect,
    Delete,
    Get,
    Head,
    Options,
    // Patch is _technically_ an extension, but it's so broadly used that it doesn't matter
    Patch,
    Post,
    Put,
    Trace,
    // }}}
    // Various extensions {{{
    Acl,
    Baseline_control,
    Bind,
    Checkin,
    Checkout,
    Copy,
    Label,
    Link,
    Lock,
    Merge,
    Mkactivity,
    Mkcalendar,
    Mkcol,
    Mkredirectref,
    Mkworkspace,
    Move,
    Orderpatch,
    Pri,
    Propfind,
    Proppatch,
    Query,
    Rebind,
    Report,
    Search,
    Unbind,
    Uncheckout,
    Unlink,
    Unlock,
    Update,
    Updatedirectref,
    Version_control,
    // }}}
};

namespace _detail {
/**
 * Contains remappings for HTTP strings to HTTPMethod. This is exclusively meant for internal use, and should not be
 * used anywhere else.
 */
static inline const std::unordered_map<std::string, HttpMethod> strToMethod {
    { "CONNECT", Connect },
    { "DELETE", Delete },
    { "GET", Get },
    { "HEAD", Head },
    { "OPTIONS", Options },
    { "PATCH", Patch },
    { "POST", Post },
    { "PUT", Put },
    { "TRACE", Trace },
    { "ACL", Acl },
    { "BASELINE-CONTROL", Baseline_control },
    { "BIND", Bind },
    { "CHECKIN", Checkin },
    { "CHECKOUT", Checkout },
    { "COPY", Copy },
    { "LABEL", Label },
    { "LINK", Link },
    { "LOCK", Lock },
    { "MERGE", Merge },
    { "MKACTIVITY", Mkactivity },
    { "MKCALENDAR", Mkcalendar },
    { "MKCOL", Mkcol },
    { "MKREDIRECTREF", Mkredirectref },
    { "MKWORKSPACE", Mkworkspace },
    { "MOVE", Move },
    { "ORDERPATCH", Orderpatch },
    { "PRI", Pri },
    { "PROPFIND", Propfind },
    { "PROPPATCH", Proppatch },
    { "QUERY", Query },
    { "REBIND", Rebind },
    { "REPORT", Report },
    { "SEARCH", Search },
    { "UNBIND", Unbind },
    { "UNCHECKOUT", Uncheckout },
    { "UNLINK", Unlink },
    { "UNLOCK", Unlock },
    { "UPDATE", Update },
    { "UPDATEDIRECTREF", Updatedirectref },
    { "VERSION-CONTROL", Version_control },
};
}

}
