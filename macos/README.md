# Xcode Archive for TestFlight

Open `DevToolsArchive.xcodeproj` and choose the shared `DevTools Archive` scheme. Set
the run destination to `Any Mac`, then choose **Product > Archive**. Xcode
signs the archive with the configured Apple Distribution certificate and
opens it in Organizer.

The build phase invokes the Tauri release build for Apple Silicon with Rust
line-table debug information enabled. It runs `dsymutil`, checks that the
dSYM UUID matches the app executable, strips DWARF from the app binary, and
places the dSYM in Xcode's archive output. Xcode signs the copied app after
the script completes.

The default marketing version is `0.9.0` and the initial build number is
`19`. Increment `CURRENT_PROJECT_VERSION` in the Xcode project before each
new App Store Connect upload. The Mac needs the Apple Distribution certificate
and private key for team `C9VXW45UBV`; Xcode must be signed in to App Store
Connect to upload the archive.
