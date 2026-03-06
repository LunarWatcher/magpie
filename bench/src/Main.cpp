#include "magpie/App.hpp"
#include "magpie/config/SSLConfig.hpp"
#include <magpie/data/CommonData.hpp>

struct Context : public magpie::data::CommonData {};

int main() {
    magpie::App<Context> mApp(magpie::AppConfig {
        .port = 60696,
        .bindAddr = "0.0.0.0",
        .ssl = magpie::SSLConfig::fromGeneratedCertificate(),
    });
    mApp.route<"/", magpie::Method::Get>([](auto*, auto&, magpie::Response& res) {
        res = magpie::Response(
            magpie::Status::OK,
            R"({"message": "hi"})"
        );
    });
    mApp.route<"/stop", magpie::Method::Get>([](Context* ctx, auto&, auto&) {
        ctx->app->shutdown();
    });
    // magpie::logger::config().logger = nullptr;
    // std::cout << "Server live at https://0.0.0.0:60696. Logging is disabled" << std::endl;

    mApp.run();
}
