#include "Http11Adapter.hpp"

#include "magpie/App.hpp"
#include "magpie/application/Common.hpp"
#include "magpie/utility/ErrorHandler.hpp"

namespace magpie::application {
Http11Adapter::Http11Adapter(
    raven::Connection* conn,
    BaseApp* app
) : app(app), conn(conn) {
    
}

bool Http11Adapter::parse(
    const raven::Buffer& buff,
    std::size_t readBytes
) {
    if (readBytes > 0) {
        auto result = this->state.read(
            buff,
            readBytes
        );

        switch (result) {
        case http11::ServerAction::ContinueRead:
            break;
        case http11::ServerAction::SetResponse: {
            const auto& router = app->getRouter();

            auto& req = this->state.req;
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
            common::setIpAddr(app, conn, req);
            auto res = std::make_shared<Response>();

            utility::runWithErrorLogging([&]() {
                router.invokeRoute(
                    // TODO: now that request->path exists, this argument is unnecessary.
                    req->path,
                    app->getContext(),
                    *req,
                    *res
                );
            }, res.get());
            // TODO: don't override content-type set manually
            // TODO: standardise (this part in particular is duplicated in the HTTP/2 adapter)
            res->headers["content-type"] = res->contentType;

            state.setResponse(res);
        } break;
        case http11::ServerAction::WriteResponse:
            // TODO: can't actually do a write here
            break;
        case http11::ServerAction::KillRequest:
            [[fallthrough]];
        default:
            conn->close();
            break;
        }
    }

    // Not currently in use
    return true;
}

bool Http11Adapter::onWriteReady(
    // TODO: why do we take connection in here? We already cache it, and the adapter is per-connection
    raven::Connection* conn,
    raven::Buffer& buff
) {
    state.write(conn, buff);
    // Not currently in use (TODO: nuke? I don't remember what this was for)
    return true;
}

}
