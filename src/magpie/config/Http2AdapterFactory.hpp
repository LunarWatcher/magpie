#pragma once
#ifndef MAGPIE_DISABLE_HTTP2

#include "magpie/application/Adapter.hpp"
#include <memory>
#include "AdapterFactory.hpp"

namespace magpie {

class BaseApp;

struct Http2AdapterFactory : public AdapterFactoryStruct {
    virtual std::optional<raven::SSLConfig>&& withSslConfigExtras(std::optional<raven::SSLConfig>&& sslConfig) override;
    virtual std::shared_ptr<application::Adapter> get(raven::Connection* conn, BaseApp* app) override;

    std::string getProto(bool hasSsl) override {
        return hasSsl ? "https" : "http";
    }
};
}
#endif
