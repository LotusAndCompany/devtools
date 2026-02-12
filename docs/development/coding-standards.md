# Coding Standards

This document describes the coding standards and conventions used in the DevTools project.

## Code Formatting

### Automatic Formatting

DevTools uses clang-format for automatic code formatting. Configuration is in `.clang-format` at the project root.

#### Run Formatting

```bash
# Format all files
cmake --build build --target format

# Check formatting without changes
cmake --build build --target format-check
```

#### IDE Integration

| IDE | Setup |
|-----|-------|
| Qt Creator | Settings > C++ > Code Style > Import `.clang-format` |
| VS Code | Install "clangd" extension |
| CLion | Settings > Editor > Code Style > Enable ClangFormat |

### Static Analysis

DevTools uses clang-tidy for static analysis. Configuration is in `.clang-tidy`.

```bash
# Run analysis
cmake --build build --target lint

# Run with automatic fixes
cmake --build build --target lint-fix

# Run all quality checks
cmake --build build --target quality-check
```

## Naming Conventions

### General Rules

| Type | Convention | Example |
|------|------------|---------|
| Classes | PascalCase | `ImageResize`, `MainWindow` |
| Functions/Methods | camelCase | `processImage()`, `loadFile()` |
| Variables | snake_case | `image_width`, `file_path` |
| Member Variables | snake_case with suffix | `image_width_` or `m_image_width` |
| Constants | SCREAMING_SNAKE_CASE | `MAX_IMAGE_SIZE`, `DEFAULT_VALUE` |
| Enums | PascalCase | `enum class ToolId { QrCode, Image }` |
| Namespaces | lowercase | `devtools`, `image` |
| Files | snake_case | `image_resize.cpp`, `main_window.h` |

### Examples

```cpp
// Class
class ImageProcessor {
public:
    // Method
    void processImage(const QImage& input);
    
    // Constants
    static constexpr int MAX_IMAGE_SIZE = 10000;
    
private:
    // Member variables
    int image_width_;
    QString file_path_;
};

// Enum
enum class ImageFormat {
    Png,
    Jpeg,
    Bmp
};

// Function
void saveToFile(const QString& path);
```

## File Organization

### Header Files (.h)

```cpp
#ifndef PROJECT_MODULE_CLASS_H
#define PROJECT_MODULE_CLASS_H

// System includes
#include <memory>
#include <string>

// Qt includes
#include <QString>
#include <QWidget>

// Project includes
#include "core/tool/tool.h"

namespace devtools {

class ClassName : public QWidget {
    Q_OBJECT

public:
    explicit ClassName(QWidget* parent = nullptr);
    ~ClassName() override;

    // Public methods
    void publicMethod();

signals:
    void signalName();

public slots:
    void slotName();

protected:
    // Protected methods

private slots:
    // Private slots

private:
    // Private methods
    void privateMethod();
    
    // Member variables
    int member_variable_;
};

}  // namespace devtools

#endif  // PROJECT_MODULE_CLASS_H
```

### Source Files (.cpp)

```cpp
#include "class_name.h"

// System includes
#include <algorithm>

// Qt includes
#include <QDebug>

namespace devtools {

ClassName::ClassName(QWidget* parent)
    : QWidget(parent)
    , member_variable_(0) {
    // Constructor implementation
}

ClassName::~ClassName() = default;

void ClassName::publicMethod() {
    // Implementation
}

void ClassName::privateMethod() {
    // Implementation
}

}  // namespace devtools
```

## Qt-Specific Guidelines

### Signals and Slots

```cpp
// Prefer new-style connections
connect(button, &QPushButton::clicked, this, &MyClass::onButtonClicked);

// Avoid old-style connections
// connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
```

### Memory Management

```cpp
// Qt parent-child ownership
auto* widget = new QWidget(this);  // 'this' takes ownership

// For non-QObject types, use smart pointers
std::unique_ptr<DataProcessor> processor = std::make_unique<DataProcessor>();
```

### UI Files

- Use `.ui` files for complex layouts
- Keep simple layouts in code
- Name UI elements descriptively: `saveButton`, `inputLineEdit`

## C++17 Features

Use modern C++ features appropriately:

### Allowed

```cpp
// Structured bindings
auto [width, height] = getImageSize();

// if with initializer
if (auto result = process(); result.isValid()) {
    // ...
}

// std::optional
std::optional<QString> findFile(const QString& name);

// constexpr
constexpr int calculate() { return 42; }

// Inline variables
inline constexpr int MAX_SIZE = 1000;
```

### Prefer

```cpp
// Use auto for complex types
auto iterator = container.begin();

// Use range-based for
for (const auto& item : items) {
    // ...
}

// Use nullptr instead of NULL or 0
Widget* ptr = nullptr;
```

## Error Handling

### Exceptions

Use project-specific exceptions defined in `core/exception/`:

```cpp
#include "core/exception/invalid_argument_exception.h"

void processValue(int value) {
    if (value < 0) {
        throw InvalidArgumentException("Value must be non-negative");
    }
}
```

### Available Exceptions

| Exception | Use Case |
|-----------|----------|
| `InvalidArgumentException` | Invalid function arguments |
| `OutOfRangeException` | Index or value out of range |
| `InvalidStateException` | Object in invalid state |
| `UnderDevelopmentException` | Feature not yet implemented |
| `CommonException` | General errors |

### Qt-style Error Handling

For Qt operations, check return values:

```cpp
QFile file(path);
if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open file:" << path;
    return false;
}
```

## Comments

### When to Comment

- Complex algorithms
- Non-obvious business logic
- Workarounds with explanations
- Public API documentation

### Comment Style

```cpp
// Single-line comment for brief explanations

/*
 * Multi-line comment for longer explanations
 * that span multiple lines.
 */

/**
 * @brief Brief description.
 * 
 * Detailed description for Doxygen documentation.
 * 
 * @param param Description of parameter
 * @return Description of return value
 */
QString processData(const QString& param);
```

### Avoid

- Obvious comments
- Commented-out code (use version control)
- TODO comments without issue references

## Include Order

1. Corresponding header (for .cpp files)
2. System headers
3. Qt headers
4. Third-party library headers
5. Project headers

```cpp
// my_class.cpp
#include "my_class.h"

#include <algorithm>
#include <vector>

#include <QString>
#include <QWidget>

#include <yaml-cpp/yaml.h>

#include "core/tool/tool.h"
#include "gui/main_window.h"
```

## Git Hooks (pre-commit)

This project uses [pre-commit](https://pre-commit.com/) for automated code quality checks.

### Installation

```bash
# Install pre-commit
brew install pre-commit  # macOS
# or: pip install pre-commit

# Install hooks
pre-commit install --install-hooks -t pre-commit -t commit-msg -t pre-push
```

### Hooks

| Stage | Hook | Description |
|-------|------|-------------|
| pre-commit | clang-format | Format C++ code |
| pre-commit | trailing-whitespace | Remove trailing whitespace |
| pre-commit | end-of-file-fixer | Ensure newline at EOF |
| pre-commit | check-added-large-files | Prevent large file commits |
| commit-msg | conventional-pre-commit | Validate commit message format |
| pre-push | cmake-build | Verify build succeeds |

### Manual Execution

```bash
# Run all hooks on all files
pre-commit run --all-files

# Run specific hook
pre-commit run clang-format --all-files

# Skip hooks temporarily
git commit --no-verify -m "WIP: work in progress"
git push --no-verify
```

### Updating Hooks

```bash
pre-commit autoupdate
```

## CI (GitHub Actions)

Code quality checks run automatically on:
- **Pull Request** to `main`

| Check | Runner | Description |
|-------|--------|-------------|
| clang-format | macOS | Format check on changed files |
| clang-tidy | Ubuntu | Static analysis on changed files |
| format-suggestion | macOS | PR comment with format suggestions |
| semantic-pr | Ubuntu | Validate PR title follows Conventional Commits |

## Related Documentation

- [Architecture](architecture.md) - System architecture
- [Adding New Tools](adding-new-tools.md) - How to add features
- [Contributing Guide](../../CONTRIBUTING.md) - Contribution process
