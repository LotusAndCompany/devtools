# M0 codebase audit

## Scope and reference

- Audit date: 2026-10-26.
- Reference branch and commit: main, 5638bb64c67a4a2329d1e57e36f255eef92fce9e.
- Local main and origin/main pointed to that commit during the audit.
- The only pre-existing working-tree item was the untracked docs/ja/articles/
  directory. It was preserved and excluded from the audit.
- The repository did not contain docs/migration/pr-intake.md, despite Issue
  #130 being closed. The current PR snapshot is reconstructed in
  [pr-intake.md](pr-intake.md).

## Application and architecture

DevTools is a C++17 macOS desktop application using Qt 6 Widgets and CMake.
The root build declares 11 static-library modules; features/CMakeLists.txt
lists the framework and nine application feature modules. The main window's
ContentsArea constructs nine user-facing tools plus a Welcome page.

Feature code commonly uses core/, gui/, and tests/ folders. This is not yet a
clean three-layer architecture: there is no separate Bridge, Application, or
Infrastructure layer, and some stateful database and network behavior is
implemented in GUI code. Qt Widgets is linked broadly across feature targets.
The inventory identifies existing extraction and behavior-preservation work;
these paths are not evidence that such a separation already exists.

The base commit contains no React/Vite application, package.json, frontend
source tree, Tauri configuration, Cargo manifest, or WebEngine host. A
tsconfig.json exists but does not establish that a frontend is present or
compatible.

The app shell includes a searchable side menu, Welcome page, About and
Settings dialogs, light/dark system theme handling, English/Japanese language
selection, window controls, and Ctrl+W. The DB screen is cached when switching
tools; other screens are recreated. SQLite file drag-and-drop and image
viewing are also present.

## Build targets and dependencies

- CMake minimum version: 3.21.1.
- Qt requirement: Qt 6.9 or newer; components include Core, Widgets,
  LinguistTools, Sql, Network, and Svg.
- qlementine is fetched through FetchContent at v1.4.2.
- vcpkg manifest dependencies are toml11 and yaml-cpp. vcpkg.json has no
  builtin baseline. CI pins vcpkg commit
  10b7a178346f3f0abef60cecd5130e295afd8da4.
- QR generation includes qrcodegen source files under
  features/qr_code/core; these are third-party vendor files and excluded from
  the project's own C++ count.
- The application bundle identifier is com.lotusandcompanyinc.dev-tools.

There are no tracked Node, Rust, React, or Tauri frontend dependencies at M0.
Do not infer support from tsconfig.json.

## Operating systems, CI, and distribution

- The project rules and CMake target macOS 15.0 or later on Apple Silicon
  (arm64). Windows is not configured as a supported target.
- CI runs formatting on macos-14 and Linux builds, static analysis, CodeQL,
  documentation, and GitHub Actions analysis on ubuntu-latest.
- CI does not run CTest, and there is no macOS application test job.
- No workflow was found to publish a signed .dmg, .ipa, or installer archive.
  Release Please opens version PRs; documentation is deployed to GitHub Pages.
- The source includes App Store entitlements and uses macdeployqt with
  App-Store-compliant packaging options, but this audit did not verify a
  signed or submitted App Store artifact.

## Code and test measurements

Counts were computed from tracked files at the base commit. The C++ set is
files with .cpp, .h, .hpp, .mm, .cc, .cxx, or .m extensions. Counts are
physical lines, including blank and comment lines. Generated filenames and
third-party vendor files are excluded. Tests are measured separately.

| Group | Files | Physical lines | Notes |
|---|---:|---:|---|
| Own production C++ | 103 | 15,717 | Baseline denominator for migration residual fraction |
| GUI and application entry point | 62 | 12,092 | Existing UI and main code |
| Core including vendor | 43 | 5,061 | Includes qrcodegen |
| Own core excluding vendor | 41 | 3,625 | No standalone Infrastructure/Bridge allocation exists |
| qrcodegen vendor | 2 | 1,436 | Excluded from own-code denominator |
| C++ test sources | 37 | 4,614 | Excluded from production denominator |

If a residual fraction is reported, define it as current own production C++
physical lines divided by the M0 value of 15,717. It may exceed 100% while
processing is extracted or tests are added; it is a tracking ratio, not a
target.

There are 29 registered CMake/CTest test targets. The tested areas include
framework utilities, image I/O and controls, JSON/YAML/TOML parser/emitter
and conversion, regex processing, command generation, Markdown processing,
and the DB connection window. Tests do not currently cover React, Bridge
contracts, Tauri, end-to-end workflows, API request UI, phrase-generation UI,
QR UI, or distribution and data migration.

The test harness is disabled by default; configure with
ENABLE_UNIT_TEST=ON to register it. The baseline CTest run is documented in
[performance-baseline.md](performance-baseline.md).

## Confirmed baseline failures and validation limits

On the local macOS 26.6.2 / arm64 environment, with Homebrew Qt 6.9.2,
toml11 4.4.0, and yaml-cpp 0.8.0, a clean Ninja build succeeded and format
check passed. CTest passed 26 of 29 targets; three targets failed:

1. test_yaml_parser: tryParse returned false for a valid fixture and an
   unhandled YAML::InvalidNode was reported.
2. test_data_conversion: YAML format detection expected enum value 3 and
   returned value 6.
3. test_db_connection_window: two path assertions expected /var/folders/...,
   while canonicalization returned /private/var/folders/....

The local build resolved Homebrew packages directly because the pinned
vcpkg toolchain root was not available. The YAML failures therefore need
reproduction with the CI-pinned vcpkg environment before attributing them to
application code. The SQLite path failures are observed on this macOS host;
they do not establish behavior on supported macOS 15 installations.

The test-emitter build also emitted two existing nodiscard warnings in
features/data_conversion/tests/emitter/test_basic_emitter.cpp, lines 33 and
35. CI does not run CTest, so these failures are not currently protected by
the baseline workflow.

The local Xcode build succeeded after using an isolated DerivedData directory
and disabling code signing. Its automatic macdeployqt post-build step emitted
unresolved-rpath diagnostics; a manual deployment with an explicit Homebrew
library path resolved Qt dependencies but is not equivalent to validating
the signed release pipeline.

## Audit limits

This is a source and local-build snapshot, not an exhaustive behavioral
specification. Startup, memory, large-input timings, interactive UI smoke
checks, other macOS releases, clean-machine install/update/uninstall,
signature, notarization, and App Store submission remain unverified. The
reproducible protocol and initial gates are in
[performance-baseline.md](performance-baseline.md).
