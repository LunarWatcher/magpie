#pragma once

#include "magpie/AppDecl.hpp"
#include "magpie/data/CommonData.hpp"
#include "magpie/dsa/RadixTree.hpp"
#include "magpie/except/RouteException.hpp"
#include "magpie/middlewares/MiddlewareProcessor.hpp"
#include "magpie/routing/BaseRouter.hpp"
#include "magpie/routing/Compile.hpp"
#include "magpie/routing/BaseRoute.hpp"
#include "magpie/routing/Route.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
#include "magpie/transfer/StatusCode.hpp"

#include <memory>
#include <string_view>
#include <variant>

namespace magpie::routing {

template <data::IsCommonData ContextType>
class Router : public BaseRouter {
private:
    dsa::RadixTree<std::shared_ptr<BaseRoute<ContextType>>> routes;
public:
    template <FixedString path>
    BaseRoute<ContextType>* registerRoute(
        const RouteCallback<path, ContextType>& callback,
        Method::HttpMethod method
    ) {
        std::shared_ptr<BaseRoute<ContextType>> route = std::make_shared<Route<path, ContextType>>(
            callback
        );
        auto splitPath = pathToComponents(path.c_str());

        this->routes.pushRoute(route, method, splitPath);
        return route.get();
    }

    constexpr void normalisePath(
        const std::string_view& path,
        std::string_view& normPath,
        std::string_view& rawGetArgs
    ) const {
        if (auto pos = path.find_first_of('?'); pos != std::string::npos) {
            rawGetArgs = path.substr(pos);
            normPath = path.substr(0, pos);
        } else {
            normPath = path;
        }
    }

    constexpr std::vector<std::string_view> pathToComponents(const std::string_view& path) const {
        size_t next;
        size_t start = 0;
        std::vector<std::string_view> out {
            "/"
        };
        while ((next = path.find('/', start + 1)) != std::string::npos) {
            if (next - start == 1) { // `/segment//whatever`
                start = next;
            } else if (next != start) { // /[test/] (selection in brackets)
                out.push_back(path.substr(start + 1, (next - start - 1)));
                out.push_back("/");
                start = next;
            }
        }

        // No trailing slash, so pick up whatever's left
        if (start < path.size() - 1) {
            out.push_back(path.substr(start + 1));
        }

        return out;
    }

    void invokeRoute(
        const std::string& path,
        data::CommonData* ctx,
        Request& req,
        Response& res
    ) const override {
        // Compact
        
        if (path.size() == 0 || path[0] != '/') {
            [[unlikely]]
            throw RouteException("Invalid route supplied");
        }

        std::string_view normPath;
        std::string_view rawGetArgs;
        normalisePath(path, normPath, rawGetArgs);
        auto segments = this->pathToComponents(normPath);

        // TODO: handle query params
        auto callback = this->routes.getRoute(segments, req.method);

        std::visit([this, &res, &req, ctx, &segments](auto& it) {
            using T = std::decay_t<decltype(it)>;
            if constexpr (std::is_same_v<dsa::FindError, T>) {
                if (it == dsa::FindError::IllegalMethod) {
                    res = Response(
                        Status::MethodNotAllowed,
                        "405 method not allowed"
                    );
                } else if (it == dsa::FindError::NoMatch) {
                    res = Response(
                        Status::NotFound,
                        "404 not found"
                    );
                }
            } else {

                auto* castCtx = static_cast<ContextType*>(ctx);
                auto* route = it.get();
                MiddlewareProcessor<ContextType>(
                    route,
                    std::vector<Middlewares<ContextType>*> {
                        static_cast<ContextApp<ContextType>*>(castCtx->app)->getMiddlewaresAsPtr(),
                        route->getMiddlewaresAsPtr(),
                    },
                    segments,
                    castCtx,
                    req,
                    res
                ).invokeRoute();
            }
        }, callback);
    }
};

}
