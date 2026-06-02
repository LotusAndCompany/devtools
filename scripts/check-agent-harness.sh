#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

snapshot_generated_adapters() {
  {
    [ -f GEMINI.md ] && printf '%s\n' GEMINI.md
    [ -d .claude/rules ] && find .claude/rules -type f -name '*.md' -print
  } | sort | while IFS= read -r path; do
    shasum -a 256 "$path"
  done
}

BEFORE_SNAPSHOT="$(mktemp)"
AFTER_SNAPSHOT="$(mktemp)"
trap 'rm -f "$BEFORE_SNAPSHOT" "$AFTER_SNAPSHOT"' EXIT

snapshot_generated_adapters > "$BEFORE_SNAPSHOT"
scripts/sync-agent-rules.py
snapshot_generated_adapters > "$AFTER_SNAPSHOT"

if ! diff -u "$BEFORE_SNAPSHOT" "$AFTER_SNAPSHOT"; then
  echo "Agent harness adapters are out of sync. Run scripts/sync-agent-rules.py." >&2
  exit 1
fi

python3 - <<'PY'
from pathlib import Path
import ast

ast.parse(Path("scripts/sync-agent-rules.py").read_text(encoding="utf-8"))
PY

git diff --check -- \
  AGENTS.md \
  .agents \
  .claude/rules \
  GEMINI.md \
  docs/development/ai-agent-harness.md \
  scripts/sync-agent-rules.py

echo "Agent harness check passed."
