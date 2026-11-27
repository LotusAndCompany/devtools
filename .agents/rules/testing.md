---
paths:
  - "src-tauri/**"
  - "frontend/src/**"
  - "tests/fixtures/**"
---

# Testing Rules

- Rust unit and integration tests live with the Tauri crate in
  src-tauri/src/; run them with
  cargo test --manifest-path src-tauri/Cargo.toml --locked.
- Frontend tests live beside the React feature or bridge code and use Vitest
  with Testing Library. Run them from frontend/ with bun run --bun test.
- Use tests/fixtures/ for shared request, image, database, and migration
  data. Never include real credentials or user data in fixtures.
- Keep bridge contract parity checks between the TypeScript contract and Rust
  operation registry.
- Exercise persistence changes for clean install, legacy migration, invalid
  input, partial failure, retry, and preservation of the source data.
