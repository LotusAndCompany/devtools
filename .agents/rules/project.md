---
paths:
  - "**/*"
---

# Project Rules

DevTools is a macOS desktop application with a C++17/Qt 6 native host and a
React/TypeScript frontend rendered by Qt WebEngine. Qt Widgets provides the
native window, menus, OS dialogs, and platform integration. Feature screens are
implemented in React; C++ feature modules provide processing services only.

## Build

```bash
# Configure with CMake (set VCPKG_ROOT to your vcpkg installation)
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx

# Build
cmake --build build --parallel 8

# Build and run with console output
cmake --build build --target run

# Enable and run unit tests
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx \
  -DENABLE_UNIT_TEST=ON
cmake --build build --parallel 8
ctest --test-dir build --output-on-failure

# Format check
cmake --build build --target format-check

# Static analysis
cmake --build build --target lint

# Frontend checks (run from frontend/)
cd frontend
bun install --frozen-lockfile
bun run lint
bun run format:check
bun run --bun typecheck
bun run --bun build
bun run --bun test
```

## Architecture

- `features/{feature}/core/`: feature processing services used through the native
  bridge. Do not introduce Qt Widgets dependencies here. QtCore, QtGui, QtSql,
  and QtNetwork are allowed.
- `features/framework/gui/`: Qt application and window host.
- `main/`: application entry point.
- `frontend/src/app/`, `features/`, `shared/`, and `platform/qt/`: React UI, tools, shared code, and Qt WebChannel adapter.
- `features/web_frontend/`: native bridge, operation handlers, and C++ service calls for the React UI.
- `tests/`: shared test helpers (mock_helper, random_data, test_util).
- `features/{feature}/tests/`: per-feature unit tests.
- Each feature is a static library module registered in `MODULE_LIST`.

## Dependencies

- vcpkg: toml11, yaml-cpp. See `vcpkg.json`.
- Qt6: Core, Widgets, LinguistTools, Sql, Network, Concurrent, WebChannel, and WebEngineWidgets.
- Bun 1.3.14: frontend dependency installation and production asset build.
- Bundled vendor code: qrcodegen under `features/qr_code/core/qrcodegen.*`.

## Pre-commit Hooks

Setup:

```bash
pre-commit install --install-hooks -t pre-commit -t pre-push
```

Hooks run clang-format, trailing-whitespace, EOF fixer, large file check, and
pre-push build verification.

## Agent Harness

- `AGENTS.md` is the bootstrap instruction file for all AI coding tools.
- Path-scoped, Claude Code-like rules live in `.agents/rules/`.
- `opencode.json` loads all `.agents/rules/*.md` via the `instructions` field.
  It is self-maintaining — no sync step required for OpenCode.
- Tool-specific configs should import or mirror `.agents/rules/` instead of
  duplicating guidance.
- After editing `AGENTS.md` or `.agents/rules/*.md`, run
  `scripts/sync-agent-rules.py` to regenerate tool-specific adapters.
