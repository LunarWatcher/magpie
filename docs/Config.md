---
{
    "title": "Configuring a server"
}
---
# Configuring a server

All general server configuration is done via `magpie::AppConfig`.

## HTTP/1.1: security limits

`AppConfig` has a `limits` field, which is a struct of multiple config options that limit input sizes. The default limits are:

```cpp
uint16_t maxHeaders = 50;
// The max length of a single header. The total header block is limited to maxHeaders * maxSingleHeaderBytes
uint32_t maxSingleHeaderBytes = 16384;
// The max length of the body
uint64_t maxBodyBytes = 16777216;
```

