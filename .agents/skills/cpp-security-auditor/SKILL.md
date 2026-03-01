---
name: cpp-security-auditor
description: Audit a C++ repository for security vulnerabilities and hardening gaps. Triggers when the user asks for a security review/audit of C++ codebases (memory safety, injection, crypto, authz/authn, unsafe APIs, build flags, deps). Does NOT perform exploit development, weaponization, or instructions for misuse—focuses on defensive findings, fixes, and verification steps.
---

# C++ Project Security Auditor (Codex Skill)

## Objective
Perform a security-focused assessment of a C++ repository and produce:
1) a prioritized findings report (severity + evidence + remediation),
2) a hardening plan (build flags, sanitizers, CI),
3) optional patchset suggestions with minimal risk.

## Operating constraints (must follow)
- Prefer **read-only analysis** first. Only modify code when asked or when the user explicitly requests fixes.
- Before running commands that change the environment or write files, ask for approval (or use a safe sandbox policy).
- Do not provide guidance that enables abuse (exploitation, payloads, bypass instructions). Provide defensive remediations and verification instead.

### Recommended Codex run settings (if you control invocation)
- Use sandboxing and approvals (examples):
  - `--sandbox read-only` for discovery
  - `--sandbox workspace-write` only when generating patches
  - `--ask-for-approval on-request` to gate commands
(These are Codex CLI flags; apply them when launching Codex.)  
Reference: Codex CLI flags for approvals/sandbox. :contentReference[oaicite:2]{index=2}

---

## Phase 0 — Intake & threat model (fast)
Extract from repo/docs:
- Product purpose, deployment model, privilege boundary, and data handled.
- Entry points: network listeners, file parsers, IPC, plugin systems, scripting, deserialization.
- Trust boundaries: untrusted input sources (network, files, env vars, argv, config, sockets).
- Security expectations: supported platforms, toolchain, CI.

Output: a short threat model section with assets, attackers, attack surfaces, and abuse cases (defensive framing).

---

## Phase 1 — Repository mapping
### 1.1 Identify build system & toolchain
Detect:
- CMake / Bazel / Meson / Make / custom scripts
- compiler(s), standard (C++17/20/23), supported OSes
- third-party deps: vendored, submodules, package managers (vcpkg/conan)

### 1.2 Inventory sensitive components
Prioritize review of:
- Parsing: JSON/XML/YAML/protobuf/custom binary formats
- Deserialization, reflection, plugins, dynamic loading (dlopen/LoadLibrary)
- Crypto/TLS/authn/authz/session management
- File I/O, temp files, path handling, archives
- Concurrency primitives (deadlocks -> DoS), atomics, lock-free structures
- Privileged operations (setuid, services/daemons, kernel/driver glue)
- Unsafe boundaries: C FFI, varargs, memcpy/memmove, manual alloc/free, casts

---

## Phase 2 — Automated signal collection (safe defaults)
Run tools only after approval.

### 2.1 Build compilation database (best-effort)
Goal: `compile_commands.json` for accurate static analysis.
- For CMake: configure with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`

If build is expensive or fails, fall back to partial indexing and file-based checks.

### 2.2 Static analysis
Prefer:
- `clang-tidy` (security checks, modernize, bugprone)
- `cppcheck` (security, style, portability)
- Compiler warnings as errors for critical modules (`-Wall -Wextra -Wpedantic` + selected hardening warnings)

Record:
- tool versions, invocation, and top signals
- false positive notes separately

### 2.3 Dependency & supply-chain scan (if applicable)
If dependencies are declared (conan/vcpkg/submodules), identify:
- pinned vs floating versions
- integrity verification (hashes/lockfiles)
- update cadence and known high-risk components (parsers, crypto libs)

(Do not claim CVEs without evidence; cite exact dependency versions and where they’re declared.)

---

## Phase 3 — Manual security review checklist (high-value)
Use targeted code reading around top risks.

### 3.1 Memory safety & lifetime
Look for:
- out-of-bounds (indexing, pointer arithmetic, length mismatches)
- integer overflow/underflow leading to allocation or bounds errors
- use-after-free / double-free / invalid free
- uninitialized reads
- unsafe ownership transfers (raw pointer APIs, custom allocators)
- concurrency lifetime hazards (races, ABA, relaxed atomics misuse)

Red flags:
- unchecked `new[]` sizes derived from untrusted input
- `memcpy/memmove/strcpy/strcat/sprintf` patterns
- manual buffer management without explicit bounds

### 3.2 Input validation & parsing
For each entry point:
- define input grammar/constraints
- ensure length caps, recursion/depth limits, timeouts where needed
- verify error handling: fail closed, consistent states, no partial writes
- avoid ambiguous parsing (e.g., multiple decoders with different rules)

### 3.3 Injection & command execution
Check:
- `system()`, `popen()`, shelling out, `CreateProcess` with concatenated args
- dynamic SQL (if any), template injection, path injection
- environment variable trust and quoting/escaping correctness

### 3.4 Filesystem & path handling
Check:
- path traversal in archive extraction, file serving, plugin loading
- symlink races, TOCTOU issues
- temp file creation (`mkstemp`-like patterns preferred)
- permissions and umask usage

### 3.5 Crypto & secrets
Check:
- custom crypto (flag)
- weak primitives/modes, insecure randomness, nonce reuse
- key/secret storage (logs, core dumps, config files)
- TLS verification, hostname checks, certificate pinning if used (and rotation story)

### 3.6 Authn/authz (if present)
Check:
- correct privilege checks at every boundary
- consistent session/token validation
- replay protections, CSRF if relevant, timing leaks where obvious
- secure defaults (deny by default)

---

## Phase 4 — Build hardening & runtime defenses
Assess and recommend:
- Compiler hardening: stack protector, fortify, PIC/PIE where relevant
- Sanitizers in CI: ASan/UBSan (and optionally TSan for concurrency)
- Fuzzing harness candidates for parsers and protocol handlers
- Logging hygiene: avoid secrets; structured logs; rate limiting on noisy errors
- Crash handling and safe failover

Deliver: a minimal “hardening patch plan” (what to change, where, expected impact).

---

## Findings format (strict)
For each finding:
- **ID:** CPPSEC-###
- **Title**
- **Severity:** Critical / High / Medium / Low (define rationale)
- **Impact**
- **Attack surface / Preconditions**
- **Evidence:** file paths, symbols, code excerpts (small), and reasoning
- **Remediation:** concrete fix guidance + safer API patterns
- **Verification:** how to test (unit test, sanitizer repro, negative test)
- **Notes:** false positive considerations, compatibility risks

Severity guidance:
- **Critical:** RCE, auth bypass, key compromise, memory corruption reachable from untrusted input
- **High:** memory corruption with constraints, privilege escalation, arbitrary file write/read, major injection
- **Medium:** DoS, info leak, unsafe defaults, missing validation with limited impact
- **Low:** defense-in-depth, minor misconfigs, best-practice gaps

---

## Patch guidance (when asked to fix)
When generating code changes:
- keep diffs minimal and reviewable
- prefer standard library safe constructs (`std::span`, `std::string_view`, bounds-checked access patterns)
- add explicit validation helpers
- add tests for exploit class (defensive regression tests)
- avoid behavior changes unless required; call out compatibility impact

---

## Deliverables
Always produce:
1) **Executive summary** (5–10 bullets)
2) **Top risks** (ranked)
3) **Findings list** (detailed, per format)
4) **Hardening recommendations** (build + runtime + CI)
5) **Next actions** (what to do this week vs later)

Optional (if requested):
- PR-ready patches
- CI snippets (sanitizers, clang-tidy, cppcheck)
- Fuzzing plan (targets + seed corpus + sanitizers)

---

## Skill invocation tips (for the user)
- Ask: “Run `cpp-security-auditor` on this repo and produce a report.”
- Or: “Focus only on parser entry points under `src/protocol/` and give me High/Critical findings first.”

