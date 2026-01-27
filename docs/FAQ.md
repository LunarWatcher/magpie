# FAQ

## Does magpie include an HTTP client?

No, HTTP clients are out of scope. If you need one on the server or for testing, you'll need to bring your own. I can strongly recommend [libcpr](https://github.com/libcpr/cpr).

Note that if you install libcpr via conan, you need to explicitly opt in to HTTP/2, as it's disabled by default:
```python
def configure(self):
    self.options["libcurl"].with_nghttp2 = True
```

This primarily applies if you plan to use libcpr to test your server implementation, or otherwise interact with HTTP/2 servers that need HTTP/2.
