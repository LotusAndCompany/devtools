fn main() {
    // Keep this list aligned with the application commands in `generate_handler!`.
    tauri_build::try_build(tauri_build::Attributes::new().app_manifest(
        tauri_build::AppManifest::new().commands(&["start_operation", "cancel_operation"]),
    ))
    .expect("failed to configure Tauri build");
}
