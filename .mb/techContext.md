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
  - Library target in `src/CMakeLists.txt`
  - Test executables in `tests/CMakeLists.txt`

### Coverage
- Module: `cmake/coverage.cmake`
- Current wiring: included from `tests/CMakeLists.txt` when
  `WS_STREAMING_ENABLE_COVERAGE` is enabled.
- Test executables conditionally call `enable_coverage_for_target(...)` when
  coverage tools are detected.
- Note: this is present in working-tree state and should be treated as
  in-progress configuration until fully normalized in top-level option docs.

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
