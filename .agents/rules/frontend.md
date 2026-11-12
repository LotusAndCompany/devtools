---
paths:
  - "frontend/src/**/*"
  - "frontend/vite.config.ts"
  - "frontend/tsconfig*.json"
  - "frontend/components.json"
---

# Frontend Architecture

- Keep startup, shell composition, navigation, and the tool registry in
  src/app/.
- Keep each screen and its private components, hooks, styles, and tests in
  src/features/<feature>/.
- Put reusable UI, contract types, and Effect helpers in src/shared/ only
  when multiple features use them.
- Use src/platform/tauri/ as the only native transport; do not add browser or
  WKWebView host fallbacks.
- Preserve the dependency direction: app -> features/platform -> shared;
  features must not import other features.
- Export feature APIs through index.ts; avoid cross-feature implementation
  imports.
- Keep tool label, icon, native ID, component, and lifetime policy in
  src/app/tool-registry.ts.
- Add feature styles beside the feature and scope selectors to its root.
- Keep bridge operations typed through the shared contract and use the shared
  Effect runner for asynchronous UI operations.
- Run bun run lint, bun run format:check, bun run --bun typecheck,
  bun run --bun test, and bun run --bun build from frontend/ after
  frontend changes.
- Keep frontend imports and shadcn output paths rooted at @/.
