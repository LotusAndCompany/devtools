# Markdown Preview

Edit Markdown text and see the rendered HTML output side-by-side in real time.

## Overview

The Markdown Preview tool helps developers write documentation, README files, and notes with immediate visual feedback. The left pane is the editor; the right pane renders the HTML output as you type.

## Accessing the Tool

1. Launch DevTools
2. Click **Markdown Preview** in the side menu

## Interface Overview

The Markdown Preview interface consists of:

1. **Toolbar**: Open, Save, Export HTML, and a scroll-sync toggle
2. **Editor Pane (left)**: Markdown source input
3. **Preview Pane (right)**: Rendered HTML output

## Features

### Real-time Rendering

- HTML preview updates as you type (debounced by ~180ms to avoid flicker)
- Rendering uses Qt's GitHub-flavored Markdown dialect

### Scroll Synchronization

- The editor and preview panes scroll together
- Toggle synchronization on or off with the **Sync Scroll** checkbox in the toolbar (enabled by default)

### Supported Markdown Syntax

Rendering is provided by Qt's `QTextDocument` using the GitHub Markdown dialect. Commonly supported elements include:

| Element | Syntax |
|---------|--------|
| Headings | `#`, `##`, `###` ... `######` |
| Bold / Italic | `**bold**`, `*italic*` |
| Lists | `-`, `*`, `1.` |
| Links | `[text](url)` |
| Images | `![alt](src)` |
| Code (inline) | `` `code` `` |
| Code blocks | ` ```lang ` fenced blocks |
| Blockquote | `> text` |

> Note: Supported syntax follows Qt's Markdown implementation and may differ from full GitHub Flavored Markdown in some edge cases.

### File Operations

- **Open**: Load a `.md` / `.markdown` file into the editor
- **Save**: Save editor content as a `.md` / `.markdown` file
- **Export HTML**: Save the rendered output as a standalone `.html` file

### Theme Support

- The editor and preview honor the application's light/dark theme

## How to Use

### Basic Usage

1. Type or paste Markdown into the editor pane
2. Watch the preview render on the right
3. Use the toolbar to open, save, or export

### Loading a File

1. Click **Open**
2. Select a `.md` or `.markdown` file
3. Content loads into the editor; preview renders automatically

### Exporting

| Target | Action |
|--------|--------|
| Markdown source | **Save** → choose a `.md` filename |
| Rendered HTML | **Export HTML** → choose an `.html` filename |

## Use Cases

- Writing and previewing README files
- Drafting documentation with live feedback
- Converting Markdown notes to HTML for sharing

## Limitations

- No syntax highlighting or line numbers in the editor
- No status bar (character / word count)
- No alternate view modes (split view only)
- No custom CSS for the preview
- Markdown extensions beyond Qt's GitHub dialect (e.g., MathJax, Mermaid) are not supported

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
