# Coding Standards

## Rust

- Keep Rust code formatted with `cargo fmt` and warning-free under Clippy.
- Use explicit input validation and structured errors at Tauri command
  boundaries.
- Avoid unnecessary `unsafe`; document the safety invariant if it is needed.
- Keep persistence and filesystem changes recoverable, and preserve source data
  during imports.
- Lock dependencies in `src-tauri/Cargo.lock`.

## React and TypeScript

- Use the shared bridge contract for native operations.
- Keep feature-specific code inside its feature folder; share UI only when
  multiple features use it.
- Use the project's shared components and localization helper.
- Keep visible strings available in English and Japanese.
- Lock dependencies in `frontend/bun.lock`.

## Documentation and generated files

- Keep current documentation consistent with macOS 15+ on Apple Silicon.
- Update existing English/Japanese document pairs together.
- Do not edit Cargo, Bun, Tauri, Vite, or other generated outputs directly.
- Use the Pencil editor for `.pen` files.
