# Installation and Updates

## Supported system

DevTools supports macOS 15.0 or later on Apple Silicon (arm64). Windows, Linux,
and Intel Macs are not in the verified support scope.

## Install

1. Download the DMG attached to the latest published GitHub release.
2. Open the DMG and drag DevTools to Applications.
3. Eject the mounted image and launch DevTools from Applications.

Public release DMGs are signed with the organization's Apple Developer ID and
notarized before upload. A release without those credentials must not be
published as a supported installer.

## Update

Download the newer signed DMG, quit DevTools, and replace the existing app in
Applications. Do not delete the app's Application Support directory; settings
and phrase templates live there. The app does not use an automatic updater.

Before upgrading from the Qt version, make a backup of:

- the Qt settings file at ~/Library/Preferences/com.trolltech.unknown-organization.DevTools.plist
- any user-edited phrase templates in the old content directory

On first launch, Tauri imports allowlisted preferences and database connection
metadata. It retains the original settings in
~/Library/Application Support/com.lotusandcompanyinc.dev-tools/migration/qsettings-source.plist.
Database passwords are not migrated or stored; enter them again when connecting.
SQLite security-scoped bookmarks are not migrated, so select the database file
again. The migration keeps the source plist until users remove it themselves.

## Backup and recovery

Quit the app and copy
~/Library/Application Support/com.lotusandcompanyinc.dev-tools to a safe location
before an upgrade. To recover, quit DevTools and restore that directory from the
backup. If migration reports an error, do not remove the source plist or backup;
restore the backup and report the failure with logs that contain no settings data.

## Uninstall

Quit DevTools and move the app from Applications to Trash. To remove app settings
and templates as well, separately remove
~/Library/Application Support/com.lotusandcompanyinc.dev-tools after taking any
needed backup. User-selected files and external databases are not removed by
uninstalling DevTools.
