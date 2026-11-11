# Feature inventory and parity ledger

This inventory covers the nine tools constructed by the M0 application shell.
The page and registration names are derived from the main window's side-menu
IDs and ContentsArea switch. It also includes shared shell behavior. Features
in unmerged PRs are listed separately and are not part of the M0 application.

Progress values follow [README.md](README.md): verified, verified with
documented limits, not started, in progress, implemented / unverified, blocked,
or out of scope (with a reason).
At M0, only the inventory itself is verified. Later milestone columns track
work and evidence per feature.

| Tool / current scope | Main operations and observable behavior | Existing tests | Known parity cases to capture | M0 | M1–M6 |
|---|---|---|---|---|---|
| Image tools | Load and view images; resize, rotate, flip, split, combine/overlay, edit transparency; save results | Image I/O, basic view/control, edit interface, color sampling and transparency view tests | Unsupported/corrupt image, large dimensions, alpha/color preservation, orientation, save permissions, cancel, Unicode paths | Verified | Rust owns image state and six operations. `tauri-image-transforms.json` compares Qt/Rust rotations, flips, nearest resize, transparency modes, and split pixels; macOS UI opened, resized, reset, and saved an image. Split/save dialogs, Finder drop, and large-image behavior remain unverified end to end. |
| Phrase generation | Load reusable templates from content/*.txt; first line is title and remaining lines are template content | No feature UI test target found | Missing/empty template directory, malformed or duplicate titles, Unicode, working-directory changes, read-only files | Verified | Implemented / unverified; non-destructive CWD-to-app-data migration and filename checks have Rust tests. The preview profile had no templates for UI verification. |
| Command generation | Generate Git and Docker command text for copying; the tool does not execute commands | Command generator test | Empty/invalid input, quoting, shell metacharacters, Unicode, platform assumptions | Verified | Shared Qt/Rust fixtures pass for Git, Docker, Compose, quoting, and invalid input (`tauri-command.json`). Commands remain text-only. |
| HTTP request | Configure URL, method, headers/body, send request, display response | No API UI test target found | Invalid URL, DNS/TLS/timeout, non-2xx, large response, cancellation, malformed encoding, secrets in headers, close during request | Verified | Implemented / unverified; local-server tests cover query encoding, status, and cancellation; Tauri UI received external HTTP 200. Proxy/TLS/redirect remain unverified. |
| Data conversion | Parse JSON, YAML, TOML; choose output format/indentation; show errors; load/save user files | Basic parser/emitter, JSON/YAML/TOML parser/emitter, conversion tests | Empty/malformed input, ambiguous format, large/nested values, Unicode, indentation, scalar/array/table types, file permission and cancellation | Verified | Shared Qt/Rust `tauri-convert-data.json` covers numeric precision, date/tag input, Unicode, block/flow output, TOML null warnings, and indentation. Error-position text and large inputs remain unverified. |
| DB tool | Connect to SQLite/MySQL/PostgreSQL when Qt SQL drivers are available; keep connection history; query tabs and browse tables; open/drop SQLite files | DB connection-window tests | Missing driver, invalid credentials, closed DB, malformed SQL, large result set, query cancellation, busy/locked DB, bookmark expiry, disconnect/close lifecycle | Verified | Implemented / unverified; SQLx SQLite fixture and real Tauri UI query pass; Qt history names normalize to `sqlite/mysql/postgres`. MySQL/PostgreSQL tests skipped without configured services. |
| QR code generation | Generate QR codes for text, URL, email, phone, SMS, location, and Wi-Fi data; save PNG | No QR UI test target found; bundled qrcodegen is vendor code | Empty input, invalid data, Unicode, capacity/size limits, error correction, save failures | Verified | Shared Qt/Rust `tauri-qr-content.json` compares all nine payload types. The Tauri UI saved a 200×200 PNG through the native picker and verified the output. |
| Markdown preview | Edit and preview Markdown; open/save .md; export HTML | Markdown parser/file-I/O tests | Empty/malformed input, large document, Unicode, paths/permissions, HTML escaping, local/resource links, save/export failure | Verified | Implemented / unverified; shared fixture verifies Tauri strips scripts, event handlers, and `javascript:` links. Qt retains the historical unsafe href; the React preview sandbox/CSP blocks scripts. The Tauri UI saved and verified a Markdown file; HTML export UI remains unverified. |
| Regex tester | Enter a pattern, test text and replacement, configure flags, display matches and copy results | Regex processing tests | Invalid pattern, empty pattern/input, Unicode/code units, all flags, zero-width match, large input, replacement edge cases | Verified | Shared Qt/Rust `tauri-regex.json` covers UTF-16 offsets, captures, replacement, flags, lookbehind, and invalid syntax. Large-input performance and operation cancellation races remain unverified. |

## Shared application behavior

The shell also owns startup tool selection, side-menu search and visibility,
About and Settings dialogs, light/dark system theme, English/Japanese
translations, always-on-top, remembered window size/position, Ctrl+W, SQLite
file drop, image drop/viewing, and app lifecycle. The DB tool is cached across
tool changes; other content widgets are recreated. These behaviors belong in
the parity ledger even if a new UI architecture implements them differently.

## Test coverage inventory

The currently registered feature test sources are under
features/{feature}/tests. Existing coverage is concentrated in framework,
image, conversion, regex, command, Markdown, and DB connection code. The
absence of a test target for an operation is not evidence that the behavior
does not exist. No React, bridge, end-to-end, OS integration, or distribution
tests exist at M0.

Manual UI smoke opened all nine registered tool screens in an ad-hoc-signed
temporary app copy; the DB connection dialog was opened without connecting to
a database. A small JSON conversion produced formatted output in both the copy
and the ordinary-profile app. The 1 MiB JSON UI run stalled before producing
an accessible result. These observations are limited smoke evidence, not
automated coverage or full feature parity.

For each feature, capture representative success output and error output
before extracting its logic. Add empty, malformed, boundary, large, Unicode,
Japanese-path, permission, cancellation, and shutdown cases where applicable.
Network and database operations also need timeout, driver, connection-loss,
and resource-lifecycle cases. Record cases as unsupported only when current
behavior proves they are unsupported.

## Features outside the M0 base

| Work | M0 status | Intake / current branch |
|---|---|---|
| Text Diff (PR #121) | Open, not in M0 base | Present in PR #145 base and moved to Rust in M5; `tauri-compare-text.json` is shared with Qt |
| Color Converter (PR #122) | Open draft, not in M0 base | Still unmerged and absent from PR #145 base; user confirmed it remains outside M5, so it is tracked as unimplemented and excluded from the 31 current operations |
| Timestamp Conversion (PR #123) | Open, not in M0 base | Present in PR #145 base and moved to Rust in M5; `tauri-timestamp.json` is shared with Qt |
| Release 0.8.1 (PR #127) | Open release PR, not in base | No feature scope credited; follow release approval |
| deploy-pages action update (PR #120) | Merged and in base | Dependency/workflow change only |

## Progress table for migration work

Use one row per feature above and update the M1–M6 cell only when code and
verification evidence exist. The M0 inventory remains historical; M5 additions
from PRs #121 and #123 are listed separately above. Recommended evidence
includes fixture names, test target/command, supported OS, and any limitation
or follow-up issue.
“Implemented / unverified” is not equivalent to complete. Keep shared UI,
translations, settings, window lifecycle, file permissions, and distribution
in this ledger rather than treating them as implied by tool parity.
