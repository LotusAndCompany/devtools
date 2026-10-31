---
paths:
  - "**/*.md"
---

# Documentation Rules

- Keep documentation accurate for a macOS 15.0+ Apple Silicon desktop app with
  a C++17/Qt 6 native host and React/TypeScript UI rendered by Qt WebEngine.
- Bilingual support exists for English and Japanese; Japanese files live under
  `docs/ja/`.
- When updating an English document, update the corresponding Japanese document
  only if it already exists.
- Do not create new Japanese mirror files unless explicitly requested.
- PR titles for docs-only changes should use `docs: <description>`.
