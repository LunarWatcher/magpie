#pragma once

#include "magpie/application/http2/Nghttp2Callbacks.hpp"
#include "magpie/config/AppConfig.hpp"
#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <asio/ssl/context.hpp>
#include <atomic>
#include <cstdint>

namespace magpie::internals {

struct Worker {
    asio::io_context ioContext;
    std::optional<asio::ssl::context> sslContext;
    asio::executor_work_guard<decltype(ioContext)::executor_type> workGuard;
    std::atomic<uint32_t> workload = 0;

    Worker(
        const AppConfig& conf
    ) : 
        sslContext(conf.ssl.and_then([](const SSLConfig& ssl) {
            asio::ssl::context ctx(
                asio::ssl::context::sslv23
            );
            ctx.use_certificate_chain_file(ssl.certFile);
            ctx.use_private_key_file(ssl.keyFile, asio::ssl::context::pem);

            SSL_CTX_set_alpn_select_cb(
                ctx.native_handle(),
                application::_detail::onAlpnSelectProto,
                nullptr
            );
            SSL_CTX_set_client_hello_cb(
                ctx.native_handle(),
                application::_detail::onClientHello,
                nullptr
            );
            SSL_CTX_set_alpn_protos(
                ctx.native_handle(),
                (const unsigned char*)"\x02h2",
                3
            );
            return std::optional(std::move(ctx));
        })),
        workGuard(asio::make_work_guard(ioContext))
    {}
    Worker(Worker&) = delete;
    Worker(Worker&&) = delete;
};

}
