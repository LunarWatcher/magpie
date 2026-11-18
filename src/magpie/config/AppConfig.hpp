#pragma once

#include "AdapterFactory.hpp"
// Not directly used, but included for backward compat
#include <magpie/config/Http11AdapterFactory.hpp>
#include <magpie/config/Http2AdapterFactory.hpp>
#include <openssl/core_dispatch.h>
#include <raven/config/SSLConfig.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <thread>

namespace magpie {

using AdapterFactory = std::shared_ptr<AdapterFactoryStruct>;

/**
 * Config with various HTTP limits. Currently only affects the HTTP/1.1 setup.
 *
 * Note that this setup will never be granular, and must represent the upper limit of what the application is expected
 * to receive. If you need route-level granularity, use a reverse proxy.
 */
struct SecurityConfig {
    uint16_t maxHeaders = 50;
    uint32_t maxSingleHeaderBytes = 16384;

    /**
     * The max permitted length of the request body. Requests above this are rejected.
     */
    uint64_t maxBodyBytes = 16777216; // 16 MiB (I think; 2^24 bytes)
};

struct AppConfig {
    uint16_t port = 8080;
    unsigned int concurrency = std::thread::hardware_concurrency();
    std::string bindAddr = "127.0.0.1";

    /**
     * Whether or not to trust the X-Real-IP header. If true, the X-Real-IP header can override the ipAddr field in the
     * Request object if set. If you set this to true, you MUST ensure that the X-Real-IP can only come from a trusted
     * source, or you're exposing the server to multiple security vulnerabilities. This means that the server MUST be
     * behind a reverse proxy, and not accessible to the general public in any other way.
     *
     * \see https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/X-Forwarded-For#security_and_privacy_concerns
     */
    bool trustXRealIp = false;

    AdapterFactory adapterFactory = std::make_shared<
#ifndef MAGPIE_DISABLE_HTTP2
        Http2AdapterFactory
#else
        Http11AdapterFactory
#endif
    >();

    /**
     * Sets abuse prevention limits for the chosen adapter.
     *
     * This is currently only respected by the HTTP/1.1 adapter.
     */
    SecurityConfig limits{};
};


}
