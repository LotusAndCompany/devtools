---
paths:
  - "**/*.cpp"
  - "**/*.h"
  - "**/*.hpp"
---

# C++ Style Rules

- C++17 compliant. Follow `.clang-format` and `.clang-tidy` settings
- Run `cmake --build build --target format` after editing to apply formatting
- Use Qt macros (Q_OBJECT, Q_PROPERTY, Q_SIGNALS, Q_SLOTS, emit) as-is
- Include order: local → Qt → third-party → standard library → system
- Keep functions under 100 lines / 50 statements
- Pass QString, QStringList, QVariant, QByteArray, QImage, QPixmap by const reference
- Wrap UI text with `tr()` for i18n
