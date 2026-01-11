# magpie

A C++20 server framework that may or may not end up being less miserable to use. 

## Background

This project largely exists because I keep running into problems with CrowCpp that takes months to years to resolve. Core features like blueprints and middlewares are just bugged, the built-in hardcoding of common mimetypes without extensibility means serving non-standard static files requires a middleware to be typed correctly, and the built-in session middleware has various session invalidation-related vulnerabilities (including building on cryptographically insecure random).

It's a pain in the ass to work with, and I'm done trying. 

## Technical design considerations

### HTTP implementations

This library does not seek to implement _any_ parts of the raw HTTP protocol. Other projects have already solved this problem and, assuming the design is flexible enough, means new protocol support is a matter of library support, and adding an adapter. 

HTTP/2 and newer, though I would prefer using them, is currently not a priority for this project, but the initial goal is to make supporting them easier. To do this, the server core needs to be fully separate from the networking. This was a mistake CrowCpp did, as they both implemented HTTP/1.1 from scratch, [and hard-coded bits of the HTTP/1.1 protocol](https://github.com/CrowCpp/Crow/blob/8236cc320d81c2ddd6053f628cdf08c30b91a780/include/crow/http_response.h#L344). Adding HTTP/2 for CrowCpp means ripping out significant amounts of core logic, whereas the goal here is to cause an absolute bare minimum impact in the core codebase for applications using HTTP protocols that were supported before. 

As an obligatory caveat, I'm not familiar enough with HTTP/2 nor HTTP/3 to tell whether or not the interface is good enough to support their special features, but the goal is for there to be less work to add support for those things in a modular way.

### Data, context, and data 

One of the biggest problems with Crow, at least for my use, is that it's heavily disconnected from state. Middlewares do provide some context, but if you need access to various globals (database thread pool, ...), that (AFAIK) relies heavily on globals and/or `std::bind` on functions that take the context as arguments. This is not great for a number of reasons, so managing global context as well as per-call context is a priority.

## Requirements

* C++20 compiler
* CMake
* `libnghttp2-dev libssl-dev libasio-dev`
    * Note: boost::asio does not work
