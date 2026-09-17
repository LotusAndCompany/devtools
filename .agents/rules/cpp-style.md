---
paths:
  - "**/*.cpp"
  - "**/*.h"
  - "**/*.hpp"
---

# C++ Style Rules

- All C++ code must conform to `.clang-format` and `.clang-tidy`.
- All files must conform to `.editorconfig`: LF line endings, final newline,
  trimmed trailing whitespace except Markdown, and spaces for indentation.
- Run `cmake --build build --target format` after editing C++ files.
- Column limit is 100 characters.
- Braces use Allman style for functions/classes/structs and K&R for control
  statements.
- Pointer/reference alignment is right-aligned: `int *ptr`, `int &ref`.
- Include order is local, Qt, third-party, standard library, then system.
- Classes, structs, and enums use `PascalCase`.
- Functions use `camelCase`.
- Variables and members use `snake_case`.
- Constants and macros use `SCREAMING_SNAKE_CASE`.
- Files use `snake_case`.
- Keep functions under 100 lines and 50 statements where practical; split
  complex logic before it becomes hard to review.
- Keep business logic in `core/` free of Qt Widgets dependencies.
- Use new-style Qt signal/slot syntax: `&Class::method`.
- Set a Qt parent for `new`-allocated widgets, or use smart pointers for
  non-QObject ownership.
- Use Qt macros as-is: `Q_OBJECT`, `Q_PROPERTY`, `Q_SIGNALS`, `Q_SLOTS`, and
  `emit`.
- Pass `QString`, `QStringList`, `QVariant`, `QByteArray`, `QImage`, and
  `QPixmap` by const reference unless moving or storing is intentional.
- Wrap user-visible UI text with `tr()`.
- Reuse `DevTools::Ui` helpers and `DevTools::Ui::Metrics` for shared layout,
  widget configuration, spacing, and sizing.
- Keep common widget styling centralized in `features/framework/gui/design_system.*`
  and use qlementine theme resources for colors; do not duplicate them in
  feature screens. Extend the design system only for a pattern reused by
  multiple screens.
