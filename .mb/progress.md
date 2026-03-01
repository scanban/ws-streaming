# Progress

## Current Status
Memory bank is being refreshed to match the latest committed parser/protocol
hardening and coverage-check wiring.

## Completed
- Added optional module-level coverage checks and helper scripts:
  `cmake/check_lcov_thresholds.cmake` and
  `cmake/clear_coverage_data.cmake`.
- Added dedicated WebSocket protocol unit tests and null-pointer hardening for
  `websocket_protocol` header decoding/generation helpers.
- Hardened semver and URL parsing edge cases and expanded parser/base64 test
  coverage for malformed and boundary paths.
- Landed three focused commits:
  - `767e803` build(cmake) coverage-check wiring
  - `2ad95aa` websocket null-safety + tests
  - `e790e55` parser hardening + edge-case tests

## Known Gaps
- Some governance/meta updates remain intentionally uncommitted in the working
  tree (`AGENTS.md`, `.gitignore`, `.clang-format`,
  `DOCUMENTATION_GUIDE.md`).
- Memory-bank files should continue to be synchronized when instrumentation
  behavior changes (especially sanitizer target coverage details).

## Update Triggers
Update memory bank after:
- Significant architecture/API changes.
- Dependency/build workflow changes.
- Completion of meaningful feature or bugfix work.
