# magpie

A C++23 server framework that may or may not end up being less miserable to use. 

## Background

This project largely exists because I keep running into problems with CrowCpp that takes months to years to resolve. Core features like blueprints and middlewares are just bugged, the built-in hardcoding of common mimetypes without extensibility means serving non-standard static files requires a middleware to be typed correctly, and the built-in session middleware has various session invalidation-related vulnerabilities, and building on cryptographically insecure random.

It's a pain in the ass to work with, and I'm done trying. 

## Current status

This is yet another backburner project. It's not expected to be testable for a bare minimum webapp for at least a few months. The API is not stable, and will break regularly.

## Technical design considerations

### HTTP implementations

Part of the goal of the server is to be more flexible in which HTTP implementations are supported. To do this, the server core needs to be fully separate from the networking. This was a mistake CrowCpp did, as they both implemented HTTP/1.1 from scratch, [and hard-coded bits of the HTTP/1.1 protocol](https://github.com/CrowCpp/Crow/blob/8236cc320d81c2ddd6053f628cdf08c30b91a780/include/crow/http_response.h#L344) in core logic. Adding HTTP/2 for CrowCpp means ripping out significant amounts of core logic, whereas the goal here is to cause an absolute bare minimum impact in the core codebase for applications using HTTP protocols that were supported before.

Initially, the goal was to fully separate the parsers as well, though with the inclusion of HTTP/1.1, this is no longer the case. This is also due to nghttp2 turning to AI slop machines, and many libraries in general using AI slop machines.

Beyond support for HTTP, magpie can theoretically support any TCP-based protocol. In the future, this will expand to UDP as well, though this requires upstream changes to raven, the underlying socket library. This is made possible due to the architecture of magpie separating the protocol translation layer from everything else, so it can be hotswapped arbitrarily - with a bit of effort at least. At some point, a proof-of-concept [gemini protocol](https://geminiprotocol.net/docs/protocol-specification.gmi) (not to be confused with Google's environment-destroying AI slop machine) may be implemented to put that to the test.

### Data, context, and data

One of the biggest problems with Crow, at least for my use, is that it's heavily disconnected from state. Middlewares do provide some context, but if you need access to various globals (database thread pool, ...), that (AFAIK) relies heavily on globals and/or `std::bind` on functions that take the context as arguments. This is not great for a number of reasons, so managing global context as well as per-call context is a priority.

### Sane standards

HTTPS and compression are both core parts of the internet at this point, so they're both bundled automatically.

## Requirements

* C++23 compiler
* CMake
* `libnghttp2-dev libssl-dev`
    * Note: `libnghttp2-dev` currently doesn't work on Linux Mint due to the package being out of date.
    * Another dependency, [raven](https://codeberg.org/LunarWatcher/raven) is bundled with magpie.

## License

MIT. See the LICENSE file.
