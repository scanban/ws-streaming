# System Patterns

## Architecture Overview
- Library-centric architecture with public API headers and internal implementation split.
- Async networking and protocol operations are executor-driven via Boost.Asio.
- Symmetric peer model: once connected, either side can publish and receive signals.

## Key Components
- `client` / `server`: connection establishment and endpoint role behaviors.
- `connection`: peer-level orchestration, signal registration, and remote signal lifecycle.
- `local_signal` / `remote_signal`: publish/subscribe and data delivery primitives.
- Metadata model: `metadata`, `metadata_builder`, dimensions, units, structure fields.
- Internal protocol/transport layer under `include/ws-streaming/detail` and `src/detail`.

## Design Patterns
- Builder pattern for metadata and structural descriptors.
- Event/callback signaling (Boost.Signals2) from executor context.
- Separation of public API and internal protocol implementation.

## Build Composition
- Root CMake delegates to `src`, optional `examples`, optional `tests`.
- Dependencies resolved through dedicated CMake modules in `dependencies/`.
- Build instrumentation is implemented with reusable CMake helper functions and
  attached per target, not globally.

## Build Instrumentation Pattern
- Sanitizer helper:
  - `cmake/sanitizers.cmake` defines
    `ws_streaming_enable_sanitizers_for_target(<target>)`.
  - Root CMake includes the module once; individual target files opt in.
  - Current tree applies this helper to each unit-test executable target.
- Coverage helper:
  - `cmake/coverage.cmake` defines `enable_coverage_for_target(<target>)`.
  - Root `CMakeLists.txt` includes the module conditionally when
    `WS_STREAMING_ENABLE_COVERAGE` is enabled.
  - Coverage flags are enabled for library and test targets only when coverage
    tools are detected.
  - `tests/CMakeLists.txt` defines module-level coverage check targets:
    `coverage-check-websocket_protocol`, `coverage-check-url`, and
    `coverage-check-semver`.
- Compiler constraint:
  - Sanitizer mode is currently guarded for GNU compilers in this repository
    state.

## Code Quality Constraints

### Priorities
1. Correctness & robustness (no UB, consistent error handling)
2. Maintainability (clarity > cleverness)
3. Resource awareness (CPU/memory/IO)
4. maximum portability between different compilers
5. strict C++ standard conforming

### Forbidden
- Undefined behavior
- Global mutable state
- Hidden static state in tests
- Flaky timing-based assertions

### Preferred
- Value semantics
- Dependency injection
- Clear Arrange/Act/Assert sections
- Explicit ownership semantics

## Security Considerations
Agents must:
- Test input validation paths
- Cover malformed input
- Validate boundary overflow scenarios
- Avoid introducing unsafe mocks

## Source considerations

###  Chunking Contract

#### Module Definition
A module consists of:
- Header (.h/.hpp)
- Implementation (.cpp)
- Direct private headers

Do not split:
- Template definitions
- Class declarations from inline methods
- Specializations from primary template

#### Context Depth
Include:
- Direct project includes (1 level deep)
Exclude:
- Transitive system/STL includes
- Entire repo scans during module work

## Test Generation Rules
- **Never** change tests if code to be tested contain bugs
- When creating tests **always** check code to be tested for bugs and if found - suggest patch(es)
- Emphasize malformed-input, null-input, and truncation-path coverage for
  parser/protocol utilities.

### Framework
All tests must use Google Test.

### Structure
- Prefer TEST_F for stateful classes
- Use fixtures for shared setup
- One behavior per test
- Use ASSERT_* for preconditions
- Use EXPECT_* for verifications

### Required Coverage Areas
- Constructor invariants
- Copy/move semantics
- Boundary values
- Exception guarantees
- nullptr handling
- Container edge cases
- Thread-safety (if applicable)

### Mocking
- Use Google Mock for interfaces
- Prefer StrictMock unless behavior is intentionally flexible
- Avoid over-specified expectations

### Test Coverage Policy
- check coverage only for tests
- do not add checking coverage to CMakeLists.txt

**Minimum thresholds**
- Line coverage ≥ 85%
- Branch coverage ≥ 80%
- Core domain logic ≥ 100%

**Must**
- Identify untested branches
- Prioritize critical path coverage
- Avoid redundant trivial tests
