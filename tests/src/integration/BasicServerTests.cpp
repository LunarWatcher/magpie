#include "integration/TestApp.hpp"
#include "magpie/App.hpp"
#include "magpie/transfer/StatusCode.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cpr/cpr.h>
#include <cpr/ssl_options.h>
#include <cpr/verbose.h>

TEST_CASE("Test plain routing", "[integration]") {
    TestApp app;

    app->route<"/">([](auto*, auto&) {
        return magpie::Response(
            magpie::Status::IM_A_TEAPOT,
            "Good girl :3"
        );
    });

    app.start();
    using namespace std::literals;

    REQUIRE(app.url().starts_with("https://"));

    auto response = app.Get(
        cpr::Url {
            app.url()
        }
    );
    INFO(response.url);
    INFO(response.error.message);
    REQUIRE(response.status_code == 418);
    REQUIRE(response.text == "Good girl :3");
    REQUIRE(response.header.at("content-type") == "text/plain");
}

TEST_CASE("Test argument routing", "[integration]") {
    TestApp app;

    app->route<"/{string}">([](auto*, magpie::Request&, const std::string_view& v) {
        return magpie::Response(
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
    REQUIRE(response.status_code == 200);
    REQUIRE(response.text == "Server got hewwo");
    REQUIRE(response.header.at("content-type") == "text/plain");
}
