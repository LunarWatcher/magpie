#pragma once

#include "magpie/config/AppConfig.hpp"
#include "magpie/data/CommonData.hpp"
#include "magpie/routing/Compile.hpp"
#include "magpie/routing/Router.hpp"
#include "magpie/transport/TCPServer.hpp"
#include <memory>
#include <type_traits>

namespace magpie {

class BaseApp {
protected:
    const AppConfig config;
public:
    BaseApp(AppConfig&& config) : config(std::move(config)) {}
    ~BaseApp() = default;

    virtual const routing::BaseRouter& getRouter() = 0;

    const AppConfig& getConfig() { return config; }
};

template <data::IsCommonData ContextType = data::CommonData>
class App : public BaseApp {
private:
    transport::TCPServer serv;

    std::shared_ptr<ContextType> dataStore;
    std::shared_ptr<routing::BaseRouter> router;

public:

    App(
        std::shared_ptr<ContextType> dataStore,
        AppConfig&& conf = {}
    ): BaseApp(std::move(conf)),
        serv(this, conf.port, conf.concurrency),
        dataStore(dataStore),
        router(std::make_shared<routing::Router<ContextType>>())
    {

    }

    template <typename = std::enable_if<std::is_trivially_default_constructible_v<ContextType>>>
    App(
        AppConfig&& conf = {}
    ): App(std::make_shared<ContextType>(), std::move(conf)) {

    }
    ~App() = default;


    template <routing::FixedString path>
    void route(
        const routing::RouteCallback<path, ContextType>& callback
    ) {
        std::static_pointer_cast<routing::Router<ContextType>>(router)
            ->template registerRoute<path>(
                callback
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

};

}
