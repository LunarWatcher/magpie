#include "AppConfig.hpp"
#include "magpie/application/Http11Adapter.hpp"
#include "magpie/application/Http2Adapter.hpp"


std::shared_ptr<magpie::application::Adapter> magpie::http11Adapter(
    raven::Connection* conn, BaseApp* app
) {
    return std::make_shared<application::Http11Adapter>(conn, app);
}

std::shared_ptr<magpie::application::Adapter> magpie::http2Adapter(
    raven::Connection* conn, BaseApp* app
) {
    return std::make_shared<application::Http2Adapter>(conn, app);
}
