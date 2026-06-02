# DevTools - Gemini CLI Guidelines

@AGENTS.md

## Shared Path-Scoped Rules

Load these shared rule files as additional project context. The `paths` front
matter in each file describes when the rule applies.

<!-- BEGIN GENERATED AGENT RULE IMPORTS -->
@.agents/rules/cmake.md
@.agents/rules/cpp-style.md
@.agents/rules/design-files.md
@.agents/rules/docs.md
@.agents/rules/exec-plans.md
@.agents/rules/generated-files.md
@.agents/rules/git-workflow.md
@.agents/rules/i18n.md
@.agents/rules/project.md
@.agents/rules/testing.md
<!-- END GENERATED AGENT RULE IMPORTS -->

## Author Attribution

IMPORTANT: Do not include Gemini CLI author information in commits, PRs, or Issues.
- Do not add `Co-Authored-By` headers
- Do not add "Generated with Gemini" footers
- Do not mention Gemini CLI in PR body
