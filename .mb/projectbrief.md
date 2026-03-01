# Project Brief: ws-streaming

## Purpose
`ws-streaming` is a modern C++ library that implements the WebSocket Streaming Protocol for high-performance, reliable, platform-independent data streaming.

## Scope
- Provide a reusable library for both client and server roles.
- Support symmetric publishing/receiving APIs for bidirectional data flow.
- Integrate with Boost.Asio executors for async I/O.
- Expose metadata-driven signal streaming (scalars, structured types, async signals).

## Deliverables
- Core C++ library (`ws-streaming`) with public headers in `include/ws-streaming`.
- Build system and package configuration via CMake.
- Example applications for common source/sink patterns.
- Unit tests for utility and protocol-related components.

## Constraints
- C++17 minimum.
- CMake >= 3.24.
- Runtime dependency on Boost compiled libs (`system`, `url`) and header-only components.

## Success Criteria
- Clean configure/build via CMake.
- Library usable as installed package (`find_package`) or in-project dependency.
- Example apps and tests build with configurable options.
