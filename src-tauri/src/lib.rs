use base64::Engine;
use plist::Value as PlistValue;
use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use std::collections::HashMap;
use std::fs::{self, File};
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex, Once, TryLockError};
use tauri::{AppHandle, Emitter, Manager, State};
use tauri_plugin_clipboard_manager::ClipboardExt;
use tauri_plugin_dialog::{DialogExt, FilePath};

mod m5;

const CONTRACT_VERSION: &str = "2";
const MAX_VALUE_BYTES: usize = 1024 * 1024;
const MAX_MESSAGE_BYTES: usize = 2 * 1024 * 1024;
const MAX_OUTPUT_BYTES: usize = 4 * 1024 * 1024;
const MAX_SETTINGS_BYTES: u64 = 64 * 1024;
const MAX_LEGACY_SETTINGS_BYTES: u64 = 4 * 1024 * 1024;
const MAX_CLIPBOARD_IMAGE_PIXELS: u64 = 100_000_000;
const EVENT_NAME: &str = "native-bridge";
const SUPPORTED_OPERATIONS: &[&str] = &[
    "formatJson",
    "convertTimestamp",
    "compareText",
    "convertData",
    "renderMarkdown",
    "generateQr",
    "saveQrImage",
    "commandData",
    "copyToClipboard",
    "evaluateRegex",
    "regexSettings",
    "phraseData",
    "databaseHistory",
    "databaseSelectFile",
    "databaseConnect",
    "databaseDisconnect",
    "databaseQuery",
    "databaseTables",
    "sendHttpRequest",
    "openTextFile",
    "saveTextFile",
    "getSettings",
    "saveSettings",
    "getAboutInfo",
    "imageOpen",
    "imageEdit",
    "imageReset",
    "imageSave",
    "imageSplit",
    "imageTakeDropped",
    "saveLastUsedTool",
];

#[derive(Clone, Default)]
struct BridgeState {
    operations: Arc<Mutex<HashMap<String, Arc<AtomicBool>>>>,
    settings_lock: Arc<Mutex<()>>,
    image: Arc<Mutex<Option<CurrentImage>>>,
    dropped_image: Arc<Mutex<Option<PathBuf>>>,
    image_lock: Arc<Mutex<()>>,
    database: Arc<Mutex<DatabaseState>>,
}

#[derive(Clone)]
struct CurrentImage {
    current: Arc<image::DynamicImage>,
    original: Arc<image::DynamicImage>,
    path: PathBuf,
}

#[derive(Default)]
struct DatabaseState {
    pool: Option<sqlx::AnyPool>,
    connection: Option<DatabaseConnection>,
    selected_file: Option<PathBuf>,
}

#[derive(Clone)]
struct DatabaseConnection {
    driver: String,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct StoredDatabaseConnection {
    #[serde(rename = "type")]
    driver: String,
    host: String,
    database: String,
    username: String,
    display_name: String,
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
    #[serde(default, skip_serializing_if = "Option::is_none")]
    window_width: Option<u32>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    window_height: Option<u32>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    window_x: Option<i32>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    window_y: Option<i32>,
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
            window_width: None,
            window_height: None,
            window_x: None,
            window_y: None,
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

#[derive(Clone, Debug, Default, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct RegexSettings {
    pattern: String,
    test_text: String,
    replace_pattern: String,
    options: RegexOptions,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct RegexOptions {
    global: bool,
    case_insensitive: bool,
    multiline: bool,
    dot_matches_everything: bool,
    use_unicode_properties: bool,
    extended_pattern_syntax: bool,
}

impl Default for RegexOptions {
    fn default() -> Self {
        Self {
            global: true,
            case_insensitive: false,
            multiline: false,
            dot_matches_everything: false,
            use_unicode_properties: false,
            extended_pattern_syntax: false,
        }
    }
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
            "Operation is not available in the Tauri host.",
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
        "convertTimestamp" => {
            if text("value")?.len() > MAX_VALUE_BYTES
                || !matches!(
                    object.get("source").and_then(Value::as_str),
                    Some("seconds" | "milliseconds" | "iso")
                )
            {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Timestamp input or source is invalid.",
                ));
            }
        }
        "compareText" => {
            for key in ["left", "right"] {
                if text(key)?.len() > MAX_VALUE_BYTES {
                    return Err(Failure::new(
                        "SIZE_LIMIT_EXCEEDED",
                        "Diff input is larger than 1 MiB.",
                    ));
                }
            }
            if split_diff_lines(text("left")?).len() > 2000
                || split_diff_lines(text("right")?).len() > 2000
            {
                return Err(Failure::new(
                    "SIZE_LIMIT_EXCEEDED",
                    "Diff is limited to 2,000 lines per side.",
                ));
            }
        }
        "convertData" => {
            if text("input")?.len() > MAX_VALUE_BYTES
                || !matches!(
                    object.get("format").and_then(Value::as_str),
                    Some("json" | "yaml-block" | "yaml-flow" | "toml")
                )
                || !matches!(
                    object.get("indentation").and_then(Value::as_str),
                    Some("2" | "4" | "tab" | "minified")
                )
            {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Input, output format or indentation is invalid.",
                ));
            }
        }
        "renderMarkdown" => {
            if text("markdown")?.len() > MAX_VALUE_BYTES {
                return Err(Failure::new(
                    "SIZE_LIMIT_EXCEEDED",
                    "Markdown is larger than 1 MiB.",
                ));
            }
        }
        "generateQr" | "saveQrImage" => {
            if !matches!(
                object.get("type").and_then(Value::as_str),
                Some(
                    "text"
                        | "url"
                        | "email"
                        | "phone"
                        | "sms"
                        | "wifi"
                        | "contact"
                        | "calendar"
                        | "geo"
                )
            ) || object
                .get("parameters")
                .and_then(Value::as_object)
                .is_none()
            {
                return Err(Failure::new("INVALID_INPUT", "QR code input is invalid."));
            }
            if object["parameters"].as_object().is_some_and(|parameters| {
                parameters.values().any(|value| {
                    !value.is_string()
                        || value
                            .as_str()
                            .is_some_and(|value| value.len() > MAX_VALUE_BYTES)
                })
            }) {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "QR code fields must be strings.",
                ));
            }
        }
        "commandData" => {
            if !matches!(
                object.get("category").and_then(Value::as_str),
                Some("git" | "docker" | "compose")
            ) {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Command category is not supported.",
                ));
            }
            match object.get("action").and_then(Value::as_str) {
                Some("list") => {}
                Some("generate")
                    if object
                        .get("functionIndex")
                        .and_then(Value::as_u64)
                        .is_some()
                        && object.get("optionIndex").and_then(Value::as_u64).is_some()
                        && text("value")?.len() <= MAX_VALUE_BYTES => {}
                _ => {
                    return Err(Failure::new(
                        "INVALID_INPUT",
                        "Command action is not supported.",
                    ))
                }
            }
        }
        "copyToClipboard" => {
            let has_text = object.get("text").and_then(Value::as_str).is_some();
            let has_image = object.get("imageDataUrl").and_then(Value::as_str).is_some();
            if has_text == has_image
                || object
                    .get(if has_text { "text" } else { "imageDataUrl" })
                    .and_then(Value::as_str)
                    .is_some_and(|value| value.len() > MAX_MESSAGE_BYTES)
            {
                return Err(Failure::new("INVALID_INPUT", "Clipboard input is invalid."));
            }
        }
        "evaluateRegex" => {
            for key in ["pattern", "text", "replacement"] {
                if text(key)?.len() > MAX_VALUE_BYTES {
                    return Err(Failure::new(
                        "SIZE_LIMIT_EXCEEDED",
                        "Regex input is larger than 1 MiB.",
                    ));
                }
            }
            for key in [
                "global",
                "caseInsensitive",
                "multiline",
                "dotMatchesEverything",
                "useUnicodeProperties",
                "extendedPatternSyntax",
            ] {
                if object.get(key).is_some_and(|value| !value.is_boolean()) {
                    return Err(Failure::new(
                        "INVALID_INPUT",
                        "Regex options must be booleans.",
                    ));
                }
            }
        }
        "regexSettings" => match object.get("action").and_then(Value::as_str) {
            Some("load") if object.len() == 1 => {}
            Some("save") => {
                let settings: RegexSettings = serde_json::from_value(payload.clone())
                    .map_err(|_| Failure::new("INVALID_INPUT", "Invalid regex settings."))?;
                if settings.pattern.len() > MAX_VALUE_BYTES
                    || settings.test_text.len() > MAX_VALUE_BYTES
                    || settings.replace_pattern.len() > MAX_VALUE_BYTES
                {
                    return Err(Failure::new(
                        "SIZE_LIMIT_EXCEEDED",
                        "Regex settings exceed the 1 MiB limit.",
                    ));
                }
            }
            _ => {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Regex settings action is not supported.",
                ))
            }
        },
        "phraseData" => match object.get("action").and_then(Value::as_str) {
            Some("list") if object.len() == 1 => {}
            Some("save") => {
                for key in ["title", "content"] {
                    if text(key)?.len() > MAX_VALUE_BYTES {
                        return Err(Failure::new(
                            "SIZE_LIMIT_EXCEEDED",
                            "Template content is larger than 1 MiB.",
                        ));
                    }
                }
                if object
                    .get("filename")
                    .is_some_and(|value| !value.is_string())
                {
                    return Err(Failure::new(
                        "INVALID_INPUT",
                        "Template identifier is invalid.",
                    ));
                }
            }
            Some("delete") => {
                text("filename")?;
            }
            _ => {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Template action is not supported.",
                ))
            }
        },
        "sendHttpRequest" => {
            if !matches!(
                object.get("method").and_then(Value::as_str),
                Some("GET" | "POST" | "PUT" | "DELETE")
            ) || text("url")?.len() > MAX_VALUE_BYTES
                || text("body")?.len() > MAX_VALUE_BYTES
                || text("username")?.len() > MAX_VALUE_BYTES
                || text("password")?.len() > MAX_VALUE_BYTES
            {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "URL, method and body are invalid.",
                ));
            }
            let headers = object
                .get("headers")
                .and_then(Value::as_object)
                .ok_or_else(|| {
                    Failure::new("INVALID_INPUT", "Request headers must be an object.")
                })?;
            for (name, value) in headers {
                let value = value.as_str().ok_or_else(|| {
                    Failure::new("INVALID_INPUT", "Request header values must be strings.")
                })?;
                if value.len() > MAX_VALUE_BYTES
                    || value.contains('\r')
                    || value.contains('\n')
                    || name.trim().eq_ignore_ascii_case("host")
                    || name.trim().eq_ignore_ascii_case("content-length")
                {
                    return Err(Failure::new(
                        "INVALID_INPUT",
                        "A request header is invalid or restricted.",
                    ));
                }
            }
            let parameters = object
                .get("parameters")
                .and_then(Value::as_array)
                .ok_or_else(|| {
                    Failure::new("INVALID_INPUT", "Query parameters must be an array.")
                })?;
            if parameters.len() > 10 {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "At most 10 query parameters are supported.",
                ));
            }
            for parameter in parameters {
                let Some(parameter) = parameter.as_object() else {
                    return Err(Failure::new(
                        "INVALID_INPUT",
                        "Query parameters must contain string fields.",
                    ));
                };
                for key in ["key", "value", "description"] {
                    if parameter
                        .get(key)
                        .and_then(Value::as_str)
                        .is_none_or(|value| value.len() > MAX_VALUE_BYTES)
                    {
                        return Err(Failure::new(
                            "INVALID_INPUT",
                            "Query parameters must contain string fields.",
                        ));
                    }
                }
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
        "imageOpen" | "imageReset" | "imageSave" => {
            if !object.is_empty() {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Operation input must be empty.",
                ));
            }
        }
        "imageEdit" => {
            if !matches!(
                object.get("action").and_then(Value::as_str),
                Some(
                    "rotate-left"
                        | "rotate-right"
                        | "flip-horizontal"
                        | "flip-vertical"
                        | "resize-size"
                        | "resize-scale"
                        | "transparent"
                )
            ) {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Image edit action is not supported.",
                ));
            }
        }
        "imageSplit" => {
            let valid = match object.get("mode").and_then(Value::as_str) {
                Some("division") => {
                    object
                        .get("horizontalDivisions")
                        .and_then(Value::as_u64)
                        .is_some_and(|value| value > 0)
                        && object
                            .get("verticalDivisions")
                            .and_then(Value::as_u64)
                            .is_some_and(|value| value > 0)
                }
                Some("cell-size") => {
                    object
                        .get("cellWidth")
                        .and_then(Value::as_u64)
                        .is_some_and(|value| value > 0)
                        && object
                            .get("cellHeight")
                            .and_then(Value::as_u64)
                            .is_some_and(|value| value > 0)
                }
                _ => false,
            };
            if !valid
                || object
                    .get("ignoreRemainders")
                    .is_some_and(|value| !value.is_boolean())
            {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Image split input is invalid.",
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
        "databaseHistory" | "databaseSelectFile" | "databaseDisconnect" | "databaseTables" => {
            if !object.is_empty() {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Operation input must be empty.",
                ));
            }
        }
        "databaseQuery" => {
            if text("sql")?.trim().is_empty() {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Enter SQL no larger than 1 MiB.",
                ));
            }
            if text("sql")?.len() > MAX_VALUE_BYTES {
                return Err(Failure::new(
                    "SIZE_LIMIT_EXCEEDED",
                    "SQL is larger than 1 MiB.",
                ));
            }
        }
        "databaseConnect" => {
            if let Some(index) = object.get("historyIndex") {
                if index.as_u64().is_none() || text("password")?.len() > MAX_VALUE_BYTES {
                    return Err(Failure::new(
                        "INVALID_INPUT",
                        "Select a valid saved connection.",
                    ));
                }
            } else {
                let driver = object
                    .get("type")
                    .and_then(Value::as_str)
                    .unwrap_or_default();
                if !matches!(driver, "sqlite" | "mysql" | "postgres") {
                    return Err(Failure::new(
                        "INVALID_INPUT",
                        "Choose SQLite, MySQL or PostgreSQL.",
                    ));
                }
                for key in ["host", "database", "username", "password"] {
                    if object.get(key).is_some_and(|value| {
                        !value.is_string()
                            || value
                                .as_str()
                                .is_some_and(|text| text.len() > MAX_VALUE_BYTES)
                    }) {
                        return Err(Failure::new(
                            "INVALID_INPUT",
                            "Database connection fields are invalid.",
                        ));
                    }
                }
                if driver != "sqlite"
                    && ["host", "database", "username"].iter().any(|key| {
                        object
                            .get(*key)
                            .and_then(Value::as_str)
                            .is_none_or(str::is_empty)
                    })
                {
                    return Err(Failure::new(
                        "INVALID_INPUT",
                        "Complete the database connection fields.",
                    ));
                }
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

fn timestamp_result(value: &str, source: &str) -> Result<Value, Failure> {
    use chrono::{DateTime, Local, NaiveDate, NaiveDateTime, SecondsFormat, Utc};

    let invalid = || Failure::new("INVALID_INPUT", "Enter a valid timestamp or ISO-8601 date.");
    let instant = match source {
        "seconds" => {
            let seconds = value.trim().parse::<i64>().map_err(|_| invalid())?;
            DateTime::<Utc>::from_timestamp(seconds, 0).ok_or_else(invalid)?
        }
        "milliseconds" => {
            let milliseconds = value.trim().parse::<i64>().map_err(|_| invalid())?;
            DateTime::<Utc>::from_timestamp_millis(milliseconds).ok_or_else(invalid)?
        }
        "iso" => {
            if let Ok(date) = DateTime::parse_from_rfc3339(value.trim()) {
                date.with_timezone(&Utc)
            } else {
                let text = value.trim();
                let local = [
                    NaiveDateTime::parse_from_str(text, "%Y-%m-%dT%H:%M:%S%.f"),
                    NaiveDateTime::parse_from_str(text, "%Y-%m-%d %H:%M:%S%.f"),
                ]
                .into_iter()
                .find_map(Result::ok)
                .or_else(|| {
                    NaiveDate::parse_from_str(text, "%Y-%m-%d")
                        .ok()?
                        .and_hms_opt(0, 0, 0)
                })
                .ok_or_else(invalid)?;
                local_datetime_to_utc(&local).ok_or_else(invalid)?
            }
        }
        _ => return Err(invalid()),
    };
    let seconds = (instant.timestamp_millis() / 1000).to_string();
    let milliseconds = instant.timestamp_millis().to_string();
    Ok(json!({
        "seconds": seconds,
        "milliseconds": milliseconds,
        "utc": instant.to_rfc3339_opts(SecondsFormat::Millis, true),
        "local": instant.with_timezone(&Local).to_rfc3339_opts(SecondsFormat::Millis, false),
    }))
}

fn local_datetime_to_utc(local: &chrono::NaiveDateTime) -> Option<chrono::DateTime<chrono::Utc>> {
    use chrono::{Duration, Local, LocalResult, Offset, TimeZone};

    match Local.from_local_datetime(local) {
        LocalResult::Single(date) => Some(date.with_timezone(&chrono::Utc)),
        LocalResult::Ambiguous(first, second) => {
            Some(first.min(second).with_timezone(&chrono::Utc))
        }
        LocalResult::None => {
            for minutes in 1..=24 * 60 {
                let before_time = local.checked_sub_signed(Duration::minutes(minutes))?;
                let after_time = local.checked_add_signed(Duration::minutes(minutes))?;
                let (LocalResult::Single(before), LocalResult::Single(after)) = (
                    Local.from_local_datetime(&before_time),
                    Local.from_local_datetime(&after_time),
                ) else {
                    continue;
                };
                let gap_seconds = after.offset().fix().local_minus_utc()
                    - before.offset().fix().local_minus_utc();
                if gap_seconds > 0 {
                    let adjusted =
                        local.checked_add_signed(Duration::seconds(i64::from(gap_seconds)))?;
                    return match Local.from_local_datetime(&adjusted) {
                        LocalResult::Single(date) => Some(date.with_timezone(&chrono::Utc)),
                        LocalResult::Ambiguous(first, second) => {
                            Some(first.min(second).with_timezone(&chrono::Utc))
                        }
                        LocalResult::None => None,
                    };
                }
            }
            None
        }
    }
}

fn split_diff_lines(text: &str) -> Vec<String> {
    let normalized = text.replace("\r\n", "\n").replace('\r', "\n");
    if normalized.is_empty() {
        return Vec::new();
    }
    let mut lines = normalized
        .split('\n')
        .map(str::to_owned)
        .collect::<Vec<_>>();
    if lines.last().is_some_and(String::is_empty) {
        lines.pop();
    }
    lines
}

fn compare_text(left: &str, right: &str, cancelled: &AtomicBool) -> Result<Value, Failure> {
    let left = split_diff_lines(left);
    let right = split_diff_lines(right);
    let columns = right.len() + 1;
    let mut lcs = vec![0u16; (left.len() + 1) * columns];
    for i in (0..left.len()).rev() {
        if cancelled.load(Ordering::Relaxed) {
            return Err(Failure::new("CANCELLED", "Operation was cancelled."));
        }
        for j in (0..right.len()).rev() {
            let index = i * columns + j;
            lcs[index] = if left[i] == right[j] {
                lcs[(i + 1) * columns + j + 1] + 1
            } else {
                lcs[(i + 1) * columns + j].max(lcs[i * columns + j + 1])
            };
        }
    }
    let mut changes = Vec::new();
    let (mut i, mut j) = (0, 0);
    while i < left.len() && j < right.len() {
        if cancelled.load(Ordering::Relaxed) {
            return Err(Failure::new("CANCELLED", "Operation was cancelled."));
        }
        if left[i] == right[j] {
            changes.push((0, left[i].clone(), right[j].clone()));
            i += 1;
            j += 1;
        } else if lcs[(i + 1) * columns + j] >= lcs[i * columns + j + 1] {
            changes.push((1, left[i].clone(), String::new()));
            i += 1;
        } else {
            changes.push((2, String::new(), right[j].clone()));
            j += 1;
        }
    }
    while i < left.len() {
        changes.push((1, left[i].clone(), String::new()));
        i += 1;
    }
    while j < right.len() {
        changes.push((2, String::new(), right[j].clone()));
        j += 1;
    }
    let mut rows = Vec::new();
    let mut index = 0;
    while index < changes.len() {
        match changes[index].0 {
            0 => {
                rows.push(json!({"left":changes[index].1, "right":changes[index].2, "leftChanged":false, "rightChanged":false}));
                index += 1;
            }
            1 if changes.get(index + 1).is_some_and(|change| change.0 == 2) => {
                rows.push(json!({"left":changes[index].1, "right":changes[index + 1].2, "leftChanged":true, "rightChanged":true}));
                index += 2;
            }
            1 => {
                rows.push(json!({"left":changes[index].1, "right":"", "leftChanged":true, "rightChanged":false}));
                index += 1;
            }
            _ => {
                rows.push(json!({"left":"", "right":changes[index].2, "leftChanged":false, "rightChanged":true}));
                index += 1;
            }
        }
    }
    Ok(json!({"rows":rows}))
}

async fn send_http_request(
    payload: &Value,
    cancelled: &AtomicBool,
    mut progress: impl FnMut(i32, &str),
) -> Result<Value, Failure> {
    use futures_util::StreamExt;
    use reqwest::header::{HeaderName, HeaderValue, CONTENT_TYPE};
    use std::time::{Duration, Instant};

    let invalid = || {
        Failure::new(
            "INVALID_INPUT",
            "Enter a valid HTTP or HTTPS URL and method.",
        )
    };
    let object = payload.as_object().expect("validated request object");
    let method = object["method"].as_str().expect("validated method");
    let original_url = object["url"].as_str().expect("validated URL");
    let parameters = object["parameters"]
        .as_array()
        .expect("validated parameters");
    let url_text = if parameters.is_empty() {
        original_url.to_owned()
    } else {
        let base_url = original_url.split('?').next().unwrap_or_default();
        let query = parameters
            .iter()
            .filter_map(|parameter| {
                let parameter = parameter.as_object().expect("validated parameter");
                let key = parameter["key"].as_str().expect("validated key");
                if key.is_empty() {
                    return None;
                }
                let value = parameter["value"].as_str().expect("validated value");
                Some(format!(
                    "{}={}",
                    percent_encode_query_component(key),
                    percent_encode_query_component(value)
                ))
            })
            .collect::<Vec<_>>()
            .join("&");
        if query.is_empty() {
            base_url.to_owned()
        } else {
            format!("{base_url}?{query}")
        }
    };
    let url = reqwest::Url::parse(&url_text).map_err(|_| invalid())?;
    if !matches!(url.scheme(), "http" | "https") || url.host_str().is_none() {
        return Err(invalid());
    }
    let mut headers = reqwest::header::HeaderMap::new();
    headers.insert(CONTENT_TYPE, HeaderValue::from_static("application/json"));
    for (name, value) in object["headers"].as_object().expect("validated headers") {
        let name = HeaderName::from_bytes(name.trim().as_bytes()).map_err(|_| invalid())?;
        let value = HeaderValue::from_str(value.as_str().expect("validated header value"))
            .map_err(|_| {
                Failure::new(
                    "INVALID_INPUT",
                    "A request header is invalid or restricted.",
                )
            })?;
        headers.insert(name, value);
    }

    let client = reqwest::Client::builder()
        .connect_timeout(Duration::from_secs(30))
        .timeout(Duration::from_secs(30))
        .build()
        .map_err(|_| Failure::new("INTERNAL_ERROR", "Could not initialize the HTTP client."))?;
    let method = reqwest::Method::from_bytes(method.as_bytes()).map_err(|_| invalid())?;
    let mut request = client.request(method, url).headers(headers);
    let username = object["username"].as_str().expect("validated username");
    let password = object["password"].as_str().expect("validated password");
    if !username.is_empty() && !password.is_empty() {
        request = request.basic_auth(username, Some(password));
    }
    if matches!(object["method"].as_str(), Some("POST" | "PUT")) {
        request = request.body(object["body"].as_str().expect("validated body").to_owned());
    }
    if cancelled.load(Ordering::Relaxed) {
        return Err(Failure::new("CANCELLED", "Operation was cancelled."));
    }

    progress(5, "Sending HTTP request");
    let started = Instant::now();
    let response = tokio::select! {
        response = request.send() => response,
        _ = wait_for_cancellation(cancelled) => return Err(Failure::new("CANCELLED", "Operation was cancelled.")),
    };
    let response = match response {
        Ok(response) => response,
        Err(error) => {
            let (error_code, message) = if error.is_timeout() {
                (4, "Operation timed out.")
            } else if error.is_connect() {
                (3, "Could not connect to the server.")
            } else {
                (99, "Network request failed.")
            };
            return Ok(json!({
                "status": 0,
                "reason": "",
                "elapsedMs": started.elapsed().as_millis() as u64,
                "errorCode": error_code,
                "error": message,
                "body": "",
                "bytes": 0,
            }));
        }
    };
    if response
        .content_length()
        .is_some_and(|length| length > MAX_OUTPUT_BYTES as u64)
    {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "HTTP response body is limited to 4 MiB.",
        ));
    }
    let status = response.status();
    let reason = status.canonical_reason().unwrap_or("");
    let mut stream = response.bytes_stream();
    let mut body = Vec::new();
    let mut read_error = false;
    loop {
        let chunk = tokio::select! {
            chunk = stream.next() => chunk,
            _ = wait_for_cancellation(cancelled) => return Err(Failure::new("CANCELLED", "Operation was cancelled.")),
        };
        match chunk {
            Some(Ok(chunk)) => {
                if body.len() + chunk.len() > MAX_OUTPUT_BYTES {
                    return Err(Failure::new(
                        "SIZE_LIMIT_EXCEEDED",
                        "HTTP response body is limited to 4 MiB.",
                    ));
                }
                body.extend_from_slice(&chunk);
            }
            Some(Err(_)) => {
                read_error = true;
                break;
            }
            None => break,
        }
    }
    progress(95, "Preparing HTTP response");
    let error_code = if read_error {
        99
    } else {
        http_status_error_code(status.as_u16())
    };
    let error = if read_error {
        "Could not read the response body."
    } else if error_code == 0 {
        ""
    } else {
        reason
    };
    Ok(json!({
        "status": status.as_u16(),
        "reason": reason,
        "elapsedMs": started.elapsed().as_millis() as u64,
        "errorCode": error_code,
        "error": error,
        "body": String::from_utf8_lossy(&body),
        "bytes": body.len(),
    }))
}

fn percent_encode_query_component(value: &str) -> String {
    const HEX: &[u8; 16] = b"0123456789ABCDEF";
    let mut encoded = String::with_capacity(value.len());
    for byte in value.bytes() {
        if byte.is_ascii_alphanumeric() || b"-._~".contains(&byte) {
            encoded.push(byte as char);
        } else {
            encoded.push('%');
            encoded.push(HEX[(byte >> 4) as usize] as char);
            encoded.push(HEX[(byte & 0x0f) as usize] as char);
        }
    }
    encoded
}

async fn wait_for_cancellation(cancelled: &AtomicBool) {
    while !cancelled.load(Ordering::Relaxed) {
        tokio::time::sleep(std::time::Duration::from_millis(50)).await;
    }
}

fn http_status_error_code(status: u16) -> i32 {
    match status {
        100..=399 => 0,
        401 => 204,
        403 => 201,
        404 => 203,
        400..=499 => 299,
        _ => 399,
    }
}

fn settings_path(app: &AppHandle) -> Result<PathBuf, Failure> {
    migrate_legacy_qsettings(app)?;
    app.path()
        .app_config_dir()
        .map(|directory| directory.join("settings.json"))
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))
}

fn regex_settings_path(app: &AppHandle) -> Result<PathBuf, Failure> {
    migrate_legacy_qsettings(app)?;
    app.path()
        .app_config_dir()
        .map(|directory| directory.join("regex-settings.json"))
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))
}

fn phrase_directory(app: &AppHandle) -> Result<PathBuf, Failure> {
    app.path()
        .app_data_dir()
        .map(|directory| directory.join("content"))
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))
}

fn valid_phrase_filename(filename: &str) -> bool {
    filename.ends_with(".txt")
        && filename.len() <= 255
        && !matches!(filename, "." | "..")
        && !filename.contains(['/', '\\', ':'])
}

fn migrate_phrase_templates(legacy: &Path, directory: &Path) -> Result<(), Failure> {
    fs::create_dir_all(directory).map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    if directory.join(".legacy-import-complete").exists() {
        return Ok(());
    }

    match fs::read_dir(legacy) {
        Ok(entries) => {
            for entry in entries {
                let entry = entry.map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
                if !entry
                    .file_type()
                    .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?
                    .is_file()
                {
                    continue;
                }
                let Some(filename) = entry.file_name().to_str().map(str::to_owned) else {
                    continue;
                };
                if !valid_phrase_filename(&filename) {
                    continue;
                }
                if entry
                    .metadata()
                    .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?
                    .len()
                    > MAX_VALUE_BYTES as u64
                {
                    return Err(Failure::new(
                        "SIZE_LIMIT_EXCEEDED",
                        "Legacy template is larger than 1 MiB.",
                    ));
                }
                let bytes = fs::read(entry.path())
                    .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
                let target = directory.join(filename);
                let mut staged = tempfile::NamedTempFile::new_in(directory)
                    .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
                staged
                    .write_all(&bytes)
                    .and_then(|()| staged.as_file().sync_all())
                    .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
                match staged.persist_noclobber(&target) {
                    Ok(_) => {}
                    Err(error) if error.error.kind() == std::io::ErrorKind::AlreadyExists => {
                        continue;
                    }
                    Err(error) => return Err(Failure::new("IO_ERROR", error.error.to_string())),
                }
            }
        }
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => return Ok(()),
        Err(error) => return Err(Failure::new("IO_ERROR", error.to_string())),
    }
    atomic_write(&directory.join(".legacy-import-complete"), b"1")
}

fn read_phrase_file(path: &Path) -> Result<Value, Failure> {
    let file = File::open(path).map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    if file
        .metadata()
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?
        .len()
        > MAX_VALUE_BYTES as u64
    {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Template is larger than 1 MiB.",
        ));
    }
    let mut bytes = Vec::new();
    file.take((MAX_VALUE_BYTES + 1) as u64)
        .read_to_end(&mut bytes)
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    if bytes.len() > MAX_VALUE_BYTES {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Template is larger than 1 MiB.",
        ));
    }
    let text = String::from_utf8(bytes)
        .map_err(|_| Failure::new("INVALID_ENCODING", "Template is not valid UTF-8."))?;
    let (title, content) = text.split_once('\n').unwrap_or((&text, ""));
    let filename = path
        .file_name()
        .and_then(|name| name.to_str())
        .unwrap_or_default();
    Ok(json!({"filename":filename, "title":title, "content":content}))
}

fn database_history_path(app: &AppHandle) -> Result<PathBuf, Failure> {
    migrate_legacy_qsettings(app)?;
    app.path()
        .app_config_dir()
        .map(|directory| directory.join("database-history.json"))
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))
}

fn migrate_legacy_qsettings(app: &AppHandle) -> Result<(), Failure> {
    if !cfg!(target_os = "macos") {
        return Ok(());
    }
    let config_dir = app
        .path()
        .app_config_dir()
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    let home = app
        .path()
        .home_dir()
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    migrate_qsettings_file(
        &config_dir,
        &home.join("Library/Preferences/com.trolltech.unknown-organization.DevTools.plist"),
    )
}

fn migrate_qsettings_file(config_dir: &Path, legacy_path: &Path) -> Result<(), Failure> {
    let settings_path = config_dir.join("settings.json");
    let regex_path = config_dir.join("regex-settings.json");
    let history_path = config_dir.join("database-history.json");
    let needs_migration = !settings_path.exists() || !regex_path.exists() || !history_path.exists();
    if !needs_migration || !legacy_path.exists() {
        return Ok(());
    }

    let mut source = File::open(legacy_path).map_err(|error| {
        Failure::new("IO_ERROR", format!("Could not read Qt settings: {error}"))
    })?;
    if source
        .metadata()
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?
        .len()
        > MAX_LEGACY_SETTINGS_BYTES
    {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Legacy Qt settings exceed the 4 MiB migration limit.",
        ));
    }
    let mut bytes = Vec::new();
    source
        .read_to_end(&mut bytes)
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;

    let backup_path = config_dir.join("migration/qsettings-source.plist");
    if !backup_path.exists() {
        atomic_write(&backup_path, &bytes).map_err(|error| {
            Failure::new(
                "IO_ERROR",
                format!(
                    "Could not back up Qt settings before migration: {}",
                    error.message
                ),
            )
        })?;
    }
    let document =
        PlistValue::from_reader(std::io::Cursor::new(bytes.as_slice())).map_err(|_| {
            Failure::new(
            "INVALID_SETTINGS",
            "Legacy Qt settings are not a valid property list; the source backup was preserved.",
        )
        })?;
    let values = document.as_dictionary().ok_or_else(|| {
        Failure::new(
            "INVALID_SETTINGS",
            "Legacy Qt settings do not contain a property list dictionary.",
        )
    })?;

    let settings_bytes = if settings_path.exists() {
        None
    } else {
        let settings = SettingsFile {
            schema_version: 1,
            settings: legacy_application_settings(values)?,
        };
        Some(
            serde_json::to_vec_pretty(&settings)
                .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))?,
        )
    };
    let regex_bytes = if regex_path.exists() {
        None
    } else {
        Some(
            serde_json::to_vec(&legacy_regex_settings(values)?)
                .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))?,
        )
    };
    let history_bytes = if history_path.exists() {
        None
    } else {
        Some(
            serde_json::to_vec(&legacy_database_history(values)?)
                .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))?,
        )
    };

    for (path, output) in [
        (&settings_path, settings_bytes),
        (&regex_path, regex_bytes),
        (&history_path, history_bytes),
    ] {
        if let Some(output) = output {
            atomic_write(path, &output).map_err(|error| {
                Failure::new(
                    "IO_ERROR",
                    format!(
                        "Qt settings migration is incomplete; the original and backup are preserved: {}",
                        error.message
                    ),
                )
            })?;
        }
    }
    Ok(())
}

fn legacy_application_settings(values: &plist::Dictionary) -> Result<ApplicationSettings, Failure> {
    let mut settings = ApplicationSettings::default();
    settings.language = legacy_string(values, "language", "en")?;
    settings.theme = legacy_string(values, "general.theme", "system")?;
    settings.show_sidebar_on_startup =
        legacy_boolean(values, "general.showSidemenuOnStartup", true)?;
    settings.show_last_tool_on_startup =
        legacy_boolean(values, "general.showLastToolOnStartup", false)?;
    settings.always_on_top = legacy_boolean(values, "window.alwaysOnTop", false)?;
    settings.remember_window_size = legacy_boolean(values, "window.rememberSize", true)?;
    settings.remember_window_position = legacy_boolean(values, "window.rememberPosition", true)?;
    if let Some(value) = legacy_integer(values, "general.lastUsedTool")? {
        settings.last_used_tool = match value {
            -1 => None,
            1..=13 => Some(value as u32),
            _ => return Err(invalid_legacy_settings("general.lastUsedTool")),
        };
    }
    let width = legacy_integer(values, "window.width")?;
    let height = legacy_integer(values, "window.height")?;
    if width.is_some() != height.is_some() {
        return Err(invalid_legacy_settings("window size"));
    }
    settings.window_width = match width {
        Some(value) => Some(
            u32::try_from(value)
                .ok()
                .filter(|size| (860..=16_384).contains(size))
                .ok_or_else(|| invalid_legacy_settings("window.width"))?,
        ),
        None => None,
    };
    settings.window_height = match height {
        Some(value) => Some(
            u32::try_from(value)
                .ok()
                .filter(|size| (520..=16_384).contains(size))
                .ok_or_else(|| invalid_legacy_settings("window.height"))?,
        ),
        None => None,
    };
    if width.is_some() && (settings.window_width.is_none() || settings.window_height.is_none()) {
        return Err(invalid_legacy_settings("window size"));
    }
    settings.window_x = match legacy_integer(values, "window.x")? {
        Some(value) => Some(i32::try_from(value).map_err(|_| invalid_legacy_settings("window.x"))?),
        None => None,
    };
    settings.window_y = match legacy_integer(values, "window.y")? {
        Some(value) => Some(i32::try_from(value).map_err(|_| invalid_legacy_settings("window.y"))?),
        None => None,
    };
    if settings.window_x.is_some() != settings.window_y.is_some() {
        return Err(invalid_legacy_settings("window position"));
    }
    if !matches!(settings.language.as_str(), "en" | "ja_JP") {
        return Err(invalid_legacy_settings("language"));
    }
    if !matches!(settings.theme.as_str(), "system" | "light" | "dark") {
        return Err(invalid_legacy_settings("general.theme"));
    }
    Ok(settings)
}

fn legacy_regex_settings(values: &plist::Dictionary) -> Result<RegexSettings, Failure> {
    let mut settings = RegexSettings::default();
    settings.pattern = legacy_string(values, "RegexTester.pattern", "")?;
    settings.test_text = legacy_string(values, "RegexTester.testText", "")?;
    settings.replace_pattern = legacy_string(values, "RegexTester.replacePattern", "")?;
    if settings.pattern.len() > MAX_VALUE_BYTES
        || settings.test_text.len() > MAX_VALUE_BYTES
        || settings.replace_pattern.len() > MAX_VALUE_BYTES
    {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Legacy regex settings exceed the 1 MiB migration limit.",
        ));
    }
    settings.options.global = legacy_boolean(values, "RegexTester.flagG", true)?;
    settings.options.case_insensitive = legacy_boolean(values, "RegexTester.flagI", false)?;
    settings.options.multiline = legacy_boolean(values, "RegexTester.flagM", false)?;
    settings.options.dot_matches_everything = legacy_boolean(values, "RegexTester.flagS", false)?;
    settings.options.use_unicode_properties = legacy_boolean(values, "RegexTester.flagU", false)?;
    settings.options.extended_pattern_syntax = legacy_boolean(values, "RegexTester.flagX", false)?;
    Ok(settings)
}

fn legacy_database_history(
    values: &plist::Dictionary,
) -> Result<Vec<StoredDatabaseConnection>, Failure> {
    let Some(entries) = values.get("db_tool.connectionHistory") else {
        return Ok(Vec::new());
    };
    let entries = entries
        .as_array()
        .ok_or_else(|| invalid_legacy_settings("db_tool.connectionHistory"))?;
    let mut history = Vec::new();
    for entry in entries.iter().take(10) {
        let encoded = entry
            .as_string()
            .ok_or_else(|| invalid_legacy_settings("db_tool.connectionHistory"))?;
        let value: Value = serde_json::from_str(encoded)
            .map_err(|_| invalid_legacy_settings("db_tool.connectionHistory"))?;
        let Some(value) = value.as_object() else {
            continue;
        };
        let get = |key: &str| -> Result<String, Failure> {
            let value = value
                .get(key)
                .and_then(Value::as_str)
                .ok_or_else(|| invalid_legacy_settings("database history entry"))?;
            if value.len() > MAX_VALUE_BYTES {
                return Err(Failure::new(
                    "SIZE_LIMIT_EXCEEDED",
                    "Legacy database history exceeds the 1 MiB migration limit.",
                ));
            }
            Ok(value.to_owned())
        };
        let driver = common_database_driver(&get("type")?)
            .ok_or_else(|| invalid_legacy_settings("database history driver"))?
            .to_owned();
        let host = get("host")?;
        let database = get("database")?;
        let username = get("username")?;
        let display_name = value
            .get("displayName")
            .and_then(Value::as_str)
            .map(str::to_owned)
            .unwrap_or_else(|| {
                if driver == "sqlite" {
                    format!(
                        "SQLite: {}",
                        Path::new(&database)
                            .file_name()
                            .and_then(|name| name.to_str())
                            .unwrap_or_default()
                    )
                } else {
                    let kind = if driver == "mysql" {
                        "MySQL"
                    } else {
                        "PostgreSQL"
                    };
                    format!("{kind}: {username}@{host}/{database}")
                }
            });
        history.push(StoredDatabaseConnection {
            driver,
            host,
            database,
            username,
            display_name,
        });
    }
    Ok(history)
}

fn legacy_string(values: &plist::Dictionary, key: &str, default: &str) -> Result<String, Failure> {
    match values.get(key) {
        Some(value) => value
            .as_string()
            .map(str::to_owned)
            .ok_or_else(|| invalid_legacy_settings(key)),
        None => Ok(default.to_owned()),
    }
}

fn legacy_boolean(values: &plist::Dictionary, key: &str, default: bool) -> Result<bool, Failure> {
    match values.get(key) {
        Some(value) => value
            .as_boolean()
            .ok_or_else(|| invalid_legacy_settings(key)),
        None => Ok(default),
    }
}

fn legacy_integer(values: &plist::Dictionary, key: &str) -> Result<Option<i64>, Failure> {
    match values.get(key) {
        Some(value) => value
            .as_signed_integer()
            .map(Some)
            .ok_or_else(|| invalid_legacy_settings(key)),
        None => Ok(None),
    }
}

fn invalid_legacy_settings(field: &str) -> Failure {
    Failure::new(
        "INVALID_SETTINGS",
        format!("Legacy Qt settings contain an invalid {field}; the source backup was preserved."),
    )
}

fn common_database_driver(driver: &str) -> Option<&'static str> {
    match driver {
        "sqlite" | "QSQLITE" => Some("sqlite"),
        "mysql" | "QMYSQL" => Some("mysql"),
        "postgres" | "QPSQL" => Some("postgres"),
        _ => None,
    }
}

fn read_database_history(path: &Path) -> Result<Vec<StoredDatabaseConnection>, Failure> {
    let mut file = match File::open(path) {
        Ok(file) => file,
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => return Ok(Vec::new()),
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
            "Database history file is too large.",
        ));
    }
    let mut bytes = Vec::new();
    file.read_to_end(&mut bytes)
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    let history: Vec<StoredDatabaseConnection> = serde_json::from_slice(&bytes)
        .map_err(|_| Failure::new("INVALID_SETTINGS", "Database history file is invalid."))?;
    Ok(history
        .into_iter()
        .filter_map(|mut item| {
            item.driver = common_database_driver(&item.driver)?.to_owned();
            Some(item)
        })
        .take(10)
        .collect())
}

fn save_database_history(path: &Path, entry: StoredDatabaseConnection) -> Result<(), Failure> {
    let mut history = read_database_history(path)?;
    history.retain(|item| {
        !(item.driver == entry.driver && item.host == entry.host && item.database == entry.database)
    });
    history.insert(0, entry);
    history.truncate(10);
    let bytes = serde_json::to_vec(&history)
        .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))?;
    atomic_write(path, &bytes)
}

fn database_history_result(app: &AppHandle, state: &BridgeState) -> Result<Value, Failure> {
    let history = read_database_history(&database_history_path(app)?)?;
    let database = state
        .database
        .lock()
        .map_err(|_| Failure::new("INTERNAL_ERROR", "Database state lock was poisoned."))?;
    let visible = history.iter().enumerate().map(|(index, item)| {
        let database_name = if item.driver == "sqlite" {
            Path::new(&item.database).file_name().and_then(|name| name.to_str()).unwrap_or_default()
        } else { &item.database };
        json!({"index":index, "type":item.driver, "host":item.host, "database":database_name, "username":item.username, "displayName":item.display_name})
    }).collect::<Vec<_>>();
    let selected_file_name = database
        .selected_file
        .as_ref()
        .and_then(|path| path.file_name())
        .and_then(|name| name.to_str())
        .unwrap_or_default();
    let connected = database.pool.as_ref().is_some_and(|pool| !pool.is_closed());
    let connected_type = if connected {
        database
            .connection
            .as_ref()
            .map_or("", |connection| connection.driver.as_str())
    } else {
        ""
    };
    Ok(
        json!({"history":visible, "selectedFileName":selected_file_name, "connected":connected, "connectedType":connected_type}),
    )
}

fn database_select_file(
    app: &AppHandle,
    state: &BridgeState,
    cancelled: &AtomicBool,
) -> Result<Value, Failure> {
    let Some(selection) = app
        .dialog()
        .file()
        .add_filter("SQLite database", &["db", "sqlite", "sqlite3"])
        .blocking_pick_file()
    else {
        return Err(Failure::new("CANCELLED", "File selection was cancelled."));
    };
    if cancelled.load(Ordering::Relaxed) {
        return Err(Failure::new("CANCELLED", "Operation was cancelled."));
    }
    let path = selected_path(selection)?;
    let suffix = path
        .extension()
        .and_then(|value| value.to_str())
        .unwrap_or_default();
    if !["db", "sqlite", "sqlite3"]
        .iter()
        .any(|supported| suffix.eq_ignore_ascii_case(supported))
    {
        return Err(Failure::new(
            "INVALID_INPUT",
            "Choose a SQLite database file.",
        ));
    }
    std::fs::OpenOptions::new()
        .read(true)
        .write(true)
        .open(&path)
        .map_err(|_| {
            Failure::new(
                "PERMISSION_DENIED",
                "Could not access the selected database file.",
            )
        })?;
    let file_name = path
        .file_name()
        .and_then(|name| name.to_str())
        .unwrap_or_default()
        .to_owned();
    state
        .database
        .lock()
        .map_err(|_| Failure::new("INTERNAL_ERROR", "Database state lock was poisoned."))?
        .selected_file = Some(path);
    Ok(json!({"selected":true, "fileName":file_name}))
}

static SQLX_DRIVER_INIT: Once = Once::new();

fn database_url(
    driver: &str,
    host: &str,
    database: &str,
    username: &str,
    password: &str,
) -> Result<reqwest::Url, Failure> {
    let mut url = if driver == "sqlite" {
        let path = Path::new(database);
        let file_url = reqwest::Url::from_file_path(path)
            .map_err(|_| Failure::new("INVALID_INPUT", "SQLite database path is invalid."))?;
        let url = reqwest::Url::parse(&file_url.as_str().replacen("file:", "sqlite:", 1))
            .map_err(|_| Failure::new("INVALID_INPUT", "SQLite database path is invalid."))?;
        let mut url = url;
        url.query_pairs_mut().append_pair("mode", "rw");
        return Ok(url);
    } else {
        reqwest::Url::parse(if driver == "mysql" {
            "mysql://localhost/"
        } else {
            "postgres://localhost/"
        })
        .map_err(|_| Failure::new("INTERNAL_ERROR", "Could not configure the database driver."))?
    };
    url.set_host(Some(host))
        .map_err(|_| Failure::new("INVALID_INPUT", "Database host is invalid."))?;
    url.set_username(username)
        .map_err(|_| Failure::new("INVALID_INPUT", "Database username is invalid."))?;
    url.set_password(Some(password))
        .map_err(|_| Failure::new("INVALID_INPUT", "Database password is invalid."))?;
    url.set_path(database);
    Ok(url)
}

async fn open_database_pool(
    driver: &str,
    host: &str,
    database: &str,
    username: &str,
    password: &str,
    cancelled: &AtomicBool,
) -> Result<sqlx::AnyPool, Failure> {
    use std::str::FromStr;
    use std::time::Duration;
    SQLX_DRIVER_INIT.call_once(sqlx::any::install_default_drivers);
    let url = database_url(driver, host, database, username, password)?;
    let options = sqlx::any::AnyConnectOptions::from_str(url.as_str())
        .map_err(|_| Failure::new("INVALID_INPUT", "Database connection settings are invalid."))?;
    let connect = sqlx::any::AnyPoolOptions::new()
        .max_connections(1)
        .min_connections(1)
        .acquire_timeout(Duration::from_secs(30))
        .connect_with(options);
    tokio::select! {
        result = tokio::time::timeout(Duration::from_secs(30), connect) => match result {
            Ok(Ok(pool)) => Ok(pool),
            Ok(Err(_)) => Err(Failure::new("IO_ERROR", "Could not connect to the database.")),
            Err(_) => Err(Failure::new("TIMEOUT", "Database connection timed out.")),
        },
        _ = wait_for_cancellation(cancelled) => Err(Failure::new("CANCELLED", "Operation was cancelled.")),
    }
}

async fn database_tables(
    pool: &sqlx::AnyPool,
    driver: &str,
    cancelled: &AtomicBool,
) -> Result<Vec<String>, Failure> {
    use sqlx::Row;
    let query = match driver {
        "sqlite" => "SELECT name FROM sqlite_master WHERE type = 'table' AND name NOT LIKE 'sqlite_%' ORDER BY name",
        "mysql" => "SHOW TABLES",
        _ => "SELECT table_name FROM information_schema.tables WHERE table_schema = current_schema() AND table_type = 'BASE TABLE' ORDER BY table_name",
    };
    let result = tokio::select! {
        result = tokio::time::timeout(std::time::Duration::from_secs(30), sqlx::query(query).fetch_all(pool)) => match result {
            Ok(Ok(rows)) => rows,
            Ok(Err(_)) => return Err(Failure::new("IO_ERROR", "Could not read database tables.")),
            Err(_) => return Err(Failure::new("TIMEOUT", "Database request timed out.")),
        },
        _ = wait_for_cancellation(cancelled) => return Err(Failure::new("CANCELLED", "Operation was cancelled.")),
    };
    result
        .into_iter()
        .map(|row| {
            row.try_get::<String, usize>(0)
                .map_err(|_| Failure::new("IO_ERROR", "Could not read database table names."))
        })
        .collect()
}

async fn database_query(
    pool: &sqlx::AnyPool,
    driver: &str,
    sql: &str,
    cancelled: &AtomicBool,
) -> Result<Value, Failure> {
    use futures_util::TryStreamExt;
    use sqlx::any::AnyTypeInfoKind;
    use sqlx::{Column, Either, Executor, Row, ValueRef};
    let mut connection = tokio::select! {
        result = tokio::time::timeout(std::time::Duration::from_secs(30), pool.acquire()) => match result {
            Ok(Ok(connection)) => connection,
            Ok(Err(_)) => return Err(Failure::new("IO_ERROR", "Could not access the database connection.")),
            Err(_) => return Err(Failure::new("TIMEOUT", "Database request timed out.")),
        },
        _ = wait_for_cancellation(cancelled) => return Err(Failure::new("CANCELLED", "Operation was cancelled.")),
    };
    let mut columns = connection
        .describe(sql)
        .await
        .map(|description| {
            description
                .columns()
                .iter()
                .map(|column| column.name().to_owned())
                .collect::<Vec<_>>()
        })
        .unwrap_or_default();
    let mut stream = Box::pin(sqlx::raw_sql(sql).fetch_many(&mut *connection));
    let mut rows = Vec::new();
    let mut affected_rows = 0_u64;
    let mut total_bytes = 0_usize;
    let mut truncated = false;
    loop {
        let next = tokio::select! {
            result = tokio::time::timeout(std::time::Duration::from_secs(30), stream.try_next()) => match result {
                Ok(Ok(value)) => value,
                Ok(Err(_)) => return Err(Failure::new("INVALID_INPUT", "Database query failed.")),
                Err(_) => return Err(Failure::new("TIMEOUT", "Database query timed out.")),
            },
            _ = wait_for_cancellation(cancelled) => return Err(Failure::new("CANCELLED", "Operation was cancelled.")),
        };
        let Some(item) = next else {
            break;
        };
        match item {
            Either::Left(result) => affected_rows = result.rows_affected(),
            Either::Right(row) => {
                if columns.is_empty() {
                    columns = row
                        .columns()
                        .iter()
                        .map(|column| column.name().to_owned())
                        .collect();
                }
                if rows.len() >= 1000 {
                    truncated = true;
                    break;
                }
                let mut output_row = Vec::with_capacity(columns.len());
                for index in 0..columns.len() {
                    if row
                        .try_get_raw(index)
                        .map_err(|_| Failure::new("IO_ERROR", "Could not read database results."))?
                        .is_null()
                    {
                        output_row.push(Value::Null);
                        continue;
                    }
                    let cell = match row
                        .try_get_raw(index)
                        .map_err(|_| Failure::new("IO_ERROR", "Could not read database results."))?
                        .type_info()
                        .kind()
                    {
                        AnyTypeInfoKind::Bool => row
                            .try_get::<bool, usize>(index)
                            .map_err(|_| {
                                Failure::new("IO_ERROR", "Could not read database results.")
                            })?
                            .to_string(),
                        AnyTypeInfoKind::SmallInt => row
                            .try_get::<i16, usize>(index)
                            .map_err(|_| {
                                Failure::new("IO_ERROR", "Could not read database results.")
                            })?
                            .to_string(),
                        AnyTypeInfoKind::Integer => row
                            .try_get::<i32, usize>(index)
                            .map_err(|_| {
                                Failure::new("IO_ERROR", "Could not read database results.")
                            })?
                            .to_string(),
                        AnyTypeInfoKind::BigInt => row
                            .try_get::<i64, usize>(index)
                            .map_err(|_| {
                                Failure::new("IO_ERROR", "Could not read database results.")
                            })?
                            .to_string(),
                        AnyTypeInfoKind::Real => row
                            .try_get::<f32, usize>(index)
                            .map_err(|_| {
                                Failure::new("IO_ERROR", "Could not read database results.")
                            })?
                            .to_string(),
                        AnyTypeInfoKind::Double => row
                            .try_get::<f64, usize>(index)
                            .map_err(|_| {
                                Failure::new("IO_ERROR", "Could not read database results.")
                            })?
                            .to_string(),
                        AnyTypeInfoKind::Text => {
                            row.try_get::<String, usize>(index).map_err(|_| {
                                Failure::new("IO_ERROR", "Could not read database results.")
                            })?
                        }
                        AnyTypeInfoKind::Blob => {
                            String::from_utf8_lossy(&row.try_get::<Vec<u8>, usize>(index).map_err(
                                |_| Failure::new("IO_ERROR", "Could not read database results."),
                            )?)
                            .into_owned()
                        }
                        AnyTypeInfoKind::Null => unreachable!("null values are handled above"),
                    };
                    if cell.encode_utf16().count() > 64 * 1024 {
                        return Err(Failure::new(
                            "SIZE_LIMIT_EXCEEDED",
                            "Database results are limited to 4 MiB; reduce the query result.",
                        ));
                    }
                    total_bytes = total_bytes.saturating_add(cell.len());
                    if total_bytes > MAX_OUTPUT_BYTES {
                        return Err(Failure::new(
                            "SIZE_LIMIT_EXCEEDED",
                            "Database results are limited to 4 MiB; reduce the query result.",
                        ));
                    }
                    output_row.push(Value::String(cell));
                }
                rows.push(Value::Array(output_row));
            }
        }
    }
    if driver == "sqlite"
        && !columns.is_empty()
        && sql
            .trim_start()
            .split_ascii_whitespace()
            .next()
            .is_some_and(|keyword| {
                ["SELECT", "WITH", "PRAGMA", "EXPLAIN"]
                    .iter()
                    .any(|query| keyword.eq_ignore_ascii_case(query))
            })
    {
        affected_rows = 0;
    }
    Ok(json!({"columns":columns, "rows":rows, "truncated":truncated, "affectedRows":affected_rows}))
}

async fn database_operation(
    app: &AppHandle,
    state: &BridgeState,
    request: &Request,
    cancelled: &AtomicBool,
) -> Result<Value, Failure> {
    match request.operation.as_str() {
        "databaseConnect" => {
            let history_path = database_history_path(app)?;
            let history = read_database_history(&history_path)?;
            let (driver, host, database, username, password, selected_file) = if let Some(index) =
                request.payload.get("historyIndex").and_then(Value::as_u64)
            {
                let record = history.get(index as usize).ok_or_else(|| {
                    Failure::new("INVALID_INPUT", "Select a valid saved connection.")
                })?;
                let selected_file = if record.driver == "sqlite" {
                    let path = PathBuf::from(&record.database);
                    if !path.is_file() {
                        return Err(Failure::new(
                            "PERMISSION_DENIED",
                            "The saved SQLite file is no longer available.",
                        ));
                    }
                    Some(path.clone())
                } else {
                    None
                };
                (
                    record.driver.clone(),
                    record.host.clone(),
                    record.database.clone(),
                    record.username.clone(),
                    request.payload["password"]
                        .as_str()
                        .unwrap_or_default()
                        .to_owned(),
                    selected_file,
                )
            } else {
                let driver = request.payload["type"]
                    .as_str()
                    .expect("validated database type")
                    .to_owned();
                if driver == "sqlite" {
                    let selected = request
                        .payload
                        .get("database")
                        .and_then(Value::as_str)
                        .filter(|value| !value.is_empty())
                        .map(PathBuf::from)
                        .or_else(|| {
                            state
                                .database
                                .lock()
                                .ok()
                                .and_then(|database| database.selected_file.clone())
                        })
                        .ok_or_else(|| {
                            Failure::new("INVALID_INPUT", "Choose a SQLite file before connecting.")
                        })?;
                    if !selected.is_file() {
                        return Err(Failure::new(
                            "PERMISSION_DENIED",
                            "Could not access the selected database file.",
                        ));
                    }
                    (
                        driver,
                        String::new(),
                        selected.to_string_lossy().into_owned(),
                        String::new(),
                        String::new(),
                        Some(selected),
                    )
                } else {
                    (
                        driver,
                        request.payload["host"]
                            .as_str()
                            .unwrap_or_default()
                            .to_owned(),
                        request.payload["database"]
                            .as_str()
                            .unwrap_or_default()
                            .to_owned(),
                        request.payload["username"]
                            .as_str()
                            .unwrap_or_default()
                            .to_owned(),
                        request.payload["password"]
                            .as_str()
                            .unwrap_or_default()
                            .to_owned(),
                        None,
                    )
                }
            };
            let display_name = if driver == "sqlite" {
                let name = Path::new(&database)
                    .file_name()
                    .and_then(|name| name.to_str())
                    .unwrap_or_default();
                format!("SQLite: {name}")
            } else {
                let kind = if driver == "mysql" {
                    "MySQL"
                } else {
                    "PostgreSQL"
                };
                format!("{kind}: {username}@{host}/{database}")
            };
            emit(
                app,
                json!({"type":"progress", "requestId":request.request_id, "percent":10, "message":"Connecting to database"}),
            );
            let pool =
                open_database_pool(&driver, &host, &database, &username, &password, cancelled)
                    .await?;
            emit(
                app,
                json!({"type":"progress", "requestId":request.request_id, "percent":75, "message":"Reading database tables"}),
            );
            let tables = match database_tables(&pool, &driver, cancelled).await {
                Ok(tables) => tables,
                Err(error) => {
                    pool.close().await;
                    return Err(error);
                }
            };
            if cancelled.load(Ordering::Relaxed) {
                pool.close().await;
                return Err(Failure::new("CANCELLED", "Operation was cancelled."));
            }
            let history_entry = StoredDatabaseConnection {
                driver: driver.clone(),
                host: host.clone(),
                database: database.clone(),
                username: username.clone(),
                display_name: display_name.clone(),
            };
            if let Err(error) = save_database_history(&history_path, history_entry) {
                pool.close().await;
                return Err(Failure::new(
                    "IO_ERROR",
                    format!(
                        "Database connected, but connection history could not be saved: {}",
                        error.message
                    ),
                ));
            }
            let old_pool = {
                let mut database_state = state.database.lock().map_err(|_| {
                    Failure::new("INTERNAL_ERROR", "Database state lock was poisoned.")
                })?;
                let old = database_state.pool.replace(pool.clone());
                database_state.connection = Some(DatabaseConnection {
                    driver: driver.clone(),
                });
                if driver == "sqlite" {
                    database_state.selected_file = selected_file;
                }
                old
            };
            if let Some(old_pool) = old_pool {
                old_pool.close().await;
            }
            Ok(json!({"connected":true, "displayName":display_name, "tables":tables}))
        }
        "databaseDisconnect" => {
            let old_pool = {
                let mut database_state = state.database.lock().map_err(|_| {
                    Failure::new("INTERNAL_ERROR", "Database state lock was poisoned.")
                })?;
                database_state.connection = None;
                database_state.selected_file = None;
                database_state.pool.take()
            };
            if let Some(pool) = old_pool {
                pool.close().await;
            }
            Ok(json!({"disconnected":true}))
        }
        "databaseTables" | "databaseQuery" => {
            let (pool, driver) = {
                let database_state = state.database.lock().map_err(|_| {
                    Failure::new("INTERNAL_ERROR", "Database state lock was poisoned.")
                })?;
                let pool = database_state
                    .pool
                    .clone()
                    .filter(|pool| !pool.is_closed())
                    .ok_or_else(|| Failure::new("INVALID_INPUT", "Connect to a database first."))?;
                let driver = database_state
                    .connection
                    .as_ref()
                    .map(|connection| connection.driver.clone())
                    .ok_or_else(|| Failure::new("INVALID_INPUT", "Connect to a database first."))?;
                (pool, driver)
            };
            if request.operation == "databaseTables" {
                Ok(json!({"tables":database_tables(&pool, &driver, cancelled).await?}))
            } else {
                database_query(
                    &pool,
                    &driver,
                    request.payload["sql"].as_str().expect("validated SQL"),
                    cancelled,
                )
                .await
            }
        }
        _ => Err(Failure::new(
            "UNSUPPORTED_OPERATION",
            "Unsupported database operation.",
        )),
    }
}

fn phrase_operation(app: &AppHandle, payload: &Value) -> Result<Value, Failure> {
    let directory = phrase_directory(app)?;
    let legacy = std::env::current_dir()
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?
        .join("content");
    migrate_phrase_templates(&legacy, &directory)?;
    match payload["action"].as_str().expect("validated phrase action") {
        "list" => {
            let mut templates = Vec::new();
            match fs::read_dir(&directory) {
                Ok(entries) => {
                    let mut paths = entries
                        .filter_map(Result::ok)
                        .filter(|entry| entry.file_type().is_ok_and(|kind| kind.is_file()))
                        .map(|entry| entry.path())
                        .filter(|path| {
                            path.file_name()
                                .and_then(|name| name.to_str())
                                .is_some_and(valid_phrase_filename)
                        })
                        .collect::<Vec<_>>();
                    paths.sort();
                    for path in paths {
                        templates.push(read_phrase_file(&path)?);
                    }
                }
                Err(error) if error.kind() == std::io::ErrorKind::NotFound => {}
                Err(error) => return Err(Failure::new("IO_ERROR", error.to_string())),
            }
            Ok(json!({"templates":templates}))
        }
        "save" => {
            let title = payload["title"].as_str().expect("validated title");
            let content = payload["content"].as_str().expect("validated content");
            if title.is_empty() {
                return Err(Failure::new("INVALID_INPUT", "Template title is required."));
            }
            if content.is_empty() {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Template content is required.",
                ));
            }
            let previous = payload["filename"].as_str().unwrap_or_default();
            if !previous.is_empty() && !valid_phrase_filename(previous) {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Template identifier is invalid.",
                ));
            }
            let filename = format!("{}.txt", uuid::Uuid::new_v4());
            let path = directory.join(&filename);
            atomic_write(&path, format!("{title}\n{content}").as_bytes())?;
            if !previous.is_empty() {
                let previous_path = directory.join(previous);
                if previous_path.exists() && previous_path != path {
                    fs::remove_file(previous_path)
                        .map_err(|error| Failure::new("IO_ERROR", format!("Template was saved, but the previous copy could not be removed: {error}")))?;
                }
            }
            Ok(json!({"filename":filename}))
        }
        "delete" => {
            let filename = payload["filename"].as_str().expect("validated filename");
            if !valid_phrase_filename(filename) {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Template identifier is invalid.",
                ));
            }
            fs::remove_file(directory.join(filename)).map_err(|_| {
                Failure::new("INVALID_INPUT", "Could not delete the selected template.")
            })?;
            Ok(json!({"deleted":true}))
        }
        _ => Err(Failure::new(
            "INVALID_INPUT",
            "Template action is not supported.",
        )),
    }
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

fn with_png_extension(path: PathBuf) -> PathBuf {
    if path
        .extension()
        .and_then(|extension| extension.to_str())
        .is_some_and(|extension| extension.eq_ignore_ascii_case("png"))
    {
        return path;
    }
    let mut path = path.into_os_string();
    path.push(".png");
    PathBuf::from(path)
}

fn clipboard_image_from_png(bytes: &[u8]) -> Result<tauri::image::Image<'static>, Failure> {
    let dimensions =
        image::ImageReader::with_format(std::io::Cursor::new(bytes), image::ImageFormat::Png)
            .into_dimensions()
            .map_err(|_| Failure::new("INVALID_INPUT", "Clipboard image is invalid."))?;
    if u64::from(dimensions.0) * u64::from(dimensions.1) > MAX_CLIPBOARD_IMAGE_PIXELS {
        return Err(Failure::new(
            "INVALID_INPUT",
            "Clipboard image exceeds the 100-million-pixel limit.",
        ));
    }

    let mut reader =
        image::ImageReader::with_format(std::io::Cursor::new(bytes), image::ImageFormat::Png);
    let mut limits = image::Limits::default();
    limits.max_image_width = Some(dimensions.0);
    limits.max_image_height = Some(dimensions.1);
    limits.max_alloc = Some(MAX_CLIPBOARD_IMAGE_PIXELS * 4 + 1024 * 1024);
    reader.limits(limits);
    let image = reader
        .decode()
        .map_err(|_| Failure::new("INVALID_INPUT", "Clipboard image is invalid."))?
        .into_rgba8();
    Ok(tauri::image::Image::new_owned(
        image.into_raw(),
        dimensions.0,
        dimensions.1,
    ))
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
        "convertTimestamp" => timestamp_result(
            payload["value"].as_str().expect("validated value"),
            payload["source"].as_str().expect("validated source"),
        ),
        "compareText" => {
            emit(
                app,
                json!({"type":"progress", "requestId":request.request_id, "percent":20, "message":"Comparing text"}),
            );
            let result = compare_text(
                payload["left"].as_str().expect("validated left"),
                payload["right"].as_str().expect("validated right"),
                cancelled,
            );
            if result.is_ok() {
                emit(
                    app,
                    json!({"type":"progress", "requestId":request.request_id, "percent":90, "message":"Building results"}),
                );
            }
            result
        }
        "convertData" => m5::convert_data(&request.payload),
        "renderMarkdown" => {
            m5::render_markdown(payload["markdown"].as_str().expect("validated Markdown"))
        }
        "commandData" => m5::command_data(&request.payload),
        "generateQr" => m5::generate_qr(&request.payload).map(|(result, _)| result),
        "saveQrImage" => {
            let (_, bytes) = m5::generate_qr(&request.payload)?;
            emit(
                app,
                json!({"type":"progress", "requestId":request.request_id, "percent":5, "message":"Waiting for file selection"}),
            );
            let Some(selection) = app
                .dialog()
                .file()
                .add_filter("PNG image", &["png"])
                .set_file_name("qrcode.png")
                .blocking_save_file()
            else {
                return Err(Failure::new("CANCELLED", "File selection was cancelled."));
            };
            if cancelled.load(Ordering::Relaxed) {
                return Err(Failure::new("CANCELLED", "Operation was cancelled."));
            }
            let path = with_png_extension(selected_path(selection)?);
            atomic_write(&path, &bytes)?;
            let file_name = path
                .file_name()
                .and_then(|name| name.to_str())
                .unwrap_or_default();
            Ok(json!({"fileName":file_name}))
        }
        "copyToClipboard" => {
            if let Some(text) = payload.get("text").and_then(Value::as_str) {
                app.clipboard()
                    .write_text(text)
                    .map_err(|_| Failure::new("IO_ERROR", "Could not copy to the clipboard."))?;
            } else {
                let data_url = payload["imageDataUrl"]
                    .as_str()
                    .expect("validated clipboard image");
                let encoded = data_url
                    .strip_prefix("data:image/png;base64,")
                    .ok_or_else(|| {
                        Failure::new("INVALID_INPUT", "Only PNG clipboard images are supported.")
                    })?;
                let bytes = base64::engine::general_purpose::STANDARD
                    .decode(encoded)
                    .map_err(|_| Failure::new("INVALID_INPUT", "Clipboard image is invalid."))?;
                if bytes.len() > MAX_OUTPUT_BYTES {
                    return Err(Failure::new(
                        "SIZE_LIMIT_EXCEEDED",
                        "Clipboard image exceeds the 4 MiB limit.",
                    ));
                }
                let image = clipboard_image_from_png(&bytes)?;
                app.clipboard()
                    .write_image(&image)
                    .map_err(|_| Failure::new("IO_ERROR", "Could not copy to the clipboard."))?;
            }
            Ok(json!({"copied":true}))
        }
        "evaluateRegex" => m5::evaluate_regex(&request.payload, cancelled),
        "regexSettings" => {
            let path = regex_settings_path(app)?;
            match payload["action"]
                .as_str()
                .expect("validated regex settings action")
            {
                "load" => {
                    let settings = match File::open(path) {
                        Ok(mut file) => {
                            if file
                                .metadata()
                                .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?
                                .len()
                                > MAX_SETTINGS_BYTES
                            {
                                return Err(Failure::new(
                                    "SIZE_LIMIT_EXCEEDED",
                                    "Regex settings file is too large.",
                                ));
                            }
                            let mut bytes = Vec::new();
                            file.read_to_end(&mut bytes)
                                .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
                            serde_json::from_slice::<RegexSettings>(&bytes).map_err(|_| {
                                Failure::new("INVALID_SETTINGS", "Regex settings file is invalid.")
                            })?
                        }
                        Err(error) if error.kind() == std::io::ErrorKind::NotFound => {
                            RegexSettings::default()
                        }
                        Err(error) => return Err(Failure::new("IO_ERROR", error.to_string())),
                    };
                    serde_json::to_value(settings)
                        .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))
                }
                "save" => {
                    let settings: RegexSettings = serde_json::from_value(request.payload.clone())
                        .map_err(|_| {
                        Failure::new("INVALID_INPUT", "Invalid regex settings.")
                    })?;
                    let bytes = serde_json::to_vec(&settings)
                        .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))?;
                    atomic_write(&path, &bytes)?;
                    Ok(json!({"saved":true}))
                }
                _ => Err(Failure::new(
                    "INVALID_INPUT",
                    "Regex settings action is not supported.",
                )),
            }
        }
        "phraseData" => phrase_operation(app, &request.payload),
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
            let mut value = serde_json::to_value(settings)
                .map_err(|error| Failure::new("INTERNAL_ERROR", error.to_string()))?;
            if let Some(object) = value.as_object_mut() {
                for key in ["windowWidth", "windowHeight", "windowX", "windowY"] {
                    object.remove(key);
                }
            }
            Ok(value)
        }
        "saveSettings" => {
            let update: SettingsUpdate = serde_json::from_value(request.payload.clone())
                .map_err(|_| Failure::new("INVALID_INPUT", "Invalid application settings."))?;
            update.validate()?;
            let always_on_top = update.always_on_top;
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
            if let Some(window) = app.get_webview_window("main") {
                window.set_always_on_top(always_on_top).map_err(|_| {
                    Failure::new("IO_ERROR", "Could not update the always-on-top setting.")
                })?;
            }
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
        "imageTakeDropped" | "imageOpen" | "imageEdit" | "imageReset" | "imageSave"
        | "imageSplit" => image_operation(app, state, request, cancelled),
        "databaseHistory" => database_history_result(app, state),
        "databaseSelectFile" => database_select_file(app, state, cancelled),
        "getAboutInfo" => Ok(json!({
            "appVersion": env!("CARGO_PKG_VERSION"),
            "revision": option_env!("DEVTOOLS_GIT_REVISION").unwrap_or("unknown"),
            "buildEnvironment": format!("Tauri preview ({}/{})", std::env::consts::OS, std::env::consts::ARCH),
            "runtime": "Tauri 2",
            "copyright": "2025 © Lotus&Company Inc. All rights reserved.",
            "licenseText": include_str!("../../LICENSE"),
        })),
        _ => Err(Failure::new(
            "UNSUPPORTED_OPERATION",
            "Unsupported operation.",
        )),
    }
}

fn image_operation(
    app: &AppHandle,
    state: &BridgeState,
    request: &Request,
    cancelled: &AtomicBool,
) -> Result<Value, Failure> {
    let _guard = match state.image_lock.try_lock() {
        Ok(guard) => guard,
        Err(TryLockError::WouldBlock) => {
            return Err(Failure::new(
                "TOO_MANY_REQUESTS",
                "Finish the current image operation first.",
            ));
        }
        Err(TryLockError::Poisoned(_)) => {
            return Err(Failure::new(
                "INTERNAL_ERROR",
                "Image operation lock was poisoned.",
            ));
        }
    };
    let progress = |percent, message| {
        emit(
            app,
            json!({"type":"progress", "requestId":request.request_id, "percent":percent, "message":message}),
        );
    };
    let open_image = |path: PathBuf| -> Result<Value, Failure> {
        if cancelled.load(Ordering::Relaxed) {
            return Err(Failure::new("CANCELLED", "Operation was cancelled."));
        }
        progress(20, "Reading image");
        let decoded = m5::load_image(&path)?;
        let image = Arc::new(decoded);
        let result = json!({
            "imageDataUrl":m5::image_preview_data_url(&image)?,
            "fileName":path.file_name().and_then(|name| name.to_str()).unwrap_or_default(),
            "width":image.width(), "height":image.height()
        });
        *state
            .image
            .lock()
            .map_err(|_| Failure::new("INTERNAL_ERROR", "Image state lock was poisoned."))? =
            Some(CurrentImage {
                current: image.clone(),
                original: image,
                path,
            });
        Ok(result)
    };
    match request.operation.as_str() {
        "imageTakeDropped" => {
            let path = state
                .dropped_image
                .lock()
                .map_err(|_| Failure::new("INTERNAL_ERROR", "Dropped image lock was poisoned."))?
                .take();
            let Some(path) = path else {
                return Ok(json!({"found":false}));
            };
            progress(5, "Reading dropped image");
            let mut result = open_image(path)?;
            result["found"] = Value::Bool(true);
            Ok(result)
        }
        "imageOpen" => {
            progress(5, "Waiting for file selection");
            let Some(selection) = app
                .dialog()
                .file()
                .add_filter("Images", &["png", "jpg", "jpeg", "bmp", "gif", "webp"])
                .blocking_pick_file()
            else {
                return Err(Failure::new("CANCELLED", "File selection was cancelled."));
            };
            if cancelled.load(Ordering::Relaxed) {
                return Err(Failure::new("CANCELLED", "Operation was cancelled."));
            }
            open_image(selected_path(selection)?)
        }
        "imageEdit" | "imageReset" | "imageSave" | "imageSplit" => {
            let current = state
                .image
                .lock()
                .map_err(|_| Failure::new("INTERNAL_ERROR", "Image state lock was poisoned."))?
                .clone()
                .ok_or_else(|| {
                    Failure::new(
                        "INVALID_INPUT",
                        "Open an image before using this operation.",
                    )
                })?;
            match request.operation.as_str() {
                "imageEdit" => {
                    progress(20, "Editing image");
                    let changed = Arc::new(m5::edit_image(
                        &current.current,
                        &request.payload,
                        cancelled,
                    )?);
                    progress(90, "Preparing image preview");
                    let preview = m5::image_preview_data_url(&changed)?;
                    let result = json!({"imageDataUrl":preview, "width":changed.width(), "height":changed.height()});
                    state
                        .image
                        .lock()
                        .map_err(|_| {
                            Failure::new("INTERNAL_ERROR", "Image state lock was poisoned.")
                        })?
                        .as_mut()
                        .expect("current image remains available")
                        .current = changed;
                    Ok(result)
                }
                "imageReset" => {
                    if cancelled.load(Ordering::Relaxed) {
                        return Err(Failure::new("CANCELLED", "Operation was cancelled."));
                    }
                    let image = current.original.clone();
                    let result = json!({"imageDataUrl":m5::image_preview_data_url(&image)?, "width":image.width(), "height":image.height()});
                    state
                        .image
                        .lock()
                        .map_err(|_| {
                            Failure::new("INTERNAL_ERROR", "Image state lock was poisoned.")
                        })?
                        .as_mut()
                        .expect("current image remains available")
                        .current = image;
                    Ok(result)
                }
                "imageSave" => {
                    let base = current
                        .path
                        .file_stem()
                        .and_then(|name| name.to_str())
                        .filter(|name| !name.is_empty())
                        .unwrap_or("image");
                    progress(5, "Waiting for file selection");
                    let Some(selection) = app
                        .dialog()
                        .file()
                        .add_filter("Images", &["png", "jpg", "jpeg", "bmp", "gif", "webp"])
                        .set_file_name(format!("{base}-edited.png"))
                        .blocking_save_file()
                    else {
                        return Err(Failure::new("CANCELLED", "File selection was cancelled."));
                    };
                    if cancelled.load(Ordering::Relaxed) {
                        return Err(Failure::new("CANCELLED", "Operation was cancelled."));
                    }
                    let path = selected_path(selection)?;
                    progress(30, "Saving image");
                    let bytes = m5::save_image_bytes(&current.current, &path)?;
                    atomic_write(&path, &bytes)?;
                    Ok(
                        json!({"fileName":path.file_name().and_then(|name| name.to_str()).unwrap_or_default()}),
                    )
                }
                "imageSplit" => {
                    progress(5, "Waiting for folder selection");
                    let Some(selection) = app
                        .dialog()
                        .file()
                        .set_title("Select save folder")
                        .blocking_pick_folder()
                    else {
                        return Err(Failure::new("CANCELLED", "Folder selection was cancelled."));
                    };
                    if cancelled.load(Ordering::Relaxed) {
                        return Err(Failure::new("CANCELLED", "Operation was cancelled."));
                    }
                    let folder = selected_path(selection)?;
                    progress(20, "Splitting image");
                    m5::split_image(
                        &current.current,
                        &current.path,
                        &folder,
                        &request.payload,
                        cancelled,
                    )?;
                    Ok(json!({"saved":true}))
                }
                _ => unreachable!(),
            }
        }
        _ => Err(Failure::new(
            "UNSUPPORTED_OPERATION",
            "Unsupported image operation.",
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
        let result = if request.operation == "sendHttpRequest" {
            let progress_app = app_for_work.clone();
            let progress_id = request_id.clone();
            send_http_request(&request.payload, &cancelled, |percent, message| {
                emit(
                    &progress_app,
                    json!({"type":"progress", "requestId":progress_id, "percent":percent, "message":message}),
                );
            })
            .await
        } else if matches!(
            request.operation.as_str(),
            "databaseConnect" | "databaseDisconnect" | "databaseQuery" | "databaseTables"
        ) {
            database_operation(&app_for_work, &state, &request, &cancelled).await
        } else {
            let work_app = app_for_work.clone();
            let work_state = state.clone();
            let work_cancelled = cancelled.clone();
            match tauri::async_runtime::spawn_blocking(move || {
                execute_operation(&work_app, &work_state, &request, &work_cancelled)
            })
            .await
            {
                Ok(result) => result,
                Err(error) => Err(Failure::new("INTERNAL_ERROR", error.to_string())),
            }
        };
        let event = match result {
            Ok(result) => json!({"type":"completed", "requestId":request_id, "result":result}),
            Err(failure) if failure.code == "CANCELLED" => {
                json!({"type":"cancelled", "requestId":request_id})
            }
            Err(failure) => {
                json!({"type":"failed", "requestId":request_id, "error":{"code":failure.code, "message":failure.message}})
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
        .plugin(tauri_plugin_clipboard_manager::init())
        .plugin(tauri_plugin_dialog::init())
        .setup(|app| {
            migrate_legacy_qsettings(app.handle()).map_err(|error| {
                eprintln!("Qt settings migration failed: {}", error.code);
                std::io::Error::other("Could not safely import legacy settings")
            })?;
            let config_dir = app.path().app_config_dir()?;
            if let Ok(settings) = load_settings(&config_dir.join("settings.json")) {
                if let Some(window) = app.get_webview_window("main") {
                    let _ = window.set_always_on_top(settings.always_on_top);
                    if settings.remember_window_size {
                        if let (Some(width), Some(height)) =
                            (settings.window_width, settings.window_height)
                        {
                            let _ = window.set_size(tauri::LogicalSize::new(
                                f64::from(width),
                                f64::from(height),
                            ));
                        }
                    }
                    if settings.remember_window_position {
                        if let (Some(x), Some(y)) = (settings.window_x, settings.window_y) {
                            let _ = window.set_position(tauri::LogicalPosition::new(
                                f64::from(x),
                                f64::from(y),
                            ));
                        }
                    }
                }
            }
            Ok(())
        })
        .on_window_event(|window, event| {
            if matches!(event, tauri::WindowEvent::CloseRequested { .. }) {
                if let Ok(state) = window.state::<BridgeState>().settings_lock.lock() {
                    if let Ok(path) = settings_path(window.app_handle()) {
                        if let Ok(mut settings) = load_settings(&path) {
                            if settings.remember_window_size {
                                if let (Ok(size), Ok(scale)) =
                                    (window.inner_size(), window.scale_factor())
                                {
                                    let size = size.to_logical::<f64>(scale);
                                    settings.window_width = Some(size.width.round() as u32);
                                    settings.window_height = Some(size.height.round() as u32);
                                }
                            }
                            if settings.remember_window_position {
                                if let (Ok(position), Ok(scale)) =
                                    (window.outer_position(), window.scale_factor())
                                {
                                    let position = position.to_logical::<f64>(scale);
                                    settings.window_x = Some(position.x.round() as i32);
                                    settings.window_y = Some(position.y.round() as i32);
                                }
                            }
                            if let Err(error) = save_settings(&path, &settings) {
                                eprintln!("Could not save window settings: {}", error.code);
                            }
                        }
                    }
                    drop(state);
                }
            }
            if let tauri::WindowEvent::DragDrop(tauri::DragDropEvent::Drop { paths, .. }) = event {
                let state = window.state::<BridgeState>();
                for path in paths {
                    match path
                        .extension()
                        .and_then(|value| value.to_str())
                        .map(str::to_ascii_lowercase)
                        .as_deref()
                    {
                        Some("db" | "sqlite" | "sqlite3") => {
                            if let Ok(mut database) = state.database.lock() {
                                database.selected_file = Some(path.clone());
                            }
                            emit(
                                window.app_handle(),
                                json!({"type":"navigate", "route":"db"}),
                            );
                        }
                        Some("png" | "jpg" | "jpeg" | "bmp" | "gif" | "webp") => {
                            if let Ok(mut dropped) = state.dropped_image.lock() {
                                *dropped = Some(path.clone());
                            }
                            emit(window.app_handle(), json!({"type":"imageDropAvailable"}));
                        }
                        _ => {}
                    }
                }
            }
        })
        .manage(BridgeState::default())
        .invoke_handler(tauri::generate_handler![start_operation, cancel_operation])
        .run(tauri::generate_context!())
        .expect("error while running DevTools Tauri host");
}

#[cfg(test)]
mod tests {
    use super::*;

    fn png_with_dimensions(width: u32, height: u32) -> Vec<u8> {
        let mut output = std::io::Cursor::new(Vec::new());
        image::DynamicImage::new_rgba8(1, 1)
            .write_to(&mut output, image::ImageFormat::Png)
            .unwrap();
        let mut png = output.into_inner();
        png[16..20].copy_from_slice(&width.to_be_bytes());
        png[20..24].copy_from_slice(&height.to_be_bytes());
        let checksum = png_crc32(&png[12..29]);
        png[29..33].copy_from_slice(&checksum.to_be_bytes());
        png
    }

    fn png_crc32(bytes: &[u8]) -> u32 {
        let mut checksum = !0_u32;
        for byte in bytes {
            checksum ^= u32::from(*byte);
            for _ in 0..8 {
                checksum = if checksum & 1 == 1 {
                    (checksum >> 1) ^ 0xedb8_8320
                } else {
                    checksum >> 1
                };
            }
        }
        !checksum
    }

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

    #[derive(Deserialize)]
    struct DiffFixture {
        input: DiffInput,
        expected: Value,
    }

    #[derive(Deserialize)]
    struct DiffInput {
        left: String,
        right: String,
    }

    #[test]
    fn compare_text_matches_shared_qt_contract_fixtures() {
        let fixtures: Vec<DiffFixture> =
            serde_json::from_str(include_str!("../../tests/fixtures/tauri-compare-text.json"))
                .expect("shared compare text fixtures are valid");
        assert!(!fixtures.is_empty());
        let cancelled = AtomicBool::new(false);
        for fixture in fixtures {
            assert_eq!(
                compare_text(&fixture.input.left, &fixture.input.right, &cancelled).unwrap(),
                fixture.expected
            );
        }
    }

    #[test]
    fn convert_timestamp_preserves_negative_milliseconds_and_timezone() {
        let negative = timestamp_result("-1", "milliseconds").unwrap();
        assert_eq!(negative["seconds"], "0");
        assert_eq!(negative["milliseconds"], "-1");
        assert_eq!(negative["utc"], "1969-12-31T23:59:59.999Z");

        let offset = timestamp_result("2026-01-02T03:04:05.006+09:00", "iso").unwrap();
        assert_eq!(offset["utc"], "2026-01-01T18:04:05.006Z");
        assert_eq!(offset["milliseconds"], "1767290645006");
    }

    #[test]
    fn convert_timestamp_matches_shared_qt_fixture() {
        let cases: Vec<Value> =
            serde_json::from_str(include_str!("../../tests/fixtures/tauri-timestamp.json"))
                .unwrap();
        for case in cases {
            if case.get("timeZone").is_some()
                && std::env::var("TZ").ok().as_deref() != case["timeZone"].as_str()
            {
                continue;
            }
            match case.get("errorCode") {
                Some(code) => assert_eq!(
                    timestamp_result(
                        case["input"]["value"].as_str().unwrap(),
                        case["input"]["source"].as_str().unwrap()
                    )
                    .unwrap_err()
                    .code,
                    code.as_str().unwrap(),
                    "{}",
                    case["name"]
                ),
                None => {
                    let result = timestamp_result(
                        case["input"]["value"].as_str().unwrap(),
                        case["input"]["source"].as_str().unwrap(),
                    )
                    .unwrap();
                    for key in ["seconds", "milliseconds", "utc"] {
                        assert_eq!(
                            result[key], case["expected"][key],
                            "{}: {key}",
                            case["name"]
                        );
                    }
                    assert!(chrono::DateTime::parse_from_rfc3339(
                        result["local"].as_str().unwrap()
                    )
                    .is_ok());
                }
            }
        }
    }

    fn local_http_payload(url: String) -> Value {
        json!({
            "method":"GET",
            "url":url,
            "headers":{},
            "username":"",
            "password":"",
            "body":"",
            "parameters":[{"key":"q","value":"hello world","description":""}]
        })
    }

    #[tokio::test]
    async fn sends_http_requests_with_query_parameters_to_a_real_local_server() {
        use std::io::{BufRead, BufReader, Write};
        use std::net::TcpListener;
        use std::thread;

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let address = listener.local_addr().unwrap();
        let server = thread::spawn(move || {
            let (mut stream, _) = listener.accept().unwrap();
            let mut reader = BufReader::new(stream.try_clone().unwrap());
            let mut request_line = String::new();
            reader.read_line(&mut request_line).unwrap();
            assert!(request_line.contains("/path?q=hello%20world"));
            let mut line = String::new();
            loop {
                line.clear();
                reader.read_line(&mut line).unwrap();
                if line == "\r\n" || line.is_empty() {
                    break;
                }
            }
            let body = "response";
            write!(
                stream,
                "HTTP/1.1 200 OK\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{}",
                body.len(),
                body
            )
            .unwrap();
        });
        let payload = local_http_payload(format!("http://{address}/path?existing=1"));
        let cancelled = AtomicBool::new(false);
        let result = send_http_request(&payload, &cancelled, |_, _| {})
            .await
            .unwrap();
        assert_eq!(result["status"], 200);
        assert_eq!(result["body"], "response");
        assert_eq!(result["bytes"], 8);
        assert_eq!(result["errorCode"], 0);
        server.join().unwrap();
    }

    #[tokio::test]
    async fn cancellation_aborts_an_active_http_request() {
        use std::io::{BufRead, BufReader, Write};
        use std::net::TcpListener;
        use std::thread;
        use std::time::Duration;

        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let address = listener.local_addr().unwrap();
        let server = thread::spawn(move || {
            let (mut stream, _) = listener.accept().unwrap();
            let mut reader = BufReader::new(stream.try_clone().unwrap());
            let mut line = String::new();
            loop {
                line.clear();
                reader.read_line(&mut line).unwrap();
                if line == "\r\n" || line.is_empty() {
                    break;
                }
            }
            thread::sleep(Duration::from_millis(250));
            let _ = write!(stream, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
        });
        let cancelled = Arc::new(AtomicBool::new(false));
        let cancel_after_start = cancelled.clone();
        tokio::spawn(async move {
            tokio::time::sleep(Duration::from_millis(20)).await;
            cancel_after_start.store(true, Ordering::Relaxed);
        });
        let result = send_http_request(
            &local_http_payload(format!("http://{address}/slow")),
            &cancelled,
            |_, _| {},
        )
        .await
        .unwrap_err();
        assert_eq!(result.code, "CANCELLED");
        server.join().unwrap();
    }

    #[tokio::test]
    async fn executes_sqlite_queries_and_keeps_transaction_state() {
        let directory = tempfile::tempdir().unwrap();
        let path = directory.path().join("日本語.sqlite");
        File::create(&path).unwrap();
        let cancelled = AtomicBool::new(false);
        let pool = open_database_pool("sqlite", "", path.to_str().unwrap(), "", "", &cancelled)
            .await
            .unwrap();

        database_query(
            &pool,
            "sqlite",
            "CREATE TABLE items (value INTEGER, label TEXT, optional TEXT)",
            &cancelled,
        )
        .await
        .unwrap();
        database_query(
            &pool,
            "sqlite",
            "INSERT INTO items VALUES (9223372036854775807, '日本語', NULL)",
            &cancelled,
        )
        .await
        .unwrap();
        database_query(&pool, "sqlite", "BEGIN", &cancelled)
            .await
            .unwrap();
        database_query(&pool, "sqlite", "UPDATE items SET value = 1", &cancelled)
            .await
            .unwrap();
        database_query(&pool, "sqlite", "ROLLBACK", &cancelled)
            .await
            .unwrap();

        let result = database_query(
            &pool,
            "sqlite",
            "SELECT value, label, optional FROM items",
            &cancelled,
        )
        .await
        .unwrap();
        assert_eq!(result["columns"], json!(["value", "label", "optional"]));
        assert_eq!(
            result["rows"],
            json!([["9223372036854775807", "日本語", null]])
        );
        assert_eq!(result["affectedRows"], 0);
        assert_eq!(
            database_tables(&pool, "sqlite", &cancelled).await.unwrap(),
            ["items"]
        );
        let fixtures: Vec<Value> =
            serde_json::from_str(include_str!("../../tests/fixtures/tauri-sqlite-query.json"))
                .unwrap();
        for fixture in fixtures {
            let result = database_query(
                &pool,
                "sqlite",
                fixture["input"]["sql"].as_str().unwrap(),
                &cancelled,
            )
            .await
            .unwrap();
            assert_eq!(result, fixture["expected"]);
        }
        pool.close().await;
    }

    #[tokio::test]
    async fn connects_to_local_mysql_and_postgres_when_configured() {
        let configurations = [
            (
                "mysql",
                "DEVTOOLS_TEST_MYSQL_HOST",
                "DEVTOOLS_TEST_MYSQL_DATABASE",
                "DEVTOOLS_TEST_MYSQL_USERNAME",
                "DEVTOOLS_TEST_MYSQL_PASSWORD",
            ),
            (
                "postgres",
                "DEVTOOLS_TEST_POSTGRES_HOST",
                "DEVTOOLS_TEST_POSTGRES_DATABASE",
                "DEVTOOLS_TEST_POSTGRES_USERNAME",
                "DEVTOOLS_TEST_POSTGRES_PASSWORD",
            ),
        ];
        let cancelled = AtomicBool::new(false);
        let mut connected = false;
        for (driver, host_key, database_key, username_key, password_key) in configurations {
            let host = std::env::var(host_key).unwrap_or_default();
            let database = std::env::var(database_key).unwrap_or_default();
            let username = std::env::var(username_key).unwrap_or_default();
            let password = std::env::var(password_key).unwrap_or_default();
            if host.is_empty() || database.is_empty() || username.is_empty() {
                continue;
            }
            connected = true;
            let pool =
                open_database_pool(driver, &host, &database, &username, &password, &cancelled)
                    .await
                    .expect("configured database server should accept a connection");
            let version = database_query(&pool, driver, "SELECT VERSION()", &cancelled)
                .await
                .expect("server version query should succeed");
            assert!(!version["rows"].as_array().unwrap().is_empty());

            for sql in [
                "CREATE TEMPORARY TABLE devtools_review_session (value INTEGER NOT NULL)",
                "INSERT INTO devtools_review_session VALUES (0)",
                "BEGIN",
                "UPDATE devtools_review_session SET value = 1",
                "ROLLBACK",
            ] {
                database_query(&pool, driver, sql, &cancelled)
                    .await
                    .unwrap_or_else(|error| panic!("{driver} query {sql:?} failed: {error:?}"));
            }
            let result = database_query(
                &pool,
                driver,
                "SELECT value FROM devtools_review_session",
                &cancelled,
            )
            .await
            .expect("rollback result query should succeed");
            assert_eq!(result["rows"], json!([["0"]]));
            pool.close().await;
        }
        if !connected {
            eprintln!("Skipping MySQL and PostgreSQL integration; no DEVTOOLS_TEST_* values set.");
        }
    }

    #[test]
    fn maps_http_status_errors_to_qt_bridge_codes() {
        assert_eq!(http_status_error_code(401), 204);
        assert_eq!(http_status_error_code(403), 201);
        assert_eq!(http_status_error_code(404), 203);
        assert_eq!(http_status_error_code(500), 399);
    }

    #[test]
    fn clipboard_png_checks_dimensions_before_decoding_pixels() {
        let (_, png) = m5::generate_qr(&json!({
            "type":"text",
            "parameters":{"text_content":"clipboard check"}
        }))
        .unwrap();
        assert!(clipboard_image_from_png(&png).is_ok());

        let oversized = png_with_dimensions(10_001, 10_000);
        let error = clipboard_image_from_png(&oversized).unwrap_err();
        assert_eq!(error.code, "INVALID_INPUT");
        assert_eq!(
            error.message,
            "Clipboard image exceeds the 100-million-pixel limit."
        );
    }

    #[test]
    fn qr_save_paths_keep_or_append_png_suffix() {
        assert_eq!(
            with_png_extension(PathBuf::from("qrcode")),
            PathBuf::from("qrcode.png")
        );
        assert_eq!(
            with_png_extension(PathBuf::from("qrcode.jpg")),
            PathBuf::from("qrcode.jpg.png")
        );
        assert_eq!(
            with_png_extension(PathBuf::from("qrcode.PNG")),
            PathBuf::from("qrcode.PNG")
        );
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
            "contractVersion":"2", "requestId":"abc123", "operation":"notImplemented", "payload":{}
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
    fn reads_text_file_using_shared_contract_fixture() {
        let fixture: Value = serde_json::from_str(include_str!(
            "../../tests/fixtures/tauri-open-text-file.json"
        ))
        .expect("shared text-file contract fixture is valid");
        let input = &fixture["input"];
        let directory = tempfile::tempdir().unwrap();
        let path = directory.path().join(input["fileName"].as_str().unwrap());
        fs::write(&path, input["text"].as_str().unwrap()).unwrap();

        assert_eq!(read_text_file(&path).unwrap(), fixture["expected"]);
    }

    #[test]
    fn migrates_legacy_phrase_files_once_without_overwriting_or_removing_sources() {
        let directory = tempfile::tempdir().unwrap();
        let legacy = directory.path().join("content");
        let target = directory.path().join("app-data/content");
        fs::create_dir_all(&legacy).unwrap();
        fs::write(
            legacy.join("my template.txt"),
            "挨拶\nこんにちは、{name}さん",
        )
        .unwrap();
        fs::write(legacy.join("kept.txt"), "new\nlegacy value").unwrap();
        fs::write(legacy.join("ignored.md"), "ignore").unwrap();
        fs::create_dir_all(&target).unwrap();
        fs::write(target.join("kept.txt"), "newer\nexisting value").unwrap();

        migrate_phrase_templates(&legacy, &target).unwrap();
        assert_eq!(
            read_phrase_file(&target.join("my template.txt")).unwrap()["content"],
            "こんにちは、{name}さん"
        );
        assert_eq!(
            read_phrase_file(&target.join("kept.txt")).unwrap()["title"],
            "newer"
        );
        assert!(!target.join("ignored.md").exists());
        assert_eq!(
            fs::read_to_string(legacy.join("my template.txt")).unwrap(),
            "挨拶\nこんにちは、{name}さん"
        );

        fs::remove_file(target.join("my template.txt")).unwrap();
        migrate_phrase_templates(&legacy, &target).unwrap();
        assert!(!target.join("my template.txt").exists());
    }

    #[test]
    fn phrase_filenames_are_safe_single_txt_names() {
        assert!(valid_phrase_filename("legacy note.txt"));
        assert!(!valid_phrase_filename("../template.txt"));
        assert!(!valid_phrase_filename("folder\\template.txt"));
        assert!(!valid_phrase_filename("template.md"));
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

    #[test]
    fn database_history_normalizes_legacy_qt_driver_names() {
        let directory = tempfile::tempdir().unwrap();
        let path = directory.path().join("database-history.json");
        fs::write(
            &path,
            br#"[{"type":"QSQLITE","host":"","database":"sample.db","username":"","displayName":"SQLite: sample.db"},{"type":"QMYSQL","host":"localhost","database":"app","username":"user","displayName":"MySQL: user@localhost/app"},{"type":"QPSQL","host":"localhost","database":"app","username":"user","displayName":"PostgreSQL: user@localhost/app"}]"#,
        )
        .unwrap();

        let history = read_database_history(&path).unwrap();
        assert_eq!(
            history
                .iter()
                .map(|entry| entry.driver.as_str())
                .collect::<Vec<_>>(),
            ["sqlite", "mysql", "postgres"]
        );
    }

    #[test]
    fn migrates_allowlisted_qt_settings_and_preserves_the_source_backup() {
        let directory = tempfile::tempdir().unwrap();
        let config = directory.path().join("config");
        let legacy = directory.path().join("Qt.plist");
        let fixture = include_bytes!("../../tests/fixtures/qt-settings-migration.plist");
        fs::write(&legacy, fixture).unwrap();

        migrate_qsettings_file(&config, &legacy).unwrap();
        let settings = load_settings(&config.join("settings.json")).unwrap();
        assert_eq!(settings.language, "ja_JP");
        assert_eq!(settings.theme, "dark");
        assert!(!settings.show_sidebar_on_startup);
        assert!(settings.show_last_tool_on_startup);
        assert_eq!(settings.last_used_tool, Some(4));
        assert_eq!(settings.window_width, Some(1280));
        assert_eq!(settings.window_height, Some(800));
        assert!(!settings.remember_window_position);

        let regex: RegexSettings =
            serde_json::from_slice(&fs::read(config.join("regex-settings.json")).unwrap()).unwrap();
        assert_eq!(regex.pattern, "(?<日本語>猫)");
        assert_eq!(regex.test_text, "🍣猫");
        assert!(regex.options.global);
        assert!(regex.options.case_insensitive);
        assert!(regex.options.dot_matches_everything);

        let history = read_database_history(&config.join("database-history.json")).unwrap();
        assert_eq!(history.len(), 2);
        assert_eq!(history[0].driver, "sqlite");
        assert_eq!(history[1].driver, "postgres");
        let stored = fs::read_to_string(config.join("database-history.json")).unwrap();
        for excluded in ["password", "bookmark", "futureField", "must-not-copy"] {
            assert!(!stored.contains(excluded));
        }

        let backup = config.join("migration/qsettings-source.plist");
        assert_eq!(fs::read(backup).unwrap(), fixture);
        assert_eq!(fs::read(&legacy).unwrap(), fixture);
        let settings_path = config.join("settings.json");
        let mut user_settings = load_settings(&settings_path).unwrap();
        user_settings.language = "en".into();
        save_settings(&settings_path, &user_settings).unwrap();
        migrate_qsettings_file(&config, &legacy).unwrap();
        assert_eq!(load_settings(&settings_path).unwrap().language, "en");
    }

    #[test]
    fn reports_invalid_qt_settings_after_backing_up_the_source() {
        let directory = tempfile::tempdir().unwrap();
        let config = directory.path().join("config");
        let legacy = directory.path().join("Qt.plist");
        fs::write(&legacy, b"not a plist").unwrap();

        let error = migrate_qsettings_file(&config, &legacy).unwrap_err();
        assert_eq!(error.code, "INVALID_SETTINGS");
        assert_eq!(
            fs::read(config.join("migration/qsettings-source.plist")).unwrap(),
            b"not a plist"
        );
        assert!(!config.join("settings.json").exists());
    }
}
