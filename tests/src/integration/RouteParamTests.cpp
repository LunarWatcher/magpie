#include "catch2/catch_test_macros.hpp"
#include "integration/TestApp.hpp"
#include "magpie/transfer/Response.hpp"

TEST_CASE("Test type capturing") {
    TestApp app;
    app->route<"/string/{string}", magpie::Method::Get>(
        [](auto*, auto&, auto& res, std::string_view v) {
            res = magpie::Response(
                magpie::Status::OK,
                std::string(v)
            );
        }
    );
    app->route<"/int/{int}", magpie::Method::Get>(
        [](auto*, auto&, auto& res, int64_t v) {
            res = magpie::Response(
                magpie::Status::OK,
                std::to_string(v)
            );
        }
    );
    app->route<"/multi/{string}", magpie::Method::Get>(
        [](auto*, auto&, auto& res, std::string_view) {
            res = magpie::Response(
                magpie::Status::OK,
                "string"
            );
        }
    );
    app->route<"/multi/{int}", magpie::Method::Get>(
        [](auto*, auto&, auto& res, int64_t) {
            res = magpie::Response(
                magpie::Status::OK,
                "int"
            );
        }
    );

    app.start();

    SECTION("Strings should be captured") {
        auto res = app.Get(app.url("/string/owo%20x3%20rawr%20nuzzles"));
        INFO(res.error.message);
        REQUIRE(res.status_code == 200);
        REQUIRE(res.text == "owo%20x3%20rawr%20nuzzles");
    }

    SECTION("Ints should be captured") {
        auto res = app.Get(app.url("/int/621"));
        INFO(res.error.message);
        REQUIRE(res.status_code == 200);
        REQUIRE(res.text == "621");
    }

    SECTION("Ints should take priority over strings when both {int} and {string} is present on a route") {
        auto res = app.Get(app.url("/multi/621"));
        INFO(res.error.message);
        REQUIRE(res.status_code == 200);
        REQUIRE(res.text == "int");
    }

    SECTION("Semi-valid ints should be rerouted to string") {
        auto res = app.Get(app.url("/multi/621abcd"));
        INFO(res.error.message);
        REQUIRE(res.status_code == 200);
        REQUIRE(res.text == "string");
    }

    SECTION("Negative numbers should be legal") {
        auto res = app.Get(app.url("/int/-69"));
        INFO(res.error.message);
        REQUIRE(res.status_code == 200);
        REQUIRE(res.text == "-69");
    }

    SECTION("Doubles should not be coerced to ints") {
        auto res = app.Get(app.url("/multi/-69.420"));
        INFO(res.error.message);
        REQUIRE(res.status_code == 200);
        REQUIRE(res.text == "string");
    }

    SECTION("Failed resolution against a pure /{int} route is a 404") {
        auto res = app.Get(app.url("/int/-69.420"));
        INFO(res.error.message);
        REQUIRE(res.status_code == 404);
    }
}
