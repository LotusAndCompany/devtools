# Quick Start

DevTools runs as a macOS Tauri application. To run it from source, install
Xcode Command Line Tools, Rust 1.99.1, and Bun 1.3.14.

~~~bash
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools/frontend
bun install --frozen-lockfile
bun run tauri:dev
~~~

To build a local DMG, run bun run tauri:build from frontend/. For the public
signed installer and upgrade steps, see the [installation guide](installation.md).
