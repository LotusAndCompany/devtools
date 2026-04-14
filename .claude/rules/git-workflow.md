# Git Workflow Rules

## Author Attribution

IMPORTANT: Do not include Claude Code author information in commits, PRs, or Issues.
- Do not add `Co-Authored-By` headers
- Do not add "Generated with Claude Code" footers
- Do not mention Claude Code in PR body

## Conventional Commits

Commit messages follow `<type>(<scope>): <subject>` format.
- type: feat, fix, docs, style, refactor, test, chore, build, ci, perf
- Subject starts lowercase, use present tense imperative mood
- Validated automatically by commit-msg hook

## Branch Naming

`<type>/<description>` format (lowercase, hyphen-separated):
- feature/, fix/, doc/, refactor/, hotfix/
- With issue number: `feature/123-description`

## PR

- Title must follow Conventional Commits format (validated by semantic-pr CI)
- Follow `.github/PULL_REQUEST_TEMPLATE.md`
