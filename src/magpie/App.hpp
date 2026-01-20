#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/logger/Logger.hpp"
#include "magpie/routing/Compile.hpp"
#include "magpie/routing/Router.hpp"
#include "magpie/transport/TCPServer.hpp"
#include <memory>
#include <type_traits>
namespace magpie {

struct AppConfig {
    unsigned short port = 8080;
    unsigned int concurrency = std::thread::hardware_concurrency();
};

class BaseApp {
public:
    ~BaseApp() = default;
    virtual const routing::BaseRouter& getRouter() = 0;
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
        const AppConfig& conf = {}
    ): 
        serv(this, conf.port, conf.concurrency),
        dataStore(dataStore),
        router(std::make_shared<routing::Router<ContextType>>())
    {

    }

    template <typename = std::enable_if<std::is_trivially_default_constructible_v<ContextType>>>
    App(
        const AppConfig& conf = {}
    ): App(std::make_shared<ContextType>(), conf) {

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

    void run(
        bool blocking = true
    ) {
        if (blocking) {
            serv.start();
        } else {
            throw std::runtime_error("Non-blocking run is not yet implemented");
        }
    }

    void shutdown() {
        serv.stop();
    }

    const routing::BaseRouter& getRouter() override {
        return *this->router;
    }

};

}
