#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
namespace magpie {

template <data::IsCommonData ContextType>
class Middleware {
public:
    virtual ~Middleware() = default;

    virtual void onRequest(
        ContextType*,
        Request&,
        Response&
    ) = 0;

    virtual void forward(ContextType ctx, Request& req, Response& res) {

    }

};

}
