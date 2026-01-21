# Router design

The router is based on a [radix tree](https://en.wikipedia.org/wiki/Radix_tree) with some simplifications that probably make it slower, but much easier to implement. Particularly, the tree is segmented by whole path segment rather than by the shortest common prefix, which sacrifices some storage space for not having to split strings as aggressively. 

However, there are two main changes from the base data structure

## Matching

The matching order is:

* exact match
* {int}
* {string}
* {path} - not implemented yet, but planned, maybe?

At this time, this comes with one important caveat: since there's no multi-path matching, a specific subroute disregards all subroutes under a template that otherwise would've matched. 

For example, given the routes:

```
/test/whatever
/test/{string}/69
```

`/test/whatever`, as expected, hits `/test/whatever`. Potentially unintuitively, `/test/whatever/69` results in a 404. 

Once a parameter like `{string}` is introduced, it acts as a catchall for everything _except_ what has been previously specified. Essentially, templates that also match a specific route are considered overridden, but in a non-inheriting way. When you declare a competing route like this, you're declaring that that subtree is entirely distinct from its similar-looking path. If it can match _into_ a path, that tree is distinct from the other trees.

This may or may not change in the future depending on interest, but the commonly cited alternatives unfortunately involve backtracking, which can and will tank performance. Many major routing libraries have this limitation, and it [is a limitation of the underlying data structure](https://github.com/julienschmidt/httprouter/issues/175#issuecomment-270075906). 

The way this particular router is implemented also means you need to be more careful with how you use `{path}`. A `{string}` at the same level as a `{path}` functionally renders the `{path}` useless, as the string always matches, and therefore always takes priority over `{path}`.

However, this design is predictable, and does not rely on route  declaration order, which helps keep routes more consistent and easier to reason about. Also makes tests less flaky if you have a setup that only sources specific routes for the test setup, and get the declaration order of two equivalent routes wrong in a test.
