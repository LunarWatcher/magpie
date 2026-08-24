# TODO

## Internals

* `OPTIONS` should probably be handled separately from all the other HTTP methods. Might be worth hooking up per-method catchalls to allow this. Capturing `OPTIONS` without manually registering a handler is necessary for CORS middlewares to be possible. This will likely require some restructuring in how HTTP methods are handled. Alternatively, CORS could not be a middleware, but a catchall of some kind. Still not sure how I want the catchall API to look though
* If setting headers becomes a template, can the map use `string_view` instead to avoid making copies of strings because they'll be stored in a fixed place in the binary or something like that?
* We probably need a way to signal a receive timeout or something, though I'm not sure if this is useful or not. It's a (D)DoS risk though, since it'll allow for exhaustion. In theory anyway, not sure if it's an issue with nginx/similar in the middle
