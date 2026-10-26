# Migration audit execution log

## M0 / audit snapshot

- Date: 2026-10-26 (UTC execution window began at 07:49 UTC).
- Base: main / origin/main at 5638bb64c67a4a2329d1e57e36f255eef92fce9e.
- Scope: source, build, tests, registered tools, settings/data, CI,
  distribution, and current PR intake.
- Working tree: pre-existing untracked docs/ja/articles/ was preserved.
- Result: M0 inventory and baseline records added under docs/migration/;
  draft PR #138 created from feature/issue-131-m0-audit.
- Not performed: no C++ changes, merge, release, interactive app launch,
  signed package, or issue close.

### Commands and outcomes

| Command / action | Outcome |
|---|---|
| Clean Ninja configure with Release and ENABLE_UNIT_TEST=ON, using the existing qlementine FetchContent source | Passed; configure 7.2 s, generation 0.9 s |
| Full Ninja build, parallelism 8 | Passed; elapsed build time was not captured |
| CTest with output on failure | 26/29 passed in 16.30 s; YAML parser, data conversion YAML detection, and DB connection path assertions failed |
| format-check target | Passed |
| Xcode generator configure and build | Passed after using isolated DerivedData and disabling signing; automatic post-build macdeployqt emitted unresolved-rpath diagnostics |
| Manual macdeployqt deployment with explicit /opt/homebrew/lib | Completed; Qt dependencies resolved; unsigned .app measured 107,516 KiB; not a signed release artifact |
| QSettings location probe and isolated app launch | HOME redirected the settings location into /private/tmp; default and escalated direct foreground launches exited 137, copied-bundle Launch Services open timed out, and offscreen launch was killed by the sandbox before UI observation |
| GitHub issue/PR inspection | Read Issues #129–#131 and the current PR snapshot; no issue comments on #131 |
| Review PR | Created draft PR #138 from the M0 base; initial title, C++, and GitHub Actions checks were in progress, with CodeRabbit skipped for draft |

Exact reproducible CMake configuration:

    cmake -S . -B /private/tmp/devtools-issue131-ninja -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_UNIT_TEST=ON -DFETCHCONTENT_SOURCE_DIR_QLEMENTINE=/Users/mitani/work/devtools/build/_deps/qlementine-src

Baseline build warnings: two nodiscard warnings in
features/data_conversion/tests/emitter/test_basic_emitter.cpp at lines 33
and 35. The local CTest failures and dependency-path limitations are
described in [codebase-audit.md](codebase-audit.md).

### Time and intervention record

At 2026-10-26 08:59:28 UTC, the Codex goal runtime was 4,206 seconds
(70 minutes 6 seconds) of elapsed wall time, including a long Launch Services
timeout. This is not model execution time per command. Individual command
timings are recorded only where the shell/build output captured them. Human
review/decision time and per-command Codex time were not independently
instrumented. No API usage cost or quota estimate is claimed.

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
2. Run the app in an execution environment that supports GUI launch with the
   verified temporary HOME; collect launch, memory, and interactive smoke
   measurements without changing the user's profile.
3. Create deterministic performance fixtures and record JSON, SQL, and
   developer-iteration measurements before M2 parity claims.
4. Validate signed distribution on a clean supported macOS installation.
5. Update this log with actual Codex elapsed time, human intervention,
   repetitions, commits, and PR when available; keep those separate from
   historic estimates.
