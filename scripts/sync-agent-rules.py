#!/usr/bin/env python3
"""Synchronize shared AI agent rules into tool-specific adapters."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re


REPO_ROOT = Path(__file__).resolve().parents[1]
SHARED_RULE_DIR = REPO_ROOT / ".agents" / "rules"
CLAUDE_RULE_DIR = REPO_ROOT / ".claude" / "rules"
GEMINI_FILE = REPO_ROOT / "GEMINI.md"

BEGIN_GEMINI_IMPORTS = "<!-- BEGIN GENERATED AGENT RULE IMPORTS -->"
END_GEMINI_IMPORTS = "<!-- END GENERATED AGENT RULE IMPORTS -->"


@dataclass(frozen=True)
class SharedRule:
    slug: str
    source_path: Path
    paths: list[str]
    body: str
    title: str


def parse_rule(path: Path) -> SharedRule:
    text = path.read_text(encoding="utf-8")
    if not text.startswith("---\n"):
        raise ValueError(f"{path} is missing front matter")

    front_matter, body = text[4:].split("\n---\n", 1)
    paths = parse_paths(front_matter, path)
    title = parse_title(body, path)
    return SharedRule(
        slug=path.stem,
        source_path=path,
        paths=paths,
        body=body.strip() + "\n",
        title=title,
    )


def parse_paths(front_matter: str, path: Path) -> list[str]:
    paths: list[str] = []
    in_paths = False
    for line in front_matter.splitlines():
        if line.strip() == "paths:":
            in_paths = True
            continue
        if in_paths and line.startswith("  - "):
            paths.append(line.removeprefix("  - ").strip().strip('"'))
            continue
        if in_paths and line and not line.startswith(" "):
            break

    if not paths:
        raise ValueError(f"{path} has no paths in front matter")
    return paths


def parse_title(body: str, path: Path) -> str:
    for line in body.splitlines():
        if line.startswith("# "):
            return line.removeprefix("# ").strip()
    raise ValueError(f"{path} has no H1 title")


def yaml_list(items: list[str]) -> str:
    return "\n".join(f'  - "{item}"' for item in items)


def sync_claude(rule: SharedRule) -> None:
    rel_source = f"../../.agents/rules/{rule.source_path.name}"
    content = (
        "---\n"
        "paths:\n"
        f"{yaml_list(rule.paths)}\n"
        "---\n\n"
        f"# {rule.title}\n\n"
        f"@{rel_source}\n"
    )
    write_file(CLAUDE_RULE_DIR / f"{rule.slug}.md", content)


def sync_gemini(rules: list[SharedRule]) -> None:
    imports = "\n".join(f"@.agents/rules/{rule.source_path.name}" for rule in rules)
    generated_block = (
        f"{BEGIN_GEMINI_IMPORTS}\n"
        f"{imports}\n"
        f"{END_GEMINI_IMPORTS}"
    )

    text = GEMINI_FILE.read_text(encoding="utf-8")
    pattern = re.compile(
        rf"{re.escape(BEGIN_GEMINI_IMPORTS)}.*?{re.escape(END_GEMINI_IMPORTS)}",
        re.DOTALL,
    )
    if pattern.search(text):
        text = pattern.sub(generated_block, text)
    else:
        heading = "## Shared Path-Scoped Rules"
        if heading not in text:
            raise ValueError(f"{GEMINI_FILE} is missing the shared rules section")
        text = text.replace(heading, f"{heading}\n\n{generated_block}", 1)

    write_file(GEMINI_FILE, text)


def write_file(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def main() -> None:
    rules = sorted(
        (parse_rule(path) for path in SHARED_RULE_DIR.glob("*.md")),
        key=lambda rule: rule.slug,
    )
    if not rules:
        raise SystemExit("No shared agent rules found")

    for rule in rules:
        sync_claude(rule)
    sync_gemini(rules)

    print(f"Synchronized {len(rules)} shared agent rules")


if __name__ == "__main__":
    main()
