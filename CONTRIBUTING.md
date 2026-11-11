# Contributing to DevTools

[日本語版](docs/ja/CONTRIBUTING.md)

## Before you start

Read the [Code of Conduct](CODE_OF_CONDUCT.md), [build guide](BUILD.md), and
[current architecture](docs/development/architecture.md). DevTools supports
macOS 15 or later on Apple Silicon. It uses Tauri 2, Rust, React, TypeScript,
and Bun.

## Development

From the repository root:

~~~sh
cd frontend
bun install --frozen-lockfile
bun run tauri:dev
~~~

Before opening a pull request, run the checks listed in [BUILD.md](BUILD.md).
For changes to saved settings or migration, cover clean install, legacy import,
failure, retry, and source preservation. Do not add real user data or secrets
to tests and fixtures.

## Code and documentation

- Keep native application behavior in `src-tauri/` and UI behavior in
  `frontend/src/`.
- Keep bridge operations typed in both the Rust registry and shared
  TypeScript contract, and update the parity check.
- Keep visible UI copy in English and Japanese using the shared localization
  helper.
- Update existing Japanese mirrors when the matching English document changes.
- Do not add Qt, CMake, vcpkg, WKWebView adapters, or sidecars to the app.
- Do not edit generated build outputs or bundled vendor code.
- Use the Pencil editor for `.pen` files; do not edit them as text.

## Pull requests

Create a focused branch from the intended base, describe user-visible changes,
and include verification results and known limits. Use the repository's
Conventional Commit and release-please conventions. Do not add AI tool
attribution to commits, pull requests, or issues.

## Releases

A public macOS installer must be built from the release workflow, signed with
the organization's Developer ID, notarized, and verified before publishing.
See [installation and release](docs/getting-started/installation.md) and the
[release gate](docs/migration/completion-report.md#release-gate).
