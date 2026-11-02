# Migration audit and baseline

This folder records the M0 audit for the staged Qt Widgets to Tauri migration
tracked by GitHub issues #129–#131. The audit is reproducible from the M0 base:
**main at 5638bb64c67a4a2329d1e57e36f255eef92fce9e** (2026-10-26).

Issue #130 is closed, but its expected PR intake artifact was not present in
the repository at this snapshot. The PR state and selected base SHA were
reconstructed from GitHub and the local checkout; see
[pr-intake.md](pr-intake.md). The audit excludes the pre-existing untracked
directory docs/ja/articles/ and does not modify it.

## Audit documents

- [codebase-audit.md](codebase-audit.md): code structure, dependencies, OS,
  CI, distribution, test inventory, C++ measurement, and known failures.
- [feature-inventory.md](feature-inventory.md): registered tools, shared UI,
  current tests, edge cases to preserve, and milestone progress.
- [data-migration.md](data-migration.md): persisted settings, connection
  history, phrase templates, user files, and migration constraints.
- [performance-baseline.md](performance-baseline.md): measured build, test,
  package, launch, memory, and small-input facts; remaining runtime gaps,
  repeatable protocol, and initial migration gates.
- [pr-intake.md](pr-intake.md): open and merged PRs and the M0 base.
- [execution-log.md](execution-log.md): commands and outcomes for this audit,
  historic estimates clearly separated from measurements, and follow-up work.
- [qt-webview-feasibility.md](qt-webview-feasibility.md): PR #142 baseline,
  operation mapping, and the Qt WebView G0 result recorded on 2026-11-02.

## Migration progress

Status meanings: **verified** means the listed M0 audit evidence exists;
**not started** means no implementation or verification has been performed;
**in progress**, **implemented / unverified**, **blocked**, and **out of scope**
must be used with evidence, and out-of-scope entries need a reason.

| Milestone | Status | Evidence / next condition |
|---|---|---|
| M-1: intake and base SHA | Verified for this snapshot | [pr-intake.md](pr-intake.md); original Issue #130 artifact was absent |
| M0: current-state audit | Verified with explicit gaps | This document set; limited runtime and UI smoke evidence is recorded, with large-workload, OS-cold/warm launch, operation, and distribution gaps remaining |
| M1: separate C++ UI and processing | Verified | [m1-final-verification.md](m1-final-verification.md); the complete 56-test suite and application build passed |
| M2: React / native bridge foundation | Implemented on current checkout; not merged | [m2-react-webengine.md](m2-react-webengine.md) preserves the historical WebEngine measurements; [qt-webview-feasibility.md](qt-webview-feasibility.md) records the current platform split and Qt 6.12.0 evidence |
| M3: move all UI to React | Implemented on current checkout; acceptance gaps remain | [m3-react-ui.md](m3-react-ui.md); all 11 registered routes use React, with host-specific security/integration checks and broader feature-equivalence/distribution work still open |
| M4: test Tauri portability | Not started | Run the same UI and contract against a Tauri adapter |
| M5: move feature processing to Rust or TypeScript | Not started | Compare behavior, errors, cancellation, and persisted data against this baseline |
| M6: remove Qt and verify distribution | Not started | Validate clean install, migration, update, recovery, and uninstall |

The parent issue requires a working app at each stage. A status must not be
advanced based on a PR description, a mock-only test, or an unrun check.
The migration audit above remains anchored to its M0 snapshot. The following
host update records later implementation evidence without rewriting that
baseline.

## Current host update (2026-11-02)

- macOS uses a direct public-API WKWebView adapter inside the Qt Widgets host;
  non-macOS builds retain the Qt WebEngine adapter.
- The shared frontend uses NativeBridge contract v2. CMake selects the matching
  native source, framework dependencies, frontend bundle mode, and integration
  test for each host.
- The Qt 6.12.0 build, 38-test CTest run, WKWebView integration test, and
  interactive macOS launch passed. The user manually confirmed Finder drops
  for an image and a SQLite database.
- The published PR #142 head is older than the current local checkout. These
  later changes are not merged; keep the GitHub milestone open until its PR
  contains the current host split and is merged.
- Mac App Store signing, Archive validation, and upload are on hold at the
  user's request. This change produced no release Archive or `.pkg`; package
  signing is separate from the app-bundle build.
