#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/dsa/RadixTree.hpp"
#include "magpie/logger/Logger.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
#include "magpie/transfer/StatusCode.hpp"

namespace magpie {

template <data::IsCommonData Context>
struct StatusHandlerNotFound {
    virtual ~StatusHandlerNotFound() = default;

    /**
     * \param err   A special error primarily used internally that's used to differentiate 404s from 405s. See FindError
     *              for more info
     */
    virtual void onRouteNotFound(
        Context*, Request&, Response& res,
        dsa::FindError err
    ) {
        if (err == dsa::FindError::IllegalMethod) {
            res = Response(
                Status::MethodNotAllowed,
                "405 method not allowed"
            );
        } else if (err == dsa::FindError::NoMatch) {
            res = Response(
                Status::NotFound,
                "404 not found"
            );
        }
    }
};

template <data::IsCommonData Context>
struct StatusHandler500 {
    virtual ~StatusHandler500() = default;

    /**
     * Provides the actual error response. This is what you want to override if you don't care much about the actual
     * error handling process, but want to customise the response itself.
     */
    virtual void provideErrorResponse(Context*, Request&, Response& res) {
        res = Response(
            magpie::Status::InternalServerError,
            "500 Internal Server Error"
        );
    }
    /**
     * Defines the actual error handler. This function must invoke provideErrorResponse if it's a generic interface.
     * provideErrorResponse can be omitted if you inline the error handling logic. 
     *
     * \param errorHandled A callback representing the code to be error-checked
     */
    virtual void tryCall(
        Context* ctx,
        Request& req,
        Response& res,
        const std::function<void()>& errorHandled
    ) {
        try {
            errorHandled();
            return;
        } catch (const std::exception& e) {
            logger::error("{}", e.what());
        } catch (...) {
            logger::error("Caught non-exception type");
        }
        provideErrorResponse(ctx, req, res);
    }
};

}
