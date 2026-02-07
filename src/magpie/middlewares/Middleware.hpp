#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
#include <memory>
namespace magpie {

template <data::IsCommonData ContextType>
class Middleware;

/**
 * Base predeclaration of the middleware processor because circular dependency.
 */
template <data::IsCommonData ContextType>
struct IMiddlewareProcessor {
    virtual ~IMiddlewareProcessor() = default;
    virtual Middleware<ContextType>* getNext() = 0;
    virtual void invokeRoute(
        const std::vector<std::string_view>& requestedPath, // used for template shit in the Route
        ContextType* ctx,
        Request& req,
        Response& res
    ) = 0;
};

template <data::IsCommonData ContextType>
class Middleware {
public:
    virtual ~Middleware() = default;

    virtual void onRequest(
        IMiddlewareProcessor<ContextType>* proc,
        ContextType*,
        Request&,
        Response&
    ) = 0;

    virtual void next(
        IMiddlewareProcessor<ContextType>* proc,
        ContextType* ctx,
        Request& req,
        Response& res
    ) {
        auto* ptr = proc->getNext();
        if (ptr == nullptr) {
            return;
        }
        ptr->onRequest(
            proc,
            ctx,
            req,
            res
        );
    }

};

template <data::IsCommonData ContextType>
struct Middlewares {
    std::vector<std::shared_ptr<Middleware<ContextType>>> middlewares;
};

}
