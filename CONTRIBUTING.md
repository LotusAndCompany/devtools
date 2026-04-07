# Contributing to DevTools

[日本語版](docs/ja/CONTRIBUTING.md)

Thank you for your interest in contributing to DevTools! This document provides guidelines and instructions for contributing.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [How to Contribute](#how-to-contribute)
- [Development Guidelines](#development-guidelines)
- [Pull Request Process](#pull-request-process)
- [Release Process](#release-process)

## Code of Conduct

This project adheres to the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

## Getting Started

### Prerequisites

Before contributing, make sure you have:

1. Read the [README](README.md)
2. Set up your development environment following [BUILD.md](BUILD.md)
3. Familiarized yourself with the project structure

### Project Structure

```
devtools/
├── core/           # Business logic and algorithms
├── gui/            # GUI components and tool interfaces
├── main/           # Application entry point
├── res/            # Resources and translation files
├── tests/          # Unit tests
├── docs/           # Documentation
└── distribution/   # Platform-specific packaging files
```

## How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported in [Issues](https://github.com/LotusAndCompany/devtools/issues)
2. If not, create a new issue using the bug report template
3. Include:
   - Clear description of the bug
   - Steps to reproduce
   - Expected vs actual behavior
   - Environment information (macOS version, Qt version, etc.)
   - Screenshots if applicable

### Requesting Features

1. Check existing issues for similar feature requests
2. Create a new issue using the feature request template
3. Describe:
   - The problem you're trying to solve
   - Your proposed solution
   - Any alternatives you've considered

### Submitting Code

1. Fork the repository
2. Create a feature branch from `main` (see [Branch Naming](#branch-naming))
3. Make your changes
4. Write or update tests as needed
5. Ensure all tests pass
6. Submit a pull request

## Development Guidelines

### Code Style

- Use C++17 features appropriately
- Follow existing code patterns in the project
- Use meaningful variable and function names
- Add comments for complex logic

### Code Formatting

This project uses `clang-format` for automatic code formatting and `clang-tidy` for static analysis.

#### Setup

Install the tools (macOS):
```bash
brew install llvm
```

#### Usage

Format all source files:
```bash
cmake --build build --target format
```

Check formatting without changes:
```bash
cmake --build build --target format-check
```

Run static analysis:
```bash
cmake --build build --target lint
```

Run analysis with automatic fixes:
```bash
cmake --build build --target lint-fix
```

#### Git Hooks (pre-commit)

This project uses [pre-commit](https://pre-commit.com/) for automated code quality checks.

**Installation:**

```bash
# Install pre-commit
brew install pre-commit  # macOS
# or: pip install pre-commit

# Install hooks
pre-commit install --install-hooks -t pre-commit -t commit-msg -t pre-push
```

**Hooks:**

| Stage | Hook | Description |
|-------|------|-------------|
| pre-commit | clang-format | Format C++ code |
| pre-commit | trailing-whitespace | Remove trailing whitespace |
| pre-commit | end-of-file-fixer | Ensure newline at EOF |
| pre-commit | check-added-large-files | Prevent large file commits |
| commit-msg | conventional-pre-commit | Validate commit message format |
| pre-push | cmake-build | Verify build succeeds |

**Manual Execution:**

```bash
# Run all hooks on all files
pre-commit run --all-files

# Run specific hook
pre-commit run clang-format --all-files

# Skip hooks temporarily
git commit --no-verify -m "WIP: work in progress"
git push --no-verify
```

**Updating Hooks:**

```bash
pre-commit autoupdate
```

#### CI (GitHub Actions)

Code quality checks run automatically on:
- **Pull Request** to `main`

| Check | Runner | Description |
|-------|--------|-------------|
| clang-format | macOS | Format check on changed files |
| clang-tidy | Ubuntu | Static analysis on changed files |
| format-suggestion | macOS | PR comment with format suggestions |
| semantic-pr | Ubuntu | Validate PR title follows Conventional Commits |

#### IDE Integration

- **Qt Creator**: Settings > C++ > Code Style > Import `.clang-format`
- **VS Code**: Install "clangd" extension, it will use `.clang-format` automatically
- **CLion**: Settings > Editor > Code Style > C/C++ > Enable ClangFormat

### Naming Conventions

| Type | Convention | Example |
|------|------------|---------|
| Classes | PascalCase | `ImageResize` |
| Functions | camelCase | `processImage()` |
| Variables | snake_case | `image_width` |
| Constants | SCREAMING_SNAKE_CASE | `MAX_IMAGE_SIZE` |
| Files | snake_case | `image_resize.cpp` |

### Adding New Modules

1. Create a new static library in `CMakeLists.txt`:
   ```cmake
   qt_add_library(${PROJECT_NAME}_your_module STATIC
       core/your_module/your_module.h
       core/your_module/your_module.cpp
       gui/your_module/your_module_gui.h
       gui/your_module/your_module_gui.cpp
       gui/your_module/your_module_gui.ui
   )
   ```

2. Add dependencies if needed:
   ```cmake
   add_dependencies(${PROJECT_NAME}_your_module ${PROJECT_NAME}_core)
   ```

3. Add to `MODULE_LIST`:
   ```cmake
   set(MODULE_LIST
       # ... existing modules
       ${PROJECT_NAME}_your_module
   )
   ```

### Adding Tests

Add tests to `tests/DevToolsTests.cmake`:
```cmake
DevTools_add_test(test_your_module
    SOURCES
    tests/core/your_module/test_your_module.cpp
)
```

Enable tests with `cmake .. -DENABLE_UNIT_TEST=ON`.

### Branch Naming

Use the following branch naming convention:

| Type | Pattern | Example |
|------|---------|---------|
| Feature | `feature/<description>` | `feature/sms-qr-code` |
| Bug fix | `fix/<description>` | `fix/image-rotation-angle` |
| Documentation | `doc/<description>` | `doc/update-build-guide` |
| Refactoring | `refactor/<description>` | `refactor/data-conversion` |
| Hotfix | `hotfix/<description>` | `hotfix/crash-on-startup` |

**Rules:**
- Always branch from `main`
- Use lowercase letters and hyphens (no underscores or spaces)
- Keep descriptions short but descriptive
- Include issue number if applicable: `feature/123-sms-qr-code`

```bash
# Example: Creating a feature branch
git checkout main
git pull origin main
git checkout -b feature/sms-qr-code
```

### Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/) format:

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**
| Type | Description |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation changes |
| `style` | Code style changes (formatting, no logic change) |
| `refactor` | Code refactoring |
| `test` | Adding or updating tests |
| `chore` | Maintenance tasks |
| `build` | Build system changes |
| `ci` | CI configuration changes |
| `perf` | Performance improvements |

**Rules:**
- Use present tense ("Add feature" not "Added feature")
- Use imperative mood ("Move cursor to..." not "Moves cursor to...")
- Keep the subject line concise (50 characters recommended)
- Wrap body lines at 72 characters
- Reference issues and pull requests in the footer

**Examples:**
```
feat(qr): Add QR code generation for phone numbers

- Implement tel: URI scheme support
- Add phone number validation
- Update UI with phone number input field

Closes #123
```

```
fix(image): Correct rotation angle calculation

The rotation was off by 90 degrees when using certain
image formats. This fix ensures consistent behavior
across all supported formats.

Fixes #456
```

```
docs: Update build instructions for macOS
```

## Pull Request Process

1. **Update documentation** if needed
2. **Write tests** for new functionality
3. **Ensure CI passes** (all tests pass, no build errors)
4. **Request review** from maintainers
5. **Address feedback** promptly
6. **Squash commits** if requested

### PR Title Format

Use a clear, descriptive title following the [Conventional Commits](https://www.conventionalcommits.org/) format.
The title is automatically validated by CI.

**Rules:**
- Start the subject (after the colon) with a lowercase letter
- Use present tense ("add feature" not "added feature")

**Examples:**
- `feat: add SMS QR code generation`
- `fix: correct image rotation angle calculation`
- `docs: update build instructions for macOS`
- `refactor: simplify data conversion logic`

### Review Process

1. A maintainer will review your PR
2. They may request changes or ask questions
3. Once approved, the PR will be merged
4. Your contribution will be acknowledged in the changelog

## Release Process

This project uses [release-please](https://github.com/googleapis/release-please) for automated release management.

### How It Works

1. **Conventional Commits determine version bumps:**
   - `fix:` commits → PATCH version (0.3.0 → 0.3.1)
   - `feat:` commits → MINOR version (0.3.0 → 0.4.0)
   - `BREAKING CHANGE:` in footer → MAJOR version (0.3.0 → 1.0.0)

2. **Automated workflow:**
   - When changes are merged to `main`, release-please creates/updates a Release PR
   - The Release PR includes:
     - Updated `CHANGELOG.md`
     - Version bumps in `CMakeLists.txt` and `vcpkg.json`
   - When the Release PR is merged, a GitHub Release and tag are created automatically

3. **Version files managed by release-please:**
   - `CMakeLists.txt` - Project version
   - `vcpkg.json` - Dependencies version
   - `version.txt` - Plain text version
   - `CHANGELOG.md` - Release notes

### For Maintainers

- Review and merge the automated Release PR when ready to release
- Do not manually edit version numbers or CHANGELOG (release-please handles this)
- Ensure commit messages follow Conventional Commits format for accurate version determination

## Questions?

If you have questions, feel free to:
- Open an issue for discussion
- Contact the maintainers

Thank you for contributing to DevTools!
