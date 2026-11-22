#include "integration/TestApp.hpp"
#include "stc/Random.hpp"
#include <catch2/catch_test_macros.hpp>

#include <stc/StringUtil.hpp>
#include <util/TestCaseGenerator.hpp>

namespace {

HTTP_ENGINE_TEST_CASE("Random body should be parsed correctly, and returned as-is", "[security][fuzzing]") {
    TestApp<> app(context, {
        .limits = {
            .maxSingleHeaderBytes = 50
        }
    });
    app.defaultRoutes();
    app.start();

    std::string payload = stc::random::getAnyRandomChars(100000);
    REQUIRE(payload.size() == 100000);

    auto res = app.Post(
        app.url("/"),
        cpr::Body { payload }
    );
    REQUIRE(res.status_code == 200);
    REQUIRE(res.text.size() == payload.size());

    REQUIRE(
        stc::string::safeString(res.text)
        ==
        stc::string::safeString(payload)
    );
    // Theoretically redundant, but doesn't hurt
    REQUIRE(res.text == payload);
}

}
