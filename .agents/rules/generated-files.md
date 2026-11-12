---
paths:
  - "src-tauri/target/**"
  - "frontend/node_modules/**"
  - "frontend/dist/**"
  - "build/**"
  - "build-*/**"
  - "CMakeFiles/**"
  - "Testing/**"
  - "vcpkg_installed/**"
  - "res/**/*.qm"
---

# Generated Files Rules

- Do not edit Cargo, Vite, Tauri, or generated app bundle outputs directly.
- Do not commit normal outputs from src-tauri/target/, frontend/node_modules/,
  frontend/dist/, or ignored legacy build*/ and vcpkg_installed/ directories.
  Old ignored Qt build artifacts may exist locally; they are not app inputs.
- Cargo and Bun lockfiles are source-controlled inputs; update them through
  their package managers when dependencies change.
- Treat src-tauri/Cargo.lock and frontend/bun.lock as dependency evidence
  during license review.
