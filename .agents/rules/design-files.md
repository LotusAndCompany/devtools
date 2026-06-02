---
paths:
  - "designs/screens/**/*.pen"
  - "docs/development/design-files.md"
---

# Design File Rules

UI design files live under `designs/screens/`, one file per screen or dialog
group. Common chrome such as the sidebar and header is intentionally excluded
from individual screen files.

- Edit `.pen` files via the Pencil editor or the `pencil` MCP server only.
- Never read, grep, or edit `.pen` files directly; they are encrypted.
- One screen per PR keeps diffs reviewable and minimizes auto-merge risk.
- There is no master or aggregate design file. Multi-screen changes should
  touch each file separately.
- `.pen` files are tracked as plain text, but encrypted payloads can corrupt
  under text auto-merge. Keep edits localized and coordinate conflicts.
- See `docs/development/design-files.md` for the full guide.
