#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/middlewares/Middleware.hpp"
#include "magpie/routing/BaseRoute.hpp"

namespace magpie {

template <data::IsCommonData ContextType>
struct MiddlewareProcessor : public IMiddlewareProcessor<ContextType> {
    routing::BaseRoute<ContextType>* route;
    std::vector<Middlewares<ContextType>*> middlewareGroups;

    size_t middlewareBlock;
    size_t currPtr;

    [[nodiscard("Discarding the processor means no routing happens")]]
    MiddlewareProcessor(
        routing::BaseRoute<ContextType>* route,
        std::vector<Middlewares<ContextType>*>&& middlewares
    ) : 
        route(route),
        middlewareGroups(std::move(middlewares)),
        middlewareBlock(0),
        currPtr(0)
    {}

    void invokeRoute(
        const std::vector<std::string_view>& requestedPath, // used for template shit in the Route
        ContextType* ctx,
        Request& req,
        Response& res
    ) override {
        auto first = getNext();
        if (first) {
            first->onRequest(
                this, ctx, req, res
            );
        }
        
        if (middlewareBlock == middlewareGroups.size()) {
            // A quirk of the middleware system is that currBlock is always incremented even if there is no next such
            // block, because the error checking is done later;
            // This adds some safety, and means that completion is equal to the middlewareBlockPtr exceeding the list of
            // middlewares. 
            // If this is met, we know we've reached the end and can forward to the route.
            this->route->invoke(
                requestedPath,
                ctx,
                req,
                res
            );
        }
    }

    Middleware<ContextType>* getNext() override {
        if (middlewareBlock >= middlewareGroups.size()) {
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
