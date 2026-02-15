# TODO

## Internals

* `OPTIONS` should probably be handled separately from all the other HTTP methods. Might be worth hooking up per-method catchalls to allow this. Capturing `OPTIONS` without manually registering a handler is necessary for CORS middlewares to be possible. This will likely require some restructuring in how HTTP methods are handled. Alternatively, CORS could not be a middleware, but a catchall of some kind. Still not sure how I want the catchall API to look though
* The data adapters should not care about nghttp2, but should proxy their stuff through something else to signal EOF. This'll make it easier to support HTTP/3 down the line. I think HTTP/1.1 might not be worth implementing at this point

