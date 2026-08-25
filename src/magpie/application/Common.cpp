#include "Common.hpp"
#include "magpie/AppDecl.hpp"

namespace magpie::application {

void common::setIpAddr(
    BaseApp* app,
    raven::Connection* conn,
    std::shared_ptr<Request>& req
) {
    const auto& config = app->getConfig();
    if (!config.trustXRealIp) {
        req->ipAddr = conn->getIP();
    } else {
        auto header = req->headers.find("x-real-ip");
        if (header == req->headers.end()) {
            req->ipAddr = conn->getIP();
        } else {
            req->ipAddr = header->second;
        }
    }
}

}
