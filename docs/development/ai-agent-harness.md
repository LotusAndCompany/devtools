# AI Agent Harness

This document describes the cross-tool AI agent configuration used in this project. Multiple AI coding assistants share a single source of truth while retaining tool-specific settings where needed.

## Overview

AI coding tools (Claude Code, Gemini CLI, OpenAI Codex, CodeRabbit, etc.) each have their own configuration format. Rather than maintaining duplicate guidelines in every tool's config, this project uses a layered architecture:

1. **AGENTS.md** — shared guidelines that all tools can read
2. **Tool-specific configs** — import or reference AGENTS.md, adding only tool-unique settings

This means a change to AGENTS.md immediately benefits every tool.

## Architecture

```
AGENTS.md  (single source of truth)
    │
    ├── CLAUDE.md .............. @AGENTS.md import + author rules
    │   └── .claude/rules/ ..... domain-scoped rules (C++, CMake, docs, git)
    │
    ├── GEMINI.md .............. @AGENTS.md import + author rules
    │   └── .gemini/settings.json .. fileName reference to GEMINI.md
    │
    ├── .codex/config.toml ..... project settings (native AGENTS.md discovery)
    │   └── .codex/rules/ ...... command execution control (Starlark)
    │
    └── .coderabbit.yaml ....... independent config, aligned with AGENTS.md
```

## Shared Guidelines (AGENTS.md)

`AGENTS.md` at the repository root contains project-wide guidelines:

- Build commands (CMake, make, ctest)
- Code formatting rules (clang-format, editorconfig)
- Naming conventions (PascalCase, camelCase, snake_case)
- Architecture overview (core/ vs gui/ separation)
- Qt conventions (signal/slot syntax, parent ownership, tr())
- Git workflow (Conventional Commits, branch naming, PR rules)
- i18n setup (Qt Linguist)
- Dependencies (vcpkg, Qt6, bundled libraries)
- Pre-commit hooks

**Update policy**: Always update AGENTS.md first. Tool-specific configs should never duplicate content that belongs in the shared file.

## Tool-Specific Configurations

### Claude Code

| File | Purpose |
|------|---------|
| `CLAUDE.md` | Imports AGENTS.md via `@AGENTS.md`, adds author attribution rules |
| `.claude/rules/cpp-style.md` | C++17 style rules, scoped to `*.cpp, *.h, *.hpp` |
| `.claude/rules/cmake.md` | CMake conventions, scoped to `CMakeLists.txt` |
| `.claude/rules/docs.md` | Bilingual documentation rules, scoped to `docs/**` |
| `.claude/rules/git-workflow.md` | Conventional Commits and branch naming rules |
| `.claude/settings.local.json` | Tool permissions (local, not committed) |

**How it works**: Claude Code reads `CLAUDE.md` at startup, which uses `@AGENTS.md` to inline the shared guidelines. The `.claude/rules/` directory provides domain-specific rules that activate only when working on matching file paths.

### Gemini CLI

| File | Purpose |
|------|---------|
| `GEMINI.md` | Imports AGENTS.md via `@AGENTS.md`, adds author attribution rules |
| `.gemini/settings.json` | Points Gemini to read `GEMINI.md` as context |

**How it works**: Gemini CLI reads `GEMINI.md` at startup, which uses `@AGENTS.md` to inline the shared guidelines. The `context.fileName` array in `.gemini/settings.json` tells Gemini CLI which files to load as persistent context.

### OpenAI Codex

| File | Purpose |
|------|---------|
| `.codex/config.toml` | Project-level settings (approval mode) |
| `.codex/rules/project.rules` | Command execution control (Starlark) |

**How it works**: Codex discovers `AGENTS.md` natively by walking the directory tree from the git root. No explicit reference is needed. The `.codex/config.toml` sets project defaults (e.g., `approval_mode`), and `.codex/rules/` controls which shell commands Codex can run without user approval.

The rules file uses Starlark `prefix_rule()` to classify commands:
- **allow** — build, test, format, read-only git operations
- **prompt** — git add, commit, push (requires user confirmation)
- **forbidden** — force push, hard reset (never allowed)

### CodeRabbit

| File | Purpose |
|------|---------|
| `.coderabbit.yaml` | PR review configuration with path-based instructions |

**How it works**: CodeRabbit is a GitHub PR review bot. It does not read AGENTS.md directly, but its `path_instructions` section mirrors the same coding standards. It is configured independently but kept aligned with AGENTS.md.

## Adding a New AI Tool

When adding support for a new AI coding assistant:

1. **Check native AGENTS.md support** — Many tools (Codex, Gemini CLI, Cursor, GitHub Copilot) can read AGENTS.md or similar markdown files. If supported, configure the tool to read it.
2. **Create a tool-specific config** — Only add settings that are unique to the tool (e.g., permission models, approval modes, author attribution rules).
3. **Do not duplicate AGENTS.md content** — The tool-specific config should reference or import the shared guidelines, not copy them.
4. **Update this document** — Add the new tool to the Architecture diagram and Tool-Specific Configurations section.

## File Reference

| File | Tool | Tracked | Description |
|------|------|---------|-------------|
| `AGENTS.md` | All | Yes | Shared project guidelines |
| `CLAUDE.md` | Claude Code | Yes | Imports AGENTS.md + author rules |
| `.claude/rules/*.md` | Claude Code | Yes | Domain-scoped rules |
| `.claude/settings.local.json` | Claude Code | No | Local tool permissions |
| `GEMINI.md` | Gemini CLI | Yes | Imports AGENTS.md + author rules |
| `.gemini/settings.json` | Gemini CLI | Yes | Context file reference |
| `.codex/config.toml` | OpenAI Codex | Yes | Project settings |
| `.codex/rules/*.rules` | OpenAI Codex | Yes | Command execution rules |
| `.coderabbit.yaml` | CodeRabbit | Yes | PR review configuration |

## Related Documentation

- [Architecture](architecture.md) — System architecture and module structure
- [Coding Standards](coding-standards.md) — Code style and naming conventions
