#pragma once

#include "magpie/application/Adapter.hpp"
#include <memory>
#include "AdapterFactory.hpp"

namespace magpie {

class BaseApp;

struct Http11AdapterFactory : public AdapterFactoryStruct {
    virtual std::shared_ptr<application::Adapter> get(raven::Connection* conn, BaseApp* app) override;
};

}
