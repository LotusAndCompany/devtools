---
paths:
  - "**/*.md"
---

# Documentation Rules

- Keep documentation accurate for a macOS 15.0+ Apple Silicon desktop app with
  a C++17/Qt 6.12.0+ native host. macOS uses WKWebView; other platforms use
  Qt WebEngine for the React/TypeScript UI.
- Bilingual support exists for English and Japanese; Japanese files live under
  `docs/ja/`.
- When updating an English document, update the corresponding Japanese document
  only if it already exists.
- Do not create new Japanese mirror files unless explicitly requested.
- PR titles for docs-only changes should use `docs: <description>`.
