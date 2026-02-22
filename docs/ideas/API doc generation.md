# API doc generation

Probably because each endpoint can only have one HTTP method[^1], it's fairly trivial to extend the `BaseRoute` with some metadata object. That could be used to generate API doc specs, at least based on some filter. 

The metadata will likely have to be provided centrally, while the actual frontend or whatever for the API doc should be separated from the core entirely. I don't like swagger UI, but it should at least be possible for some implementation to grab the data and forward it. 

Not sure if the frontend implementation is something I want to implement, especially because the web component for that would require making decisions on templating that the core really shouldn't take. Exposing it as JSON might be enough? Not sure what standards there are available though.

## API

Same concept as streamed input (idea) and middlewares (implemented):
```cpp
app->route<...>(...)
    ->documentRoute({
        .name = "Name",
        .description = "Description",
        .body = "no clue what format this would be"
    })
```

It's possible it's worth waiting for C++26 (reflection) before doing this, but I'm not sure if that actually solves anything. Arbitrary objects would still need to be represented somehow in code.

... Is compile-time JSON a thing? It probably could be with reflection, but holy shit would that be cursed. `std::string_view body = CompileTimeJson<SomeDTO>()` would be some of the most disgusting code I'd ever have written, but to the point where it kinda wraps around to being a really fucking cool idea. At that point, all of `documentRoute` could probably be compile-time generated to a JSON string

[^1]: One per call to route, not that each `/endpoint` only can have one HTTP method
