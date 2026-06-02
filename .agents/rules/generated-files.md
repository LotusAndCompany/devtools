---
paths:
  - "build/**"
  - "build-*/**"
  - "CMakeFiles/**"
  - "Testing/**"
  - "vcpkg_installed/**"
  - "res/**/*.qm"
  - "app_info.autogen.cpp"
  - "**/moc_*.cpp"
  - "**/moc_*.h"
  - "**/ui_*.h"
  - "**/qrc_*.cpp"
---

# Generated Files Rules

- Do not edit build outputs, Qt-generated files, or compiled translation files
  directly.
- Do not commit normal build artifacts from `build/`, `build-*`, `CMakeFiles/`,
  `Testing/`, or `vcpkg_installed/`.
- Update `.ts` translation sources only through the explicit translation update
  workflow when translatable strings change.
- Treat bundled third-party code such as `core/qr_tool/qrcodegen.*` as vendor
  code; avoid style-only edits there.
