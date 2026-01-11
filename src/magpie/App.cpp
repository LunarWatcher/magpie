#include "App.hpp"

namespace magpie {

App::App(
    const AppConfig& conf
): serv(conf.port, conf.concurrency) {

}

App::~App() {

}

void App::run(
    bool blocking
) {
    if (blocking) {
        serv.start(nullptr);
    } else {
        throw std::runtime_error("Non-blocking run is not yet implemented");
    }
}

}
