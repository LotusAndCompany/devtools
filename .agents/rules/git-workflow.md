---
paths:
  - "**/*"
---

# Git Workflow Rules

- Use Conventional Commits: `<type>(<scope>): <subject>`.
- Keep subjects lowercase, present tense, and imperative.
- Allowed commit types are `feat`, `fix`, `docs`, `style`, `refactor`, `test`,
  `chore`, `build`, `ci`, and `perf`.
- Branch names should use `<type>/<description>` with lowercase, hyphenated
  descriptions. Allowed prefixes include `feature/`, `fix/`, `doc/`,
  `refactor/`, and `hotfix/`.
- PR titles must follow Conventional Commits and use lowercase subject text.
- Follow `.github/PULL_REQUEST_TEMPLATE.md` for pull request bodies.
- release-please determines versions from Conventional Commits.
- Do not manually edit release-please managed version files or `CHANGELOG.md`
  unless the task is explicitly release-related.
- Do not include AI tool author attribution in commits, PRs, or issues.
