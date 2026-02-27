#include "integration/TestApp.hpp"
#include "magpie/application/Methods.hpp"
#include "magpie/transfer/Response.hpp"
#include "magpie/transfer/StatusCode.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cpr/cpr.h>

TEST_CASE("Test basic relative redirects") {
    TestApp app;
    app->route<"/", magpie::Method::Get>([](auto*, auto&, magpie::Response& res) {
        magpie::Response::redirect(res, "/target", false);
    });
    app->route<"/target", magpie::Method::Get>([](auto*, auto&, magpie::Response& res) {
        res = magpie::Response(
            magpie::Status::OK,
            "/target hit"
        );
    });

    app.start();
    SECTION("Validate redirect (no follow)") {
        auto response = app.Get(
            app.url(),
            cpr::Redirect{false}
        );
        REQUIRE(response.status_code == magpie::Status::TemporaryRedirect);
        REQUIRE(response.header["location"] == "/target");
    }
    SECTION("Validate redirect (follow)") {

        auto response = app.Get(
            app.url(),
            cpr::Redirect{true}
        );
        REQUIRE(response.status_code == 200);
        REQUIRE(response.text == "/target hit");
    }
}

TEST_CASE("Verify that permanent redirects return the proper code") {
    TestApp app;
    app->route<"/", magpie::Method::Get>([](auto*, auto&, magpie::Response& res) {
        magpie::Response::redirect(res, "/target", true);
    });

    app.start();
    auto response = app.Get(
        app.url(),
        cpr::Redirect{false}
    );
    REQUIRE(response.status_code == magpie::Status::PermanentRedirect);
    REQUIRE(response.header["location"] == "/target");
}

TEST_CASE("Verify that MovedPermanently works") {
    TestApp app;
    app->route<"/", magpie::Method::Get>([](auto*, auto&, magpie::Response& res) {
        magpie::Response::moved(res, "/target");
    });
    app->route<"/target", magpie::Method::Get>([](auto*, auto&, magpie::Response& res) {
        res = magpie::Response(
            magpie::Status::OK,
            "/target hit"
        );
    });

    app.start();
    SECTION("Validate redirect (no follow)") {
        auto response = app.Get(
            app.url(),
            cpr::Redirect{false}
        );
        REQUIRE(response.status_code == magpie::Status::MovedPermanently);
        REQUIRE(response.header["location"] == "/target");
    }
    SECTION("Validate redirect (follow)") {

        auto response = app.Get(
            app.url(),
            cpr::Redirect{true}
        );
        REQUIRE(response.status_code == 200);
        REQUIRE(response.text == "/target hit");
    }
}
