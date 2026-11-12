# Icon Assets

The macOS app icon sources are the icon files referenced by
`src-tauri/tauri.conf.json`. The frontend uses its React assets under
`frontend/src/`.

When changing an app icon, update the source asset, build the Tauri app bundle,
and inspect the generated app bundle and Xcode Archive. Do not edit generated
bundle assets.
For screen layouts, use the Pencil workflow in [Design Files](design-files.md).
