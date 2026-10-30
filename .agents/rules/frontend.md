---
paths:
  - "frontend/src/**/*"
  - "frontend/vite.config.ts"
  - "frontend/tsconfig*.json"
  - "frontend/components.json"
---

# Frontend Architecture

- Keep application startup, shell composition, navigation, and the tool registry in `src/app/`.
- Keep each screen and its private components, hooks, styles, and tests in `src/features/<feature>/`.
- Put reusable UI, contract types, and Effect helpers in `src/shared/` only when multiple features use them.
- Keep Qt WebChannel and Qt-specific connection code in `src/platform/qt/`.
- Preserve the dependency direction: `app -> features/platform -> shared`; features must not import other features.
- Export feature APIs through its `index.ts`. Avoid importing another feature's implementation files.
- Keep tool label, icon, native ID, component, and lifetime policy in `src/app/tool-registry.ts`.
- Add feature-specific styles under the feature root and scope selectors to that feature's root element.
- Keep bridge operations typed through the shared contract and run asynchronous UI operations through the shared Effect runner.
- Run `bun run lint`, `bun run format:check`, `bun run typecheck`, `bun run build`, and `bun run test` from `frontend/` after frontend changes.
- CMake watches `src/**/*.ts`, `src/**/*.tsx`, and `src/**/*.css` recursively. Keep frontend imports and shadcn output paths rooted at `@/`.
