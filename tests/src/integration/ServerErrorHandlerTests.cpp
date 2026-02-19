#include "catch2/catch_test_macros.hpp"
#include "integration/TestApp.hpp"
#include "magpie/application/Methods.hpp"
#include "magpie/transfer/StatusCode.hpp"

TEST_CASE("Fallback route error handler") {
    TestApp app;

    app->route<"/", magpie::Method::Get>([](auto*, auto&, auto& res) {
        res = {
            magpie::Status::ImATeapot,
            "Good girl :3"
        };

        throw std::runtime_error("bad girl >:3");
    });

    app.start();
    using namespace std::literals;

    auto response = app.Get(
        app.url()
    );
    INFO(response.url);
    INFO(response.error.message);
    REQUIRE(response.status_code == magpie::Status::InternalServerError);
}
