#include "magpie/data/CommonData.hpp"
#include "magpie/routing/Compile.hpp"
#include <catch2/catch_test_macros.hpp>
#include <string_view>

using namespace magpie::routing;

TEST_CASE("Substring equality") {
    STATIC_REQUIRE(
        startsWithAtOffset(
            "{int}", "{int}", 0
        )
    );
    STATIC_REQUIRE_FALSE(
        startsWithAtOffset(
            "{int}", "{int}", 1
        )
    );
    STATIC_REQUIRE(
        startsWithAtOffset(
            "trans rights are human {rights}", "{rights}", 23
        )
    );
}

TEST_CASE("ConstString to FixedString") {
    constexpr auto string = ConstString("hi {string}");
    constexpr auto substring = string
            .subrangeToFixedString<8, 3>();
    auto asStr = std::string(substring.c_str(), substring.size);
    REQUIRE(asStr == "{string}");
}

TEST_CASE("Type expansion") {
    SECTION("No args") {
        constexpr auto params = getParameterTypes<"hi">();
        STATIC_REQUIRE(params.size() == 0);
    }
    SECTION("One arg") {
        constexpr auto params = getParameterTypes<"{string}">();
        STATIC_REQUIRE(params.size() == 1);
        STATIC_REQUIRE(
            std::string(params.at(0).str, params.at(0).size - 1) == std::string("{string}")
        );
        STATIC_REQUIRE(
            params[0].subrangeToFixedString<params[0].size - 1, 0>().data
            == std::array { '{', 's', 't', 'r', 'i', 'n', 'g', '}', '\0' }
        );
        std::function<void(typename TypeInfo<

            params[0].subrangeToFixedString<params[0].size - 1, 0>()
        >::type)> x = [](const std::string_view&) {};
        STATIC_REQUIRE(
            std::is_same_v<
                typename TypeInfo<
                    params[0].subrangeToFixedString<params[0].size - 1, 0>()
                >::type,
                std::string_view
            >
        );
    }
    SECTION("Two args") {
        constexpr auto params = getParameterTypes<"{string}{int}">();
        STATIC_REQUIRE(params.size() == 2);

    }
}

TEST_CASE("Signature validation") {
    SECTION("No args") {
        FunctionSignature<"/trans-rights-are-human-rights", int>::type func = []() {
            return 69;
        };
        REQUIRE(func() == 69);
    }
    SECTION("One arg") {
        std::string source = "I like trains";
        std::string_view buff;
        FunctionSignature<"/test/{string}">::type func = [&buff](const std::string_view& v) {
            buff = v;
        };

        func(source);
        REQUIRE(buff == source);
    }
    SECTION("One arg plus return value") {
        std::string source = "I like trains";
        FunctionSignature<"/test/{string}", int>::type func = [](const std::string_view&) {
            return 69;
        };

        REQUIRE(func(source) == 69);
    }
    SECTION("All types") {
        FunctionSignature<"/{string}/{int}">::type func = [](
            std::string_view, int
        ) {

        };
        FunctionSignature<"/{int}/{string}">::type func2 = [](
            int, std::string_view
        ) {

        };
    }
}

TEST_CASE("Test extras") {
    FunctionSignature<"/{string}", int, magpie::data::CommonData*>::type func = [](
        magpie::data::CommonData* data,
        std::string_view str
    ) {
        REQUIRE(data != nullptr);
        REQUIRE(str == "hi");
        return 69;
    };
    magpie::data::CommonData data;
    REQUIRE(func(&data, "hi") == 69);
}

TEST_CASE("Path component validation") {
    STATIC_REQUIRE_FALSE(
        isValidPath<"{string}">()
    );
    STATIC_REQUIRE(
        isValidPath<"/{string}">()
    );
    STATIC_REQUIRE(
        isValidPath<"/{string}/">()
    );
    STATIC_REQUIRE_FALSE(
        isValidPath<"/v{string}">()
    );
    STATIC_REQUIRE_FALSE(
        isValidPath<"/{string}v">()
    );
    STATIC_REQUIRE_FALSE(
        isValidPath<"/v{string}v">()
    );
    // This is handled by a separate static assert in FunctionSignature. 
    // This is left here in case this behaviour changes, as it will break something if it does change.
    STATIC_REQUIRE(
        isValidPath<"whatever">()
    );
}
