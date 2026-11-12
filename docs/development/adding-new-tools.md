# Adding a Tool

## Add a frontend-only tool

Read the [UI development guide](ui-development.md) before implementing the
screen. It describes the shared primitives, layout conventions, feature CSS,
and shadcn CLI workflow.

1. Add the screen, feature-local components, and styles under
   `frontend/src/features/` in a feature-specific directory.
2. Reuse shared UI from `frontend/src/shared/components/` and register the
   tool's title, icon, component, and ID in the app tool registry.
3. If adding a missing shadcn component, use the project CLI workflow in the
   UI development guide and review its source output.
4. Register feature CSS in `frontend/src/shared/styles/index.css`, scope it to
   the tool's data-page value, and use the shared theme tokens.
5. Add English and Japanese UI strings with the shared localization helper.
   Add focused tests and update the user guide when behavior changes.

## Add a native operation

1. Define its input, result, and operation name in the shared TypeScript bridge
   contract.
2. Implement and register the validated Tauri command under `src-tauri/src/`.
3. Update the operation contract parity check and add Rust tests with fixtures
   for success and error cases.
4. Call the shared typed operation from the feature; do not add another native
   transport.

## Verify

Run the Rust and frontend checks in [BUILD.md](../../BUILD.md). For changes to
persisted data or migration, also verify recovery and that original data is
preserved.
