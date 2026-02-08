#include "integration/TestApp.hpp"
#include "magpie/App.hpp"
#include "magpie/transfer/StatusCode.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cpr/cpr.h>
#include <cpr/ssl_options.h>
#include <cpr/verbose.h>

TEST_CASE("Test plain routing", "[integration]") {
    TestApp app;

    app->route<"/", magpie::Method::Get>([](auto*, auto&, auto& res) {
        res = {
            magpie::Status::ImATeapot,
            "Good girl :3"
        };
    });

    app.start();
    using namespace std::literals;

    SECTION("Can access route as intended") {
        REQUIRE(app.url().str().starts_with("https://"));

        auto response = app.Get(
            app.url()
        );
        INFO(response.url);
        INFO(response.error.message);
        REQUIRE(response.status_code == magpie::Status::ImATeapot);
        REQUIRE(response.text == "Good girl :3");
        REQUIRE(response.header.at("content-type") == "text/plain");

    }
    SECTION("HTTP methods are respected") {
        auto response = app.Post(
            app.url()
        );
        REQUIRE(response.status_code == magpie::Status::MethodNotAllowed);
    }
}

TEST_CASE("Test plain routing without SSL", "[integration]") {
    TestApp app{{}, false};

    app->route<"/", magpie::Method::Get>([](auto*, auto&, auto& res) {
        res = {
            magpie::Status::ImATeapot,
            "Good girl :3"
        };
    });

    app.start();
    using namespace std::literals;

    SECTION("Can access route as intended") {
        REQUIRE(app.url().str().starts_with("http://"));

        auto response = app.Get(
            app.url()
        );
        INFO(response.url);
        INFO(response.error.message);
        REQUIRE(response.status_code == magpie::Status::ImATeapot);
        REQUIRE(response.text == "Good girl :3");
        REQUIRE(response.header.at("content-type") == "text/plain");

    }
}

TEST_CASE("Test argument routing", "[integration]") {
    using namespace std::literals;
    TestApp app;

    cpr::Response response;
    SECTION("GET") {
        app->route<"/{string}", magpie::Method::Get>([](auto*, magpie::Request&, magpie::Response& res, const std::string_view& v) {
            res = magpie::Response(
                magpie::Status::OK,
                std::format("Server got {}", v)
            );
        });

        app.start();

        auto wrongMethod = app.Post(
            app.url("/hewwo")
        );
        REQUIRE(wrongMethod.status_code == magpie::Status::MethodNotAllowed);

        response = app.Get(
            app.url("/hewwo")
        );
    }

    SECTION("POST") {
        app->route<"/{string}", magpie::Method::Post>([](auto*, magpie::Request&, magpie::Response& res, const std::string_view& v) {
            res = magpie::Response(
                magpie::Status::OK,
                std::format("Server got {}", v)
            );
        });

        app.start();

        auto wrongMethod = app.Get(
            app.url("/hewwo")
        );
        REQUIRE(wrongMethod.status_code == magpie::Status::MethodNotAllowed);

        response = app.Post(
            app.url("/hewwo")
        );

    }

    if (!app) {
        FAIL("Test section failed");
    }

    REQUIRE(app.url().str().starts_with("https://"));
    INFO(response.url);
    INFO(response.error.message);
    REQUIRE(response.status_code == magpie::Status::OK);
    REQUIRE(response.text == "Server got hewwo");
    REQUIRE(response.header.at("content-type") == "text/plain");
}

TEST_CASE("The server should allow arbitrarily large responses") {
    
    TestApp app;

    cpr::Response response;
    app->route<"/", magpie::Method::Get>([](auto*, magpie::Request&, magpie::Response& res) {
        std::stringstream data;

        for (size_t i = 0; i < 20'000'000; ++i) {
            data << "a";
        }

        res = magpie::Response(
            magpie::Status::OK,
            data.str()
        );
    });

    app.start();

    SECTION("Unencoded") {
        auto res = app.Get(
            app.url("/"),
            cpr::Header {
                { "Accept-Encoding", "identity" },
            }
        );
        REQUIRE(res.status_code == magpie::Status::OK);
        REQUIRE(res.text.size() == 20'000'000);
        // TODO: Not sure if this tests what I want it to test. Doesn't look like there's a combined transfer (body +
        // headers), so not sure if this actually reflects the network size, or if this is just res.text.size() with
        // extra steps.
        REQUIRE(res.downloaded_bytes == 20'000'000);

        for (auto& ch : res.text) {
            if (ch != 'a') {
                // FAIL lets us avoid 20 million assertions, which would just fuck over the assertion stats
                FAIL("Bad character: " << ch);
            }
        }
    }

}
