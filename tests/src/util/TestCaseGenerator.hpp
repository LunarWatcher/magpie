#pragma once
#include "magpie/config/AppConfig.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/internal/catch_unique_name.hpp>

namespace test {

enum class HttpVersion {
    Http11,
    Http2
};

/**
 * Base context class for engine tests.
 *
 * In the future, this class may be abstract and used to provide more specific details about the protocol. For now, it
 * only contains the adapterFactory required to create the specific type of engine being tested.
 */
struct EngineTestContext {
    const magpie::AdapterFactory adapterFactory;

    EngineTestContext(const magpie::AdapterFactory& adapterFactory) : adapterFactory(adapterFactory) {}
    virtual ~EngineTestContext() = default;
};

struct HttpEngineTestContext : public EngineTestContext {
    const HttpVersion version;

    HttpEngineTestContext(
        HttpVersion version,
        const magpie::AdapterFactory& adapterFactory
    ) : EngineTestContext(adapterFactory), version(version) {}
};

struct EngineTestFixture {
    virtual ~EngineTestFixture() = default;

    void withHttp11() {
        HttpEngineTestContext ctx {
            HttpVersion::Http11,
            std::make_shared<magpie::Http11AdapterFactory>()
        };
        testCase(&ctx);
    }
    void withHttp2() {
        HttpEngineTestContext ctx {
            HttpVersion::Http2,
            std::make_shared<magpie::Http2AdapterFactory>()
        };
        testCase(&ctx);
    }

    virtual void testCase(
        EngineTestContext* context
    ) = 0;
};

}

#define INTERNAL_HTTP_ENGINE_TEST_CASE(TestIdentifier, Name, ...)       \
    struct TestIdentifier : public test::EngineTestFixture {            \
        virtual void testCase(test::EngineTestContext* context) override; \
    };                                                                  \
    TEST_CASE_METHOD(TestIdentifier, "HTTP/1.1: " Name, "[http11]" __VA_ARGS__) { \
        withHttp11();                                                   \
    }                                                                   \
    TEST_CASE_METHOD(TestIdentifier, "HTTP/2: " Name, "[http2]" __VA_ARGS__) { \
        withHttp2();                                                    \
    }                                                                   \
    void TestIdentifier::testCase(test::EngineTestContext* context)

#define HTTP_ENGINE_TEST_CASE(Name, ...)                \
    INTERNAL_HTTP_ENGINE_TEST_CASE(                     \
        INTERNAL_CATCH_UNIQUE_NAME(MAGPIE_TEST_IMPL),   \
        Name,                                           \
        __VA_ARGS__                                     \
    )

