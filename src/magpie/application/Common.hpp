#pragma once

#include "magpie/transfer/Request.hpp"
#include "raven/conn/Connection.hpp"

namespace magpie {

class BaseApp;

namespace application::common {

extern void setIpAddr(
    BaseApp* app,
    raven::Connection* conn,
    std::shared_ptr<Request>& req
);

}

}
