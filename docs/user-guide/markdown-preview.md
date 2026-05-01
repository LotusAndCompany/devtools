# Markdown Preview

Edit Markdown text and see the rendered HTML output side-by-side in real time.

## Overview

The Markdown Preview tool helps developers write documentation, README files, and notes with immediate visual feedback. The left pane is the editor; the right pane renders the HTML output as you type.

## Accessing the Tool

1. Launch DevTools
2. Click **Markdown Preview** in the side menu

## Interface Overview

The Markdown Preview interface consists of:

1. **Toolbar**: File actions, view toggle, export button
2. **Editor Pane (left)**: Markdown source input with syntax highlighting and line numbers
3. **Preview Pane (right)**: Rendered HTML output, scroll-synced with the editor
4. **Status Bar**: Character count, word count, cursor position

## Features

### Real-time Rendering

- HTML preview updates as you type (debounced to avoid flicker)
- Editor and preview panes are scroll-synchronized

### Supported Markdown Syntax

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
| Tables | GFM pipe tables |
| Task lists | `- [ ]`, `- [x]` |
| Horizontal rule | `---` |

### View Modes

| Mode | Description |
|------|-------------|
| Split | Editor and preview side-by-side (default) |
| Editor Only | Focus on writing |
| Preview Only | Focus on reading |

### File Operations

- **Open**: Load a `.md` file into the editor
- **Save**: Save editor content as `.md`
- **Export HTML**: Save the rendered preview as a standalone `.html` file

### Editor Features

- Syntax highlighting for Markdown and fenced code blocks
- Line numbers
- Soft wrap toggle
- Tab width setting (2 / 4 spaces)

## How to Use

### Basic Usage

1. Type or paste Markdown into the editor pane
2. Watch the preview render on the right
3. Use the toolbar to toggle view mode or export

### Loading a File

1. Click **Open**
2. Select a `.md` file
3. Content loads into the editor; preview renders automatically

### Exporting

| Target | Action |
|--------|--------|
| Markdown source | **Save** → choose `.md` filename |
| Rendered HTML | **Export HTML** → choose `.html` filename |

## Use Cases

- Writing and previewing README files
- Drafting documentation with live feedback
- Verifying GFM table and task list formatting before committing
- Converting Markdown notes to HTML for sharing

## Requirements

### Functional Requirements

- FR-1: Render Markdown to HTML on every editor change (debounced ≤ 200ms)
- FR-2: Support CommonMark + GitHub Flavored Markdown (tables, task lists, strikethrough, autolinks)
- FR-3: Synchronize scroll between editor and preview
- FR-4: Provide Split / Editor Only / Preview Only view modes
- FR-5: Open and save `.md` files
- FR-6: Export rendered HTML as standalone `.html`
- FR-7: Show character count and word count in the status bar
- FR-8: Persist the last view mode and tab width across sessions

### Non-functional Requirements

- NFR-1: Render documents up to 10,000 lines without noticeable lag (≥ 60fps scrolling)
- NFR-2: Honor the application light/dark theme in both editor and preview
- NFR-3: Sanitize rendered HTML to prevent script execution from untrusted input

## Limitations

- No custom CSS for the preview
- No support for Markdown extensions beyond GFM (e.g., MathJax, Mermaid) in the initial version
- Large images are not optimized; they are rendered as-is

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
