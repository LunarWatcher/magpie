#pragma once

#include "magpie/routing/Compile.hpp"
#include <vector>
#include <string_view>

namespace magpie::routing {


template <FixedString path, data::IsCommonData ContextType>
using RouteCallback = FunctionSignature<path, void, ContextType*>::type;

template <data::IsCommonData ContextType>
struct BaseRoute {
    virtual ~BaseRoute() = default;

    virtual void invoke(
        const std::vector<std::string_view>& requestedRoute,
        ContextType* context
    ) = 0;
};

}
