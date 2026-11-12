# M6 completion report

**Status:** The Tauri-only implementation and local CI checks pass on
`feature/issue-137-integration` (PR #147), based on
`feature/issue-136-tauri-migration`. An Xcode Archive path builds a signed
arm64 archive with a matching dSYM, and third-party notices are bundled and
shown in About. DevTools' public distribution channel is the Mac App Store;
no DMG has been publicly distributed. The Tauri version is an update to that
existing channel and is not yet publicly released. Dependency-license review,
clean-install lifecycle checks, and performance comparisons remain outstanding.

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
- Added macOS arm64 CI for Rust, React, contract tests, and Tauri app bundle
  packaging; added Rust/TypeScript CodeQL and Rustdoc deployment.
- Added a shared Xcode Archive scheme and build script for App Store Connect
  uploads, including a UUID-checked dSYM for the Apple Distribution-signed app.

The supported target is macOS 15 or later on Apple Silicon (arm64). Windows,
Linux, and Intel Macs have not been validated and are outside this release
scope.

## Build, App Store submission, update, recovery, and uninstall

1. For local development, follow [BUILD.md](../../BUILD.md); its
   `bun run tauri:build` command creates a local macOS app bundle.
2. Before publicly releasing the Tauri update, complete the
   dependency-license review described in the License gate below. The current
   notice inventory is prepared and bundled, but its `objc2` redistribution
   question remains open.
3. Use the shared Xcode Archive scheme described in
   [macos/README.md](../../macos/README.md), then upload the archive through
   Xcode Organizer to App Store Connect. Use TestFlight for beta distribution;
   submit the public update for App Review.
4. Users install and update DevTools through the Mac App Store. The app has no
   in-app updater.
5. Before upgrading, copy
   `~/Library/Application Support/com.lotusandcompanyinc.dev-tools` to a safe
   location. Restore it to recover settings and app data.
6. To uninstall, remove DevTools from Applications. Remove that Application
   Support directory separately only if settings and templates should also be
   deleted.

No public DMG workflow is part of the release path. The App Store archive is
signed with the Apple Distribution certificate; Developer ID signing and
notarization apply only to a separate direct-distribution channel, which
DevTools does not use. The current tree produced a local arm64 `0.9.0 (20)`
archive with a valid Apple Distribution signature and a matching app/dSYM UUID
(`BC30C782-78E2-392D-B35D-E7DB717ADFB6`). The bundled notice file's SHA-256
also matches the repository copy. This validation archive has not been
uploaded to App Store Connect.

## App Store Connect / TestFlight

On 2026-11-11, Xcode Organizer reported that the Tauri macOS app upload to App
Store Connect completed for version `0.9.0` (build `18`). The only warning was
a missing app dSYM for UUID `3FB0952A-6A07-3969-AFC8-C22BB7A39BE0`; the app
binary upload completed. This warning affects crash-report symbolication, not
the app binary upload.

Build `18` was a one-off, manually signed Xcode archive built from the Tauri
app using the existing App Store provisioning profile and temporary App Store
configuration. Its Rust executable had no DWARF information, so that archive
could not produce a matching dSYM. The repository now provides a repeatable
Xcode scheme and build script that generate and UUID-check a dSYM before Xcode
signs the app. The local build `20` archive verifies that path; it has not been
uploaded, so only build `18` has a confirmed App Store Connect upload.

The public App Store listing still shows version `0.8.0`; uploading build `18`
to App Store Connect did not publish the Tauri update. The migrated `0.9.0`
version remains pending App Review and public release.

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

Rust migration tests and legacy-format fixtures pass. A temporary app-data
profile retained a changed setting when the app bundle was replaced with a
newer validation build. This did not exercise an uninstall. A fresh macOS user
profile importing a real legacy plist, interrupted migration, recovery and
retry, and uninstall have not been independently exercised.

## Verification evidence

The evidence below records the M6 validation before this App Store-only
follow-up. The updated bundle target and CI workflow have not been rerun.

Environment: macOS 26.7.2, arm64; Rust/Cargo 1.99.1; Bun 1.3.14.

| Area | Result |
|---|---|
| Source removal | Passed: no current app-owned C++/Qt/CMake/QWebChannel host or frontend host adapter remains. Historical migration records and legacy import fixtures remain intentionally. |
| Rust formatting | Passed: `cargo fmt --manifest-path src-tauri/Cargo.toml --all -- --check` |
| Rust lint | Passed: `cargo clippy --manifest-path src-tauri/Cargo.toml --locked --all-targets -- -D warnings` |
| Rust tests | Passed: 32 tests, 0 failures. The optional MySQL/PostgreSQL test is environment-gated and was not configured for this run. |
| Rust API docs | Passed: cargo doc --locked --no-deps --manifest-path src-tauri/Cargo.toml. |
| Frontend | Passed: lint, format check, typecheck, all 20 tests, and production build. |
| Bridge contract | Passed as part of the frontend tests: all 31 TypeScript operation names match the Rust registry. |
| Tauri app | Passed: app bundle built for `aarch64-apple-darwin`; executable is arm64; identifier is `com.lotusandcompanyinc.dev-tools`; version is `0.9.0`; minimum OS is `15.0`. |
| Local DMG experiment | Passed as an unpublished packaging experiment: unsigned `DevTools_0.9.0_aarch64.dmg` created and verified with `hdiutil`. No DMG was publicly released; it is not part of the distribution path. |
| App Store Connect/TestFlight upload | Passed with warning: Xcode Organizer completed upload of arm64 app version `0.9.0` (build `18`); the app dSYM was missing for UUID `3FB0952A-6A07-3969-AFC8-C22BB7A39BE0`. |
| Xcode Archive and dSYM | Passed locally for arm64 `0.9.0 (20)`: Apple Distribution signature verified and the app/dSYM UUIDs matched (`BC30C782-78E2-392D-B35D-E7DB717ADFB6`). This archive was not uploaded. |
| Tauri App Store release | Pending: the public listing still shows version `0.8.0`; build `18` was uploaded to App Store Connect but the Tauri update has not been publicly released. |
| Third-party notices | Passed: 385 locked Rust crates and 21 production frontend packages are inventoried in `THIRD-PARTY-NOTICES.md`; the file is in the app bundle and About license view. The `objc2` redistribution question is unresolved. |
| Manual app smoke checks | Partial: all 11 feature routes opened. In the temporary app, image split/save produced four 64×64 PNGs; phrase-template save worked; SQLite table query succeeded, invalid SQL showed an error, and a later query recovered; a 1,048,560-byte JSON file converted to a valid 1,048,566-byte file; canceled saves created no output; local HTTP 404 was reported as Not Found; QR save produced a verified 168×168 PNG; and Markdown preview/export stripped script and `javascript:` content while preserving visible text. Earlier checks covered image resize, regex, timestamp, command generation, HTTP 200, and About license display. Settings persisted after replacing the app bundle in temporary app data. Finder drop and full feature parity remain unverified. |
| Agent/workflow config | Shared rules were synchronized after this follow-up; workflow parsing and CI have not been rerun for these edits. |
| Clean install/migration lifecycle | Unverified: no separate clean macOS 15 user profile was available on this host. |
| Full feature parity | Unverified: all routes were opened and the listed operations were smoke-tested, but this was not a complete M6 manual regression pass. |
| Performance | Unverified: no same-condition launch, responsiveness, workload, or app/child memory comparison was run. |

The Vite production build reports a 696.43 kB JavaScript chunk, above its
500 kB advisory threshold. The build succeeds; no code-splitting change was
made in this migration.

## Package size

The local arm64 build reported an uncompressed app bundle of 29.50 MiB. The
app's directory occupied 30,212 KiB. The generated app's bundle identifier,
version, minimum macOS version, and arm64 executable were inspected.

M5 recorded a Tauri app bundle of 29.52 MiB. That is a comparable Tauri bundle
measurement; the 0.02 MiB difference is within packaging variation.

M0 recorded a Qt app bundle of 18,084 KiB before dependency deployment and
107,516 KiB after deployment. M6's app directory is about 72% smaller on disk
than the deployed M0 Qt bundle, but those checkpoints differ in build system,
dependencies, OS patch version, and packaging method. Treat that comparison
as directional only.

## License gate

The application license is Apache-2.0 in [LICENSE](../../LICENSE). The former
Qt LGPL, toml11, yaml-cpp, and qrcodegen notices were removed because those
components are no longer app dependencies. The current inventory is in
[`THIRD-PARTY-NOTICES.md`](../../THIRD-PARTY-NOTICES.md); the bundled app and
About license view both include it.

Before submitting a public Mac App Store release:

1. Complete legal review of the exact Rust and frontend dependency sets in
   `src-tauri/Cargo.lock` and `frontend/bun.lock`, including transitive
   dependencies, package metadata, upstream terms, and NOTICE requirements.
2. Resolve the upstream `objc2` note that its bindings derive from Apple SDKs
   and redistribution implications are unclear. This notice inventory records
   the issue but does not resolve it.
3. Recheck the lockfile hashes and notice contents whenever either lockfile
   changes, then record the review result before uploading the release build
   to App Store Connect.

The inventory includes 385 Rust crates and 21 frontend production packages,
with lockfile hashes recorded in the notice file. The notices are packaged
and visible in About, but legal approval is outstanding. Keep the public
release gate closed until that review and the other release checks are complete.

## Reproduction

The passing local package checks used:

~~~sh
CI=true bun run tauri:build -- --target aarch64-apple-darwin --bundles app
~~~

The App Store submission artifact is the Xcode Archive produced by the shared
scheme; GitHub Releases do not carry an app installer.
