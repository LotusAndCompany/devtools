# Testing Guide

## Rust

Rust unit and integration tests live in `src-tauri/src/`. From the repository
root:

~~~sh
cargo fmt --manifest-path src-tauri/Cargo.toml --all -- --check
cargo clippy --manifest-path src-tauri/Cargo.toml --locked --all-targets -- -D warnings
cargo test --manifest-path src-tauri/Cargo.toml --locked
~~~

## Frontend

Vitest tests live beside the React feature or bridge code. From `frontend/`:

~~~sh
bun install --frozen-lockfile
bun run lint
bun run format:check
bun run --bun typecheck
bun run --bun test
bun run --bun build
~~~

The bridge contract parity test checks the shared TypeScript operations against
the Rust command registry. Shared request, database, image, and migration cases
belong in `tests/fixtures/`; keep credentials and real user data out of them.

## Data migration and release checks

Migration changes need evidence for a fresh profile, legacy input, malformed or
partial input, retry, backup restoration, and preservation of the source.
Before an App Store release, follow the clean-install, update, recovery,
uninstallation, App Store Connect archive, and dependency-license checks in
the [completion report](../migration/completion-report.md).
