# Regex Tester

Build and test regular expressions interactively with instant match highlighting, capture group inspection, and substitution preview.

## Overview

The Regex Tester helps developers craft regular expressions without the write-run-print loop. Matches are highlighted directly in the test input as you type the pattern, so you can iterate quickly.

## Accessing the Tool

1. Launch DevTools
2. Click **Regex Tester** in the side menu

## Interface Overview

The Regex Tester interface consists of:

1. **Pattern Field**: Regular expression input with flag toggles
2. **Flags**: `g`, `i`, `m`, `s`, `u`, `x` toggles
3. **Test Input**: Multi-line text area with match highlighting
4. **Matches Panel**: Ordered list of matches with captured groups
5. **Replace Section**: Replacement pattern input and result preview
6. **Quick Reference**: Collapsible cheat sheet of common tokens

## Features

### Live Match Highlighting

As the pattern or test input changes, matches are highlighted in-place in the test input area. Invalid patterns surface an inline error message under the pattern field.

### Flags

| Flag | Meaning |
|------|---------|
| `g` | Global - find all matches |
| `i` | Case-insensitive |
| `m` | Multiline - `^` and `$` match line boundaries |
| `s` | Dotall - `.` matches newlines |
| `u` | Unicode |
| `x` | Extended - ignore whitespace in pattern |

### Match Inspector

For each match the panel displays:

- Match index (0-based)
- Full match text
- Byte offset (start, end)
- Named and numbered capture groups

### Substitution Preview

1. Enter a replacement pattern (supports `$1`, `$<name>`)
2. See the fully substituted output in real time
3. Copy the result to clipboard

### Pattern Presets

Quick-insert common patterns:

| Preset | Use |
|--------|-----|
| Email | RFC-5322 simplified email |
| URL | HTTP / HTTPS URL |
| IPv4 | Dotted-quad address |
| Date (YYYY-MM-DD) | ISO date |
| Phone (E.164) | International phone number |
| UUID v4 | Random UUID |

### Engine

Uses the ECMAScript regex engine so behavior matches what developers see in JavaScript / TypeScript runtimes.

## How to Use

### Basic Matching

1. Enter a pattern, e.g. `\b\w+@\w+\.\w+\b`
2. Toggle flags as needed (e.g. `g` for all matches)
3. Paste or type sample text into the test input
4. Review highlighted matches and the matches panel

### Using Capture Groups

Pattern: `(\w+)@(\w+)`

For input `alice@example`, the matches panel shows:
- Group 1: `alice`
- Group 2: `example`

### Replace

1. Enter pattern: `(\w+)@(\w+)`
2. Enter replacement: `$2/$1`
3. Preview: `example/alice`
4. Click **Copy Result**

## Use Cases

- Validating log line formats
- Extracting tokens from semi-structured text
- Composing search-and-replace operations before running them in an editor
- Teaching and debugging unfamiliar regex patterns

## Requirements

### Functional Requirements

- FR-1: Evaluate the pattern against the test input on every change (debounced ≤ 100ms)
- FR-2: Highlight every match in the test input when the `g` flag is on; otherwise highlight only the first
- FR-3: Display start/end offsets and all capture groups (named + numbered) for each match
- FR-4: Surface regex syntax errors inline without crashing the tool
- FR-5: Provide substitution preview using `$1`..`$9` and `$<name>` back-references
- FR-6: Offer at least 6 built-in pattern presets (email, URL, IPv4, ISO date, phone, UUID)
- FR-7: Copy the pattern, the matches list, or the substitution result to clipboard with a single click
- FR-8: Persist the last pattern, flags, and test input across sessions

### Non-functional Requirements

- NFR-1: Detect and abort pattern evaluation that exceeds 500ms to guard against catastrophic backtracking
- NFR-2: Handle test inputs up to 100,000 characters without blocking the UI
- NFR-3: Match ECMAScript regex semantics exactly (the engine used at runtime)

## Safety

- Pattern evaluation runs with a timeout to avoid freezes on pathological input
- The tool is read-only; it never modifies files on disk automatically

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
