# Installation and Updates

## Supported system

DevTools supports macOS 15.0 or later on Apple Silicon (arm64). Windows, Linux,
and Intel Macs are not in the verified support scope.

## Install

Install DevTools from the
[Mac App Store](https://apps.apple.com/jp/app/devtools/id6736692957?mt=12).

## Update

Install available updates through the Mac App Store. DevTools has no in-app
updater. Updating the app does not delete its Application Support directory;
settings and phrase templates remain there.

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
