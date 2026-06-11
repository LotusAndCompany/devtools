---
paths:
  - "**/*"
---

# Project Rules

DevTools is a macOS desktop application built with C++17 and Qt6 Widgets.

## Build

```bash
# Configure with CMake (vcpkg toolchain)
cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=/opt/homebrew/share/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
make -j8

# Build and run with console output
cmake --build . --target run

# Enable and run unit tests
cmake .. -DENABLE_UNIT_TEST=ON
ctest

# Format check
cmake --build . --target format-check

# Static analysis
cmake --build . --target lint
```

## Architecture

- `core/`: business logic. Do not introduce Qt Widgets dependencies here.
  QtCore, QtGui, QtSql, QtNetwork are allowed.
- `gui/`: Qt6 Widgets UI layer.
- `tests/`: Google Test based unit tests.
- Each feature is a static library module.

## Dependencies

- vcpkg: toml11, yaml-cpp. See `vcpkg.json`.
- Qt6: Core, Widgets, LinguistTools, Sql, Network.
- Bundled vendor code: qrcodegen under `core/qr_tool/qrcodegen/`.

## Pre-commit Hooks

Setup:

```bash
pre-commit install --install-hooks -t pre-commit -t commit-msg -t pre-push
```

Hooks run clang-format, trailing-whitespace, EOF fixer, large file check,
commit message validation, and pre-push build verification.

## Agent Harness

- `AGENTS.md` is the bootstrap instruction file for all AI coding tools.
- Path-scoped, Claude Code-like rules live in `.agents/rules/`.
- `opencode.json` loads all `.agents/rules/*.md` via the `instructions` field.
  It is self-maintaining — no sync step required for OpenCode.
- Tool-specific configs should import or mirror `.agents/rules/` instead of
  duplicating guidance.
- After editing `AGENTS.md` or `.agents/rules/*.md`, run
  `scripts/sync-agent-rules.py` to regenerate tool-specific adapters.
