# Adding New Tools

Every tool screen is implemented in React and registered in the React tool
registry. C++ feature modules provide reusable processing services; the Qt host
provides native integration through the WebEngine bridge. Do not add a
per-feature Qt Widgets screen.

## React Tool Workflow

### 1. Create the screen

Create `frontend/src/features/{tool}/` with an `index.ts`, screen component, and
feature-local styles or components as needed. Keep screen-specific state in the
feature. Put reusable controls in `frontend/src/shared/` only when multiple
features use them, and use the project's shadcn components for common UI.

### 2. Register the tool

Add a `ToolDefinition` to `frontend/src/app/tool-registry.ts`. The registry
provides its route, translated labels, description, icon, native ID, and screen
component. Set `keepAlive: true` only when leaving the screen must preserve its
mounted state. Tool navigation is managed by the React shell; do not add a
parallel native menu entry.

### 3. Add native operations when needed

For native work, add the input and output to `OperationContracts` in
`frontend/src/shared/bridge/types.ts`. Implement the handler and register it in
`features/web_frontend/gui/bridge/operation_registry.cpp`. Put reusable
processing in `features/{feature}/core/` and list new service sources in that
feature's `CMakeLists.txt`.

Keep OS dialogs, clipboard access, file paths, and Qt WebChannel calls in the
native host and bridge layers. React features use the typed operation runner;
they do not call Qt APIs directly. Update the bridge contract version only when
the wire contract changes.

### 4. Add coverage

Keep React interaction tests beside the feature. Add a C++ service test when
processing behavior changes, and a bridge or Qt WebEngine integration test when
native request handling changes. See the [Testing Guide](testing-guide.md).

### 5. Run the checks

From `frontend/`, run the checks used by CI:

```bash
bun install --frozen-lockfile
bun run lint
bun run format:check
bun run --bun typecheck
bun run --bun test
bun run --bun build
```

Build the native app from the repository root to validate bridge and service
integration:

```bash
cmake --build build --parallel
```

## Adding a C++ Feature Service

Create a `core/` directory for processing logic and a `tests/` directory for
focused service tests. Do not create a `gui/` directory for feature screens.
Services should not depend on Qt Widgets or the WebEngine bridge. Keep platform
dialogs and user-facing state in the native handler.

Register the static target in the root `CMakeLists.txt`:

```cmake
qt_add_library(${PROJECT_NAME}_your_feature STATIC)
```

List its service sources in `features/your_feature/CMakeLists.txt`:

```cmake
target_sources(DevTools_your_feature PRIVATE
    core/your_service.h
    core/your_service.cpp
)
```

Register the module in `features/CMakeLists.txt` and add it to the root
`MODULE_LIST`. Link only the Qt modules and other dependencies the service
requires. Expose the service to React through a typed bridge operation when the
screen needs native processing.
