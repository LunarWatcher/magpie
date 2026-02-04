#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
#include <string>
namespace magpie::routing {

class BaseRouter {
public:
    virtual ~BaseRouter() = default;
    virtual void invokeRoute(
        const std::string& path,
        data::CommonData* ctx,
        Request& req,
        Response& res
    ) const = 0;
};


}
