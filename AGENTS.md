# DevTools - AI Agent Bootstrap

DevTools is a macOS 15+ Apple Silicon desktop app with a Tauri 2 Rust host and a React/TypeScript UI built with shadcn/ui and Base UI. macOS WebKit is supplied by the operating system.

This file is intentionally short. Detailed path-scoped guidance lives in
.agents/rules/ and is synchronized into tool-specific adapters.

## How to Use Rules

- Always read this file first.
- For implementation details, consult every .agents/rules/*.md whose
  paths front matter matches files you will inspect or edit.
- OpenCode loads the shared rules through opencode.json.
- After editing this file or .agents/rules/*.md, run
  scripts/sync-agent-rules.py.

## Rule Index

- .agents/rules/project.md: architecture, build, dependencies, and CI
- .agents/rules/frontend.md: React and Tauri bridge boundaries
- .agents/rules/testing.md: Rust and frontend tests
- .agents/rules/i18n.md: English/Japanese UI text
- .agents/rules/docs.md: documentation language and accuracy
- .agents/rules/design-files.md: Pencil .pen handling
- .agents/rules/generated-files.md: generated output and vendor files
- .agents/rules/git-workflow.md: PRs and release-please

## Critical Guardrails

- Do not edit generated output under src-tauri/target/,
  frontend/node_modules/, or frontend/dist/ directly.
- Do not read, grep, or edit Pencil .pen files directly; use the Pencil editor
  or pencil MCP server.
- Do not include AI tool author attribution in commits, PRs, or issues.
