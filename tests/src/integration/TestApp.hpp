#pragma once

#include "magpie/App.hpp"
#include "magpie/config/AppConfig.hpp"
#include "magpie/config/SSLConfig.hpp"
#include "magpie/data/CommonData.hpp"
#include <cpr/cpr.h>
#include <future>
#include <iostream>

template <magpie::data::IsCommonData CtxType = magpie::data::CommonData>
struct TestApp {
    std::shared_ptr<
        magpie::App<CtxType>
    > app;

    bool isSsl;

    std::future<void> runner;

    template <typename = std::enable_if<
        std::is_trivially_default_constructible_v<CtxType>>
    >
    TestApp(
        magpie::AppConfig&& config = {},
        bool autoSsl = true
    ) : TestApp<CtxType>(std::make_shared<CtxType>(), std::move(config), autoSsl) {
    }

    TestApp(
        std::shared_ptr<CtxType> ctx,
        magpie::AppConfig&& config = {},
        bool autoSsl = true
    ) {
        // Used to make the logs somewhat clearer. This should also be made better by me actually getting around to
        // writing a test reporter that isn't shit
        config.port = 0;
        if (autoSsl) {
            config.ssl = std::optional(
                magpie::SSLConfig::fromGeneratedCertificate()
            );
        }
        isSsl = config.ssl.has_value();

        app = std::make_shared<magpie::App<CtxType>>(
            ctx,
            std::move(config)
        );
    }

    void start() {
        using namespace std::literals;
        runner = std::async([&]() { this->app->run(); });
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
        if (this->isSsl) {
            sess.SetVerifySsl(false);
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
        sess.SetTimeout(cpr::Timeout {
            std::chrono::seconds(10)
        });
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
