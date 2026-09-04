# Icon Assets

DevTools uses the Qt 6.9 named-glyph icon support with the Material Symbols
Outlined application font. The standard icon helper is
`IconUtils::themedIcon()`, which wraps `QIcon::fromTheme()` with a standard Qt
fallback. Icons are rendered using the active application palette.

## Material Symbols Outlined

| Field | Value |
|-------|-------|
| Local file | `res/fonts/MaterialSymbolsOutlined.ttf` |
| Attribution | Material Symbols by Google |
| Upstream repository | <https://github.com/google/material-design-icons> |
| Upstream file | `variablefont/MaterialSymbolsOutlined[FILL,GRAD,opsz,wght].ttf` |
| Pinned commit | `84ccef280841abfac506afc4ad4a2782f6d0a1d0` |
| Commit date | 2026-08-28 |
| Retrieval date | 2026-09-01 |
| File size | 10,648,512 bytes |
| SHA-256 | `57911dca9ff5d28c1fd381101e8540e80ff4fd8d56221face030120802d155c9` |
| License | Apache License 2.0 |
| License source | <https://github.com/google/material-design-icons/blob/84ccef280841abfac506afc4ad4a2782f6d0a1d0/LICENSE> |
| Modification | Font bytes are unmodified; only the local filename is normalized |
| NOTICE | No separate `NOTICE` file is included at the pinned commit |

The font is loaded with `QFontDatabase::addApplicationFont()` at application
startup. Qt 6.9 or later is required because earlier Qt versions do not
provide the named-glyph `QIcon` engine.

## Glyph Mapping

The following table lists the primary glyph names used by the application UI.
It is not an exhaustive list of glyphs used in examples or tests.

| Use | Material Symbols glyph |
|-----|------------------------|
| HTTP request tool | `lan` |
| Image tool | `image` |
| Phrase generation | `library_books` |
| Command tool | `terminal` |
| Data conversion | `transform` |
| Database tool | `database` |
| QR code tool | `qr_code` |
| Markdown preview | `article` |
| Open file | `file_open` |
| Save | `save` |
| Copy | `content_copy` |
| Paste | `content_paste` |
| Close | `close` |
| Add | `add` |
| Delete | `delete` |
| Refresh | `refresh` |
| Rotate counter-clockwise | `rotate_left` |
| Rotate clockwise | `rotate_right` |
| Flip horizontally | `swap_horiz` |
| Flip vertically | `swap_vert` |
| Zoom in | `zoom_in` |
| Zoom out | `zoom_out` |
| Open side panel | `left_panel_open` |
| Close side panel | `left_panel_close` |
| Always-on-top control | `flip_to_front` |

When adding an icon, confirm the name with `QIcon::hasThemeIcon()` and do not
add a light/dark SVG or update an icon `.qrc` file. `IconUtils::themedIcon()`
provides a standard Qt icon when the font or glyph cannot be resolved.

## Former Remix Icon Assets

The former Remix Icon SVGs were introduced in the initial DevTools commit
(`3a1a3d63bc8ec4d074aa2f13fd9c58fe1d34022f`, 2026-01-21). Their geometry
matches the v4.8.0 line assets released on 2025-12-29:

Source repository: <https://github.com/Remix-Design/RemixIcon/tree/v4.8.0>

License source: <https://github.com/Remix-Design/RemixIcon/blob/v4.8.0/License>

| Former local file | v4.8.0 source file |
|------------------|-------------------|
| `remix/save.svg` | `icons/Device/save-line.svg` |
| `remix/file.svg` | `icons/Document/file-3-line.svg` |
| `remix/anticlockwise.svg` | `icons/Design/anticlockwise-line.svg` |
| `remix/clockwise.svg` | `icons/Design/clockwise-line.svg` |
| `remix/flip_horizontal.svg` | `icons/Design/flip-horizontal-line.svg` |
| `remix/flip_vertical.svg` | `icons/Design/flip-vertical-line.svg` |

The v4.8.0 assets were distributed under Apache License 2.0. All of these
assets are removed by the Material Symbols migration, so no Remix Icon asset
is distributed by the application. The current Remix Icon License v1.0,
introduced in January 2026, does not apply to the migrated application.
