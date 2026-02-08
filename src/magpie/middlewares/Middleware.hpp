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
    virtual void invokeRoute() = 0;
};

/**
 * Base definition for middlewares used by magpie.
 *
 * There's no technical distinction between global and scoped middlewares in their implementation. The call order for
 * middlewares is:
 * 1. Global
 * 2. [Reserved (Not implemented): blueprint middlewares]
 * 3. Per-route middlewares
 *
 * The order within the middlewares is defined by their declaration order. Middlewares can therefore be arbitrarily
 * chained by their declaration order. See App::registerGlobalMiddlewares and BaseRoute::registerMiddlewares
 *
 * Chaining is controlled by the `next` function. To call the next step of the chain, invoke `next`. Every middleware
 * must invoke `next`. See the documentation for the `next` function for more information about the chaining. 
 * Example middleware:
 * ```cpp
 * class ExampleMiddleware : public magpie::Middleware<MiddlewareTestContext> {
 * public:
 *     void onRequest(
 *         magpie::IMiddlewareProcessor<MiddlewareTestContext> *proc,
 *         MiddlewareTestContext* ctx,
 *         magpie::Request& req,
 *         magpie::Response& res
 *     ) override {
 *         if (req.headers.contains(std::string("x-reject"))) {
 *             // Here, the middleware responds to the request. By not calling `next()`, this middleware ends the chain.
 *             // The route and any middlewares after this point are not invoked.
 *             res = magpie::Response (
 *                 magpie::Status::Gone,
 *                 "Now you're gone"
 *             );
 *         } else {
 *             // Here, we forward to the next handler, either a middleware or a route.
 *
 *             // This may not be present in the final response. If the endpoint does move assignment (the default
 *             // recommendation), any headers set here will be wiped. It's strongly advised you don't alter the request
 *             // until _after_ the `next` call.
 *             res.headers["before"] = "yes";
 *             // Anything before the call to `next` is invoked before the route. Anything after is either invoked after
 *             // the route, or after a middleware later in the chain has created a response. 
 *             next(proc, ctx, req, res);
 *             // Request modification after the response is set works as expected, so the `after` header will always be
 *             // present, barring any earlier middlewares further modifying the headers or response.
 *             res.headers["after"] = "yes";
 *         }
 *     }
 * 
 * };
 * ```
 */
template <data::IsCommonData ContextType>
class Middleware {
public:
    virtual ~Middleware() = default;

    /**
     * Called when a request is about to be handled.
     *
     * The `next` function must be called somewhere in this function. The call to `next` is required for a route to be
     * invoked, as well as for the next middleware in the chain to be invoked. 
     *
     * Conceptually, the `onRequest` call can be thought of as a function containing:
     * ```cpp
     * void onRequest(...) {
     *     beforeRequest();
     *     next();
     *     afterRequest();
     * }
     * ```
     *
     * However, the `next` function being exposed enables the middlewares to also become error handlers. An error
     * handler middleware can simply put a try-catch around the call to `next` to handle exceptions. This is only
     * necessary if you want a special exception format, as there is a default exception handler built into magpie to
     * keep the server alive through most standard exceptions.
     *
     * \param proc      The middleware processor. This is exclusively used to pass forward to the `next` call, to tell
     *                  what to invoke next.
     * \param ctx       The context object. Can be used by the middleware.
     * \param req       The request object. Can be used by the middleware.
     * \param res       The response object. Can be used by the middleware.
     */
    virtual void onRequest(
        IMiddlewareProcessor<ContextType>* proc,
        ContextType* ctx,
        Request& req,
        Response& res
    ) = 0;

    /**
     * Forwards to the next middleware, or to the route of the current middleware is the last middleware in the chain.
     * The call to `next` separates `before` and `after` handlers. Callers do not need to worry about the inner working
     * of this function, only the semantics it implies.
     *
     * ## Invocation order
     *
     * As described in the documentation for this class, the middlewares are called in the same order they're declared.
     * Because the `next` function directly invokes the next step of the chain, everything after the next is invoked in
     * reverse order. The first middleware to be entered is the last to be exited.
     */
    void next(
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

/**
 * Meta-container for middlewares. Currently just a typed vector in disguise.
 */
template <data::IsCommonData ContextType>
struct Middlewares {
    std::vector<std::shared_ptr<Middleware<ContextType>>> middlewares;
};

}
