---
{
    "title": "Configuring a server"
}
---
# Configuring a server

All general server configuration is done via `magpie::AppConfig` that's passed as an argument to `magpie::App`. 

## Standard options

```cpp
int main() {
    std::shared_ptr<Context> ctx = std::make_shared<Context>();
    magpie::App<Context> app {
        ctx,
        magpie::AppConfig {
            // The port to run on. Only one port is supported
            .port = 8080,
            // How many threads to use for handling requests
            .concurrency = std::thread::hardware_concurrency(),
            // the address to bind on. Defines how the server is accessible
            .bindAddr = "127.0.0.1",
            // Whether or not to trust the X-Real-IP header. This header must be correctly
            // set in reverse proxy config for this to be safe to use.
            .trustXRealIp = false,
            // Which protocol to use. Defaults to HTTP/1.1 if HTTP/2 is disabled, and
            // HTTP/2 otherwise.
            // This may be non-HTTP protocols as well, though additional code is required on the implementer
            // side to support things like non-HTTP status codes.
            .adapterFactory = std::make_shared<Http11AdapterFactory>(),
            // see the section "HTTP/1.1: security limits". This only takes effect with HTTP/1.1
            .limits = {},
        }
    };
```

## HTTP/1.1: security limits

> [!note]
>
> New in 0.4.0

`AppConfig` has a `limits` field, which is a struct of multiple config options that limit input sizes. The default limits are:

```cpp
// The max number of headers allowed
uint16_t maxHeaders = 50;
// The max length of a single header. The total header block length is limited to maxHeaders * maxSingleHeaderBytes
uint32_t maxSingleHeaderBytes = 16384;
// The max length of the body
uint64_t maxBodyBytes = 16777216;
```


