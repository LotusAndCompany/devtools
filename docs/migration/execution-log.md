# Migration audit execution log

## M0 / audit snapshot

- Date: 2026-10-26.
- Base: main / origin/main at 5638bb64c67a4a2329d1e57e36f255eef92fce9e.
- Scope: source, build, tests, registered tools, settings/data, CI,
  distribution, and current PR intake.
- Result: M0 inventory and baseline records added under docs/migration/;
  draft PR #138 created from the M0 base.
- Not performed: no C++ changes, merge, release, signed package, or issue
  close.

### Commands and outcomes

| Command / action | Outcome |
|---|---|
| Clean Ninja configure with Release and ENABLE_UNIT_TEST=ON, using the existing qlementine FetchContent source | Passed; configure 7.2 s, generation 0.9 s |
| Full Ninja build, parallelism 8 | Passed; elapsed build time was not captured |
| CTest with output on failure | 26/29 passed in 16.30 s; YAML parser, data conversion YAML detection, and DB connection path assertions failed |
| format-check target | Passed |
| Runtime and UI observations | See [performance-baseline.md](performance-baseline.md) for approved measurements, scope, and limitations; these are limited smoke observations, not a complete comparison |
| Xcode generator configure and build | Passed after using isolated DerivedData and disabling signing; automatic post-build macdeployqt emitted unresolved-rpath diagnostics |
| Manual macdeployqt deployment | Completed; Qt dependencies resolved; unsigned .app measured 107,516 KiB; not a signed release artifact |
| GitHub issue/PR inspection | Read Issues #129–#131 and the current PR snapshot; no issue comments on #131 |
| Review PR | Created draft PR #138 from the M0 base |



The Ninja configure used Release, ENABLE_UNIT_TEST=ON, and the existing
qlementine FetchContent source. From an out-of-tree build directory, configure
with:

    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_UNIT_TEST=ON ..

Baseline build warnings: two nodiscard warnings in
features/data_conversion/tests/emitter/test_basic_emitter.cpp at lines 33
and 35. The local CTest failures and dependency-path limitations are
described in [codebase-audit.md](codebase-audit.md).

### Time and intervention record

At 2026-10-26 10:53:15 UTC, the Codex goal elapsed-time counter was 10,835
seconds (180 minutes 35 seconds). This is wall-clock elapsed time and includes
pauses, waiting, and approval; it is not model inference or per-command
execution time. Human review/decision time and per-command model time were not
independently instrumented. No API usage cost or quota estimate is claimed.

### Historic estimates (not measurements)

These numbers were copied from Issues #130 and #131 as historical hypotheses.
They are not current estimates, actuals, schedule commitments, or Codex-time
conversions. Do not add overlapping estimates unless the scope is disjoint.

| Historic scope | Old estimate |
|---|---:|
| M-1: six PR investigations | 30–90 minutes |
| M-1: fixes/tests for three new-feature PRs | 1–4 hours |
| M-1: conflict resolution/integration verification | 30–120 minutes |
| M-1 total | 2–7.5 hours |
| M0 | 3–5 person-days |
| M1 | 5–12 person-days |
| M2 | 3–5 person-days |
| M3 | 10–20 person-days |
| M4 | 3–6 person-days |
| M5 | 8–20 person-days |
| M6 | 4–8 person-days |
| M0–M6 total | 36–76 person-days |
| Direct full migration | 25–50 person-days, formerly described as 200–400 hours / 5–10 weeks |
| Tauri UI proof of concept | 3–5 person-days |
| Tauri minimum feature set | 8–15 person-days |
| C++ sidecar migration | 15–30 person-days |
| Qt WebEngine + JSON Formatter proof of concept | 4–8 person-days |
| Full UI move while keeping C++ | 12–25 person-days |
| Tauri after Qt WebEngine | 13–32 person-days |

Historic WebEngine proof-of-concept breakdown: WebEngine 0.5–1, Vite/React/
shadcn 0.5–1, QWebChannel 1–2, JSON Formatter UI 1–2, and HMR/distribution/
memory 1–2 person-days. Historic full-UI breakdown: React/WebEngine 2–3,
Bridge 2–4, all feature UIs 5–10, C++ separation 2–5, and integration/
distribution 1–3 person-days. Historic Tauri breakdown: foundation 1–2,
Bridge replacement 2–4, C++ to Rust 5–15, OS/files 2–5, and tests/
distribution/CI 3–6 person-days.

Historic Codex-time hypotheses by milestone, with human-intervention
hypotheses after the slash: M0 1–3 hours / 15–30 minutes; M1 4–12 hours /
1–2 hours; M2 2–5 hours / 30–60 minutes; M3 8–20 hours / 2–4 hours; M4
2–5 hours / 30–60 minutes; M5 6–16 hours / 1–3 hours; M6 3–8 hours /
1–2 hours. The summed 26–69 Codex hours and 6 hours 15 minutes–13 hours
30 minutes of human time are historical estimates, not actuals. The earlier
28–76.5-hour M-1-plus-M0–M6 total is also only a sum of old hypotheses.

Former C++ residual-ratio estimates were UI-only React 40–70%, TypeScript
processing 15–35%, thin Qt host 5–20%, and Qt WebEngine completion 10–20%.
These had no denominator or counting method; the Qt WebEngine 10–20% idea was
later withdrawn. The M0 counting definition and baseline are in
[codebase-audit.md](codebase-audit.md).

## Next measurements

1. Reproduce the two YAML failures using CI's pinned vcpkg commit and record
   whether they persist.
2. Repeat ordinary-profile process-cold and warm launch measurements, and
   record peak memory with app and helper processes.
3. Repeat SQL and Markdown measurements with larger fixed workloads, and
   investigate the 1 MiB JSON UI stall and cancellation behavior before
   testing larger JSON sizes.
4. Measure at least five equivalent edit-to-visible-review iterations.
5. Validate signed distribution on a clean supported macOS installation.
6. Update this log with actual Codex elapsed time, human intervention,
   repetitions, commits, and PR when available; keep those separate from
   historic estimates.
