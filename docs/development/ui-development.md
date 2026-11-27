# UI Development

This guide describes the current React UI patterns for DevTools. Treat the
project's shadcn configuration, source components, and shared styles as the
implementation source of truth if this guide and the code differ.

## Stack and shadcn CLI

The frontend uses React with shadcn/ui source components built on Base UI,
Tailwind CSS v4, and Lucide icons. The shadcn configuration is in
[components.json](../../frontend/components.json); dependencies and the CLI
version are controlled by the frontend package manifest and Bun lockfile.

Run the project-local CLI from the frontend directory so it uses the version
in the Bun lockfile. Check the resolved config and installed components
before adding anything:

~~~sh
cd frontend
bunx --bun shadcn info --json
~~~

The current config resolves the base-nova style, Base UI primitives, the
`@/shared/components/ui` output path, and `@/` import aliases. Always check
the CLI output and components.json for current values rather than assuming
defaults.

Search for an existing component, then use the CLI to locate the official
documentation for its API before adding a new primitive:

~~~sh
bunx --bun shadcn search @shadcn -q "combobox"
bunx --bun shadcn docs button dialog combobox
bunx --bun shadcn add combobox
~~~

The docs command returns official component documentation links. Read the
relevant API and examples before using a component pattern that is unfamiliar
in this Base UI project.

Use the CLI only for components that are not already present. These
components are source files owned by this repository; inspect every added
file and keep its imports aligned with the configured aliases and Base UI
API. For upstream updates, preview the change first:

~~~sh
bunx --bun shadcn add button --dry-run
bunx --bun shadcn add button --diff button.tsx
~~~

Do not run `init`, apply a preset, or add every component for routine UI work.
Do not use `--overwrite` unless explicitly requested. Initialization or
preset application can replace the project's theme; overwriting can replace
local component changes. Preserve `bun.lock` when changing dependencies.

Use Base UI APIs from this project. For custom triggers, follow the existing
component's `render` pattern; do not copy Radix examples that use `asChild`
without verifying that the configured component supports it.

## Component and screen placement

- Screen components, hooks, and screen-only controls belong in
  `frontend/src/features/feature-name/`.
- Reuse primitives from `frontend/src/shared/components/ui/` and app-specific
  components from `frontend/src/shared/components/`.
- Keep reusable UI in `shared/` when multiple features use it. Do not import
  another feature's private implementation.
- Export the feature's public screen through its `index.ts`.
- Compose existing components before writing custom markup. Use the Card
  header/title/content structure, shared form fields, and existing Button
  variants where they fit.
- For forms, follow the existing FormField and Field/FieldGroup patterns
  instead of hand-building label and control wrappers.
- Keep controls accessible: label inputs, provide dialog titles, preserve
  keyboard operation, and mark decorative icons as hidden from assistive
  technology.
- Use Lucide icons. Set data-icon to inline-start or inline-end for icons
  inside Buttons, and hide decorative icons from assistive technology.

ToolStage wraps each screen in a section with data-page set to the tool ID.
Most screens use a top-level tool-page element. Use that page ID to scope
feature styles.

## Design references

When a task includes a Pencil screen reference, follow the
[design file workflow](design-files.md). Use the Pencil editor or its MCP
server to inspect or edit .pen files; do not read them as text. Treat the
design as a visual reference and build the React screen with the shared UI
components described above.

## Layout and styles

Global styles are loaded from `frontend/src/shared/styles/index.css`. It
imports Tailwind, theme tokens, common layout styles, and each feature's
stylesheet.

- Use shared layout classes from
  `frontend/src/shared/styles/components.css` before creating another global
  pattern. Common classes include `tool-page`, `input-card`, `result-card`,
  `toolbar-row`, `button-row`, `action-row`, `form-grid`, and `two-column`.
- Put app-wide theme variables and design tokens in
  `frontend/src/shared/styles/tokens.css`. It defines the Tailwind v4 theme
  mapping, light/dark/system palettes, semantic colors, spacing, typography,
  radii, and shadows.
- Put screen-specific layout and visual rules in
  `frontend/src/features/feature-name/styles.css`. Scope selectors to the page
  root with `:where([data-page="tool-id"])`.
- Add an import for a new feature stylesheet to
  `frontend/src/shared/styles/index.css`; creating the file alone does not
  load it.
- Use Tailwind utilities for simple local composition and semantic colors
  such as `bg-background`, `text-foreground`, `text-muted-foreground`,
  `border-border`, and `bg-primary`. In CSS, use matching variables such as
  `var(--background)` and `var(--muted-foreground)`.
- Use the spacing, type, radius, and shadow tokens for new CSS. Do not add
  raw palette colors or broad selectors to a feature stylesheet.
- Keep responsive rules beside the feature rules they change. Check the
  narrow layout as well as the normal desktop layout.

Example feature selector:

~~~css
:where([data-page="example"]) .example-layout {
    display: grid;
    grid-template-columns: minmax(0, 1fr) minmax(0, 1fr);
    gap: var(--layout-gap);
}
~~~

## Feedback

The global right-bottom toaster is mounted once by
`frontend/src/app/App.tsx`. Use the shared manager from
`frontend/src/shared/components/ui/toast.tsx`:

Call it only inside the successful-result branch, such as when
`result?.copied` is true or a save operation returns a result:

~~~tsx
import { toast } from "@/shared/components/ui/toast";
import { text } from "@/shared/tool-page";

if (result?.copied) {
    toast.add({
        title: text(locale, "Copied to clipboard.", "クリップボードにコピーしました。"),
        type: "success",
    });
}
~~~

Use a toast for a successful action whose result does not remain visible,
such as copying or saving. Show it only after the operation returns a
successful result. Do not show it when a file dialog is cancelled or when
the same result is already visible in the screen.

Show operation errors inline with `OutputMessage` or `FeedbackAlert`, using
`useOperation` for normal asynchronous feature operations. Inline errors stay
visible until dismissed or the next operation begins. Localize notification
text and accessible labels in both English and Japanese.

## Verification

For frontend changes, run the checks listed in the
[frontend rule](../../.agents/rules/frontend.md) from the frontend directory.
For changes to layout or appearance, also launch:

~~~sh
bun run tauri:dev
~~~

Inspect the affected screen in light and dark themes and at a narrower window
width. Capture screenshots when the task asks for visual evidence.
