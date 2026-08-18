# Localization Guide

This guide explains how to add and manage translations in DevTools.

## Overview

DevTools uses Qt's internationalization (i18n) system with Qt translation tools. Currently
supported languages:

- English (en) - Default source language, no `.ts` file
- Japanese (ja_JP)

## How It Works

```
Source Code        Qt Tools           Runtime
    │                 │                  │
    ▼                 ▼                  ▼
tr("text") ──► lupdate ──► .ts ──► lrelease ──► .qm ──► Application
                         (tracked)        (generated)
```

1. **Source Code**: English strings marked with `tr()`
2. **lupdate**: Extracts strings to .ts files when explicitly requested
3. **.ts files**: XML translation source files tracked in Git
4. **lrelease**: Compiles to binary .qm files during normal builds
5. **Runtime**: Application loads .qm files for translated languages; English uses the source text

## Translation Files

```
res/
├── dev-tools_ja_JP.ts    # Japanese translations
└── dev-tools_ja_JP.qm    # Compiled Japanese (generated)
```

## Marking Strings for Translation

### In C++ Code

```cpp
// Basic translation. Use English as the source string.
QString message = tr("Hello, World!");

// With context (for disambiguation)
QString save = tr("Save", "file operation");
QString save2 = tr("Save", "game operation");

// With arguments
QString greeting = tr("Hello, %1!").arg(userName);

// Plural forms
QString items = tr("%n item(s)", "", count);
```

## Updating Translations

### Using CMake

```bash
# Update .ts files with new strings
cmake --build build --target update_devtools_translations

# Compile .qm files
cmake --build build --target release_devtools_translations
```

### Manual Commands

```bash
# Update specific .ts file
lupdate -locations none -no-obsolete -recursive . -ts res/dev-tools_ja_JP.ts

# Compile to .qm
lrelease res/dev-tools_ja_JP.ts
```

## Editing Translations

Edit the tracked `.ts` files when translation text changes. The update target uses
`-locations none -no-obsolete` so source line numbers and obsolete messages do not create
unrelated diffs.

### Translation Entry Shape

```xml
<context>
    <name>SettingsDialog</name>
    <message>
        <source>Language:</source>
        <translation>言語:</translation>
    </message>
</context>
```

## Adding a New Language

### 1. Update CMakeLists.txt

```cmake
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ja_JP fr)  # Add 'fr'
```

### 2. Add Translation File

```cmake
set(TRANSLATION_TS_FILES
    res/dev-tools_ja_JP.ts
    res/dev-tools_fr.ts
)
```

### 3. Update and Translate

```bash
cmake --build build --target update_devtools_translations
```

### 4. Update Language Selection

Add the new language to the settings dialog.

## Best Practices

### String Guidelines

```cpp
// Good: Complete sentences
tr("The file could not be saved.")

// Bad: Sentence fragments
tr("could not be") + tr("saved")  // Breaks in other languages

// Good: Use arguments for dynamic content
tr("File '%1' saved successfully.").arg(filename)

// Bad: Concatenation
tr("File '") + filename + tr("' saved successfully.")
```

### Context for Disambiguation

```cpp
// When same English word has different meanings
tr("Open", "menu action");      // メニュー: 開く
tr("Open", "file state");       // ファイル状態: 開いている

// Class name is automatic context
class SaveDialog : public QDialog {
    // Strings here have context "SaveDialog"
    tr("Save");  // Context: SaveDialog
};
```

### Handling Plurals

```cpp
// English: "1 file" / "2 files"
// Japanese: "1 ファイル" / "2 ファイル" (same form)

int count = 5;
QString text = tr("%n file(s) selected", "", count);
```

### Numbers and Dates

```cpp
// Use locale-aware formatting
QLocale locale;
QString number = locale.toString(12345.67);  // "12,345.67" or "12.345,67"
QString date = locale.toString(QDate::currentDate());
```

## Testing Translations

### Switch Language at Runtime

For testing, you can switch language in preferences:

1. DevTools > Preferences
2. Select Language
3. Restart application

### Check for Untranslated Strings

```bash
# Find untranslated strings
lconvert -i res/dev-tools_ja_JP.ts -o report.txt -of csv
grep "type=\"unfinished\"" res/dev-tools_ja_JP.ts
```

### Visual Testing

1. Switch to target language
2. Navigate through all screens
3. Check for:
   - Truncated text
   - Layout issues
   - Missing translations
   - Wrong context translations

## Common Issues

### Text Truncation

```cpp
// Problem: Fixed-size button
button->setFixedWidth(100);  // May truncate translations

// Solution: Use size hints
button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
```

### Layout Breaking

- Use layouts that resize
- Avoid fixed sizes for text elements
- Test with longest translations

### Missing Translations

1. Check string is marked with tr()
2. Run `cmake --build build --target update_devtools_translations`
3. Find `type="unfinished"` entries in the `.ts` files

## Translation Workflow

### For Developers

1. Write new UI with tr()
2. Run `cmake --build build --target update_devtools_translations`
3. Update `.ts` translations when needed
4. Commit `.ts` changes with the code change

### Release Preparation

1. Ensure all strings translated
2. Run `cmake --build build --target release_devtools_translations`
3. Test in target language
4. Keep `.qm` files as generated build artifacts

## Related Documentation

- [Adding New Tools](adding-new-tools.md) - Module creation
- [Coding Standards](coding-standards.md) - Code style
- [Qt Internationalization](https://doc.qt.io/qt-6/localization.html)
