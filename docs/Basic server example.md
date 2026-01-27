# Basic server example

## Setting up the context object

The context object is a global passed to every route. Functionally, it acts as userdata in many C APIs.

A default implementation, `magpie::data::CommonData` exists, though creating your own object even if you don't use data is strongly recommended. This makes it easier to add to the object after the fact, especially if you prefer to avoid, or otherwise cannot use `auto` in your function signatures:

```cpp
struct Context : public magpie::data::CommonData {
};
```

You can put anything you want in this struct, but note that it's shared across all threads, including concurrently, and provides no facilities for thread safety. If you need thread safety, you must implement it yourself, or otherwise make sure the things you put in it are safe.

This context object can be used to keep track of session stores, database threadpools, or whatever other global state you have. It's provided to all endpoints and middlewares.

## A basic server

```cpp
int main() {
    // If you don't create and pass the `ctx`, one will be created for you if the Context object
    // is default-initializable. If you use your own, I again strongly recommend just creating
    // it sooner rather than later
    std::shared_ptr<Context> ctx = std::make_shared<Context>();
    magpie::App<Context> app {
        ctx,
        magpie::AppConfig {
            .port = 8080,
            // Note: fromGeneratedCertificate is for test and development use only. You should lock this behind a macro or other form of
            // check. Never use this in a production environment.
            //
            // Due to the use of HTTP/2, a certificate is required if you want to test against browsers,
            // as browsers do not support HTTP/2 over cleartext. If you're creating an API not used by browsers,
            // this is much more optional
            // Note that due to its intended use being in tests and local development, this function provides 0 customisation options,
            // and will never do so. If you wish to generate certs in production, create your 
            // own function or script for it; this is outside magpie's scope.
            .ssl = magpie::SSLConfig::fromGeneratedCertificate(),
        },
    };

    // In the simplest setup, you can just use lambdas. However, the use of a userdata-like
    // object means that if you use namespaces for structuring and use proper functions, 
    // you don't need to `std::bind` it just to get in extra data. Class methods still need
    // to be bound due to the hidden `this` argument, though.
    app.route<"/", magpie::Method::GET>([](Context*, magpie::Request&, magpie::Response& res) {
        res = magpie::Response(
            magpie::Status::OK, "Content"
        );
    });

    // Routes support templates; these are documented elsewhere.
    app.route<"/{string}", magpie::Method::GET>([](auto*, auto&, auto& res, const std::string_view& v) {
        // You don't have to use `res = magpie::Response(...)`; you can also compose the object
        res.code = &magpie::Status::IM_A_TEAPOT;
        res.body = std::format("Hello, {}", v);
    });

    app.run();
}
```
