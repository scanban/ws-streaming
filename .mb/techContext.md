# Tech Context

## Languages and Standards
- C++17
- CMake build system (minimum 3.24)

## Dependencies
- Boost >= 1.84
  - Header-only: asio, beast, serialization, signals2
  - Runtime/linked: system, url
- nlohmann-json >= 3.12.0
- GoogleTest >= 1.17.0 (for tests)
- Threads (via CMake)

## Git specific
- Never try or suggest to push to upstream repository

## Build and Run

### testing and devlopment
- Configure: `cmake -B {build directory} -DWS_STREAMING_BUILD_TESTS=ON -DWS_STREAMING_INSTALL=OFF -DWS_STREAMING_BUILD_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Debug`
- Build: `cmake --build {build directory} --parallel`

### Production
- Configure: `cmake -B {build directory} -DWS_STREAMING_BUILD_TESTS=OFF -DWS_STREAMING_INSTALL=OFF -DWS_STREAMING_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release`
- Install: `cmake --install {build directory}` (when install constraints are met)

## Key CMake Options
- `WS_STREAMING_BUILD_EXAMPLES`
- `WS_STREAMING_BUILD_TESTS`
- `WS_STREAMING_ENABLE_SANITIZERS`
- `WS_STREAMING_ENABLE_COVERAGE`
- `WS_STREAMING_IGNORE_INSTALLED_BOOST`
- `WS_STREAMING_INSTALL`

## Build Instrumentation

### Sanitizers
- Module: `cmake/sanitizers.cmake`
- Option: `WS_STREAMING_ENABLE_SANITIZERS`
- Behavior:
  - Applies `-fsanitize=address,undefined` and frame-pointer flags at target
    level through `ws_streaming_enable_sanitizers_for_target(...)`.
  - Guarded to GNU compiler in current working tree; non-GNU with the option
    enabled triggers a CMake fatal error.
- Current target coverage:
  - Test executables in `tests/CMakeLists.txt`

### Coverage
- Module: `cmake/coverage.cmake`
- Current wiring: included from root `CMakeLists.txt` when
  `WS_STREAMING_ENABLE_COVERAGE` is enabled.
- Library and test targets conditionally call
  `enable_coverage_for_target(...)` when coverage tools are detected.
- Module-specific threshold checks are provided by
  `coverage-check-websocket_protocol`, `coverage-check-url`, and
  `coverage-check-semver`.
- Threshold policy in checks:
  - line coverage >= 85%
  - branch coverage >= 80%

### Sanitized Test Build
- Configure:
  `cmake -B build -DWS_STREAMING_BUILD_TESTS=ON -DWS_STREAMING_ENABLE_SANITIZERS=ON -DWS_STREAMING_BUILD_EXAMPLES=OFF -DWS_STREAMING_INSTALL=OFF`
- Build:
  `cmake --build build`
- Run tests:
  `ctest --test-dir build --output-on-failure`

## Repository Layout
- Public headers: `include/ws-streaming/`
- Source: `src/` and `src/detail/`
- Tests: `tests/`
- Examples: `examples/`
- Documentation: `docs/`
