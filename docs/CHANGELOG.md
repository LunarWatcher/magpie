# Changelog

## [unreleased]

## [v0.3.0] (2026-11-03)

### Added

* Added `{path}` parameters

### Changed

* Added support for `/path` to match handlers for `/path/` (which, in new behaviour, allows `/path` to match `/path/{path}`)

## [v0.2.0]

### Added

* Added `-DMAGPIE_DISABLE_HTTP2` (default: on) to disable nghttp2. Nghttp2 is, unfortunately, an AI slop dependency, though support is currently kept because HTTP/1.1 may or may not be dying. To my knowledge, there are no slop-free alternatives at this time.

## [v0.1.0]

Initial versioned release

[unreleased]: https://codeberg.org/LunarWatcher/magpie/compare/v0.4.0...master
[v0.3.0]: https://codeberg.org/LunarWatcher/magpie/releases/tag/v0.3.0
[v0.2.0]: https://codeberg.org/LunarWatcher/magpie/releases/tag/v0.2.0
[v0.1.0]: https://codeberg.org/LunarWatcher/magpie/releases/tag/v0.1.0
