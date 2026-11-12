# Quick Start

DevTools runs as a macOS Tauri application. To run it from source, install
Xcode Command Line Tools, Rust 1.99.1, and Bun 1.3.14.

~~~bash
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools/frontend
bun install --frozen-lockfile
bun run tauri:dev
~~~

To build a local macOS app bundle, run `bun run tauri:build` from `frontend/`.
Install and update the public release through the
[Mac App Store](https://apps.apple.com/jp/app/devtools/id6736692957?mt=12).
For App Store Connect archives, see [macos/README.md](../../macos/README.md).
