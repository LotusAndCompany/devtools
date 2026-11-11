# Common Issues

## Bun or Rust version mismatch

Use Bun 1.3.14 and Rust 1.99.1 with rustfmt and clippy. Run
`bun install --frozen-lockfile` from `frontend/`; run Cargo commands from the
repository root with `--manifest-path src-tauri/Cargo.toml`.

## The app does not start in development

Check that Xcode Command Line Tools are installed, dependencies finished
installing, and port 5173 is available. Start the Tauri application with
`bun run tauri:dev` from `frontend/`. The frontend's `bun run dev` command
alone starts only the Vite server.

## macOS blocks a locally built app

A local DMG built without Apple credentials is unsigned and is intended for
development only. Public users should install the signed and notarized DMG from
a published GitHub release. Do not bypass macOS security prompts for an
unverified installer.

## Legacy settings migration reports an error

Keep the source plist and the app data backup. Quit DevTools, restore the
backup if needed, then retry after resolving the reported issue. Do not include
the plist, database passwords, or personal connection details in a bug report.

## Database connection fails

Check the server address, port, database name, and account. Credentials are
entered again after migration; the app does not import saved passwords.
SQLite files with old security-scoped bookmarks must be selected again.

For more support questions, see the [FAQ](faq.md) and
[installation and recovery guide](../getting-started/installation.md).
