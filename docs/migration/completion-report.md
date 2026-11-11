# M6 completion report

**Status:** The Tauri-only implementation and local CI checks pass on
`feature/issue-137-tauri-only-distribution`, based on
`feature/issue-136-tauri-migration` at `ac772b6`. M6 is not ready for public
release: clean-install lifecycle checks, signing credentials, performance
comparisons, and third-party license notices remain outstanding.

## Scope delivered

- Removed the app-owned C++/Qt host and feature layer, QWebChannel bridge,
  CMake/vcpkg build, Qt translation assets, C++ tests, and Qt deployment
  scripts.
- Removed the frontend WKWebView adapter and host selector. The React UI now
  uses the Tauri bridge as its only native transport.
- Kept all 31 Tauri operations, migration fixtures, and the shared
  TypeScript/Rust operation parity check.
- Restored the legacy bundle identifier, set the app version from
  `frontend/package.json`, and configured macOS 15.0 as the minimum system
  version.
- Replaced current development, build, test, contributor, and troubleshooting
  instructions with the Tauri-only path. Removed C++ formatting and CMake
  build hooks.
- Added macOS arm64 CI for Rust, React, contract tests, and DMG packaging; added
  Rust/TypeScript CodeQL and Rustdoc deployment.
- Added a manual release-candidate workflow. It packages an existing draft
  release, verifies the signed arm64 app, verifies and notarizes the DMG, then
  uploads the artifact to the draft. The release manager publishes only after
  the draft and release checks pass.

The supported target is macOS 15 or later on Apple Silicon (arm64). Windows,
Linux, and Intel Macs have not been validated and are outside this release
scope.

## Build, signing, update, recovery, and uninstall

1. For local development, follow [BUILD.md](../../BUILD.md). A local
   `bun run tauri:build` bundle is unsigned when Apple signing credentials are
   absent.
2. Before creating a draft release, complete the dependency-license review and
   prepare the third-party notices described in the License gate below.
3. Store the Apple Developer ID certificate, certificate password, Apple ID,
   app-specific password, and team ID in GitHub Actions secrets. Dispatch the
   macOS Release Candidate workflow with the existing draft release tag.
4. The workflow builds an arm64 app, verifies its signature and architecture,
   creates a DMG, checks the disk image, staples and validates notarization,
   and uploads the DMG to the draft. Do not publish if any step fails.
5. Users update by quitting DevTools, downloading the newer signed DMG, and
   replacing the app in Applications. There is no automatic updater.
6. Before upgrading, copy
   `~/Library/Application Support/com.lotusandcompanyinc.dev-tools` to a safe
   location. Restore it to recover settings and app data.
7. To uninstall, remove DevTools from Applications. Remove that Application
   Support directory separately only if settings and templates should also be
   deleted.

The release workflow is configured but has not been run with organization
secrets. Tauri's [macOS signing guide](https://v2.tauri.app/distribute/sign/macos/)
describes the signing and notarization setup.

## Legacy data migration

On first run the Rust app reads
`~/Library/Preferences/com.trolltech.unknown-organization.DevTools.plist`.
It backs up the source to
`~/Library/Application Support/com.lotusandcompanyinc.dev-tools/migration/qsettings-source.plist`
and imports allowlisted preferences, history, and connection metadata.
Phrase templates are copied only when the new location does not already
contain them. The original plist is retained for recovery.

- Database passwords are not migrated or stored in history. Enter them again
  when reconnecting.
- SQLite security-scoped bookmarks are not migrated. Select each database
  file again.
- If migration fails, preserve both the source plist and the app-data backup.
  Do not attach those files or credential-bearing logs to an issue.

Rust migration tests and legacy-format fixtures pass. A fresh macOS user
profile, interrupted migration, recovery and retry, update, and uninstall have
not been independently exercised.

## Verification evidence

Environment: macOS 26.7.2, arm64; Rust/Cargo 1.99.1; Bun 1.3.14.

| Area | Result |
|---|---|
| Source removal | Passed: no current app-owned C++/Qt/CMake/QWebChannel host or frontend host adapter remains. Historical migration records and legacy import fixtures remain intentionally. |
| Rust formatting | Passed: `cargo fmt --manifest-path src-tauri/Cargo.toml --all -- --check` |
| Rust lint | Passed: `cargo clippy --manifest-path src-tauri/Cargo.toml --locked --all-targets -- -D warnings` |
| Rust tests | Passed: 31 tests, 0 failures. The optional MySQL/PostgreSQL test is environment-gated and was not configured for this run. |
| Rust API docs | Passed: cargo doc --locked --no-deps --manifest-path src-tauri/Cargo.toml. |
| Frontend | Passed: lint, format check, typecheck, all 20 tests, and production build. |
| Bridge contract | Passed as part of the frontend tests: all 31 TypeScript operation names match the Rust registry. |
| Tauri app | Passed: app bundle built for `aarch64-apple-darwin`; executable is arm64; identifier is `com.lotusandcompanyinc.dev-tools`; version is `0.9.0`; minimum OS is `15.0`. |
| Local DMG | Passed: unsigned `DevTools_0.9.0_aarch64.dmg` created; `hdiutil verify` reported a valid checksum. |
| Agent/workflow config | Passed: shared rules synchronized and checked; Ruby YAML parsing passed for the four GitHub workflows and CodeRabbit config. |
| Public signature/notarization | Unverified: organization signing and notarization secrets are unavailable in this environment. |
| Clean install/migration lifecycle | Unverified: no separate clean macOS 15 profile was used. |
| Full feature parity | Unverified: M5 exercised all 11 routes and the 31-operation contract; a complete M6 manual regression pass was not run. |
| Performance | Unverified: no same-condition launch, responsiveness, workload, or app/child memory comparison was run. |

The Vite production build reports a 696.43 kB JavaScript chunk, above its
500 kB advisory threshold. The build succeeds; no code-splitting change was
made in this migration.

## Package size

The local arm64 build reported an uncompressed app bundle of 29.50 MiB and a
compressed DMG of 12.76 MiB (13,064 KiB on disk). The app's directory occupied
30,212 KiB. The generated app's bundle identifier, version, minimum macOS
version, and arm64 executable were inspected.

M5 recorded a Tauri app bundle of 29.52 MiB. That is a comparable Tauri bundle
measurement; the 0.02 MiB difference is within packaging variation.

M0 recorded a Qt app bundle of 18,084 KiB before dependency deployment and
107,516 KiB after deployment. M6's app directory is about 72% smaller on disk
than the deployed M0 Qt bundle, but those checkpoints differ in build system,
dependencies, OS patch version, and packaging method. Treat that comparison
as directional only. M0 produced no compatible compressed DMG measurement.

## License gate

The application license is Apache-2.0 in [LICENSE](../../LICENSE). The former
Qt LGPL, toml11, yaml-cpp, and qrcodegen notices were removed because those
components are no longer app dependencies.

Before public distribution:

1. Review the exact Rust and frontend dependency sets in
   `src-tauri/Cargo.lock` and `frontend/bun.lock`, including transitive
   dependencies.
2. Check package license metadata against upstream license files and NOTICE
   requirements. Resolve missing, conflicting, or incompatible terms.
3. Create `THIRD-PARTY-NOTICES.md` with all required copyright and license
   notices. Include it in the installer and About license view.
4. Record the lockfile revisions, review result, and notice file in the release
   checklist before creating the draft release.

A complete dependency-license inventory and third-party notice file have not
been reviewed or packaged. The public release gate must remain closed until
that review is complete.

## Reproduction

The passing local package checks used:

~~~sh
CI=true bun run tauri:build -- --target aarch64-apple-darwin --bundles app
CI=true frontend/node_modules/.bin/tauri bundle --config src-tauri/tauri.conf.json --target aarch64-apple-darwin --bundles dmg
hdiutil verify src-tauri/target/aarch64-apple-darwin/release/bundle/dmg/DevTools_0.9.0_aarch64.dmg
~~~

The app bundle is removed by Tauri after DMG creation, so the release workflow
verifies the app in the app-only build step before creating the DMG.
