# M1 application settings service

## Scope and responsibility

`ApplicationSettingsService` owns shared application preference persistence and
window geometry persistence. `GuiApplication`, `MainWindow`, and
`SettingsDialog` call the service and keep platform/UI behavior such as applying
language translations, changing window flags, showing the sidebar, and resizing
or moving the window.

```mermaid
flowchart LR
    UI[Application Widgets] -->|typed settings values| Service[ApplicationSettingsService]
    Service -->|settings values| UI
    Service --> Storage[Qt Core QSettings]
```

The service depends on Qt Core and has no Qt Widgets dependency. Tests can pass
a temporary `QSettings` instance; production uses the platform-default
`QSettings` backend. Existing key names, defaults, geometry save conditions,
language-change signal timing, and the settings dialog's synchronous `sync()`
are preserved. The service updates individual keys and does not clear unrelated
settings.

## Compatibility and Bridge proposal

The persisted keys remain `language`, `general/showSidemenuOnStartup`,
`general/showLastToolOnStartup`, `general/lastUsedTool`, `window/alwaysOnTop`,
`window/rememberSize`, `window/rememberPosition`, `window/width`,
`window/height`, `window/x`, and `window/y`. The first read initializes the
sidebar preference defaults as before. Window geometry is written only when
the matching remember preference is enabled.

A future Bridge can expose `readApplicationPreferences() -> {language,
showSidemenuOnStartup, showLastToolOnStartup, lastUsedTool, alwaysOnTop,
rememberWindowSize, rememberWindowPosition}`,
`saveApplicationPreferences({…}) -> {success, error}`, and
`saveWindowGeometry({width, height, x, y}) -> {success, error}`. The host UI
still applies the preferences to native windows and translates language
changes. A WebView host should own permission and validation rules before
accepting geometry or preference updates from renderer code.

Rust candidates are settings DTO validation and preference migration logic if
the stored schema later changes. The current `QSettings` backend stays behind
the service until the native settings format and cross-platform migration
requirements are defined.

## Verification and limits

Environment: macOS 26.6.2 arm64, Homebrew Qt 6.9.2, existing `build` directory
with `ENABLE_UNIT_TEST=ON`.

- `cmake --build build --target DevTools test_application_settings_service
  test_settings_dialog`: passed.
- `QT_QPA_PLATFORM=offscreen ctest --test-dir build -R
  '^test_(application_settings_service|settings_dialog)$
  --output-on-failure`: 2/2 passed. Core coverage includes defaults, exact
  keys, unrelated-key preservation, persistence reload, and conditional
  geometry writes. The UI regression covers dialog defaults, applying changed
  values, persisted values, and the language-changed signal.
- `otool -L build/test_application_settings_service`: Qt Core and Test; no Qt
  Gui or Widgets.
- `cmake --build build --target format-check` and `git diff --check`: passed.
- No manual visual review or other-OS settings-backend check was performed.

Shared application settings now flow through the service. Regex-tool settings
remain in its Widget and are the next persistence extraction; aggregate
architecture and class/dependency inventory work remains for Issue #132.

## Execution record

At 2026-10-27 18:37 JST, the Issue #132 goal runtime was 4,001 seconds. Runtime
includes work and waits across the task, not model execution time. Human
intervention and per-command Codex execution time were not independently
measured. No PR or issue state change was made.
