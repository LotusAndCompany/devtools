#!/bin/bash
set -euo pipefail

: "${PROJECT_DIR:?Xcode must set PROJECT_DIR}"
: "${PROJECT_TEMP_DIR:?Xcode must set PROJECT_TEMP_DIR}"
: "${TARGET_BUILD_DIR:?Xcode must set TARGET_BUILD_DIR}"
: "${FULL_PRODUCT_NAME:?Xcode must set FULL_PRODUCT_NAME}"
: "${DWARF_DSYM_FOLDER_PATH:?Xcode must set DWARF_DSYM_FOLDER_PATH}"
: "${CURRENT_PROJECT_VERSION:?Set the Xcode build number before archiving}"

if [[ ! "$CURRENT_PROJECT_VERSION" =~ ^[0-9]+$ ]]; then
  echo "CURRENT_PROJECT_VERSION must be an integer: $CURRENT_PROJECT_VERSION" >&2
  exit 1
fi

repo_root="$(cd "$PROJECT_DIR/.." && pwd)"
tauri_config="$PROJECT_TEMP_DIR/tauri-xcode-archive.conf.json"
cargo_target_dir="$PROJECT_TEMP_DIR/cargo-target"
app_store_info="$PROJECT_DIR/DevToolsArchive/AppStoreInfo.plist"
export PATH="$HOME/.bun/bin:$HOME/.cargo/bin:/opt/homebrew/bin:$PATH"

command -v bun >/dev/null || { echo "bun is required; install it before archiving" >&2; exit 1; }
command -v cargo >/dev/null || { echo "cargo is required; install Rust before archiving" >&2; exit 1; }

cat > "$tauri_config" <<EOF
{
  "bundle": {
    "targets": ["app"],
    "macOS": {
      "bundleVersion": "$CURRENT_PROJECT_VERSION",
      "infoPlist": "$app_store_info"
    }
  }
}
EOF

(
  cd "$repo_root/frontend"
  CARGO_TARGET_DIR="$cargo_target_dir" CARGO_PROFILE_RELEASE_DEBUG=1 bun run tauri:build -- \
    --target aarch64-apple-darwin \
    --bundles app \
    --no-sign \
    --config "$tauri_config"
)

source_app="$cargo_target_dir/aarch64-apple-darwin/release/bundle/macos/DevTools.app"
if [[ ! -d "$source_app" ]]; then
  echo "Tauri did not produce the expected app bundle: $source_app" >&2
  exit 1
fi

executable="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "$source_app/Contents/Info.plist")"
binary="$source_app/Contents/MacOS/$executable"
if [[ ! -f "$binary" ]]; then
  echo "Tauri app executable is missing: $binary" >&2
  exit 1
fi

dsym="$DWARF_DSYM_FOLDER_PATH/DevTools.app.dSYM"
mkdir -p "$DWARF_DSYM_FOLDER_PATH"
rm -rf "$dsym"
xcrun dsymutil "$binary" --out "$dsym"

binary_uuid="$(xcrun dwarfdump --uuid "$binary" | awk 'NR == 1 { print $2 }')"
dsym_binary="$dsym/Contents/Resources/DWARF/$executable"
dsym_uuid="$(xcrun dwarfdump --uuid "$dsym_binary" | awk 'NR == 1 { print $2 }')"
if [[ -z "$binary_uuid" || "$binary_uuid" != "$dsym_uuid" ]]; then
  echo "dSYM UUID does not match app binary: binary=$binary_uuid dSYM=$dsym_uuid" >&2
  exit 1
fi

/usr/bin/strip -S "$binary"
stripped_uuid="$(xcrun dwarfdump --uuid "$binary" | awk 'NR == 1 { print $2 }')"
if [[ "$binary_uuid" != "$stripped_uuid" ]]; then
  echo "Stripping changed the app UUID: before=$binary_uuid after=$stripped_uuid" >&2
  exit 1
fi

destination="$TARGET_BUILD_DIR/$FULL_PRODUCT_NAME"
rm -rf "$destination"
ditto "$source_app" "$destination"
echo "Tauri app archived with matching dSYM UUID $binary_uuid"
