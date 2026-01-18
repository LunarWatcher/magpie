#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/routing/Compile.hpp"
namespace magpie::routing {

template <data::IsCommonData ContextType>
class Router {
private:

public:
    Router() = default;
    ~Router() = default;

    template <FixedString path>
    void registerRoute(
        const FunctionSignature<
            path,
            void,
            ContextType*
        >::type& callback
    ) {
    }
};

}
