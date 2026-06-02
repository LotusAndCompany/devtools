# Execution Plans

Execution plans are checked-in working notes for complex agent-driven tasks.
They make intent, decisions, validation, and follow-up work visible to both
humans and agents.

Use an execution plan when a task spans multiple modules, requires multiple
turns, changes architecture, or needs decisions that should survive context
compaction.

## Layout

- `active/`: plans for work currently in progress
- `completed/`: plans retained for historical context
- `tech-debt-tracker.md`: known follow-up work and stale harness/docs items

## Plan Shape

Each plan should include:

- objective
- scope and non-goals
- relevant rules/docs to consult
- implementation steps
- validation commands
- decision log
- completion notes

Keep plans concise. They should guide work, not replace the code or the
canonical documentation.
