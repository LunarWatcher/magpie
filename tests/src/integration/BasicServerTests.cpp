#include "integration/TestApp.hpp"
#include "magpie/App.hpp"
#include "magpie/transfer/StatusCode.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cpr/cpr.h>
#include <cpr/ssl_options.h>
#include <cpr/verbose.h>

TEST_CASE("Test plain routing", "[integration]") {
    TestApp app;

    app->route<"/", magpie::Method::GET>([](auto*, auto&, auto& res) {
        res = {
            magpie::Status::IM_A_TEAPOT,
            "Good girl :3"
        };
    });

    app.start();
    using namespace std::literals;

    SECTION("Can access route as intended") {
        REQUIRE(app.url().starts_with("https://"));

        auto response = app.Get(
            cpr::Url {
                app.url()
            }
        );
        INFO(response.url);
        INFO(response.error.message);
        REQUIRE(response.status_code == magpie::Status::IM_A_TEAPOT);
        REQUIRE(response.text == "Good girl :3");
        REQUIRE(response.header.at("content-type") == "text/plain");

    }
    SECTION("HTTP methods are respected") {
        auto response = app.Post(
            cpr::Url {
                app.url()
            }
        );
        REQUIRE(response.status_code == magpie::Status::METHOD_NOT_ALLOWED);
    }
}

TEST_CASE("Test argument routing", "[integration]") {
    TestApp app;

    app->route<"/{string}", magpie::Method::GET>([](auto*, magpie::Request&, magpie::Response& res, const std::string_view& v) {
        res = magpie::Response(
            magpie::Status::OK,
            std::format("Server got {}", v)
        );
    });

    app.start();
    using namespace std::literals;

    REQUIRE(app.url().starts_with("https://"));

    auto response = app.Get(
        cpr::Url {
            app.url("/hewwo")
        }
    );

    INFO(response.url);
    INFO(response.error.message);
    REQUIRE(response.status_code == magpie::Status::OK);
    REQUIRE(response.text == "Server got hewwo");
    REQUIRE(response.header.at("content-type") == "text/plain");
}
