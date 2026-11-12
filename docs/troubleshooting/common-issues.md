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

A local app bundle is for development and may not have the distribution
signature used for App Store releases. Install the supported public version
from the [Mac App Store](https://apps.apple.com/jp/app/devtools/id6736692957?mt=12).
Do not bypass macOS security prompts for an unverified build.

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
