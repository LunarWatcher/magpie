#include <catch2/catch_test_macros.hpp>
#include "integration/TestApp.hpp"
#include "util/TestCaseGenerator.hpp"

namespace {

HTTP_ENGINE_TEST_CASE("Header count limits should be respected", "[limits][security]") {
    if (context->isHttp2()) {
        SKIP("HTTP/2 does not implement limits");
    }
    TestApp<> app(context, {
        .limits = {
            .maxHeaders = 1
        }
    });
    app.defaultRoutes();
    app.start();

    auto res = app.Get(app.url("/"));
    REQUIRE(
        res.status_code == 400
    );
    REQUIRE(
        res.text == ""
    );
    REQUIRE(
        res.header["content-length"] == "0"
    );
}

HTTP_ENGINE_TEST_CASE("Long headers should be rejected", "[limits][security]") {
    if (context->isHttp2()) {
        SKIP("HTTP/2 does not implement limits");
    }
    TestApp<> app(context, {
        .limits = {
            .maxSingleHeaderBytes = 50
        }
    });
    app.defaultRoutes();
    app.start();

    SECTION("Reference") {
        auto res = app.Get(app.url("/"));
        REQUIRE(res.status_code == 200);
        REQUIRE(res.text == "Good girl");
    }

    SECTION("Long value") {
        auto res = app.Get(
            app.url("/"),
            cpr::Header {
                {"Key", "This is a sufficiently long header value that must exceed 50 bytes"}
            }
        );
        REQUIRE(
            res.status_code == 400
        );
        REQUIRE(
            res.text == ""
        );
        REQUIRE(
            res.header["content-length"] == "0"
        );
    }
    
    SECTION("Long key") {
        auto res = app.Get(
            app.url("/"),
            cpr::Header {
                {"KeyWithALotOfBytesToTestTheLimitsAndMakeSureIDidntDoADumDum", "v"}
            }
        );
        REQUIRE(
            res.status_code == 400
        );
        REQUIRE(
            res.text == ""
        );
        REQUIRE(
            res.header["content-length"] == "0"
        );
    }
    SECTION("Both illegal") {
        auto res = app.Get(
            app.url("/"),
            cpr::Header {
                {"ThisShouldBeAround25Bytes", "ThisShouldBeAround25Bytes"}
            }
        );
        REQUIRE(
            res.status_code == 400
        );
        REQUIRE(
            res.text == ""
        );
        REQUIRE(
            res.header["content-length"] == "0"
        );
    }
}

HTTP_ENGINE_TEST_CASE("Body length limits should be respected", "[limits][security]") {
    if (context->isHttp2()) {
        SKIP("HTTP/2 does not implement limits");
    }
    TestApp<> app(context, {
        .limits = {
            .maxBodyBytes = 2
        }
    });
    app.defaultRoutes();
    app.start();

    auto res = app.Post(
        app.url("/"),
        cpr::Body {
            "owo x3 rawr"
        }
    );
    REQUIRE(
        res.status_code == 400
    );
    REQUIRE(
        res.text == ""
    );
    REQUIRE(
        res.header["content-length"] == "0"
    );
}

}
