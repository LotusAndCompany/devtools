---
paths:
  - "**/*.cpp"
  - "**/*.h"
  - "**/*.hpp"
---

# C++ Style Rules

- Run `cmake --build build --target format` after editing to apply formatting
- Keep functions under 100 lines / 50 statements
- Pass QString, QStringList, QVariant, QByteArray, QImage, QPixmap by const reference
