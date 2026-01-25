#pragma once

#include "magpie/routing/Compile.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
#include <vector>
#include <string_view>

namespace magpie::routing {


template <FixedString path, data::IsCommonData ContextType>
using RouteCallback = FunctionSignature<path, Response, ContextType*, Request&>::type;

template <data::IsCommonData ContextType>
struct BaseRoute {
    virtual ~BaseRoute() = default;

    virtual Response invoke(
        const std::vector<std::string_view>& requestedRoute,
        ContextType* context,
        Request& req
    ) = 0;
};

}
