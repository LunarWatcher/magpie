#include "integration/TestApp.hpp"
#include "magpie/data/CommonData.hpp"
#include "magpie/dsa/RadixTree.hpp"
#include "magpie/handlers/StatusHandlers.hpp"
#include "magpie/transfer/Response.hpp"
#include "magpie/transfer/StatusCode.hpp"
#include <catch2/catch_test_macros.hpp>

struct NotFound : public magpie::StatusHandlerNotFound<magpie::data::CommonData> {
    void onRouteNotFound(
        magpie::data::CommonData*,
        magpie::Request&,
        magpie::Response& res,
        magpie::dsa::FindError err
    ) override {
        res = magpie::Response(
            magpie::Status::ImATeapot,
            err == magpie::dsa::FindError::IllegalMethod ? "405 owo" : "404 uwu"
        );
    }
};

struct CustomErrorMessageHandler : public magpie::StatusHandler500<magpie::data::CommonData> {
    void provideErrorResponse(magpie::data::CommonData*, magpie::Request&, magpie::Response& res) override {
        res = magpie::Response(
            magpie::Status::ImATeapot,
            "*boop*"
        );
    }
};

TEST_CASE("StatusHandlerNotFound should wrok as expected on standard routing") {
    TestApp app;

    app->useNotFoundErrorHandler<NotFound>();

    app->route<"/", magpie::Method::Get>([](auto*, magpie::Request&, magpie::Response& res) {
        res = magpie::Response(magpie::Status::OK, "Hewwo");
    });

    app.start();

    SECTION("The / route should work") {
        auto res = app.Get(app.url("/"));
        REQUIRE(res.status_code == magpie::Status::OK);
        REQUIRE(res.text == "Hewwo");
    }

    SECTION("The / route on a wrong HTTP method should return the catchall response") {
        auto res = app.Post(app.url("/"));
        REQUIRE(res.status_code == magpie::Status::ImATeapot);
        REQUIRE(res.text == "405 owo");
    }

    SECTION("/something on a GET should trigger the catchall") {
        auto res = app.Post(app.url("/something"));
        REQUIRE(res.status_code == magpie::Status::ImATeapot);
        REQUIRE(res.text == "404 uwu");
    }

}

TEST_CASE("Custom error handler messages should work") {
    TestApp app;
    app->use500ErrorHandler<CustomErrorMessageHandler>();
    app->route<"/", magpie::Method::Get>([](auto*, auto&, auto&) {
        throw std::runtime_error("1337");
    });
    app.start();

    auto res = app.Get(app.url());
    REQUIRE(res.status_code == magpie::Status::ImATeapot);
    REQUIRE(res.text == "*boop*");
}
