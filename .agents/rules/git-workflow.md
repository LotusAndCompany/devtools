---
paths:
  - "**/*"
---

# Git Workflow Rules

- Use squash merges for pull requests.
- PR titles must follow Conventional Commits and use lowercase subject text;
  the squash commit title is what release-please reads on `main`.
- Follow `.github/PULL_REQUEST_TEMPLATE.md` for pull request bodies.
- release-please determines versions from the squash-merged PR title.
- Do not manually edit release-please managed version files or `CHANGELOG.md`
  unless the task is explicitly release-related.
- Do not include AI tool author attribution in commits, PRs, or issues.
