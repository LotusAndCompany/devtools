# Adding a Tool

## Add a frontend-only tool

1. Add the screen and feature-local components under `frontend/src/features/`.
2. Register its title, icon, component, and tool ID in the app tool registry.
3. Add English and Japanese UI strings with the shared localization helper.
4. Add focused component or utility tests and update the user guide.

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
