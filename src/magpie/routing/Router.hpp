#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/dsa/RadixTree.hpp"
#include "magpie/except/RouteException.hpp"
#include "magpie/routing/Compile.hpp"
#include "magpie/routing/BaseRoute.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
#include "magpie/transfer/StatusCode.hpp"
#include <tuple>

namespace magpie::routing {

template <FixedString path, data::IsCommonData ContextType>
struct Route : public BaseRoute<ContextType> {
    RouteCallback<path, ContextType> callback;
    constexpr static size_t Size = guessParams<path>();
    constexpr static inline auto typeTuples = getForwardableIndices<path>();

    Route(
        const RouteCallback<path, ContextType>& callback
    ) : callback(callback) {

    }

    template <std::size_t... I>
    constexpr auto typeToFuncArgs(const std::vector<std::string_view>& requestedPath, std::index_sequence<I...>) {
        return std::tuple {
            TypeInfo<
                FixedString<typeTuples.at(I).first.size - 1>(typeTuples.at(I).first)
                // "{string}"
            >::convert(
                requestedPath.at(
                    std::get<1>(typeTuples.at(I))
                )
            )...
        };
    }

    virtual Response invoke(
        const std::vector<std::string_view>& requestedPath,
        ContextType* context,
        Request& req
    ) override {
        if constexpr (Size == 0) {
            return callback(context, req);
        } else {
            return std::apply(
                [&](auto&&... converted) {
                    return callback(
                        context,
                        req,
                        std::forward<decltype(converted)>(converted)...
                    );
                },
                typeToFuncArgs(
                    requestedPath,
                    std::make_index_sequence<typeTuples.size()>{}
                )
            );
        }
    }

};

class BaseRouter {
public:
    virtual ~BaseRouter() = default;
    virtual Response invokeRoute(
        const std::string& path,
        Request& req
    ) const = 0;
};

template <data::IsCommonData ContextType>
class Router : public BaseRouter {
private:
    dsa::RadixTree<std::shared_ptr<BaseRoute<ContextType>>> routes;
public:
    template <FixedString path>
    void registerRoute(
        const RouteCallback<path, ContextType>& callback
    ) {
        std::shared_ptr<BaseRoute<ContextType>> route = std::make_shared<Route<path, ContextType>>(
            callback
        );
        auto splitPath = pathToComponents(path.c_str());

        this->routes.pushRoute(route, splitPath);
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
        std::vector<std::string_view> out;
        while ((next = path.find('/', start + 1)) != std::string::npos) {
            // `/segment//whatever`
            if (next - start == 1) {
                start = next;
            } else if (next != start) {
                out.push_back(path.substr(start + 1, (next - start - 1)));
                start = next;
            }
        }

        // No trailing slash, so pick up whatever's left
        if (start < path.size() - 1) {
            out.push_back(path.substr(start + 1));
        }

        return out;
    }

    Response invokeRoute(
        const std::string& path,
        Request& req
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
        auto callback = this->routes.getRoute(segments);

        if (callback) {
            auto& ptr = *callback;
            return ptr->invoke(
                segments,
                nullptr,
                req
            );
        } else {
            // 404
            return Response(
                Status::NOT_FOUND,
                "404 not found"
            );
        }
        
    }
};

}
