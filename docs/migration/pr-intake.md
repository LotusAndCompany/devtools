# M-1 PR intake snapshot

Snapshot date: 2026-10-26. M0 base: main at
5638bb64c67a4a2329d1e57e36f255eef92fce9e. Local main and origin/main agreed
at the time of audit. Issue #130 is closed, but this expected artifact was
absent from the repository; this file reconstructs the current status from
GitHub and the local base.

No PR was merged or release approved as part of M-1/M0. Do not include open
PR functionality in the M0 baseline.

| PR | State / base status | Changed scope and CI evidence | M0 decision |
|---|---|---|---|
| #113 Prevent direct main/master commits | Open; head dcf0cf442a419d59cd20d47f731a6b2d2946b648; not in base | Adds a pre-commit configuration. Checks: CodeQL, Analyze GitHub Actions, Analyze C++, and one PR-title check succeeded; another title check was skipped. CodeRabbit reports review completed. | Keep open for project review; no workflow change adopted in M0 |
| #120 deploy-pages 5.0.0 to 5.0.1 | Merged 2026-10-13 and in base | Commit 5638bb6; workflow dependency update | Already included |
| #121 Text Diff | Open, non-draft; head ada7bee62ee5e6c51f14e827f71ea2696af030e3; not in base | Adds a stateless, UI-independent diff tool and LCS implementation. PR body claims 8 unit tests. CodeQL, formatting, static analysis, GitHub Actions analysis, PR title, and C++ checks succeeded; CodeRabbit says manual review is required. Large-input complexity remains a review point. | Keep out of baseline; evaluate implementation and large inputs before merge |
| #122 Color Converter | Open draft; head e015a4431140dcd5101873f815344c58ea9cb32b; not in base | Adds color converter core/GUI files. Generic PR description; no checks recorded. | Keep out of baseline; request implementation details and test evidence during review |
| #123 Timestamp Conversion | Open, non-draft; head 6b2769ac5e8cea303325ff2fb6d2f6d2e8054833; not in base | Adds timestamp conversion and tests. PR body claims 21 new tests. Seven CI checks succeeded; CodeRabbit says manual review is required. | Keep out of baseline; review timezone and boundary behavior before merge |
| #127 Release 0.8.1 | Open release PR; head 1080b53ee0accc9f85d5b34cb1797c1053b2d9d8; not in base | Release Please generated. No checks recorded at snapshot. Mentions SQLite file access change in release context. | Keep out of baseline; use normal release approval |

PR descriptions and check results are evidence about those PRs, not proof of
behavior in this base. #121, #122, and #123 features become part of the
baseline only after merge; update the feature inventory and rerun relevant
tests after that change. #127 remains subject to release approval.

## Reproducibility and follow-up

The M0 reference is the full SHA above, not a moving branch name. Record the
new SHA and rerun affected audit entries whenever the base changes. The
repository had no M-1 artifact to preserve, so this snapshot records the
current API view and local base rather than claiming it was created during
the earlier Issue #130 work.
