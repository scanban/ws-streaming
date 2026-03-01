# Active Context

## Current Focus
Build and test instrumentation updates (sanitizers + coverage wiring) and
memory-bank alignment with current repo governance rules.

## Repository Snapshot
- Top-level project: `ws-streaming` version `3.0.2`.
- Core modules implemented in `src/` with corresponding headers in `include/ws-streaming/`.
- Optional build targets: examples and tests.

## Recent Changes
- Added CMake option `WS_STREAMING_ENABLE_SANITIZERS` in root `CMakeLists.txt`.
- Added `cmake/sanitizers.cmake` with GNU-only guard and target helper
  `ws_streaming_enable_sanitizers_for_target(...)`.
- Applied sanitizer helper to library target in `src/CMakeLists.txt`.
- Applied sanitizer helper to test executables in `tests/CMakeLists.txt`.
- Wired coverage module include in `tests/CMakeLists.txt` behind
  `WS_STREAMING_ENABLE_COVERAGE`.
- Added README section for sanitized test builds.
- Added repo governance instructions via `AGENTS.md` and commit policy in
  `COMMIT_STYLE.md`.

## Next Steps
- Keep `.mb/*.md` synchronized with in-flight CMake and README changes.
- Clarify and document top-level governance for coverage option usage so test
  coverage wiring is consistently configured.
- Continue updating `activeContext.md` and `progress.md` first during each
  implementation session.

## Working Assumptions
- Working tree state (including uncommitted changes) is the active source of
  truth for this session.
- Build flow remains CMake-driven with optional examples/tests and optional
  instrumentation flags.
