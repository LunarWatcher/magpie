#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/routing/BaseRoute.hpp"
#include "magpie/routing/Compile.hpp"

#include <vector>
#include <tuple>
#include <vector>
#include <string_view>
#include <cstddef>

namespace magpie::routing {

template <FixedString path, data::IsCommonData ContextType>
struct Route : public BaseRoute<ContextType> {
    const RouteCallback<path, ContextType> callback;
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

    virtual void invoke(
        const std::vector<std::string_view>& requestedPath,
        ContextType* context,
        Request& req,
        Response& res
    ) override {
        if constexpr (Size == 0) {
            callback(
                context,
                std::ref(req),
                std::ref(res)
            );
        } else {
            return std::apply(
                [&](auto&&... converted) {
                    callback(
                        context,
                        std::ref(req),
                        std::ref(res),
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


}
