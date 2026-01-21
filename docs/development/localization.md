# Localization Guide

This guide explains how to add and manage translations in DevTools.

## Overview

DevTools uses Qt's internationalization (i18n) system with Qt Linguist tools. Currently supported languages:

- English (en) - Default
- Japanese (ja_JP)

## How It Works

```
Source Code        Qt Tools           Runtime
    │                 │                  │
    ▼                 ▼                  ▼
tr("text") ──► lupdate ──► .ts ──► lrelease ──► .qm ──► Application
                         (edit)
```

1. **Source Code**: Strings marked with `tr()`
2. **lupdate**: Extracts strings to .ts files
3. **.ts files**: XML translation files (editable)
4. **lrelease**: Compiles to binary .qm files
5. **Runtime**: Application loads .qm files

## Translation Files

```
res/
├── dev-tools_en.ts       # English translations
├── dev-tools_ja_JP.ts    # Japanese translations
├── dev-tools_en.qm       # Compiled English (generated)
└── dev-tools_ja_JP.qm    # Compiled Japanese (generated)
```

## Marking Strings for Translation

### In C++ Code

```cpp
// Basic translation
QString message = tr("Hello, World!");

// With context (for disambiguation)
QString save = tr("Save", "file operation");
QString save2 = tr("Save", "game operation");

// With arguments
QString greeting = tr("Hello, %1!").arg(userName);

// Plural forms
QString items = tr("%n item(s)", "", count);
```

### In .ui Files

Qt Designer automatically marks visible text for translation. Just use the Properties panel to set text.

### In QML (if used)

```qml
Text {
    text: qsTr("Hello, World!")
}
```

## Updating Translations

### Using CMake

```bash
# Update .ts files with new strings
cmake --build build --target update_translations

# Compile .qm files
cmake --build build --target release_translations
```

### Manual Commands

```bash
# Update specific .ts file
lupdate -recursive . -ts res/dev-tools_ja_JP.ts

# Compile to .qm
lrelease res/dev-tools_ja_JP.ts
```

## Editing Translations

### Using Qt Linguist

1. Open Qt Linguist
2. File > Open > Select .ts file
3. Navigate to strings in the left panel
4. Enter translations in the bottom panel
5. Mark as translated (checkmark or Ctrl+Enter)
6. Save (Ctrl+S)

### Linguist Interface

```
┌──────────────────────────────────────────────┐
│ Context/Source │     Translation Editor      │
├────────────────┼─────────────────────────────┤
│ MainWindow     │ Source: "Save"              │
│ ├─ "Save"      │ Translation: "保存"          │
│ ├─ "Open"      │                             │
│ └─ "Exit"      │ [✓] Translated              │
├────────────────┴─────────────────────────────┤
│ Warnings/Errors                              │
└──────────────────────────────────────────────┘
```

### Translation States

| State | Icon | Meaning |
|-------|------|---------|
| Untranslated | ? | No translation entered |
| Translated | ✓ | Translation complete |
| Obsolete | ~ | Source string removed |

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+Enter | Mark as done & next |
| Ctrl+Shift+Enter | Mark as done |
| Alt+Down | Next unfinished |
| Alt+Up | Previous unfinished |
| Ctrl+S | Save |

## Adding a New Language

### 1. Update CMakeLists.txt

```cmake
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ja_JP en fr)  # Add 'fr'
```

### 2. Create Translation File

```bash
# Create empty .ts file
lupdate -recursive . -ts res/dev-tools_fr.ts
```

### 3. Translate

Open in Qt Linguist and translate all strings.

### 4. Compile

```bash
cmake --build build --target release_translations
```

### 5. Add to Resources

Update `res/application.qrc`:

```xml
<RCC>
    <qresource prefix="/i18n">
        <file>dev-tools_en.qm</file>
        <file>dev-tools_ja_JP.qm</file>
        <file>dev-tools_fr.qm</file>  <!-- New -->
    </qresource>
</RCC>
```

### 6. Update Language Selection

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
2. Run lupdate
3. Open .ts in Linguist
4. Find "Untranslated" entries

## Translation Workflow

### For Developers

1. Write new UI with tr()
2. Run `make update_translations`
3. Notify translators

### For Translators

1. Open .ts file in Linguist
2. Translate new/modified strings
3. Save file
4. Commit changes

### Release Preparation

1. Ensure all strings translated
2. Run `make release_translations`
3. Test in target language
4. Include .qm files in build

## Related Documentation

- [Adding New Tools](adding-new-tools.md) - Module creation
- [Coding Standards](coding-standards.md) - Code style
- [Qt Linguist Manual](https://doc.qt.io/qt-6/linguist-translators.html)
