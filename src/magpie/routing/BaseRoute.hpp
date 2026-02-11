#pragma once

#include "magpie/middlewares/Middleware.hpp"
#include "magpie/routing/Compile.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
#include <vector>
#include <string_view>

namespace magpie::routing {

template <FixedString path, data::IsCommonData ContextType>
using RouteCallback = FunctionSignature<path, void, ContextType*, Request&, Response&>::type;

template <data::IsCommonData ContextType>
class BaseRoute {
protected:
    Middlewares<ContextType> middlewares;
public:

    virtual ~BaseRoute() = default;

    virtual void invoke(
        const std::vector<std::string_view>& requestedRoute,
        ContextType* context,
        Request& req,
        Response& res
    ) = 0;

    virtual BaseRoute* registerMiddlewares(std::vector<std::shared_ptr<Middleware<ContextType>>>&& middlewares) {
        this->middlewares.middlewares = std::move(middlewares);
        return this;
    }

    virtual Middlewares<ContextType>* getMiddlewaresAsPtr() {
        return &middlewares;
    }
};

}
