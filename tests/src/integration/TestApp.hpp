#pragma once

#include "magpie/App.hpp"
#include "magpie/config/AppConfig.hpp"
#include "magpie/config/SSLConfig.hpp"
#include <cpr/cpr.h>
#include <future>
#include <iostream>

struct TestApp {
    std::shared_ptr<
        magpie::App<>
    > app;

    bool isSsl;

    std::future<void> runner;
    TestApp(
        magpie::AppConfig&& config = {}
    ) {
        // Used to make the logs somewhat clearer. This should also be made better by me actually getting around to
        // writing a test reporter that isn't shit
        std::cout << "-------------------- BEGIN NEW TEST --------------------" << std::endl;
        config.port = 0;
        config.ssl = std::optional(
            magpie::SSLConfig::fromGeneratedCertificate()
        );
        app = std::make_shared<magpie::App<>>(
            std::move(config)
        );

        isSsl = config.ssl.has_value();
    }
    void start() {
        using namespace std::literals;
        runner = std::async([&]() { this->app->run(); });
        std::this_thread::sleep_for(1s);
    }

    ~TestApp() {
        app->shutdown();
        runner.get();
    }

    std::string baseUrl() {
        return std::format(
            "{}://localhost:{}",
            isSsl ? "https" : "http",
            this->app->getPort()
        );
    }

    std::string url(const std::string& route = "") {
        if (route.size() != 0 && route[0] != '/') {
            throw std::runtime_error("Programmer error: route must start with / (or be empty)");
        }
        return baseUrl() + route;
    }

    magpie::App<>* operator->() {
        return app.get();
    }

    void injectDefault(cpr::Session& sess) {
        sess.SetVerifySsl(false);
        sess.SetHttpVersion(
            cpr::HttpVersion {
                cpr::HttpVersionCode::VERSION_2_0
            }
        );
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

    /**
     * Utility wrapper around cpr that sets default SSL, ALPN, and HTTP version options.
     * Should be preferred over using cpr::{HttpMethod} directly to avoid boilerplate.
     */
    template <typename... Ts>
    cpr::Response Post(Ts&&... ts) {
        cpr::Session session;
        injectDefault(session);
        cpr::priv::set_option(session, CPR_FWD(ts)...);
        return session.Post();
    }
};
