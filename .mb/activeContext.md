# Active Context

## Current Focus
Parser/protocol hardening, module-level coverage checks, and memory-bank
alignment with committed repository state.

## Repository Snapshot
- Top-level project: `ws-streaming` version `3.0.2`.
- Core modules implemented in `src/` with corresponding headers in `include/ws-streaming/`.
- Optional build targets: examples and tests.

## Recent Changes
- Added top-level CMake option `WS_STREAMING_ENABLE_COVERAGE` and root-level
  include of `cmake/coverage.cmake` when enabled.
- Added `cmake/check_lcov_thresholds.cmake` and
  `cmake/clear_coverage_data.cmake` to support module-specific coverage checks.
- Added custom test coverage targets:
  `coverage-check-websocket_protocol`, `coverage-check-url`,
  `coverage-check-semver`.
- Hardened WebSocket protocol helpers for null-pointer inputs and added
  `tests/test_websocket_protocol.cpp`.
- Hardened semver and URL parsing edge cases and expanded parser/base64 tests.

## Next Steps
- Keep `.mb/*.md` synchronized with future CMake and test evolution.
- Track and resolve remaining uncommitted governance/meta files
  (`AGENTS.md`, `.mb/systemPatterns.md`, `.mb/techContext.md`, `.gitignore`,
  `.clang-format`, `DOCUMENTATION_GUIDE.md`) when ready.
- Continue updating `activeContext.md` and `progress.md` first during each
  implementation session.

## Working Assumptions
- Working tree state (including uncommitted changes) is the active source of
  truth for this session.
- Build flow remains CMake-driven with optional examples/tests and optional
  instrumentation flags.
