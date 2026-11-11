# Xcode Archive for TestFlight

Open `DevToolsArchive.xcodeproj` and choose the shared `DevTools Archive` scheme. Set
the run destination to `Any Mac`, then choose **Product > Archive**. Xcode
automatically signs the archive for the configured developer team and opens
it in Organizer.

The build phase invokes the Tauri release build for Apple Silicon with Rust
line-table debug information enabled. It runs `dsymutil`, checks that the
dSYM UUID matches the app executable, strips DWARF from the app binary, and
places the dSYM in Xcode's archive output. Xcode signs the copied app after
the script completes.

The default marketing version is `0.9.0` and the initial build number is
`19`. Increment `CURRENT_PROJECT_VERSION` in the Xcode project before each
new App Store Connect upload. Xcode must be signed in to the `C9VXW45UBV`
developer team and have permission to manage App Store provisioning.
