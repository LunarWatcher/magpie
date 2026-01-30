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

    /**
     * This is a regression test for a bug where an off-by-one error that was correct at the time of writing was missed
     * when `/` was moved to its own, dedicated path segment. A -1 meant to remove a / from the segment caused two
     * separate bugs:
     * 1. With the old setup, a trailing {int} would not necessarily have a slash at the end, so the -1 could 
     *    cause incorrect validation.
     * 2. With the new setup, a trailing non-int character could be smuggled past the initial validation filter. 
     *    This is not in any way severe, but would just cause somewhat unexpected routing behaviour under certain
     *    edge-cases, and likely a HTTP 500 instead of a HTTP 404.
     */
    SECTION("Semi-valid ints with one invalid character should be rerouted to string") {
        auto res = app.Get(app.url("/multi/621a"));
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
