# Performance testing server

This folder contains a small benchmark server largely meant for performance testing the framework. It's not intended for comparisons against other frameworks, because benchmarks hard. 

The actual performance testing is done with [locust](https://github.com/locustio/locust). As a best practice for performance testing if attempting to replicate results, note that two different computers should be used; one for the server, and one for locust.

The testing I do happens on a 1Gbps link between my main desktop (Ryzen 7 5800X, 16 threads@5.36GHz) and main server (i7-4790, 8 threads@4GHz).

The testing is HTTPS-only, but not behind a reverse proxy to avoid it taking off parts of the load.
