#include "AppConfig.hpp"
#include "magpie/application/Http11Adapter.hpp"
#include "magpie/application/Http2Adapter.hpp"
#include "magpie/application/http2/Nghttp2Callbacks.hpp"

namespace magpie {

std::shared_ptr<application::Adapter> Http11AdapterFactory::get(
    raven::Connection* conn, BaseApp* app
) {
    return std::make_shared<application::Http11Adapter>(conn, app);
}

std::shared_ptr<application::Adapter> Http2AdapterFactory::get(
    raven::Connection* conn, BaseApp* app
) {
    return std::make_shared<application::Http2Adapter>(conn, app);
}

std::optional<raven::SSLConfig>&& Http2AdapterFactory::withSslConfigExtras(std::optional<raven::SSLConfig>&& sslConfig) {
    if (sslConfig.has_value()) {
        SSL_CTX_set_alpn_select_cb(
            sslConfig->getHandle(),
            application::_detail::onAlpnSelectProto,
            nullptr
        );
        SSL_CTX_set_client_hello_cb(
            sslConfig->getHandle(),
            application::_detail::onClientHello,
            nullptr
        );
        SSL_CTX_set_alpn_protos(
            sslConfig->getHandle(),
            (const unsigned char*)"\x02h2",
            3
        );
    }
    return std::move(sslConfig);
}

}
