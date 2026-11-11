# AI Agent Harness

This repository shares project instructions across AI tools while keeping
tool-specific settings in their native configuration files.

## Sources and generated adapters

- `AGENTS.md` contains repository-wide instructions and the shared rule index.
- `.agents/rules/*.md` contains path-scoped rules.
- `scripts/sync-agent-rules.py` creates Claude Code rule adapters under
  `.claude/rules/`.
- `.codex/rules/project.rules` controls Codex shell commands.
- `.coderabbit.yaml` configures pull request reviews.
- `opencode.json` loads the shared rules for OpenCode.

When changing `AGENTS.md` or a shared rule, run:

~~~sh
scripts/sync-agent-rules.py
~~~

Run `scripts/check-agent-harness.sh` to check the generated adapters before
submitting changes to the agent harness.

## Shared rule index

| Rule | Purpose |
|---|---|
| `project.md` | Tauri architecture, build, dependencies, and CI |
| `frontend.md` | React and Tauri bridge boundaries |
| `testing.md` | Rust and frontend tests |
| `i18n.md` | English and Japanese UI copy |
| `docs.md` | Current documentation and historical migration reports |
| `design-files.md` | Pencil .pen file workflow |
| `generated-files.md` | Generated outputs and vendor files |
| `git-workflow.md` | Pull requests and release-please |

The shared rules are the source of truth. Keep tool-specific configuration
limited to command permissions, review settings, and other tool-specific
behavior.
