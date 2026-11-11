# Migration audit and baseline

This folder preserves the M0 audit and historical checkpoints for the
migration from the Qt Widgets host to Tauri 2. The original baseline is
anchored to commit `5638bb64c67a4a2329d1e57e36f255eef92fce9e` (2026-10-26).
M0–M5 documents describe the code and evidence at their recorded snapshots;
they are not instructions for the current build.

## Audit documents

- [M0 codebase audit](codebase-audit.md)
- [Feature inventory](feature-inventory.md)
- [Data migration analysis](data-migration.md)
- [Performance baseline](performance-baseline.md)
- [PR intake](pr-intake.md)
- [Execution log](execution-log.md)
- [M4 Tauri proof](m4-tauri.md)
- [M5 Tauri implementation](m5-tauri.md)
- [M6 completion report](completion-report.md)

## Migration progress

| Milestone | Status | Evidence |
|---|---|---|
| M-1: intake and base SHA | Verified for the historical snapshot | [PR intake](pr-intake.md) |
| M0: current-state audit | Historical audit verified with explicit gaps | [Codebase audit](codebase-audit.md) |
| M1: separate C++ UI and processing | Historical checkpoint verified | [M1 verification](m1-final-verification.md) |
| M2: React / native bridge foundation | Historical implementation | [M2 report](m2-react-webengine.md) |
| M3: move UI to React | Historical implementation | [M3 report](m3-react-ui.md) |
| M4: test Tauri portability | Historical Tauri proof | [M4 report](m4-tauri.md) |
| M5: move feature processing to Rust or TypeScript | Implemented with recorded limits | [M5 report](m5-tauri.md) |
| M6: remove Qt and establish Tauri-only distribution | Implemented on this branch; acceptance evidence incomplete | [M6 completion report](completion-report.md) |

M6 is not a release sign-off until the unverified clean-machine, migration,
distribution, performance, and dependency-license checks in its report pass.
