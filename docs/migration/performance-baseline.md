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
| CTest | 26/29 targets passed; elapsed 16.30 s | Three failures described below; CI does not run CTest |
| Format check | Passed | format-check target |
| Pre-deployment .app bundle | 18,084 KiB | Local Xcode output |
| Manually deployed .app bundle | 107,516 KiB (about 105.0 MiB) | Unsigned; macdeployqt with explicit /opt/homebrew/lib; contained 16 framework directories |
| Signed installer / archive | Not available | No signed .dmg, .ipa, or installer artifact was produced |

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

## Runtime baseline still unmeasured

The QSettings profile was safely isolated by setting HOME to a temporary
directory; a Qt probe resolved the settings file under
/private/tmp/devtools-issue131-home/Library/Preferences. Direct foreground
launches of the temporary app bundle returned status 137 both in the default
and escalated execution paths before a window could be observed. The app was
not launched from the user's normal profile, and no interactive smoke check
was performed.
Opening a copied bundle through Launch Services also timed out after 2,173
seconds; a subsequent app inventory showed it was not running. An offscreen
launch attempt was killed by the sandbox with signal 9 after about 0.01
seconds. These attempts produced no usable launch or memory sample.
The following M0 values therefore remain unknown:

- cold and warm launch time;
- idle, peak, and helper-process memory;
- JSON, SQL, Markdown, and UI interaction latency under fixed input;
- application responsiveness and cancellation during large operations;
- developer edit-to-review cycle time.

Do not substitute PR body claims or measurements from another commit for
these values. Collect them on the same supported OS, device, build type, and
fixed fixture before claiming a runtime improvement or regression.

## Repeatable protocol and initial gates

Use this protocol for M0 follow-up and every migration comparison:

1. Record commit, device, macOS version, architecture, compiler, Qt version,
   build type, signing state, and exact command.
2. Use a release build. Run at least 5 cold launches and 10 warm launches;
   report median and p95. Define how the app process is fully quit between
   cold samples.
3. Record the app and helper process memory at idle after 60 seconds and peak
   memory during each fixed workload. State whether values are resident or
   peak resident memory and which processes are included.
4. Use the same checked-in/generated fixtures for each build: JSON at 1 MiB,
   10 MiB, and 50 MiB; a fixed SQL dataset and query; representative Markdown;
   and a large-diff workload only after Text Diff is merged into the base.
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
test_db_connection_window path assertions (/var/folders versus
/private/var/folders). Full details and likely environment boundaries are in
[codebase-audit.md](codebase-audit.md).
