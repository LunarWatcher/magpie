#pragma once

#include "magpie/application/Adapter.hpp"
#include <openssl/core_dispatch.h>
#include <raven/config/SSLConfig.hpp>
#include <string>
#include <cstdint>
#include <thread>

namespace magpie {

class BaseApp;

struct AdapterFactoryStruct {
    virtual ~AdapterFactoryStruct() = default;

    virtual std::optional<raven::SSLConfig>&& withSslConfigExtras(std::optional<raven::SSLConfig>&& sslConfig) {
        return std::move(sslConfig);
    }
    virtual std::shared_ptr<application::Adapter> get(raven::Connection* conn, BaseApp* app) = 0;
};

struct Http11AdapterFactory : public AdapterFactoryStruct {
    virtual std::shared_ptr<application::Adapter> get(raven::Connection* conn, BaseApp* app) override;
};

struct Http2AdapterFactory : public AdapterFactoryStruct {
    virtual std::optional<raven::SSLConfig>&& withSslConfigExtras(std::optional<raven::SSLConfig>&& sslConfig) override;
    virtual std::shared_ptr<application::Adapter> get(raven::Connection* conn, BaseApp* app) override;
};

using AdapterFactory = std::shared_ptr<AdapterFactoryStruct>;

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

    AdapterFactory adapterFactory = std::make_shared<Http2AdapterFactory>();
};


}
