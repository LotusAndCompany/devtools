# Persisted data and migration inventory

This inventory describes the M0 base at
5638bb64c67a4a2329d1e57e36f255eef92fce9e. It distinguishes application
settings from user-owned files and external databases. The native
QSettings domain for a clean supported install and the data recovery path
have not been established.

## Application identity and settings

The app sets its QCoreApplication name to DevTools and its bundle identifier
to com.lotusandcompanyinc.dev-tools. No explicit organization name, domain,
QSettings path, or settings format is configured in source. QSettings uses
the platform default backend; do not hard-code a presumed plist location into
a migration until the domain mapping has been verified on a clean supported
macOS account.

The settings keys found in source are:

| Key | Value / behavior |
|---|---|
| language | Selected UI language |
| general/showSidemenuOnStartup | Whether the side menu opens at startup |
| general/showLastToolOnStartup | Whether to restore the previous tool |
| general/lastUsedTool | Last selected tool ID |
| window/alwaysOnTop | Window always-on-top preference |
| window/rememberSize, window/width, window/height | Remembered window dimensions |
| window/rememberPosition, window/x, window/y | Remembered window position |
| RegexTester/pattern | Last regex pattern |
| RegexTester/testText | Last test input |
| RegexTester/replacePattern | Last replacement text |
| RegexTester/flagG, flagI, flagM, flagS, flagU, flagX | Last regex flag values |
| db_tool/connectionHistory | Serialized database connection history |

Settings are read/written through QSettings. The code does not use Keychain
APIs or an explicit encryption layer for these values. The listed regex
pattern and test text may themselves contain user-sensitive content.

## Database connection history

db_tool/connectionHistory stores a JSON list. Each entry can include:

- type: database driver/type
- host
- database
- username
- displayName
- securityScopedBookmark: base64-encoded macOS security-scoped bookmark for
  sandboxed SQLite file access, where applicable

At most 10 history entries are retained. Passwords are not stored; reconnects
to non-SQLite databases prompt for a password. Connection history is not a
credential vault. A future bridge or Tauri implementation must preserve the
bookmark/security-scope lifecycle on macOS and must not silently migrate
credentials into plain settings.

## Phrase templates

Reusable templates are read from content/*.txt relative to the process
working directory. The first line is the title; the rest is the template.
There is no explicit QStandardPaths-based app-data directory, recovery
mechanism, or versioned migration for this content. Changing the working
directory or packaging convention can make existing templates disappear.
Before changing storage, identify which files are bundled defaults and which
are user-edited data.

## User-selected and external data

| Data | Current interaction / ownership |
|---|---|
| Images | User-selected input and output files; image workflows can edit or export a result |
| JSON/YAML/TOML | User-selected input and output files in data conversion |
| Markdown | User-selected .md open/save files and HTML export |
| SQLite | External database file selected by the user; may require a macOS security-scoped bookmark |
| MySQL/PostgreSQL | External service connections; connection metadata may be remembered, password is prompted |
| HTTP request/response | Network input/output in the request tool; review secret-bearing headers and request history behavior before porting |
| Phrase templates | Working-directory-relative content/*.txt files |
| QR output | User-chosen PNG path |
| Window and tool settings | QSettings platform backend |

The M0 audit did not find a general app-owned document database, explicit
recovery store, or a global recent-file history.

## Migration requirements

For any storage move:

1. Record the old key/file format, type, default, app identity, and supported
   source version.
2. Preserve values exactly where practical, including Japanese text, paths,
   regex flags, window state, selected tool, and connection metadata.
3. Treat SQLite bookmarks as opaque OS capabilities. Validate resolution,
   stale-bookmark handling, and permission refresh on the supported macOS
   versions.
4. Keep password handling separate from ordinary preferences; do not claim a
   secret has migrated unless an explicitly approved secure store is tested.
5. Make migration idempotent, report partial failure, preserve source data
   until successful verification, and document rollback/recovery.
6. Test a clean install, upgrade from the M0 settings and files, restart,
   cancellation or failure, and uninstall behavior.

## Open questions

- What native QSettings domain should a migration target on a clean supported
  install, and how should existing profile settings be found and recovered?
- Are phrase templates modified by users, and where are their installed and
  writable copies located?
- How long should database history persist, and should users be able to clear
  individual entries?
- Does any request or response content persist outside the explicit settings
  keys above?
- What recovery behavior should be provided if settings or a bookmark cannot
  be migrated?
