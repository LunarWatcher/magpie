#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/middlewares/Middleware.hpp"
#include "magpie/routing/BaseRoute.hpp"

namespace magpie {

template <data::IsCommonData ContextType>
class MiddlewareProcessor : public IMiddlewareProcessor<ContextType> {
private:
    routing::BaseRoute<ContextType>* route;
    std::vector<Middlewares<ContextType>*> middlewareGroups;

    size_t middlewareBlock;
    size_t currPtr;

    const std::vector<std::string_view>& requestedPath;
    ContextType* ctx;
    Request& req;
    Response& res;

    bool invoked = false;

public:
    [[nodiscard("Discarding the processor means no routing happens")]]
    MiddlewareProcessor(
        routing::BaseRoute<ContextType>* route,
        std::vector<Middlewares<ContextType>*>&& middlewares,
        const std::vector<std::string_view>& requestedPath, // used for template shit in the Route
        ContextType* ctx,
        Request& req,
        Response& res
    ) : 
        route(route),
        middlewareGroups(std::move(middlewares)),
        middlewareBlock(0),
        currPtr(0),
        requestedPath(requestedPath),
        ctx(ctx),
        req(req),
        res(res)
    {}

    void invokeRoute() override {
        auto first = getNext();
        if (first) {
            first->onRequest(
                this, ctx, req, res
            );
        }
        
    }

    Middleware<ContextType>* getNext() override {
        if (middlewareBlock >= middlewareGroups.size()) {
            if (invoked) {
                throw std::runtime_error(
                    "Do not invoke next twice"
                );
            }
            invoked = true;
            this->route->invoke(
                requestedPath,
                ctx,
                req,
                res
            );
            return nullptr;
        }
        auto* currBlock = middlewareGroups.at(middlewareBlock);
        if (
            // Should never happen
            currBlock == nullptr
            // Block empty or ptr exceeds middleware
            || currPtr >= currBlock->middlewares.size()
        ) {
            // Increment block, reset ptr, try again
            ++middlewareBlock;
            currPtr = 0;
            return getNext();
        } 

        return currBlock->middlewares.at(currPtr++).get();
    }

};

}
