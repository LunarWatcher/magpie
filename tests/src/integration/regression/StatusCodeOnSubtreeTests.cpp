#include "catch2/catch_test_macros.hpp"
#include "integration/TestApp.hpp"
#include "magpie/transfer/StatusCode.hpp"

TEST_CASE("Non-existing routes that are part of a subtree of a route that does exist should 404") {
    TestApp app;
    app->route<"/some/route", magpie::Method::Get>([](auto*, auto&, magpie::Response& res) {
        res = magpie::Response(
            magpie::Status::OK,
            "owo"
        );
    });
    app.start();

    SECTION("The route should work") {
        auto res = app.Get(app.url("/some/route"));
        REQUIRE(res.status_code == magpie::Status::OK);
    }

    SECTION("The route should 405 on POST") {
        auto res = app.Post(app.url("/some/route"));
        REQUIRE(res.status_code == magpie::Status::MethodNotAllowed);
    }

    SECTION("Extra route components should 404") {
        auto res = app.Get(app.url("/some/route/with/extra/shit"));
        REQUIRE(res.status_code == magpie::Status::NotFound);
    }

    SECTION("Insufficient route components should 404 on the same method") {
        auto res = app.Get(app.url("/some"));
        REQUIRE(res.status_code == magpie::Status::NotFound);
    }

    SECTION("Insufficient route components should 404 on a different method") {
        auto res = app.Post(app.url("/some"));
        REQUIRE(res.status_code == magpie::Status::NotFound);
    }
}
