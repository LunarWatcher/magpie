#pragma once

#include "magpie/application/Adapter.hpp"
#include <optional>
#include <raven/config/SSLConfig.hpp>
#include <raven/conn/Connection.hpp>
#include <memory>

namespace magpie {

class BaseApp;

struct AdapterFactoryStruct {
    virtual ~AdapterFactoryStruct() = default;

    virtual std::optional<raven::SSLConfig>&& withSslConfigExtras(std::optional<raven::SSLConfig>&& sslConfig) {
        return std::move(sslConfig);
    }
    virtual std::shared_ptr<application::Adapter> get(raven::Connection* conn, BaseApp* app) = 0;

    virtual std::string getProto(bool hasSsl) = 0;
};

}
