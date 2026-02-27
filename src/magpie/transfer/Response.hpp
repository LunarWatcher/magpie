#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <magpie/routing/Compile.hpp>
#include "StatusCode.hpp"
#include "magpie/transfer/adapters/DataAdapter.hpp"

namespace magpie {

struct CompressedResponse;
struct Response {
    /**
     * The response headers.
     */
    std::unordered_map<std::string, std::string> headers;

    const StatusCode* code;

    std::shared_ptr<
        DataAdapter
    > body;
    // TODO: does it really make sense for the content-type to be stored like this?
    std::string contentType = "text/plain";

    Response();
    Response(
        const StatusCode& code,
        std::string&& body,
        std::string&& contentType = "text/plain"
    );
    Response(
        const StatusCode& code,
        std::shared_ptr<DataAdapter>&& bodyAdapter,
        std::string&& contentType
    );

    Response(Response&&) = delete;
    Response(Response&) = delete;

    virtual ~Response() = default;

    void setBody(std::string&& body);

    Response& operator=(Response&& other) = default;
    Response& operator=(CompressedResponse&& other);

    /**
     * Creates a Response object for redirecting to another target.
     *
     * \warning There is no validation on `dest`, so user-generated redirects may end up malicious. Ways to sanitise
     *          the URIs is a future feature.
     *
     * \param out[out]      The request object to modify
     * \param dest          The destination to redirect to. 
     * \param permanent     Whether or not the redirect is permanent
     */
    static void redirect(
        Response& out,
        // TODO: I wonder if it makes sense to introduce a URI object to feal with absolute redirects in the future
        std::string&& dest,
        bool permanent = false
    );

    /**
     * Creates a Response object for redirecting to another target. This function specifically sends MovedPermanently
     * instead of PermanentRedirect or TemporaryRedirect, like Response::redirect does
     *
     * \warning There is no validation on `dest`, so user-generated redirects may end up malicious. Ways to sanitise
     *          the URIs is a future feature.
     *
     * \param out[out]      The request object to modify
     * \param dest          The destination to redirect to. 
     */
    static void moved(
        Response& out,
        std::string&& dest
    );
};

}
