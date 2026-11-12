---
paths:
  - "frontend/src/**/*"
  - "frontend/vite.config.ts"
  - "frontend/tsconfig*.json"
  - "frontend/components.json"
---

# Frontend Architecture

- Follow docs/development/ui-development.md for shadcn CLI, component
  composition, layout, styling, and feedback patterns.
- Keep startup, shell composition, navigation, and the tool registry in
  src/app/.
- Keep each screen and its private components, hooks, styles, and tests in
  src/features/<feature>/.
- Put reusable UI, contract types, and Effect helpers in src/shared/ only
  when multiple features use them.
- Reuse components from src/shared/components/ui/ and
  src/shared/components/ before adding or recreating primitives.
- Keep UI components on the configured Base UI API; do not assume Radix
  props such as asChild.
- Use src/platform/tauri/ as the only native transport; do not add browser or
  WKWebView host fallbacks.
- Preserve the dependency direction: app -> features/platform -> shared;
  features must not import other features.
- Export feature APIs through index.ts; avoid cross-feature implementation
  imports.
- Keep tool label, icon, native ID, component, and lifetime policy in
  src/app/tool-registry.ts.
- Add feature styles beside the feature, scope them with its data-page value,
  and import the stylesheet from src/shared/styles/index.css.
- Use shared layout classes from src/shared/styles/components.css and
  semantic theme tokens from src/shared/styles/tokens.css. Do not introduce
  feature-only global selectors or raw palette colors.
- Keep bridge operations typed through the shared contract and use the shared
  Effect runner for asynchronous UI operations.
- Run bun run lint, bun run format:check, bun run --bun typecheck,
  bun run --bun test, and bun run --bun build from frontend/ after
  frontend changes.
- Keep frontend imports and shadcn output paths rooted at @/.
