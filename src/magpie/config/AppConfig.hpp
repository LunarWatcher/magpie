#pragma once

#include "magpie/config/SSLConfig.hpp"
#include <optional>
#include <thread>

namespace magpie {

struct AppConfig {
    uint16_t port = 8080;
    unsigned int concurrency = std::thread::hardware_concurrency();
    std::string bindAddr = "127.0.0.1";

    std::optional<SSLConfig> ssl = std::nullopt;
    /**
     * Whether or not to trust the X-Real-IP header. If true, the X-Real-IP header can override the ipAddr field in the
     * Request object if set. If you set this to true, you MUST ensure that the X-Real-IP can only come from a trusted
     * source, or you're exposing the server to multiple security vulnerabilities. This means that the server MUST be
     * behind a reverse proxy, and not accessible to the general public in any other way.
     *
     * \see https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/X-Forwarded-For#security_and_privacy_concerns
     */
    bool trustXRealIp = false;
};


}
