# Performance and distribution baseline

## Reference environment

The source baseline is main at
5638bb64c67a4a2329d1e57e36f255eef92fce9e, audited 2026-10-26.

| Component | Local measurement environment | CI reference |
|---|---|---|
| OS / architecture | macOS 26.6.2, arm64 | macos-14 formatting; ubuntu-latest builds and analysis |
| Compiler | AppleClang 21.0.0.21000334 | CI compiler/toolchain varies by workflow |
| CMake | 4.0.3 | Workflow-managed |
| Qt | 6.9.2 from Homebrew | 6.9.3 in the build workflow |
| toml11 / yaml-cpp | Homebrew 4.4.0 / 0.8.0 | vcpkg pinned at 10b7a178346f3f0abef60cecd5130e295afd8da4 |
| Build generator | Ninja and Xcode | macOS / Linux workflows |

The local dependency resolution differs from CI: the pinned vcpkg toolchain
root was unavailable, so the clean Ninja build resolved Homebrew packages.
Do not use the local YAML test failures as a CI failure without reproduction
under the pinned toolchain.

## Measurements available at M0

| Metric | M0 observation | Conditions / limitation |
|---|---|---|
| Clean Ninja configure | Configure 7.2 s; generate 0.9 s | Release, ENABLE_UNIT_TEST=ON, existing qlementine FetchContent source; local host above |
| Full Ninja build | Succeeded | Wall time was not captured; one dependency warning context is documented in the audit |
| Xcode configure | Configure 28.2 s; generate 2.3 s | Isolated out-of-tree build |
| Xcode application build | Succeeded | Used isolated DerivedData and disabled code signing |
| CTest | 26/29 tests passed; elapsed 16.30 s | Three failures described below; CI does not run CTest |
| Format check | Passed | format-check target |
| Pre-deployment .app bundle | 18,084 KiB | Local Xcode output |
| Manually deployed .app bundle | 107,516 KiB (about 105.0 MiB) | Unsigned; macdeployqt with an explicit Homebrew library path; contained 16 framework directories |
| Signed installer / archive | Not available | No signed macOS .dmg, release archive, or installer was produced |

The project's automatic macdeployqt -appstore-compliant post-build step
reported unresolved-rpath diagnostics in the local Xcode build. A manual
macdeployqt deployment with an explicit Homebrew library path resolved the Qt
dependencies. It is only an unsigned local .app bundle; it does not validate
signing, notarization, App Store compliance, or a release archive. macdeployqt
also warned that it would not overwrite the existing qt.conf. The resulting
qt.conf contains Plugins = PlugIns.

The 107,516 KiB value is a local .app bundle measurement, not a compressed
download or installed footprint. Compare like artifact with like artifact;
keep .app, signed archive, installer, and post-install footprint separate.

## Runtime measurements

The audit copy used a temporary Xcode app bundle with an audit-specific identifier and an ad-hoc signature. These are process-cold / OS-cache-warm measurements, not clean-machine or post-reboot cold-start results.

| Metric | M0 observation | Conditions / limitation |
|---|---|---|
| Launch to accessibility tree | 1.044, 1.014, 1.010, 1.039, 1.031 s; median 1.031 s, p95 1.044 s | Five process starts through CUA getApp; includes Launch Services and accessibility response; app copy with modified identifier |
| Ordinary-profile launch to accessibility tree | 2.870 s | One launch through CUA getApp; includes Launch Services and accessibility response; not a repeatable startup distribution |
| Idle app and helper memory | DevTools 89.2 MB; ThemeWidgetControlViewService helper 9.4 MB | One Activity Monitor sample while the small JSON remained in the Data Conversion screen; memory column, not RSS or peak |
| Small JSON conversion | A small JSON value produced formatted output | The synchronous setValue call took 25.2 ms; accessibility observation confirmed the result afterward. This is not UI paint latency or a statistical sample |
| 1 MiB JSON conversion | No accessible result; the UI became unresponsive before the app was stopped | The UI path did not establish whether conversion or editor/accessibility rendering caused the stall; no output parity result |

A 1 MiB JSON input did not produce accessible output. Do not claim large JSON performance parity from this run. Larger JSON sizes were not tested.

Still unknown: a true OS-cold launch, repeatable ordinary-profile cold and warm launch distributions, peak memory, repeatable SQL and Markdown performance distributions or larger workloads, cancellation behavior, Text Diff (not in base), developer edit-to-review time, and signed release or installed distribution. Collect remaining metrics with fixed workloads before claiming a general runtime comparison.

## Repeatable protocol and initial gates

Use this protocol for M0 follow-up and every migration comparison:

1. Record commit, device, macOS version, architecture, compiler, Qt version,
   build type, signing state, and exact command.
2. Use a release build. Run at least 5 process-cold launches and 10 warm
   launches; report median and p95. Define how the app process is fully quit
   between samples. Report OS-cold results separately; do not describe
   process restarts with cached files as OS-cold.
3. Record the app and helper process memory at idle after 60 seconds and peak
   memory during each fixed workload. State whether values are resident or
   peak resident memory and which processes are included.
4. Use the same checked-in/generated fixtures for each build: JSON at 1 MiB,
   10 MiB, and 50 MiB; stop and record if 1 MiB makes the UI unresponsive
   before proceeding to larger inputs. Use a fixed SQL dataset and query;
   representative Markdown; and a large-diff workload only after Text Diff
   is merged into the base.
   Record operation median/p95, UI responsiveness, cancellation, and output
   parity.
5. Record the same artifact classes separately: .app bundle, signed release
   archive/installer, and installed footprint. Do not compare unsigned local
   bundles with signed production installers.
6. For developer iteration, time at least 5 equivalent UI changes from edit
   to visible verification, including build and reload steps.

Initial comparison gates are:

- Output and error behavior must match the captured M0 cases; no known
  operation or data type may be dropped.
- Once runtime measurements are collected, startup, idle/peak memory, and
  feature-operation median and p95 may not regress by more than 10% versus
  the same-build-type M0 values without a written reason and an explicit
  acceptance decision.
- The same artifact class may not grow by more than 10% without documenting
  the added runtime or capability and recording the reviewer decision.
- Large operations must preserve responsive feedback and supported
  cancellation; timing alone is not a pass if the UI becomes unusable.
- Developer iteration time is reported as an observed metric. M0 has no
  target until equivalent tasks and measurement steps are agreed.

The relative gates are provisional until missing M0 runtime values are
collected and M2 confirms the comparison workload. If a gate changes, record
the old value, new value, date, and reason. An unknown M0 value blocks a
performance-parity claim; it does not block documenting the current code
structure.

## CTest failures to track

The local run failed in test_yaml_parser (YAML::InvalidNode / parser result),
test_data_conversion (YAML format enum mismatch), and two
test_db_connection_window path assertions caused by temporary-directory
canonicalization differences. Full details and likely environment boundaries are in
[codebase-audit.md](codebase-audit.md).
