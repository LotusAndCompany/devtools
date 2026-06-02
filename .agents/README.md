# Shared AI Agent Harness

This directory contains shared, tool-neutral rules for AI coding agents.

`AGENTS.md` remains the repository-wide source of truth. Files in
`.agents/rules/` add path-scoped rules that are useful for tools with a
Claude Code-like rules system.

## Rule Schema

Rules use Markdown with a small front matter block:

```yaml
---
paths:
  - "**/*.cpp"
  - "**/*.h"
---
```

The `paths` list uses glob patterns and describes when the rule should apply.
Tools that cannot activate rules by path should load these files as general
context and still respect the path hints.

## Adapter Policy

- Claude Code: `.claude/rules/*.md` should be thin adapters that import the
  matching shared rule.
- Gemini CLI: `GEMINI.md` imports shared rules directly.
- Codex: `AGENTS.md` points Codex to this directory; `.codex/rules/` remains
  command-permission policy, not coding guidance.
- CodeRabbit: `.coderabbit.yaml` path instructions should mirror these rules.

When changing project guidance, update `AGENTS.md` first if the rule is global.
Use `.agents/rules/` only for path-scoped or domain-specific guidance.

## Sync

Run this after editing `AGENTS.md` or `.agents/rules/*.md`:

```bash
scripts/sync-agent-rules.py
```

The script regenerates:

- `.claude/rules/*.md`
- the generated import block in `GEMINI.md`

Run the full harness validation with:

```bash
scripts/check-agent-harness.sh
```
