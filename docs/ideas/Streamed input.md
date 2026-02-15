# Ideas for streamed input

## The problem

In streamed mode, the HTTP adapter becomes an event-driven system with two components that wish to control the data flow: the actual connection (input), and the endpoint (consumer; seeks to control the input rate).

This necessarily forces any implementation to use two threads and a callback for syncing when a new block is sent. 

This would convert streamed endpoints into a hybrid endpoint:
```cpp
app.route<...>([](auto*, auto& req, auto& res) {
    req.setChunkCallback([](auto& chunk) {

    });
    req.waitForBodyTransferred();
    // Do stuff
});
```

But also means spawning more threads, and more multithreading means more ways to fuck things up. `waitForBodyTransferred` now has to be an abort point for the thread that has to be ensured notified by at least one destructor. This is messy, error-prone, and introduces half a million ways to footgun. There needs to be a better way to do it.

## Scoping the streaming

In many applications, there's not going to be a reason nor a way to stream most input data. If you're operating on megabytes of raw JSON input, that might be a use-case, but at some point, we'd transition from input to file input. If the server needs to work on the JSON, it still needs to be loaded into memory at some point, whereas if it's file upload, the endpoints may not need to actually care.

If we scope streaming to file upload, that simplifies some things, especially since multipart requests are another complicating factor I haven't mentioned yet. 

There's [nothing that prevents multipart requests from having multiple files](https://stackoverflow.com/a/913749), which means there's a risk of operating on multiple files. Though not as applicable for backends for websites[^1], the request can also just contain a raw file. 

The problem then of streaming a body shifts into "how do you describe one or more file download handlers in the endpoint?". This is somewhat more manageable.

Starting with 935ab9d5, `app.route<...>()` returns a BaseRoute instance that can be used for further configuration. 935ab9d5 only added support for middlewares, but it wouldn't be unreasonable to expand the scope to allowing a second handler: an `onStreamedFileCallback` or something to that effect.

This would enable
```cpp
app.route<...>([](auto*, auto& req, auto& res) {
    auto files = req.getFiles();
    // ... validate files

    res = magpie::Response(...);
})
    ->onStreamedFileCallback([](auto& req, magpie::Multipart& multipartMetadata, const std::string& chunk) {
        // Do whatever and write the file
    });
```

This still has state management issues, since there optimally should be a way to persist an `std::ofstream` so it doesn't have to keep being reopened. 

Additionally, this would let the data flow completely bypass middlewares unless some real cursed shit is done first. The middlewares currently form a callstack chain so stack semantics can be used to create `before` and `after` in a single function. 

However, it could be argued that auth shouldn't be a middleware at all, but rather be a post-header-callback so authentication is done before any data is received at all. Very much in the same spirit as the `onStreamedFileCallback`, this would be an `onAuthCheckCallback` called immediately after processing the headers. This would force some reduction in the usefulness of middlewares, but in at least some ways, this is already going to be the case. 

One other comlpicating factor with this approach is precisely `req.getFiles()`, and an equivalent system for `onAuthCheckCallback`; they both have a state requirement that they'd have to pass to the endpoint. That said, I do question whether or not this ends up being overengineering the problem. From recent experience, authentication is a Whole Thing:tm: in some cases.

... on the other hand:tm:, storing a multipart object in the request may end up being a requirement anyway, and that multipart object can be a special object that either provides the raw content, or that provides the path to the file that was uploaded.

---

That said, both of these do have functionality that approach ktor's plugins. I don't really like ktor plugins, because they're mostly just weird middlewares in most cases, but it would make a lot of sense for both files, auth, and possibly for content negotiation. Most of my server implementations with crow so far have not given a single fuck about the input headers for content types, because checking it is an annoyance. 

CORS/`OPTION` is already planned to be a separate pseudo-middleware that basically hijacks the `OPTION` http method with a similar setup. Though that discussion ties just as much into the discussion for how catchalls should be implemented.

Although this strategy as a whole may make certain things more difficult without just disabling streaming, I think these special handlers might just end up being the better way to handle this and many other things. There are certain things that are just so low-level that everything else depends on it anyway, so it wouldn't be too unreasonable to do it this way.

[^1]: AFAIK, forms always end up with a multipart request
