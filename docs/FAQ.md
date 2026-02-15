# FAQ

## Does magpie include an HTTP client?

No, HTTP clients are out of scope. If you need one on the server or for testing, you'll need to bring your own. I can strongly recommend [libcpr](https://github.com/libcpr/cpr).

Note that if you install libcpr via conan, you need to explicitly opt in to HTTP/2, as it's disabled by default:
```python
def configure(self):
    self.options["libcurl"].with_nghttp2 = True
```

This primarily applies if you plan to use libcpr to test your server implementation, or otherwise interact with HTTP/2 servers that need HTTP/2.

## Does magpie include websocket support?

No, but websocket support is considered for the far future. 

Until then, you can mix in another library. The webserver and websocket server need to listen on different ports anyway, so they'll have separate resource pools regardless of whether you use one or two libraries. I have previously used [IXWebSocket](https://github.com/machinezone/IXWebSocket), as it's one of far too few libraries that don't use boost. However, seeing as magpie currently uses (non-boost) asio, any websocket library that uses non-boost asio is up there. 

There is a plan to eventually replace asio with something that isn't as chonky, but that too is a far-future plan.
