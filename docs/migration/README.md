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
- [performance-baseline.md](performance-baseline.md): measured build/test/
  package facts, missing runtime measurements, repeatable protocol, and
  initial migration gates.
- [pr-intake.md](pr-intake.md): open and merged PRs and the M0 base.
- [execution-log.md](execution-log.md): commands and outcomes for this audit,
  historic estimates clearly separated from measurements, and follow-up work.

## Migration progress

Status meanings: **verified** means the listed M0 audit evidence exists;
**not started** means no implementation or verification has been performed;
**in progress**, **implemented / unverified**, **blocked**, and **out of scope**
must be used with evidence, and out-of-scope entries need a reason.

| Milestone | Status | Evidence / next condition |
|---|---|---|
| M-1: intake and base SHA | Verified for this snapshot | [pr-intake.md](pr-intake.md); original Issue #130 artifact was absent |
| M0: current-state audit | Verified with explicit gaps | This document set; runtime performance and interactive smoke checks remain unmeasured |
| M1: separate C++ UI and processing | Not started | Keep C++ / Qt / CMake as the working application stack until M1 is complete |
| M2: React / Qt WebEngine foundation | Not started | Define and test a real NativeBridge contract and deployable host |
| M3: move all UI to React | Not started | Preserve every registered tool and shared interaction |
| M4: test Tauri portability | Not started | Run the same UI and contract against a Tauri adapter |
| M5: move feature processing to Rust or TypeScript | Not started | Compare behavior, errors, cancellation, and persisted data against this baseline |
| M6: remove Qt and verify distribution | Not started | Validate clean install, migration, update, recovery, and uninstall |

The parent issue requires a working app at each stage. A status must not be
advanced based on a PR description, a mock-only test, or an unrun check.
Unmerged PR features remain outside this baseline until their code is merged
and this inventory is updated.
