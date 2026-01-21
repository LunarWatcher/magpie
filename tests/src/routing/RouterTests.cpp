#include "internals/ParameterisedTest.hpp"
#include "magpie/routing/Router.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Path splitting", "[Router]") {
    // TODO: rewrite with test::ParameterisedTest (and also add a default executor so I don't need to specify it for
    // trivial return setups)
    std::vector<
        std::tuple<
            std::string,
            std::vector<std::string_view>,
            std::string
        >
    > testCases = {
        { "/", {} , "A single slash has no components"},
        {"/test", {"test"}, "A standard route includes no slashes"},
        {"/test/path/whatever", {"test", "path", "whatever"}, "A long standard route includes no slashes"},
        {"//", {}, "Double slashes are normalised"},
        {"///////", {}, "A bunch of slashes are normalised"},
        {"/test//whatever", {"test", "whatever"}, "Double slashes are normalised in long paths"},
        {"/a///b//c", {"a", "b", "c"}, "Multi-slashes are normalised in short path segments"},
        {"/a///b//c////", {"a", "b", "c"}, "Multi-slashes are normalised in short path segments with trailing slashes"},
    };

    magpie::routing::Router<magpie::data::CommonData> router;

    for (auto& [value, expected, name] : testCases) {
        SECTION(name) {
            REQUIRE(
                router.pathToComponents(value) == expected
            );
        }
    }
}

TEST_CASE("Path normalisation", "[Router]") {
    using namespace std::placeholders;
    magpie::routing::Router<magpie::data::CommonData> router;
    using TestType = test::ParameterisedTest<std::pair<std::string_view, std::string_view>,
          std::string_view>;

    TestType(
        {
            { { "/test" }, { "/test", "" }, "Simple route is not changed" },
            { { "/?whatever" }, {"/", "?whatever"}, "Simple params are separated" },
            { { "/whatever?test/whatever" }, {"/whatever", "?test/whatever"}, "Params with slash don't fuck the system" },
            { { "/whatever???test/whatever" }, {"/whatever", "???test/whatever"}, "Double question marks is not this system's problem" },
        }, 
        [&](const auto& data) {
            std::string_view normPath, rawGetArgs;
            data.forward(
                std::bind(&decltype(router)::normalisePath, &router, _1, std::ref(normPath), std::ref(rawGetArgs))
            );
            auto [expectedNormPath, expectedRawGetArgs] = data.output;
            REQUIRE(normPath == expectedNormPath);
            REQUIRE(rawGetArgs == expectedRawGetArgs);
        }
    ).run();
}
