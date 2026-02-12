# Application Overview

DevTools is a unified desktop application that combines various development tools into a single, convenient interface.

## Purpose

DevTools eliminates the need for multiple web services by providing all essential developer utilities in one place. Instead of opening multiple browser tabs for different tools, you can access everything from a single application.

## Main Features

| Feature | Description |
|---------|-------------|
| [QR Code Generation](qr-code-generation.md) | Create QR codes for text, URLs, emails, phone numbers, SMS, and locations |
| [Image Processing](image-processing.md) | Resize, rotate, split images, and add transparency |
| [Data Conversion](data-conversion.md) | Convert between JSON, YAML, and TOML formats |
| [API Testing](api-testing.md) | Send HTTP requests and view responses |
| [Command Execution](command-execution.md) | Run shell commands from the GUI |
| [Phrase Generation](phrase-generation.md) | Generate random text and phrases |

## User Interface

### Layout

```
+------------------+--------------------------------+
|                  |                                |
|   Side Menu      |      Main Content Area         |
|                  |                                |
|   - QR Code      |   (Selected tool interface)    |
|   - Image        |                                |
|   - Data Conv.   |                                |
|   - API          |                                |
|   - Command      |                                |
|   - Phrase       |                                |
|                  |                                |
+------------------+--------------------------------+
```

### Side Menu

The side menu provides navigation between tools:

- Click a tool name to switch to that tool
- The current tool is highlighted
- Icons help identify each tool quickly

### Main Content Area

The main content area displays the selected tool's interface. Each tool has its own UI designed for its specific purpose.

### Menu Bar

| Menu | Contents |
|------|----------|
| **DevTools** | About, Preferences, Quit |
| **File** | Open, Save, Export options |
| **Edit** | Copy, Paste, Select All |
| **View** | Theme selection, Window options |
| **Help** | Documentation links |

## Preferences

Access preferences via **DevTools > Preferences** or `Cmd + ,`.

### General Settings

- **Language**: Switch between English and Japanese
- **Theme**: Light or Dark mode

### Tool-Specific Settings

Some tools have additional configuration options accessible within their interface.

## Keyboard Shortcuts

### Global Shortcuts

| Shortcut | Action |
|----------|--------|
| `Cmd + Q` | Quit DevTools |
| `Cmd + ,` | Open Preferences |
| `Cmd + W` | Close Window |
| `Cmd + M` | Minimize Window |

### Common Actions

| Shortcut | Action |
|----------|--------|
| `Cmd + C` | Copy |
| `Cmd + V` | Paste |
| `Cmd + A` | Select All |
| `Cmd + S` | Save (context-dependent) |
| `Cmd + O` | Open File (context-dependent) |

## Themes

DevTools supports two themes:

### Light Theme
- Clean, bright interface
- Best for well-lit environments
- Default theme

### Dark Theme
- Dark background with light text
- Reduces eye strain in low-light conditions
- Easier on the eyes during extended use

To change themes: **View > Theme > Light/Dark**

## Language Support

DevTools is available in:

- **English** (default)
- **Japanese** (日本語)

To change language:
1. Open **Preferences**
2. Select the **Language** tab
3. Choose your language
4. Restart the application

## File Formats

### Supported Input Formats

| Tool | Formats |
|------|---------|
| Image Processing | PNG, JPEG, BMP, GIF, TIFF |
| Data Conversion | JSON, YAML, TOML |

### Supported Output Formats

| Tool | Formats |
|------|---------|
| QR Code | PNG |
| Image Processing | PNG, JPEG |
| Data Conversion | JSON, YAML, TOML |

## System Requirements

- **OS**: macOS 15.0 or later
- **Architecture**: Apple Silicon (arm64) and Intel (x86_64)
- **Memory**: 4GB RAM recommended
- **Storage**: 100MB free space

## Getting Help

- [Common Issues](../troubleshooting/common-issues.md)
- [FAQ](../troubleshooting/faq.md)
- [GitHub Issues](https://github.com/LotusAndCompany/devtools/issues)

## Related Documentation

- [Quick Start](../getting-started/quick-start.md) - Get started quickly
- [Installation](../getting-started/installation.md) - Setup instructions
