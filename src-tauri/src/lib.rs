use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use std::collections::HashMap;
use std::fs::{self, File};
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};
use tauri::{AppHandle, Emitter, Manager, State};
use tauri_plugin_dialog::{DialogExt, FilePath};

const CONTRACT_VERSION: &str = "2";
const MAX_VALUE_BYTES: usize = 1024 * 1024;
const MAX_MESSAGE_BYTES: usize = 2 * 1024 * 1024;
const MAX_OUTPUT_BYTES: usize = 4 * 1024 * 1024;
const MAX_SETTINGS_BYTES: u64 = 64 * 1024;
const EVENT_NAME: &str = "native-bridge";
const SUPPORTED_OPERATIONS: &[&str] = &[
    "formatJson",
    "openTextFile",
    "saveTextFile",
    "getSettings",
    "saveSettings",
    "getAboutInfo",
    "imageTakeDropped",
    "saveLastUsedTool",
];

#[derive(Clone, Default)]
struct BridgeState {
    operations: Arc<Mutex<HashMap<String, Arc<AtomicBool>>>>,
    settings_lock: Arc<Mutex<()>>,
}

#[derive(Debug)]
struct Failure {
    code: &'static str,
    message: String,
}

impl Failure {
    fn new(code: &'static str, message: impl Into<String>) -> Self {
        Self {
            code,
            message: message.into(),
        }
    }
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
struct Request {
    contract_version: String,
    request_id: String,
    operation: String,
    payload: Value,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct ApplicationSettings {
    language: String,
    theme: String,
    show_sidebar_on_startup: bool,
    show_last_tool_on_startup: bool,
    #[serde(skip_serializing_if = "Option::is_none")]
    last_used_tool: Option<u32>,
    always_on_top: bool,
    remember_window_size: bool,
    remember_window_position: bool,
}

impl Default for ApplicationSettings {
    fn default() -> Self {
        Self {
            language: "en".into(),
            theme: "system".into(),
            show_sidebar_on_startup: true,
            show_last_tool_on_startup: false,
            last_used_tool: None,
            always_on_top: false,
            remember_window_size: true,
            remember_window_position: true,
        }
    }
}

#[derive(Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct SettingsFile {
    schema_version: u32,
    settings: ApplicationSettings,
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct SettingsUpdate {
    language: String,
    theme: String,
    show_sidebar_on_startup: bool,
    show_last_tool_on_startup: bool,
    always_on_top: bool,
    remember_window_size: bool,
    remember_window_position: bool,
}

impl SettingsUpdate {
    fn validate(&self) -> Result<(), Failure> {
        if !matches!(self.language.as_str(), "en" | "ja_JP") {
            return Err(Failure::new("INVALID_INPUT", "Unsupported language."));
        }
        if !matches!(self.theme.as_str(), "system" | "light" | "dark") {
            return Err(Failure::new("INVALID_INPUT", "Unsupported theme."));
        }
        Ok(())
    }
}

fn invalid_request(code: &'static str, message: impl Into<String>) -> String {
    format!("{code}: {}", message.into())
}

fn validate_request(value: Value) -> Result<Request, String> {
    let bytes = serde_json::to_vec(&value)
        .map_err(|error| invalid_request("INVALID_INPUT", format!("Invalid request: {error}")))?;
    if bytes.len() > MAX_MESSAGE_BYTES {
        return Err(invalid_request(
            "SIZE_LIMIT_EXCEEDED",
            "Bridge message is larger than 2 MiB.",
        ));
    }
    let request: Request = serde_json::from_value(value)
        .map_err(|error| invalid_request("INVALID_INPUT", format!("Invalid request: {error}")))?;
    if request.contract_version != CONTRACT_VERSION {
        return Err(invalid_request(
            "CONTRACT_VERSION_MISMATCH",
            format!("Expected bridge contract version {CONTRACT_VERSION}."),
        ));
    }
    if request.request_id.is_empty()
        || request.request_id.len() > 64
        || !request
            .request_id
            .bytes()
            .all(|byte| byte.is_ascii_alphanumeric() || byte == b'-' || byte == b'_')
    {
        return Err(invalid_request("INVALID_INPUT", "Invalid request ID."));
    }
    if !SUPPORTED_OPERATIONS.contains(&request.operation.as_str()) {
        return Err(invalid_request(
            "UNSUPPORTED_OPERATION",
            "Operation is not available in the Tauri M4 proof.",
        ));
    }
    validate_payload(&request.operation, &request.payload)
        .map_err(|failure| invalid_request(failure.code, failure.message))?;
    Ok(request)
}

fn validate_payload(operation: &str, payload: &Value) -> Result<(), Failure> {
    let object = payload
        .as_object()
        .ok_or_else(|| Failure::new("INVALID_INPUT", "Operation payload must be an object."))?;
    let text = |key: &str| {
        object
            .get(key)
            .and_then(Value::as_str)
            .ok_or_else(|| Failure::new("INVALID_INPUT", format!("{key} must be a string.")))
    };
    let optional_file_type = || -> Result<Option<&str>, Failure> {
        let file_type = object.get("fileType").map(|value| {
            value
                .as_str()
                .ok_or_else(|| Failure::new("INVALID_INPUT", "fileType must be a string."))
        });
        let file_type = file_type.transpose()?.unwrap_or("text");
        if !matches!(file_type, "text" | "data" | "markdown" | "html") {
            return Err(Failure::new("INVALID_INPUT", "Unsupported fileType."));
        }
        Ok(Some(file_type))
    };
    match operation {
        "formatJson" => {
            if text("text")?.len() > MAX_VALUE_BYTES {
                return Err(Failure::new(
                    "SIZE_LIMIT_EXCEEDED",
                    "JSON input is larger than 1 MiB.",
                ));
            }
            if !matches!(
                object.get("indentation").and_then(Value::as_str),
                Some("2" | "4" | "tab" | "minified")
            ) {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Unsupported JSON indentation.",
                ));
            }
        }
        "openTextFile" => {
            optional_file_type()?;
        }
        "saveTextFile" => {
            if text("text")?.len() > MAX_VALUE_BYTES {
                return Err(Failure::new(
                    "SIZE_LIMIT_EXCEEDED",
                    "Text to save is larger than 1 MiB.",
                ));
            }
            if object
                .get("suggestedName")
                .is_some_and(|value| !value.is_string())
            {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "suggestedName must be a string.",
                ));
            }
            optional_file_type()?;
        }
        "getSettings" | "getAboutInfo" | "imageTakeDropped" => {
            if !object.is_empty() {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Operation input must be empty.",
                ));
            }
        }
        "saveSettings" => {
            let update: SettingsUpdate = serde_json::from_value(payload.clone())
                .map_err(|_| Failure::new("INVALID_INPUT", "Invalid application settings."))?;
            update.validate()?;
        }
        "saveLastUsedTool" => {
            if object
                .get("toolId")
                .and_then(Value::as_u64)
                .map_or(true, |value| value == 0 || value > 13)
            {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "toolId is outside the supported range.",
                ));
            }
        }
        _ => {
            return Err(Failure::new(
                "UNSUPPORTED_OPERATION",
                "Unsupported operation.",
            ))
        }
    }
    Ok(())
}

fn format_json(text: &str, indentation: &str) -> Result<String, Failure> {
    let value: Value = serde_json::from_str(text)
        .map_err(|error| Failure::new("INVALID_INPUT", error.to_string()))?;
    let output = if indentation == "minified" {
        serde_json::to_string(&value)
            .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))?
    } else {
        let indent = match indentation {
            "2" => b"  ".as_slice(),
            "4" => b"    ".as_slice(),
            "tab" => b"\t".as_slice(),
            _ => {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Unsupported JSON indentation.",
                ))
            }
        };
        let mut bytes = Vec::new();
        let formatter = serde_json::ser::PrettyFormatter::with_indent(indent);
        let mut serializer = serde_json::Serializer::with_formatter(&mut bytes, formatter);
        value
            .serialize(&mut serializer)
            .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))?;
        bytes.push(b'\n');
        String::from_utf8(bytes)
            .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))?
    };
    if output.len() > MAX_OUTPUT_BYTES {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Formatted output is larger than 4 MiB.",
        ));
    }
    Ok(output)
}

fn settings_path(app: &AppHandle) -> Result<PathBuf, Failure> {
    app.path()
        .app_config_dir()
        .map(|directory| directory.join("settings.json"))
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))
}

fn load_settings(path: &Path) -> Result<ApplicationSettings, Failure> {
    let mut file = match File::open(path) {
        Ok(file) => file,
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => {
            return Ok(ApplicationSettings::default());
        }
        Err(error) => return Err(Failure::new("IO_ERROR", error.to_string())),
    };
    if file
        .metadata()
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?
        .len()
        > MAX_SETTINGS_BYTES
    {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Settings file is too large.",
        ));
    }
    let mut bytes = Vec::new();
    file.read_to_end(&mut bytes)
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    let stored: SettingsFile = serde_json::from_slice(&bytes)
        .map_err(|error| Failure::new("INVALID_SETTINGS", error.to_string()))?;
    if stored.schema_version != 1 {
        return Err(Failure::new(
            "INVALID_SETTINGS",
            "Unsupported settings schema version.",
        ));
    }
    Ok(stored.settings)
}

fn atomic_write(path: &Path, bytes: &[u8]) -> Result<(), Failure> {
    let directory = path
        .parent()
        .ok_or_else(|| Failure::new("IO_ERROR", "Target path has no parent directory."))?;
    fs::create_dir_all(directory).map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    let mut temporary = tempfile::NamedTempFile::new_in(directory)
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    temporary
        .write_all(bytes)
        .and_then(|()| temporary.as_file().sync_all())
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    temporary
        .persist(path)
        .map_err(|error| Failure::new("IO_ERROR", error.error.to_string()))?;
    Ok(())
}

fn save_settings(path: &Path, settings: &ApplicationSettings) -> Result<(), Failure> {
    let file = SettingsFile {
        schema_version: 1,
        settings: settings.clone(),
    };
    let bytes = serde_json::to_vec_pretty(&file)
        .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))?;
    atomic_write(path, &bytes)
}

fn selected_path(file: FilePath) -> Result<PathBuf, Failure> {
    file.into_path()
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))
}

fn read_text_file(path: &Path) -> Result<Value, Failure> {
    let file = File::open(path).map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    if file
        .metadata()
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?
        .len()
        > MAX_VALUE_BYTES as u64
    {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Selected file is larger than 1 MiB.",
        ));
    }
    let mut bytes = Vec::new();
    file.take((MAX_VALUE_BYTES + 1) as u64)
        .read_to_end(&mut bytes)
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    if bytes.len() > MAX_VALUE_BYTES {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Selected file is larger than 1 MiB.",
        ));
    }
    let text = String::from_utf8(bytes)
        .map_err(|_| Failure::new("INVALID_ENCODING", "The selected file is not valid UTF-8."))?;
    let file_name = path
        .file_name()
        .and_then(|name| name.to_str())
        .unwrap_or_default();
    Ok(json!({"text": text, "fileName": file_name}))
}

fn write_text_file(path: &Path, text: &str) -> Result<Value, Failure> {
    let bytes = text.as_bytes();
    if bytes.len() > MAX_VALUE_BYTES {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Text to save is larger than 1 MiB.",
        ));
    }
    atomic_write(path, bytes)?;
    let file_name = path
        .file_name()
        .and_then(|name| name.to_str())
        .unwrap_or_default();
    Ok(json!({"fileName": file_name, "bytesWritten": bytes.len()}))
}

fn file_filter(file_type: &str) -> (&'static str, &'static [&'static str]) {
    match file_type {
        "data" => ("Data files", &["json", "yaml", "yml", "toml"]),
        "markdown" => ("Markdown files", &["md", "markdown"]),
        "html" => ("HTML files", &["html", "htm"]),
        _ => ("Text files", &["txt"]),
    }
}

fn safe_suggested_name(name: &str, file_type: &str) -> String {
    let suffix = match file_type {
        "data" => "json",
        "markdown" => "md",
        "html" => "html",
        _ => "txt",
    };
    let safe: String = name
        .replace('/', "_")
        .replace('\\', "_")
        .replace("..", "")
        .trim()
        .chars()
        .take(128)
        .collect();
    if safe.is_empty() {
        format!("output.{suffix}")
    } else {
        safe
    }
}

fn emit(app: &AppHandle, event: Value) {
    let _ = app.emit(EVENT_NAME, event);
}

fn execute_operation(
    app: &AppHandle,
    state: &BridgeState,
    request: &Request,
    cancelled: &AtomicBool,
) -> Result<Value, Failure> {
    if cancelled.load(Ordering::Relaxed) {
        return Err(Failure::new("CANCELLED", "Operation was cancelled."));
    }
    let payload = request
        .payload
        .as_object()
        .expect("validated object payload");
    match request.operation.as_str() {
        "formatJson" => {
            let text = payload["text"].as_str().expect("validated text");
            let indentation = payload["indentation"]
                .as_str()
                .expect("validated indentation");
            let output = format_json(text, indentation)?;
            if cancelled.load(Ordering::Relaxed) {
                return Err(Failure::new("CANCELLED", "Operation was cancelled."));
            }
            Ok(json!({"text": output}))
        }
        "openTextFile" | "saveTextFile" => {
            emit(
                app,
                json!({"type":"progress", "requestId":request.request_id, "percent":5, "message":"Waiting for file selection"}),
            );
            let file_type = payload
                .get("fileType")
                .and_then(Value::as_str)
                .unwrap_or("text");
            let (filter_name, extensions) = file_filter(file_type);
            let dialog = app.dialog().file().add_filter(filter_name, extensions);
            let selection = if request.operation == "openTextFile" {
                dialog.blocking_pick_file()
            } else {
                let name = payload
                    .get("suggestedName")
                    .and_then(Value::as_str)
                    .unwrap_or_default();
                dialog
                    .set_file_name(safe_suggested_name(name, file_type))
                    .blocking_save_file()
            };
            let Some(selection) = selection else {
                return Err(Failure::new("CANCELLED", "File selection was cancelled."));
            };
            // shortcut: cannot cancel an open system dialog, revisit if the API adds support.
            if cancelled.load(Ordering::Relaxed) {
                return Err(Failure::new("CANCELLED", "Operation was cancelled."));
            }
            let path = selected_path(selection)?;
            if request.operation == "openTextFile" {
                emit(
                    app,
                    json!({"type":"progress", "requestId":request.request_id, "percent":20, "message":"Reading file"}),
                );
                read_text_file(&path)
            } else {
                emit(
                    app,
                    json!({"type":"progress", "requestId":request.request_id, "percent":20, "message":"Writing file"}),
                );
                write_text_file(&path, payload["text"].as_str().expect("validated text"))
            }
        }
        "getSettings" => {
            let _guard = state
                .settings_lock
                .lock()
                .map_err(|_| Failure::new("INTERNAL_ERROR", "Settings lock was poisoned."))?;
            let settings = load_settings(&settings_path(app)?)?;
            serde_json::to_value(settings)
                .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))
        }
        "saveSettings" => {
            let update: SettingsUpdate = serde_json::from_value(request.payload.clone())
                .map_err(|_| Failure::new("INVALID_INPUT", "Invalid application settings."))?;
            update.validate()?;
            let _guard = state
                .settings_lock
                .lock()
                .map_err(|_| Failure::new("INTERNAL_ERROR", "Settings lock was poisoned."))?;
            let path = settings_path(app)?;
            let mut settings = load_settings(&path)?;
            settings.language = update.language;
            settings.theme = update.theme;
            settings.show_sidebar_on_startup = update.show_sidebar_on_startup;
            settings.show_last_tool_on_startup = update.show_last_tool_on_startup;
            settings.always_on_top = update.always_on_top;
            settings.remember_window_size = update.remember_window_size;
            settings.remember_window_position = update.remember_window_position;
            save_settings(&path, &settings)?;
            Ok(json!({"saved":true}))
        }
        "saveLastUsedTool" => {
            let _guard = state
                .settings_lock
                .lock()
                .map_err(|_| Failure::new("INTERNAL_ERROR", "Settings lock was poisoned."))?;
            let path = settings_path(app)?;
            let mut settings = load_settings(&path)?;
            settings.last_used_tool = payload["toolId"].as_u64().map(|value| value as u32);
            save_settings(&path, &settings)?;
            Ok(json!({"saved":true}))
        }
        // shortcut: ignore drops until Tauri window drag/drop events are implemented.
        "imageTakeDropped" => Ok(json!({"found":false})),
        "getAboutInfo" => Ok(json!({
            "appVersion": env!("CARGO_PKG_VERSION"),
            "revision": option_env!("DEVTOOLS_GIT_REVISION").unwrap_or("unknown"),
            "buildEnvironment": format!("Tauri preview ({}/{})", std::env::consts::OS, std::env::consts::ARCH),
            "qtVersion": "Not applicable (Tauri preview)",
            "copyright": "2025 © Lotus&Company Inc. All rights reserved.",
            "licenseText": include_str!("../../LICENSE"),
        })),
        _ => Err(Failure::new(
            "UNSUPPORTED_OPERATION",
            "Unsupported operation.",
        )),
    }
}

#[tauri::command]
fn start_operation(
    app: AppHandle,
    state: State<'_, BridgeState>,
    request: Value,
) -> Result<(), String> {
    let request = validate_request(request)?;
    let cancelled = Arc::new(AtomicBool::new(false));
    {
        let mut operations = state
            .operations
            .lock()
            .map_err(|_| invalid_request("INTERNAL_ERROR", "Operation lock was poisoned."))?;
        if operations.len() >= 128 {
            return Err(invalid_request("BUSY", "Too many active operations."));
        }
        if operations.contains_key(&request.request_id) {
            return Err(invalid_request("INVALID_INPUT", "Duplicate request ID."));
        }
        operations.insert(request.request_id.clone(), cancelled.clone());
    }
    emit(
        &app,
        json!({"type":"started", "requestId":request.request_id, "operation":request.operation}),
    );
    let app_for_work = app.clone();
    let app_for_event = app.clone();
    let state = state.inner().clone();
    tauri::async_runtime::spawn(async move {
        let request_id = request.request_id.clone();
        let work_app = app_for_work.clone();
        let work_state = state.clone();
        let work_cancelled = cancelled.clone();
        let result = tauri::async_runtime::spawn_blocking(move || {
            execute_operation(&work_app, &work_state, &request, &work_cancelled)
        })
        .await;
        let event = match result {
            Ok(Ok(result)) => json!({"type":"completed", "requestId":request_id, "result":result}),
            Ok(Err(failure)) if failure.code == "CANCELLED" => {
                json!({"type":"cancelled", "requestId":request_id})
            }
            Ok(Err(failure)) => {
                json!({"type":"failed", "requestId":request_id, "error":{"code":failure.code, "message":failure.message}})
            }
            Err(error) => {
                json!({"type":"failed", "requestId":request_id, "error":{"code":"INTERNAL_ERROR", "message":error.to_string()}})
            }
        };
        emit(&app_for_event, event);
        if let Ok(mut operations) = state.operations.lock() {
            operations.remove(&request_id);
        }
    });
    Ok(())
}

#[tauri::command]
fn cancel_operation(state: State<'_, BridgeState>, request_id: String) {
    if let Ok(operations) = state.operations.lock() {
        if let Some(cancelled) = operations.get(&request_id) {
            cancelled.store(true, Ordering::Relaxed);
        }
    }
}

pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .manage(BridgeState::default())
        .invoke_handler(tauri::generate_handler![start_operation, cancel_operation])
        .run(tauri::generate_context!())
        .expect("error while running DevTools Tauri host");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[derive(Deserialize)]
    #[serde(rename_all = "camelCase")]
    struct FormatFixture {
        input: FormatInput,
        expected: String,
    }

    #[derive(Deserialize)]
    struct FormatInput {
        text: String,
        indentation: String,
    }

    #[test]
    fn format_json_matches_shared_qt_contract_fixtures() {
        let fixtures: Vec<FormatFixture> =
            serde_json::from_str(include_str!("../../tests/fixtures/tauri-format-json.json"))
                .expect("shared format JSON fixtures are valid");
        assert!(!fixtures.is_empty());
        for fixture in fixtures {
            assert_eq!(
                format_json(&fixture.input.text, &fixture.input.indentation).unwrap(),
                fixture.expected
            );
        }
    }

    #[test]
    fn validates_contract_version_ids_limits_and_payloads() {
        let request = |version: &str, id: &str, text: &str| {
            json!({
                "contractVersion": version,
                "requestId": id,
                "operation": "formatJson",
                "payload": {"text": text, "indentation": "2"}
            })
        };
        assert!(validate_request(request("2", "abc123", "{}")).is_ok());
        assert!(validate_request(request("1", "abc123", "{}"))
            .unwrap_err()
            .contains("CONTRACT_VERSION_MISMATCH"));
        assert!(validate_request(request("2", "bad id", "{}"))
            .unwrap_err()
            .contains("INVALID_INPUT"));
        assert!(
            validate_request(request("2", "abc123", &"x".repeat(MAX_VALUE_BYTES + 1)))
                .unwrap_err()
                .contains("SIZE_LIMIT_EXCEEDED")
        );
        assert!(validate_request(json!({
            "contractVersion":"2", "requestId":"abc123", "operation":"sendHttpRequest", "payload":{}
        }))
        .unwrap_err()
        .contains("UNSUPPORTED_OPERATION"));
    }

    #[test]
    fn reads_and_writes_utf8_text_with_limits() {
        let directory = tempfile::tempdir().unwrap();
        let path = directory.path().join("日本語.txt");
        let output = write_text_file(&path, "こんにちは").unwrap();
        assert_eq!(output["bytesWritten"], "こんにちは".len());
        assert_eq!(read_text_file(&path).unwrap()["text"], "こんにちは");
        fs::write(&path, [0xff]).unwrap();
        assert_eq!(read_text_file(&path).unwrap_err().code, "INVALID_ENCODING");
        fs::write(&path, vec![b'x'; MAX_VALUE_BYTES + 1]).unwrap();
        assert_eq!(
            read_text_file(&path).unwrap_err().code,
            "SIZE_LIMIT_EXCEEDED"
        );
    }

    #[test]
    fn settings_are_versioned_and_saved_atomically() {
        let directory = tempfile::tempdir().unwrap();
        let path = directory.path().join("settings.json");
        let settings = ApplicationSettings::default();
        save_settings(&path, &settings).unwrap();
        assert_eq!(load_settings(&path).unwrap().language, "en");
        fs::write(&path, br#"{"schemaVersion":2,"settings":{}}"#).unwrap();
        assert_eq!(load_settings(&path).unwrap_err().code, "INVALID_SETTINGS");
    }
}
