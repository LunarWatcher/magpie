#pragma once

#include "magpie/AppDecl.hpp"
#include "magpie/application/Methods.hpp"
#include "magpie/config/AppConfig.hpp"
#include "magpie/data/CommonData.hpp"
#include "magpie/middlewares/Middleware.hpp"
#include "magpie/routing/Compile.hpp"
#include "magpie/routing/Router.hpp"
#include "magpie/transport/TCPServer.hpp"
#include <memory>
#include <type_traits>

namespace magpie {

template <data::IsCommonData ContextType = data::CommonData>
class App : public ContextApp<ContextType> {
private:
    transport::TCPServer serv;

    std::shared_ptr<ContextType> dataStore;
    std::shared_ptr<routing::BaseRouter> router;

    std::shared_ptr<Middlewares<ContextType>> middlewares;

public:

    App(
        std::shared_ptr<ContextType> dataStore,
        AppConfig&& conf = {}
    ): ContextApp<ContextType>(std::move(conf)),
        serv(this, conf.port, conf.concurrency),
        dataStore(dataStore),
        router(std::make_shared<routing::Router<ContextType>>()),
        middlewares(std::make_shared<Middlewares<ContextType>>())
    {
        dataStore->app = (BaseApp*) this;
    }

    template <typename = std::enable_if<std::is_trivially_default_constructible_v<ContextType>>>
    App(
        AppConfig&& conf = {}
    ): App(std::make_shared<ContextType>(), std::move(conf)) {

    }
    ~App() = default;

    /**
     * Adds a route. 
     *
     * Each call to this method is only allowed to specify one HTTP method. This is because it's considered bad practice
     * to support more than one HTTP method per function call. However, you can still get around this by calling the
     * route function several times with the same function.
     *
     * HTTP methods have distinct meanings per the standard. It's therefore arguably never appropriate to use the same
     * handler for different methods. If you have code in common, instead of using the same handler, move that code into
     * reusable modules.
     */
    template <
        routing::FixedString path,
        Method::HttpMethod method
    >
    void route(
        const routing::RouteCallback<path, ContextType>& callback
    ) {
        std::static_pointer_cast<routing::Router<ContextType>>(router)
            ->template registerRoute<path>(
                callback,
                method
            );
    }

    void run() {
        serv.start();
    }

    void shutdown() {
        serv.stop();
    }

    uint16_t getPort() {
        return this->serv.getPort();
    }

    const routing::BaseRouter& getRouter() override {
        return *this->router;
    }

    data::CommonData* getContext() const override {
        return this->dataStore.get();
    }

    void registerGlobalMiddlewares(
        const std::vector<std::shared_ptr<Middleware<ContextType>>>& middlewares
    ) {
        this->middlewares->middlewares = middlewares;
    }
    
    Middlewares<ContextType>* getMiddlewaresAsPtr() override {
        return this->middlewares.get();
    }

};

}
