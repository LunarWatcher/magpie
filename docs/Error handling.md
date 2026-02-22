# Error handling

The majority of request-oriented error handling is outsourced to the endpoints, which means the implementation varies between servers. This means various forms of input validation is handled in each endpoint implementation, with the exception of input parsing that happens at such a low level that it happens before the user-written code is invoked. 

This page is instead about handling 404, 405, and 500 errors. In the future, this will also cover 401 and 403 errors issued by authentication components. 

401 and 403-405 happens at a low enough level that it's before user-controlled middlewares. However, the handlers are still invoked within the standard global 

500 errors also have a special low-level handler that's invoked outside the standard control flow. To override 500 errors with contextual responses, you can add a middleware somewhere in your chain. This is described later.

## 401 and 403

Not implemented yet

## 404 and 405

> [!note]
> This handler bypasses the middleware chain, including global middlewares. This may be changed in the future.

404 and 405 are both handled by the same handler. A default implementation is provided, but you can override it if you wish:
```cpp
struct Context : public magpie::data::CommonData {};
struct NotFound : public magpie::StatusHandlerNotFound<Context> {
    void onRouteNotFound(
        Context*,
        magpie::Request&,
        magpie::Response& res,
        magpie::dsa::FindError err
    ) override {
        res = magpie::Response(
            magpie::Status::ImATeapot,
            err == magpie::dsa::FindError::IllegalMethod ? "405 owo" : "404 uwu"
        );
    }
};
```

A convenience method is available for setting it:
```cpp
app->useNotFoundErrorHandler<NotFound>();
```

The default assumption is that these handlers do not take any constructor arguments; it's generally better practice to pass what you need through the `Context` object

## 500

### Default handler

> [!note]
> This handler bypasses the middleware chain, including global middlewares. This may be changed in the future.

The default handler is used at low-level routing. These handlers therefore bypass the entire middleware chain, which means you cannot assume the context for any middlewares have been populated.

This handling method is still useful for REST APIs or other APIs where there isn't a user interface that benefits from context to be able to render the page properly. Alternatively, you need to find other ways to inject the data.

The default handler is similar to the 404/405 handler, and has a special pre-defined interface you can use:
```cpp
struct Context : public magpie::data::CommonData {};
struct CustomErrorMessageHandler : public magpie::StatusHandler500<Context> {
    void provideErrorResponse(Context*, magpie::Request&, magpie::Response& res) override {
        res = magpie::Response(
            magpie::Status::ImATeapot,
            "*boop*"
        );
    }
};
```

Again like the StatusHandlerNotFound, there's a convenience method for setting it:
```cpp
app->use500ErrorHandler<CustomErrorMessageHandler>();
```

The 500 error handler has two methods you can override: `provideErrorResponse` and `tryCall`.

Override `provideErrorResponse` if you only care about modifying the message. 

Override `tryCall` if you want to modify the logging or the error handling itself, for example by catching different exception types. `tryCall` is the function that provides the actual error catching, so if you override it, you are responsible for providing a `try-catch`. This is the default implementation:
```cpp
virtual void tryCall(ContextType*, Request&, Response&, const std::function<void()>& errorHandled) {
    try {
        errorHandled();
        return;
    } catch (const std::exception& e) {
        logger::error("{}", e.what());
    } catch (...) {
        logger::error("Caught non-exception type");
    }
    provideErrorResponse(ctx, req, res);
}
```

You are not required to call `provideErrorResponse` if you override `tryCall`; you can inline all the response logic within `tryCall`. The arguments to `provideErrorResponse` are intentionally restricted, as its only goal is to provide the minimum amount of data required to generate common error messages.

### Middleware handler

If you need error handling that depends more heavily on context, you can create a middleware for it. Creating a middleware is documented in the Middleware doc comments. 

If you want to extend a middleware with error handling capabilities, all you need is a `try-catch` around the call to `next()`:
```cpp
void onRequest(
    magpie::IMiddlewareProcessor<Context>* proc,
    Context* ctx,
    magpie::Request& req,
    magpie::Response& res
) override {
    try {
        next(proc, ctx, req, res);
    } catch (const std::exception& e) {
        // handle exception
    } catch (...) {
        // ...
    }
}
```

As long as you keep your middleware chain in order, you can place it somewhere that gets you all the error context you need. Note, however, that placing it early in the chain gives you the ability to catch errors in the middlewares after it, but means you cannot assume you have the context objects from those middlewares. Adding it later to the chain means you can assume those context objects are available, but obviously means you can't error handle those middlewares with your error middleware.
