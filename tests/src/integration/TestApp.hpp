#pragma once

#include "magpie/App.hpp"
#include "magpie/config/AppConfig.hpp"
#include "raven/config/SSLConfig.hpp"
#include "magpie/data/CommonData.hpp"
#include "util/TestCaseGenerator.hpp"
#include <cpr/cpr.h>
#include <future>

template <magpie::data::IsCommonData CtxType = magpie::data::CommonData>
struct TestApp {
    std::shared_ptr<
        magpie::App<CtxType>
    > app;

    bool isSsl;

    std::future<void> runner;
    test::EngineTestContext* testContext;

    template <typename = std::enable_if<
        std::is_trivially_default_constructible_v<CtxType>>
    >
    TestApp(
        test::EngineTestContext* testContext,
        magpie::AppConfig&& config = {},
        bool autoSsl = true
    ) : TestApp<CtxType>(testContext, std::make_shared<CtxType>(), std::move(config), autoSsl) {
    }

    TestApp(
        test::EngineTestContext* testContext,
        std::shared_ptr<CtxType> ctx,
        magpie::AppConfig&& config = {},
        bool autoSsl = true
    ) : testContext(testContext) {
        // Used to make the logs somewhat clearer. This should also be made better by me actually getting around to
        // writing a test reporter that isn't shit
        config.port = 0;
        config.adapterFactory = testContext->adapterFactory;
        std::optional<raven::SSLConfig> sslConf = std::nullopt;
        if (autoSsl) {
            sslConf.emplace(
                raven::SSLConfig(
                    "certs/tests/cert.pem",
                    "certs/tests/key.pem",
                    true
                )
            );
        }
        isSsl = sslConf.has_value();

        app = std::make_shared<magpie::App<CtxType>>(
            ctx,
            std::move(config),
            std::move(sslConf)
        );
    }

    void start() {
        using namespace std::literals;
        runner = std::async([&]() { this->app->run(); });
        this->app->sync();
        magpie::logger::info("Server live");
    }

    ~TestApp() {
        if (runner.valid()) {
            app->shutdown();
            runner.get();
        }
    }

    std::string test() {
        return "hi";
    }

    std::string baseUrl() {
        return std::format(
            "{}://localhost:{}",
            isSsl ? "https" : "http",
            this->app->getPort()
        );
    }

    cpr::Url url(const std::string& route = "") {
        if (route.size() != 0 && route[0] != '/') {
            throw std::runtime_error("Programmer error: route must start with / (or be empty)");
        }
        return baseUrl() + route;
    }

    magpie::App<CtxType>* operator->() {
        return app.get();
    }

    void injectDefault(cpr::Session& sess) {
        if (auto ptr = dynamic_cast<test::HttpEngineTestContext*>(this->testContext); testContext != nullptr) {
            if (this->isSsl) {
                sess.SetVerifySsl(false);
            }
            if (ptr->version == test::HttpVersion::Http2) {
                if (this->isSsl) {
                    sess.SetHttpVersion(
                        cpr::HttpVersion {
                            cpr::HttpVersionCode::VERSION_2_0
                        }
                    );
                } else {
                    sess.SetHttpVersion(
                        cpr::HttpVersion {
                            cpr::HttpVersionCode::VERSION_2_0_PRIOR_KNOWLEDGE
                        }
                    );
                }
            }
            sess.SetTimeout(
                cpr::Timeout {
                    std::chrono::seconds(10)
                }
            );
        } else {
            FAIL("tried to use injectDefault with non-HTTP test context.");
        }
    }

    /**
     * Utility wrapper around cpr that sets default SSL, ALPN, and HTTP version options.
     * Should be preferred over using cpr::{HttpMethod} directly to avoid boilerplate.
     */
    template <typename... Ts>
    cpr::Response Get(Ts&&... ts) {
        cpr::Session session;
        injectDefault(session);
        cpr::priv::set_option(session, std::forward<Ts>(ts)...);
        return session.Get();
    }

    operator bool() const { return this->runner.valid(); }

    /**
     * Utility wrapper around cpr that sets default SSL, ALPN, and HTTP version options.
     * Should be preferred over using cpr::{HttpMethod} directly to avoid boilerplate.
     */
    template <typename... Ts>
    cpr::Response Post(Ts&&... ts) {
        cpr::Session session;
        injectDefault(session);
        cpr::priv::set_option(session, std::forward<Ts>(ts)...);
        return session.Post();
    }
};
