#pragma once

#include "magpie/routing/Compile.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
#include <vector>
#include <string_view>

namespace magpie::routing {

template <FixedString path, data::IsCommonData ContextType>
using RouteCallback = FunctionSignature<path, void, ContextType*, Request&, Response&>::type;

template <data::IsCommonData ContextType>
struct BaseRoute {
    virtual ~BaseRoute() = default;

    virtual void invoke(
        const std::vector<std::string_view>& requestedRoute,
        ContextType* context,
        Request& req,
        Response& res
    ) = 0;
};

}
