# DevTools - AI Agent Bootstrap

DevTools is a macOS desktop application built with C++17 and Qt6 Widgets.

This file is intentionally short. Detailed, path-scoped guidance lives in
`.agents/rules/` and is synchronized into tool-specific adapters.

## How to Use Rules

- Always read this file first.
- For implementation details, consult every `.agents/rules/*.md` file whose
  `paths` front matter matches the files you will inspect or edit.
  OpenCode loads these automatically via `opencode.json`.
- Rules with `paths: ["**/*"]` apply globally.
- After editing `AGENTS.md` or `.agents/rules/*.md`, run:

```bash
scripts/sync-agent-rules.py
```

## Rule Index

- `.agents/rules/project.md`: build, architecture, dependencies, hooks, harness
- `.agents/rules/cpp-style.md`: C++/Qt style, formatting, naming
- `.agents/rules/cmake.md`: CMake modules, dependencies, test registration
- `.agents/rules/testing.md`: Google Test and CTest rules
- `.agents/rules/i18n.md`: Qt translation workflow
- `.agents/rules/docs.md`: English/Japanese documentation rules
- `.agents/rules/exec-plans.md`: execution plan and tech-debt tracker rules
- `.agents/rules/design-files.md`: Pencil `.pen` design file handling
- `.agents/rules/generated-files.md`: generated artifacts and vendor files
- `.agents/rules/git-workflow.md`: branches, commits, PRs, release-please

## Critical Guardrails

- Do not edit generated build outputs, Qt-generated files, compiled
  translations, or bundled vendor code directly.
- Do not read, grep, or edit Pencil `.pen` files directly; use the Pencil editor
  or `pencil` MCP server.
- Do not include AI tool author attribution in commits, PRs, or issues.
