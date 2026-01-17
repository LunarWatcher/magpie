#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/routing/Compile.hpp"
#include "magpie/transport/TCPServer.hpp"
#include <type_traits>
namespace magpie {

struct AppConfig {
    unsigned short port = 8080;
    unsigned int concurrency = std::thread::hardware_concurrency();
};

template <data::IsCommonData ContextType = data::CommonData>
class App {
private:
    transport::TCPServer serv;

    std::shared_ptr<ContextType> dataStore;

public:

    App(
        std::shared_ptr<ContextType> dataStore,
        const AppConfig& conf = {}
    ): serv(conf.port, conf.concurrency), dataStore(dataStore) {

    }

    template <typename = std::enable_if<std::is_trivially_default_constructible_v<ContextType>>>
    App(
        const AppConfig& conf = {}
    ): App(std::make_shared<ContextType>(), conf) {

    }
    ~App() = default;


    template <routing::FixedString path>
    void route(
        const routing::FunctionSignature<
            path,
            void,
            ContextType*
        >::type& callback
    ) {
    }

    void run(
        bool blocking = true
    ) {
        if (blocking) {
            serv.start(nullptr);
        } else {
            throw std::runtime_error("Non-blocking run is not yet implemented");
        }
    }

    void shutdown() {
        serv.stop();
    }

};

}
