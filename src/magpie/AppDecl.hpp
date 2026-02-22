#pragma once

#include "magpie/config/AppConfig.hpp"
#include "magpie/handlers/StatusHandlers.hpp"
#include "magpie/middlewares/Middleware.hpp"
#include "magpie/routing/BaseRouter.hpp"

namespace magpie {

/**
 * Base class for the app. This is used in places where the exact type of the app does not matter, and/or cannot be
 * derived.
 */
class BaseApp {
protected:
    const AppConfig config;
public:
    BaseApp(AppConfig&& config) : config(std::move(config)) {}
    virtual ~BaseApp() = default;

    virtual const routing::BaseRouter& getRouter() = 0;

    virtual data::CommonData* getContext() const = 0;
    const AppConfig& getConfig() { return config; }
};

/**
 * Base class for the parts of the app using the ContextType. This can be used in places where the base methods are
 * required, but App cannot be used directly due to circular dependencies.
 *
 * TODO: The CommonData class should probably be made into a template as well so we avoid this extra layer of
 * indirection.
 */
template <data::IsCommonData ContextType = data::CommonData>
class ContextApp : public BaseApp {
public:
    std::shared_ptr<StatusHandlerNotFound<ContextType>> notFoundErrorHandler 
        = std::make_shared<StatusHandlerNotFound<ContextType>>();
    std::shared_ptr<StatusHandler500<ContextType>> errorHandler 
        = std::make_shared<StatusHandler500<ContextType>>();

    ContextApp(AppConfig&& config) : BaseApp(std::move(config)) {}
    virtual ~ContextApp() = default;

    template <typename T>
    void useNotFoundErrorHandler() {
        notFoundErrorHandler = std::make_shared<T>();
    }

    template <typename T>
    void use500ErrorHandler() {
        errorHandler = std::make_shared<T>();
    }

    virtual Middlewares<ContextType>* getMiddlewaresAsPtr() = 0;
};

}
