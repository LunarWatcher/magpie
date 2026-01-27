#pragma once

#include "magpie/data/CommonData.hpp"
#include "magpie/transfer/Request.hpp"
#include "magpie/transfer/Response.hpp"
namespace magpie {

// Initial thoughts on the implementation:
// * I think it makes sense to require forwarding, similar to middlewares in drogon. This gives much better control over
//   the data flow, at least compared to crow's response.end().
//   The next middleware should still be implicit so the chains can be arbitrarily composed
// * I don't know how I want to approach middleware context. Probably makes sense to forward it on the `Request`,
//  something like
//  ```cpp
//  void beforeRequest(...) override {
//      // This function also creates and stores a pointer to the type
//      auto* ctx = response.withContext<MyContextObject>();
//      ...
//      // While we're at it:
//      if (reject) {
//          res = Response(...)
//      } else {
//          // Fuck knows how this'll actually work, since this means every middleware would need information about the
//          // middlewares for the route. It's possible each Route should keep track of this, or at least provide an
//          // interface for it, then we forward the route and somehow figure out how to forward. Somehow because just 
//          // brute-force finding the current middleware and then adding a +1 to the iterator is inefficient.
//          // Actually, we do need the route, because we need to know how to forward if there are no further
//          // middlewares
//          // Also, is this how you invoke a super method? I don't remember and I don't care enough to look it up
//          Middleware::forward(ctx, req, res);
//      }
//  }
//  ```
//  ```cpp
//  app.route<"/">([](auto*, auto& req) {
//      auto& myCtx = req.getContext<MyContextObject>();
//  });
//  ```

template <data::IsCommonData ContextType>
class Middleware {
public:
    virtual ~Middleware() = default;

    virtual void beforeRequest(
        ContextType*,
        Request&,
        Response&
     ) = 0;

    virtual void afterRequest(
        ContextType*,
        Request&,
        Response&
    ) = 0;

};

}
