# DevTools - AI Agent Guidelines

macOS C++17/Qt6 desktop application providing integrated developer utilities.

## Code Formatting

- All C++ code MUST conform to `.clang-format` (Google base + Qt adaptations)
- All files MUST conform to `.editorconfig` rules:
  - Files MUST end with a newline (`insert_final_newline = true`)
  - Use LF line endings
  - Trim trailing whitespace
  - Indent with spaces (4 for C++, 2 for XML/Qt UI files)
- Column limit: 100 characters
- When generating or modifying code, ensure `.clang-format` rules are followed:
  - Braces: Allman style for functions/classes/structs, K&R for control statements
  - Pointer/reference alignment: right-aligned (`int *ptr`, `int &ref`)
  - Include order: local → Qt → third-party → standard library → system

## Naming Conventions

- Classes/Structs/Enums: `PascalCase`
- Functions: `camelCase`
- Variables/Members: `snake_case`
- Constants/Macros: `SCREAMING_SNAKE_CASE`
- Files: `snake_case`

## Architecture

- `core/`: business logic (no Qt Widgets dependency; QtCore, QtSql, QtNetwork allowed)
- `gui/`: Qt6 Widgets UI layer
- `tests/`: Google Test based unit tests
- Each feature is a static library module

## Qt Conventions

- Use new signal/slot syntax: `&Class::method`
- Set parent for `new`-allocated widgets (Qt parent-child ownership)
- Wrap UI text with `tr()` for i18n
- Qt macros (Q_OBJECT, Q_PROPERTY, Q_SIGNALS, Q_SLOTS, emit) are used as-is

## Git Workflow

- Commit messages: `<type>(<scope>): <subject>` (Conventional Commits)
- Types: feat, fix, docs, style, refactor, test, chore, build, ci, perf
- Subject: lowercase, imperative mood
- PR titles follow the same Conventional Commits format
