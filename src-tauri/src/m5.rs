use crate::Failure;
use base64::engine::general_purpose::STANDARD;
use base64::Engine;
use fancy_regex::{Captures, RegexBuilder};
use image::{DynamicImage, GrayImage, ImageBuffer, ImageFormat, Luma};
use pulldown_cmark::{html, Options, Parser};
use qrcode::{EcLevel, QrCode};
use serde::de::{MapAccess, SeqAccess, Visitor};
use serde::ser::{SerializeMap, SerializeSeq};
use serde::{Deserialize, Deserializer, Serialize};
use serde_json::{json, Map, Number, Value};
use std::fmt;
use std::fs;
use std::io::Cursor;
use std::path::Path;

const MAX_CONVERTED_OUTPUT_BYTES: usize = 1024 * 1024;
const MAX_REGEX_MATCHES: usize = 10_000;
const MAX_REGEX_OUTPUT_BYTES: usize = 4 * 1024 * 1024;
const MAX_QR_CONTENT_BYTES: usize = 2_953;

pub(crate) fn convert_data(payload: &Value) -> Result<Value, Failure> {
    let input = payload["input"].as_str().expect("validated input");
    if input.trim().is_empty() {
        return Ok(json!({"output":"", "message":""}));
    }
    let (data, _) = parse_data(input)
        .ok_or_else(|| Failure::new("INVALID_INPUT", "Cannot parse input text"))?;
    let indentation = payload["indentation"]
        .as_str()
        .expect("validated indentation");
    let mut message = String::new();
    let output = match payload["format"].as_str().expect("validated format") {
        "json" => {
            if !data.is_array() && !data.is_object() {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "JSON does not support top level value",
                ));
            }
            crate::format_json(
                &serde_json::to_string(&data).expect("JSON value serializes"),
                indentation,
            )?
        }
        "yaml-block" => {
            if indentation == "tab" {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "YAML does not support tab indentation",
                ));
            }
            let mut options = serde_saphyr::SerializerOptions::default();
            options.indent_step = match indentation {
                "4" => 4,
                "minified" => 1,
                _ => 2,
            };
            options.no_lang_directive = true;
            options.compact_list_indent = false;
            serde_saphyr::to_string_with_options(&YamlOutputValue(&data), options)
                .map_err(|_| Failure::new("INVALID_INPUT", "Could not serialize YAML."))?
        }
        "yaml-flow" => format!("{}\n", flow_yaml(&data)),
        "toml" => {
            let mut warnings = Vec::new();
            let Some(value) = json_to_toml(&data, "", &mut warnings)? else {
                return Err(Failure::new("INVALID_INPUT", "Invalid input value"));
            };
            if !value.is_table() {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "TOML does not support top level array",
                ));
            }
            message = warnings.join("\n");
            serialize_toml(&value, indentation)?
        }
        _ => {
            return Err(Failure::new(
                "INVALID_INPUT",
                "Output format must be JSON, YAML or TOML.",
            ))
        }
    };
    if output.len() > MAX_CONVERTED_OUTPUT_BYTES {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Converted output is larger than 1 MiB.",
        ));
    }
    Ok(json!({"output":output, "message":message}))
}

fn parse_data(input: &str) -> Option<(Value, &'static str)> {
    serde_json::from_str(input)
        .map(|value| (value, "JSON"))
        .or_else(|_| {
            toml::from_str::<toml::Value>(input).map(|value| (toml_to_json(value), "TOML"))
        })
        .or_else(|_| parse_yaml(input).map(|value| (value, "YAML")))
        .ok()
}

fn parse_yaml(input: &str) -> Result<Value, serde_saphyr::Error> {
    serde_saphyr::from_str::<YamlValue>(input).map(|value| value.0)
}

struct YamlValue(Value);

struct YamlOutputValue<'a>(&'a Value);

impl Serialize for YamlOutputValue<'_> {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        match self.0 {
            Value::Null => serializer.serialize_unit(),
            Value::Bool(value) => serializer.serialize_bool(*value),
            Value::Number(value) => {
                if let Some(value) = value.as_i64() {
                    return serializer.serialize_i64(value);
                }
                if let Some(value) = value.as_u64() {
                    return serializer.serialize_u64(value);
                }
                let value = value.to_string();
                if let Ok(value) = value.parse::<i128>() {
                    return serializer.serialize_i128(value);
                }
                if let Ok(value) = value.parse::<u128>() {
                    return serializer.serialize_u128(value);
                }
                if value
                    .bytes()
                    .all(|byte| byte.is_ascii_digit() || byte == b'-')
                {
                    return Err(serde::ser::Error::custom(
                        "Integer is outside the supported YAML range.",
                    ));
                }
                let value = value.parse::<f64>().map_err(serde::ser::Error::custom)?;
                serializer.serialize_f64(value)
            }
            Value::String(value) => serializer.serialize_str(value),
            Value::Array(values) => {
                let mut sequence = serializer.serialize_seq(Some(values.len()))?;
                for value in values {
                    sequence.serialize_element(&YamlOutputValue(value))?;
                }
                sequence.end()
            }
            Value::Object(values) => {
                let mut map = serializer.serialize_map(Some(values.len()))?;
                for (key, value) in values {
                    map.serialize_entry(key, &YamlOutputValue(value))?;
                }
                map.end()
            }
        }
    }
}

impl<'de> Deserialize<'de> for YamlValue {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        struct YamlValueVisitor;

        impl<'de> Visitor<'de> for YamlValueVisitor {
            type Value = YamlValue;

            fn expecting(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
                formatter.write_str("a YAML value")
            }

            fn visit_bool<E>(self, value: bool) -> Result<Self::Value, E> {
                Ok(YamlValue(Value::String(value.to_string())))
            }

            fn visit_i64<E>(self, value: i64) -> Result<Self::Value, E> {
                Ok(YamlValue(Value::String(value.to_string())))
            }

            fn visit_u64<E>(self, value: u64) -> Result<Self::Value, E> {
                Ok(YamlValue(Value::String(value.to_string())))
            }

            fn visit_f64<E>(self, value: f64) -> Result<Self::Value, E> {
                Ok(YamlValue(Value::String(value.to_string())))
            }

            fn visit_str<E>(self, value: &str) -> Result<Self::Value, E> {
                Ok(YamlValue(Value::String(value.to_owned())))
            }

            fn visit_string<E>(self, value: String) -> Result<Self::Value, E> {
                Ok(YamlValue(Value::String(value)))
            }

            fn visit_none<E>(self) -> Result<Self::Value, E> {
                Ok(YamlValue(Value::Null))
            }

            fn visit_unit<E>(self) -> Result<Self::Value, E> {
                Ok(YamlValue(Value::Null))
            }

            fn visit_seq<A>(self, mut sequence: A) -> Result<Self::Value, A::Error>
            where
                A: SeqAccess<'de>,
            {
                let mut values = Vec::new();
                while let Some(value) = sequence.next_element::<YamlValue>()? {
                    values.push(value.0);
                }
                Ok(YamlValue(Value::Array(values)))
            }

            fn visit_map<A>(self, mut map: A) -> Result<Self::Value, A::Error>
            where
                A: MapAccess<'de>,
            {
                let mut values = Map::new();
                while let Some(YamlScalar(key)) = map.next_key::<YamlScalar>()? {
                    values.insert(key, map.next_value::<YamlValue>()?.0);
                }
                Ok(YamlValue(Value::Object(values)))
            }
        }

        deserializer.deserialize_any(YamlValueVisitor)
    }
}

struct YamlScalar(String);

impl<'de> Deserialize<'de> for YamlScalar {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        struct YamlScalarVisitor;

        impl Visitor<'_> for YamlScalarVisitor {
            type Value = YamlScalar;

            fn expecting(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
                formatter.write_str("a scalar YAML key")
            }

            fn visit_bool<E>(self, value: bool) -> Result<Self::Value, E> {
                Ok(YamlScalar(value.to_string()))
            }

            fn visit_i64<E>(self, value: i64) -> Result<Self::Value, E> {
                Ok(YamlScalar(value.to_string()))
            }

            fn visit_u64<E>(self, value: u64) -> Result<Self::Value, E> {
                Ok(YamlScalar(value.to_string()))
            }

            fn visit_f64<E>(self, value: f64) -> Result<Self::Value, E> {
                Ok(YamlScalar(value.to_string()))
            }

            fn visit_str<E>(self, value: &str) -> Result<Self::Value, E> {
                Ok(YamlScalar(value.to_owned()))
            }

            fn visit_string<E>(self, value: String) -> Result<Self::Value, E> {
                Ok(YamlScalar(value))
            }
        }

        deserializer.deserialize_any(YamlScalarVisitor)
    }
}

fn toml_to_json(value: toml::Value) -> Value {
    match value {
        toml::Value::String(value) => Value::String(value),
        toml::Value::Integer(value) => json!(value),
        toml::Value::Float(value) => Number::from_f64(value).map_or(Value::Null, Value::Number),
        toml::Value::Boolean(value) => Value::Bool(value),
        toml::Value::Datetime(value) => Value::String(format_toml_datetime(value)),
        toml::Value::Array(values) => Value::Array(values.into_iter().map(toml_to_json).collect()),
        toml::Value::Table(values) => Value::Object(
            values
                .into_iter()
                .map(|(key, value)| (key, toml_to_json(value)))
                .collect(),
        ),
    }
}

fn format_toml_datetime(value: toml::value::Datetime) -> String {
    let date = value
        .date
        .map(|date| format!("{:04}-{:02}-{:02}", date.year, date.month, date.day));
    let time = value.time.map(|time| {
        format!(
            "{:02}:{:02}:{:02}.{:03}",
            time.hour,
            time.minute,
            time.second.unwrap_or_default(),
            time.nanosecond.unwrap_or_default() / 1_000_000
        )
    });
    match (date, time, value.offset) {
        (Some(date), Some(time), Some(offset)) => {
            let minutes = match offset {
                toml::value::Offset::Z => 0,
                toml::value::Offset::Custom { minutes } => minutes,
            };
            format!(
                "{date}T{time}{}{:02}:{:02}",
                if minutes < 0 { '-' } else { '+' },
                minutes.unsigned_abs() / 60,
                minutes.unsigned_abs() % 60
            )
        }
        (Some(date), Some(time), None) => format!("{date}T{time}"),
        (Some(date), None, None) => date,
        (None, Some(time), None) => time,
        _ => String::new(),
    }
}

fn json_to_toml(
    value: &Value,
    key: &str,
    warnings: &mut Vec<String>,
) -> Result<Option<toml::Value>, Failure> {
    match value {
        Value::Null => Ok(None),
        Value::Bool(value) => Ok(Some(toml::Value::Boolean(*value))),
        Value::Number(value) => {
            if let Some(value) = value.as_i64() {
                Ok(Some(toml::Value::Integer(value)))
            } else if let Some(value) = value.as_u64().and_then(|n| i64::try_from(n).ok()) {
                Ok(Some(toml::Value::Integer(value)))
            } else if value
                .to_string()
                .bytes()
                .all(|byte| byte.is_ascii_digit() || byte == b'-')
            {
                Err(Failure::new(
                    "INVALID_INPUT",
                    "TOML integer is outside the supported range.",
                ))
            } else if let Some(value) = value.as_f64() {
                Ok(Some(toml::Value::Float(value)))
            } else {
                Err(Failure::new(
                    "INVALID_INPUT",
                    "TOML number is outside the supported range.",
                ))
            }
        }
        Value::String(value) => Ok(Some(toml::Value::String(value.clone()))),
        Value::Array(values) => {
            let mut items = Vec::with_capacity(values.len());
            for value in values {
                if let Some(value) = json_to_toml(value, key, warnings)? {
                    items.push(value);
                } else {
                    warnings.push(format!("ignore null value in array: {key}"));
                }
            }
            Ok(Some(toml::Value::Array(items)))
        }
        Value::Object(values) => {
            let mut table = toml::map::Map::new();
            for (name, value) in values {
                let child_key = if key.is_empty() {
                    name.clone()
                } else {
                    format!("{key}.{name}")
                };
                if value.is_null() {
                    warnings.push(format!("ignore null value of key: {child_key}"));
                    continue;
                }
                if let Some(value) = json_to_toml(value, &child_key, warnings)? {
                    table.insert(name.clone(), value);
                }
            }
            Ok(Some(toml::Value::Table(table)))
        }
    }
}

fn serialize_toml(value: &toml::Value, indentation: &str) -> Result<String, Failure> {
    let mut output = if indentation == "minified" {
        toml::to_string(value)
    } else {
        toml::to_string_pretty(value)
    }
    .map_err(|_| Failure::new("INVALID_INPUT", "Could not serialize TOML."))?;

    let mut blocks: Vec<(usize, String, String)> = Vec::new();
    let mut block = String::new();
    for line in output.lines() {
        if !line.starts_with([' ', '\t']) && !line.is_empty() && !block.is_empty() {
            let (rank, key) = toml_block_order(value, &block);
            blocks.push((rank, key, std::mem::take(&mut block)));
        }
        block.push_str(line);
        block.push('\n');
    }
    if !block.is_empty() {
        let (rank, key) = toml_block_order(value, &block);
        blocks.push((rank, key, block));
    }
    blocks.sort_by(|left, right| (left.0, &left.1).cmp(&(right.0, &right.1)));
    output = blocks.into_iter().map(|(_, _, block)| block).collect();

    if indentation != "4" && indentation != "minified" {
        let unit = if indentation == "tab" { "\t" } else { "  " };
        output = output
            .lines()
            .map(|line| {
                let spaces = line.bytes().take_while(|byte| *byte == b' ').count();
                format!("{}{}\n", unit.repeat(spaces / 4), &line[spaces..])
            })
            .collect();
    }
    Ok(output)
}

fn toml_block_order(value: &toml::Value, block: &str) -> (usize, String) {
    let first_line = block.lines().next().unwrap_or_default();
    let key = if first_line.starts_with('[') {
        first_line.trim_start_matches('[').trim_end_matches(']')
    } else {
        first_line
            .split_once(" = ")
            .map_or(first_line, |(key, _)| key)
    };
    let Some((name, child)) = value
        .as_table()
        .and_then(|table| table.iter().find(|(name, _)| name.as_str() == key))
    else {
        return (3, key.to_owned());
    };
    let rank = match child {
        toml::Value::Table(_) => 2,
        toml::Value::Array(_) => 1,
        _ => 0,
    };
    (rank, name.clone())
}

fn flow_yaml(value: &Value) -> String {
    match value {
        Value::Null => "null".into(),
        Value::Bool(value) => value.to_string(),
        Value::Number(value) => value.to_string(),
        Value::String(value) => serde_json::to_string(value).expect("string serializes"),
        Value::Array(values) => format!(
            "[{}]",
            values.iter().map(flow_yaml).collect::<Vec<_>>().join(", ")
        ),
        Value::Object(values) => format!(
            "{{{}}}",
            values
                .iter()
                .map(|(key, value)| format!(
                    "{}: {}",
                    serde_json::to_string(key).expect("key serializes"),
                    flow_yaml(value)
                ))
                .collect::<Vec<_>>()
                .join(", ")
        ),
    }
}

pub(crate) fn render_markdown(markdown: &str) -> Result<Value, Failure> {
    let mut options = Options::empty();
    options.insert(Options::ENABLE_TABLES);
    options.insert(Options::ENABLE_FOOTNOTES);
    options.insert(Options::ENABLE_STRIKETHROUGH);
    options.insert(Options::ENABLE_TASKLISTS);
    options.insert(Options::ENABLE_HEADING_ATTRIBUTES);
    let mut rendered = String::new();
    html::push_html(&mut rendered, Parser::new_ext(markdown, options));
    let clean = ammonia::clean(&rendered);
    Ok(
        json!({"html": format!("<!doctype html>\n<html><head><meta charset=\"utf-8\"></head><body>{clean}</body></html>")}),
    )
}

pub(crate) fn command_data(payload: &Value) -> Result<Value, Failure> {
    let category = payload["category"].as_str().expect("validated category");
    let functions = command_functions(category)
        .ok_or_else(|| Failure::new("INVALID_INPUT", "Command category is not supported."))?;
    if payload["action"] == "list" {
        return Ok(json!({"functions":functions}));
    }
    let function_index = payload["functionIndex"]
        .as_u64()
        .ok_or_else(|| Failure::new("INVALID_INPUT", "Select a valid command and option."))?
        as usize;
    let option_index = payload["optionIndex"]
        .as_u64()
        .ok_or_else(|| Failure::new("INVALID_INPUT", "Select a valid command and option."))?
        as usize;
    let value = payload["value"].as_str().expect("validated value");
    if value.contains(['"', '\'', '`']) {
        return Err(Failure::new(
            "INVALID_INPUT",
            "Quotes are not allowed in command values.",
        ));
    }
    let function = functions
        .get(function_index)
        .ok_or_else(|| Failure::new("INVALID_INPUT", "Select a command and option."))?;
    let option = function["options"]
        .as_array()
        .and_then(|options| options.get(option_index))
        .ok_or_else(|| Failure::new("INVALID_INPUT", "Select a command and option."))?;
    let mut command = function["name"].as_str().unwrap_or_default().to_owned();
    if option["name"] != "None" {
        command.push(' ');
        command.push_str(option["name"].as_str().unwrap_or_default());
    }
    if option["required"] == true {
        command.push(' ');
        if option["quotes"] == true {
            command.push('"');
            command.push_str(value);
            command.push('"');
        } else {
            command.push_str(value);
        }
    }
    Ok(json!({"command":command}))
}

fn command_functions(category: &str) -> Option<Vec<Value>> {
    let none = || json!({"name":"None", "required":false, "title":"", "quotes":false});
    let required = |name: &str, title: &str, quotes: bool| json!({"name":name, "required":true, "title":title, "quotes":quotes});
    let optional = |name: &str| json!({"name":name, "required":false, "title":"", "quotes":false});
    let function = |name: &str, description: &str, options: Vec<Value>| json!({"name":name, "description":description, "options":options});
    Some(match category {
        "git" => vec![
            function(
                "git add",
                "Add changes",
                vec![required("None", "Path", false)],
            ),
            function(
                "git commit",
                "Create a commit",
                vec![none(), required("-m", "Message", true), optional("-a")],
            ),
            function("git log", "Show commit history", vec![none()]),
            function("git status", "Show branch status", vec![none()]),
            function(
                "git diff",
                "Show differences",
                vec![required("None", "Target", false)],
            ),
            function("git merge", "Merge", vec![none(), optional("--abort")]),
            function(
                "git remote",
                "Remote",
                vec![
                    optional("-v"),
                    required("set-url", "Remote name and URL", false),
                ],
            ),
        ],
        "docker" => vec![
            function(
                "docker build",
                "Build",
                vec![
                    required("None", "", false),
                    required("-t", "Name and tag", false),
                    required("-f", "File", false),
                ],
            ),
            function(
                "docker images",
                "List images",
                vec![
                    required("None", "", false),
                    optional("--all"),
                    optional("--quiet"),
                ],
            ),
            function(
                "docker run",
                "Create and run a container",
                vec![required("None", "", false)],
            ),
            function("docker image prune", "Remove unused images", vec![none()]),
            function(
                "docker exec",
                "Run a command in a running container",
                vec![required("None", "", false)],
            ),
            function(
                "docker pull",
                "Download an image from a repository",
                vec![required("None", "", false)],
            ),
            function(
                "docker push",
                "Push an image to a repository",
                vec![required("None", "", false)],
            ),
            function(
                "docker logs",
                "Show logs",
                vec![required("None", "Container", false)],
            ),
            function(
                "docker stop",
                "Stop a container",
                vec![required("None", "Container", false)],
            ),
            function(
                "docker rm",
                "Remove a stopped container",
                vec![required("None", "", false)],
            ),
            function(
                "docker rmi",
                "Remove an image",
                vec![required("None", "", false)],
            ),
            function("docker network ls", "List created networks", vec![none()]),
            function("docker volume ls", "List created volumes", vec![none()]),
        ],
        "compose" => vec![
            function("docker compose build", "Build services", vec![none()]),
            function(
                "docker compose exec",
                "Run a command in a running container",
                vec![none()],
            ),
            function("docker compose up", "Start containers", vec![none()]),
            function(
                "docker compose down",
                "Stop containers and remove related resources",
                vec![none()],
            ),
            function("docker compose restart", "Restart containers", vec![none()]),
            function("docker compose stop", "Stop containers", vec![none()]),
            function("docker compose logs", "Show container logs", vec![none()]),
            function("docker compose ps", "Show container status", vec![none()]),
            function(
                "docker compose start",
                "Start stopped services",
                vec![none()],
            ),
            function("docker compose kill", "Force stop containers", vec![none()]),
        ],
        _ => return None,
    })
}

pub(crate) fn generate_qr(payload: &Value) -> Result<(Value, Vec<u8>), Failure> {
    let kind = payload["type"].as_str().expect("validated type");
    let parameters = payload["parameters"]
        .as_object()
        .expect("validated parameters");
    let value = |key: &str| {
        parameters
            .get(key)
            .and_then(Value::as_str)
            .unwrap_or_default()
    };
    validate_qr_fields(kind, parameters)?;
    let content = match kind {
        "text" => value("text_content").to_owned(),
        "url" => value("url_content").to_owned(),
        "email" => format!("mailto:{}", value("email_address")),
        "phone" => format!("tel:{}", value("phone_number")),
        "sms" => format!("SMSTO:{}:{}", value("sms_phone"), value("sms_message")),
        "wifi" => format!(
            "WIFI:T:{};S:{};P:{};;",
            value("wifi_security"),
            value("wifi_ssid"),
            value("wifi_password")
        ),
        "contact" => {
            let mut contact = format!("BEGIN:VCARD\nVERSION:3.0\nFN:{}", value("contact_name"));
            if !value("contact_phone").is_empty() {
                contact.push_str(&format!("\nTEL:{}", value("contact_phone")));
            }
            if !value("contact_email").is_empty() {
                contact.push_str(&format!("\nEMAIL:{}", value("contact_email")));
            }
            contact.push_str("\nEND:VCARD");
            contact
        }
        "calendar" => {
            let summary = value("cal_summary");
            let start = calendar_timestamp(value("cal_start"))?;
            let end = calendar_timestamp(value("cal_end"))?;
            format!("BEGIN:VEVENT\nSUMMARY:{summary}\nDTSTART:{start}\nDTEND:{end}\nEND:VEVENT")
        }
        "geo" => {
            let lat = value("geo_lat")
                .parse::<f64>()
                .map_err(|_| Failure::new("INVALID_INPUT", "Invalid latitude."))?;
            let lng = value("geo_lng")
                .parse::<f64>()
                .map_err(|_| Failure::new("INVALID_INPUT", "Invalid longitude."))?;
            if !(-90.0..=90.0).contains(&lat) || !(-180.0..=180.0).contains(&lng) {
                return Err(Failure::new("INVALID_INPUT", "Invalid coordinates."));
            }
            format!("geo:{lat:.6},{lng:.6}")
        }
        _ => {
            return Err(Failure::new(
                "INVALID_INPUT",
                "QR code type is not supported.",
            ))
        }
    };
    if content.trim().is_empty() || content.len() > MAX_QR_CONTENT_BYTES {
        return Err(Failure::new(
            "INVALID_INPUT",
            "QR code content is empty or exceeds its size limit.",
        ));
    }
    let code =
        QrCode::with_error_correction_level(content.as_bytes(), EcLevel::L).map_err(|_| {
            Failure::new(
                "INVALID_INPUT",
                "QR code content is empty or exceeds its size limit.",
            )
        })?;
    let width = code.width() as u32;
    let colors = code.to_colors();
    let image: GrayImage = ImageBuffer::from_fn(width * 8, width * 8, |x, y| {
        if colors[((y / 8) * width + x / 8) as usize] == qrcode::Color::Dark {
            Luma([0])
        } else {
            Luma([255])
        }
    });
    let png = encode_png(&DynamicImage::ImageLuma8(image.clone()))?;
    let preview = preview_png(image)?;
    Ok((
        json!({"content":content, "imageDataUrl":format!("data:image/png;base64,{}", STANDARD.encode(preview))}),
        png,
    ))
}

const MAX_IMAGE_PIXELS: u64 = 100_000_000;
const MAX_IMAGE_PREVIEW_BYTES: usize = 768 * 1024;

pub(crate) fn load_image(path: &Path) -> Result<DynamicImage, Failure> {
    let reader = image::ImageReader::open(path)
        .and_then(image::ImageReader::with_guessed_format)
        .map_err(|_| {
            Failure::new(
                "INVALID_INPUT",
                "The selected file is not a supported image.",
            )
        })?;
    let (width, height) = reader.into_dimensions().map_err(|_| {
        Failure::new(
            "INVALID_INPUT",
            "The selected file is not a supported image.",
        )
    })?;
    if u64::from(width) * u64::from(height) > MAX_IMAGE_PIXELS {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "The selected image exceeds the pixel limit.",
        ));
    }
    let mut reader = image::ImageReader::open(path)
        .and_then(image::ImageReader::with_guessed_format)
        .map_err(|_| {
            Failure::new(
                "INVALID_INPUT",
                "The selected file is not a supported image.",
            )
        })?;
    let mut limits = image::Limits::default();
    limits.max_image_width = Some(width);
    limits.max_image_height = Some(height);
    limits.max_alloc = Some(MAX_IMAGE_PIXELS * 4 + 1024 * 1024);
    reader.limits(limits);
    reader.decode().map_err(|_| {
        Failure::new(
            "INVALID_INPUT",
            "The selected file is not a supported image or exceeds the pixel limit.",
        )
    })
}

pub(crate) fn image_preview_data_url(image: &DynamicImage) -> Result<String, Failure> {
    let mut preview = image.thumbnail(1024, 1024).to_rgba8();
    loop {
        let bytes = encode_png(&DynamicImage::ImageRgba8(preview.clone()))?;
        if bytes.len() <= MAX_IMAGE_PREVIEW_BYTES {
            return Ok(format!("data:image/png;base64,{}", STANDARD.encode(bytes)));
        }
        if preview.width() <= 128 || preview.height() <= 128 {
            return Err(Failure::new(
                "INTERNAL_ERROR",
                "Could not create an image preview.",
            ));
        }
        preview = image::imageops::resize(
            &preview,
            (preview.width() / 2).max(1),
            (preview.height() / 2).max(1),
            image::imageops::FilterType::CatmullRom,
        );
    }
}

pub(crate) fn edit_image(
    source: &DynamicImage,
    payload: &Value,
    cancelled: &std::sync::atomic::AtomicBool,
) -> Result<DynamicImage, Failure> {
    use std::sync::atomic::Ordering;
    if cancelled.load(Ordering::Relaxed) {
        return Err(Failure::new("CANCELLED", "Operation was cancelled."));
    }
    let action = payload["action"].as_str().expect("validated image action");
    let changed = match action {
        "rotate-left" => source.rotate270(),
        "rotate-right" => source.rotate90(),
        "flip-horizontal" => source.fliph(),
        "flip-vertical" => source.flipv(),
        "resize-size" | "resize-scale" => {
            let keep_aspect = payload["keepAspectRatio"].as_bool().unwrap_or(false);
            let (width, height) = if action == "resize-size" {
                let width = positive_integer(payload, "width")?;
                let height = positive_integer(payload, "height")?;
                if keep_aspect {
                    let scaled_height = (f64::from(width)
                        / (f64::from(source.width()) / f64::from(source.height())))
                    .round();
                    (width, scaled_height as u32)
                } else {
                    (width, height)
                }
            } else {
                let horizontal = positive_percent(payload, "horizontalPercent")?;
                let vertical = if keep_aspect {
                    horizontal
                } else {
                    positive_percent(payload, "verticalPercent")?
                };
                (
                    (f64::from(source.width()) * horizontal / 100.0)
                        .round()
                        .max(1.0) as u32,
                    (f64::from(source.height()) * vertical / 100.0)
                        .round()
                        .max(1.0) as u32,
                )
            };
            if width == 0 || height == 0 || u64::from(width) * u64::from(height) > MAX_IMAGE_PIXELS
            {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Image dimensions exceed the allowed size.",
                ));
            }
            let filter = if payload["smoothScaling"].as_bool().unwrap_or(false) {
                image::imageops::FilterType::CatmullRom
            } else {
                image::imageops::FilterType::Nearest
            };
            DynamicImage::ImageRgba8(image::imageops::resize(
                &source.to_rgba8(),
                width,
                height,
                filter,
            ))
        }
        "transparent" => transparent_image(source, payload)?,
        _ => {
            return Err(Failure::new(
                "INVALID_INPUT",
                "Image edit action is not supported.",
            ))
        }
    };
    if u64::from(changed.width()) * u64::from(changed.height()) > MAX_IMAGE_PIXELS {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "The edited image exceeds the pixel limit.",
        ));
    }
    if cancelled.load(Ordering::Relaxed) {
        return Err(Failure::new("CANCELLED", "Operation was cancelled."));
    }
    Ok(changed)
}

fn positive_integer(payload: &Value, key: &str) -> Result<u32, Failure> {
    let value = payload[key]
        .as_u64()
        .and_then(|value| u32::try_from(value).ok())
        .filter(|value| (1..=32_768).contains(value))
        .ok_or_else(|| {
            Failure::new("INVALID_INPUT", "Image dimensions exceed the allowed size.")
        })?;
    Ok(value)
}

fn positive_percent(payload: &Value, key: &str) -> Result<f64, Failure> {
    let value = payload[key]
        .as_f64()
        .filter(|value| (0.1..=1000.0).contains(value))
        .ok_or_else(|| Failure::new("INVALID_INPUT", "Image scale exceeds the allowed size."))?;
    Ok(value)
}

fn transparent_image(source: &DynamicImage, payload: &Value) -> Result<DynamicImage, Failure> {
    use std::collections::VecDeque;
    let width = source.width();
    let height = source.height();
    let x = payload["x"]
        .as_u64()
        .and_then(|value| u32::try_from(value).ok())
        .ok_or_else(|| {
            Failure::new(
                "INVALID_INPUT",
                "Choose a point and valid transparency settings.",
            )
        })?;
    let y = payload["y"]
        .as_u64()
        .and_then(|value| u32::try_from(value).ok())
        .ok_or_else(|| {
            Failure::new(
                "INVALID_INPUT",
                "Choose a point and valid transparency settings.",
            )
        })?;
    let tolerance = payload["tolerance"]
        .as_f64()
        .filter(|value| (0.0..=1.0).contains(value))
        .ok_or_else(|| {
            Failure::new(
                "INVALID_INPUT",
                "Choose a point and valid transparency settings.",
            )
        })?;
    let transparency = payload["transparency"]
        .as_f64()
        .filter(|value| (0.0..=1.0).contains(value))
        .ok_or_else(|| {
            Failure::new(
                "INVALID_INPUT",
                "Choose a point and valid transparency settings.",
            )
        })?;
    if x >= width || y >= height {
        return Err(Failure::new(
            "INVALID_INPUT",
            "Choose a point and valid transparency settings.",
        ));
    }
    let mode = payload["colorMode"].as_str().unwrap_or("rgb");
    if !matches!(mode, "rgb" | "hsl" | "hsv") {
        return Err(Failure::new(
            "INVALID_INPUT",
            "Color mode is not supported.",
        ));
    }
    let opacity = (255.0 * (1.0 - transparency)) as u8;
    let mut image = source.to_rgba8();
    let selected = *image.get_pixel(x, y);
    if payload["contiguousOnly"].as_bool().unwrap_or(false) {
        let mut visited = vec![false; (u64::from(width) * u64::from(height)) as usize];
        let mut queue = VecDeque::from([(x, y)]);
        visited[(u64::from(y) * u64::from(width) + u64::from(x)) as usize] = true;
        while let Some((cx, cy)) = queue.pop_front() {
            image.get_pixel_mut(cx, cy).0[3] = opacity;
            for (nx, ny) in [
                (cx.wrapping_sub(1), cy),
                (cx.saturating_add(1), cy),
                (cx, cy.wrapping_sub(1)),
                (cx, cy.saturating_add(1)),
            ] {
                if nx >= width || ny >= height {
                    continue;
                }
                let index = (u64::from(ny) * u64::from(width) + u64::from(nx)) as usize;
                if !visited[index] {
                    visited[index] = true;
                    if color_distance(&selected, image.get_pixel(nx, ny), mode)
                        <= tolerance * tolerance
                    {
                        queue.push_back((nx, ny));
                    }
                }
            }
        }
    } else {
        for pixel in image.pixels_mut() {
            if color_distance(&selected, pixel, mode) <= tolerance * tolerance {
                pixel.0[3] = opacity;
            }
        }
    }
    Ok(DynamicImage::ImageRgba8(image))
}

fn color_distance(left: &image::Rgba<u8>, right: &image::Rgba<u8>, mode: &str) -> f64 {
    let left = color_vector(left, mode);
    let right = color_vector(right, mode);
    let squared = left
        .iter()
        .zip(right)
        .map(|(a, b)| (a - b) * (a - b))
        .sum::<f64>();
    let max = if mode == "rgb" { 3.0 } else { 4.0 };
    squared / max
}

fn color_vector(color: &image::Rgba<u8>, mode: &str) -> [f64; 3] {
    let [r, g, b] = [
        color[0] as f64 / 255.0,
        color[1] as f64 / 255.0,
        color[2] as f64 / 255.0,
    ];
    if mode == "rgb" {
        return [r, g, b];
    }
    let max = r.max(g).max(b);
    let min = r.min(g).min(b);
    let delta = max - min;
    let hue = if delta == 0.0 {
        -1.0
    } else if max == r {
        ((g - b) / delta).rem_euclid(6.0) / 6.0
    } else if max == g {
        ((b - r) / delta + 2.0) / 6.0
    } else {
        ((r - g) / delta + 4.0) / 6.0
    };
    let value = if mode == "hsv" {
        max
    } else {
        (max + min) / 2.0
    };
    let saturation = if delta == 0.0 {
        0.0
    } else if mode == "hsv" {
        delta / max
    } else {
        delta / (1.0 - (2.0 * value - 1.0).abs())
    };
    if hue < 0.0 {
        [0.0, 0.0, value]
    } else {
        [
            saturation * (std::f64::consts::TAU * hue).cos(),
            saturation * (std::f64::consts::TAU * hue).sin(),
            value,
        ]
    }
}

pub(crate) fn save_image_bytes(image: &DynamicImage, path: &Path) -> Result<Vec<u8>, Failure> {
    let format = image::ImageFormat::from_path(path).unwrap_or(image::ImageFormat::Png);
    let mut bytes = Cursor::new(Vec::new());
    image
        .write_to(&mut bytes, format)
        .map_err(|_| Failure::new("IO_ERROR", "Could not save the image."))?;
    Ok(bytes.into_inner())
}

pub(crate) fn split_image(
    image: &DynamicImage,
    original_path: &Path,
    folder_path: &Path,
    payload: &Value,
    cancelled: &std::sync::atomic::AtomicBool,
) -> Result<(), Failure> {
    let width = image.width();
    let height = image.height();
    let ignore = payload["ignoreRemainders"].as_bool().unwrap_or(false);
    let (x_count, y_count, step_w, step_h) = match payload["mode"].as_str().unwrap_or("division") {
        "cell-size" => {
            let cell_w = positive_integer(payload, "cellWidth")?;
            let cell_h = positive_integer(payload, "cellHeight")?;
            if cell_w > width || cell_h > height {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Cell dimensions must fit within the image.",
                ));
            }
            let count = |source: u32, cell: u32| {
                if ignore || source.is_multiple_of(cell) {
                    source / cell
                } else {
                    source / cell + 1
                }
            };
            (
                count(width, cell_w),
                count(height, cell_h),
                f64::from(cell_w),
                f64::from(cell_h),
            )
        }
        "division" => {
            let x = positive_integer(payload, "horizontalDivisions")?;
            let y = positive_integer(payload, "verticalDivisions")?;
            if x > width || y > height {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Division counts must fit within the image.",
                ));
            }
            (
                x,
                y,
                f64::from(width) / f64::from(x),
                f64::from(height) / f64::from(y),
            )
        }
        _ => {
            return Err(Failure::new(
                "INVALID_INPUT",
                "Image split mode is not supported.",
            ))
        }
    };
    if u64::from(x_count) * u64::from(y_count) > 10_000 {
        // shortcut: cap staged split files at 10,000; raise after measuring peak disk use.
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Image split is limited to 10,000 files.",
        ));
    }
    let source_base = original_path
        .file_stem()
        .and_then(|name| name.to_str())
        .filter(|name| !name.is_empty())
        .unwrap_or("image");
    let suffix = original_path
        .extension()
        .and_then(|value| value.to_str())
        .unwrap_or("png");
    let format = image::ImageFormat::from_extension(suffix).unwrap_or(image::ImageFormat::Png);
    let staging = tempfile::tempdir_in(folder_path)
        .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
    let mut outputs = Vec::new();
    for y in 0..y_count {
        for x in 0..x_count {
            if cancelled.load(std::sync::atomic::Ordering::Relaxed) {
                return Err(Failure::new("CANCELLED", "Operation was cancelled."));
            }
            let left = ((f64::from(x) * step_w).floor() as u32).min(width);
            let top = ((f64::from(y) * step_h).floor() as u32).min(height);
            let mut right = (((f64::from(x) + 1.0) * step_w).floor() as u32).min(width);
            let mut bottom = (((f64::from(y) + 1.0) * step_h).floor() as u32).min(height);
            if !ignore && x == x_count - 1 {
                right = width;
            }
            if !ignore && y == y_count - 1 {
                bottom = height;
            }
            if right <= left || bottom <= top {
                continue;
            }
            let name = format!("{source_base}_{x}_{y}.{suffix}");
            let staged_path = staging.path().join(&name);
            let mut bytes = Cursor::new(Vec::new());
            image
                .crop_imm(left, top, right - left, bottom - top)
                .write_to(&mut bytes, format)
                .map_err(|_| Failure::new("IO_ERROR", "Could not save divided images."))?;
            fs::write(&staged_path, bytes.into_inner())
                .map_err(|error| Failure::new("IO_ERROR", error.to_string()))?;
            outputs.push((staged_path, folder_path.join(name)));
        }
    }
    if outputs.iter().any(|(_, target)| target.exists()) {
        return Err(Failure::new(
            "IO_ERROR",
            "Split image files already exist in the selected folder.",
        ));
    }
    let mut written = Vec::new();
    for (staged, target) in outputs {
        if let Err(error) = std::fs::rename(staged, &target) {
            for path in written {
                let _ = std::fs::remove_file(path);
            }
            return Err(Failure::new(
                "IO_ERROR",
                format!("Could not save divided images: {error}"),
            ));
        }
        written.push(target);
    }
    Ok(())
}

fn calendar_timestamp(value: &str) -> Result<String, Failure> {
    Ok(parse_calendar_datetime(value)?
        .format("%Y%m%dT%H%M%S")
        .to_string())
}

fn parse_calendar_datetime(value: &str) -> Result<chrono::NaiveDateTime, Failure> {
    use chrono::{DateTime, NaiveDateTime};
    if let Ok(date) = DateTime::parse_from_rfc3339(value) {
        return Ok(date.naive_local());
    }
    NaiveDateTime::parse_from_str(value, "%Y-%m-%dT%H:%M:%S%.f")
        .or_else(|_| NaiveDateTime::parse_from_str(value, "%Y-%m-%d %H:%M:%S%.f"))
        .or_else(|_| NaiveDateTime::parse_from_str(value, "%Y-%m-%dT%H:%M"))
        .or_else(|_| NaiveDateTime::parse_from_str(value, "%Y-%m-%d %H:%M"))
        .map_err(|_| {
            Failure::new(
                "INVALID_INPUT",
                "Enter valid local calendar dates and times.",
            )
        })
}

fn validate_qr_fields(kind: &str, parameters: &Map<String, Value>) -> Result<(), Failure> {
    let value = |key: &str| {
        parameters
            .get(key)
            .and_then(Value::as_str)
            .unwrap_or_default()
    };
    let filled = |key: &str| !value(key).trim().is_empty();
    let missing = || Failure::new("INVALID_INPUT", "Complete the required QR code fields.");
    match kind {
        "text" if !filled("text_content") => return Err(missing()),
        "url"
            if !filled("url_content")
                || !(value("url_content").starts_with("http://")
                    || value("url_content").starts_with("https://")) =>
        {
            return Err(missing())
        }
        "email" if !filled("email_address") || !value("email_address").contains('@') => {
            return Err(missing())
        }
        "phone" if !valid_phone(value("phone_number")) => return Err(missing()),
        "sms" if !valid_phone(value("sms_phone")) || !filled("sms_message") => {
            return Err(missing())
        }
        "wifi" => {
            if !parameters.contains_key("wifi_security") || !filled("wifi_ssid") {
                return Err(missing());
            }
            if matches!(value("wifi_security"), "WPA" | "WEP")
                && parameters.contains_key("wifi_password")
                && !filled("wifi_password")
            {
                return Err(missing());
            }
        }
        "contact" => {
            if !filled("contact_name") || (!filled("contact_phone") && !filled("contact_email")) {
                return Err(missing());
            }
            if filled("contact_phone") && !valid_phone(value("contact_phone")) {
                return Err(missing());
            }
            if filled("contact_email") && !value("contact_email").contains('@') {
                return Err(missing());
            }
        }
        "calendar" => {
            if !filled("cal_summary")
                || !parameters.contains_key("cal_start")
                || !parameters.contains_key("cal_end")
            {
                return Err(missing());
            }
            if parse_calendar_datetime(value("cal_start"))?
                >= parse_calendar_datetime(value("cal_end"))?
            {
                return Err(Failure::new(
                    "INVALID_INPUT",
                    "Enter valid local calendar dates and times.",
                ));
            }
        }
        "geo" => {
            let latitude = value("geo_lat").parse::<f64>().map_err(|_| missing())?;
            let longitude = value("geo_lng").parse::<f64>().map_err(|_| missing())?;
            if !latitude.is_finite()
                || !longitude.is_finite()
                || !(-90.0..=90.0).contains(&latitude)
                || !(-180.0..=180.0).contains(&longitude)
            {
                return Err(missing());
            }
        }
        _ => {}
    }
    Ok(())
}

fn valid_phone(value: &str) -> bool {
    use fancy_regex::Regex;
    if value.trim().is_empty() {
        return false;
    }
    [
        r"^\+\d{1,3}[-\s]?\d{1,4}[-\s]?\d{4}[-\s]?\d{4}$",
        r"^0\d{1,4}[-\s]?\d{2,4}[-\s]?\d{4}$",
        r"^\d{10,11}$",
    ]
    .iter()
    .any(|pattern| {
        Regex::new(pattern).is_ok_and(|regex| regex.is_match(value.trim()).unwrap_or(false))
    })
}

fn encode_png(image: &DynamicImage) -> Result<Vec<u8>, Failure> {
    let mut bytes = Cursor::new(Vec::new());
    image
        .write_to(&mut bytes, ImageFormat::Png)
        .map_err(|_| Failure::new("INTERNAL_ERROR", "Could not create a PNG image."))?;
    Ok(bytes.into_inner())
}

fn preview_png(image: GrayImage) -> Result<Vec<u8>, Failure> {
    let mut preview = image;
    loop {
        let bytes = encode_png(&DynamicImage::ImageLuma8(preview.clone()))?;
        if bytes.len() <= 1024 * 1024 {
            return Ok(bytes);
        }
        if preview.width() <= 128 || preview.height() <= 128 {
            return Err(Failure::new(
                "INTERNAL_ERROR",
                "Could not create a QR preview.",
            ));
        }
        preview = image::imageops::resize(
            &preview,
            preview.width() / 2,
            preview.height() / 2,
            image::imageops::FilterType::Nearest,
        );
    }
}

pub(crate) fn evaluate_regex(
    payload: &Value,
    cancelled: &std::sync::atomic::AtomicBool,
) -> Result<Value, Failure> {
    let pattern = payload["pattern"].as_str().expect("validated pattern");
    let text = payload["text"].as_str().expect("validated text");
    let replacement = payload["replacement"]
        .as_str()
        .expect("validated replacement");
    let global = payload
        .get("global")
        .and_then(Value::as_bool)
        .unwrap_or(true);
    if pattern.is_empty() {
        return Ok(json!({"matches":[], "replacedText":text}));
    }
    let mut builder = RegexBuilder::new(pattern);
    builder.case_insensitive(payload["caseInsensitive"].as_bool().unwrap_or(false));
    builder.multi_line(payload["multiline"].as_bool().unwrap_or(false));
    builder.dot_matches_new_line(payload["dotMatchesEverything"].as_bool().unwrap_or(false));
    builder.ignore_whitespace(payload["extendedPatternSyntax"].as_bool().unwrap_or(false));
    builder.unicode_mode(payload["useUnicodeProperties"].as_bool().unwrap_or(false));
    builder.backtrack_limit(1_000_000);
    let regex = builder
        .build()
        .map_err(|error| Failure::new("INVALID_INPUT", error.to_string()))?;
    let capture_names = regex
        .capture_names()
        .map(|name| name.unwrap_or_default().to_owned())
        .collect::<Vec<_>>();
    let mut matches = Vec::new();
    let mut replaced = String::new();
    let mut last = 0usize;
    for result in regex.captures_iter(text) {
        if cancelled.load(std::sync::atomic::Ordering::Relaxed) {
            return Err(Failure::new("CANCELLED", "Operation was cancelled."));
        }
        let captures = result.map_err(|error| Failure::new("INVALID_INPUT", error.to_string()))?;
        let whole = captures.get(0).expect("match always has capture zero");
        let max_group = (0..captures.len())
            .rev()
            .find(|index| captures.get(*index).is_some())
            .unwrap_or(0);
        let groups = (0..=max_group).map(|index| {
            let capture = captures.get(index);
            let (value, offset, length) = capture.map_or(("", -1, -1), |value| (value.as_str(), utf16_offset(text, value.start()), value.as_str().encode_utf16().count() as i64));
            json!({"index":index, "name":capture_names.get(index).cloned().unwrap_or_default(), "value":value, "offset":offset, "length":length})
        }).collect::<Vec<_>>();
        matches.push(json!({"index":matches.len(), "offset":utf16_offset(text, whole.start()), "length":whole.as_str().encode_utf16().count(), "value":whole.as_str(), "groups":groups}));
        replaced.push_str(&text[last..whole.start()]);
        replaced.push_str(&expand_replacement(replacement, &captures, &capture_names));
        last = whole.end();
        if !global {
            break;
        }
        if matches.len() > MAX_REGEX_MATCHES {
            return Err(Failure::new(
                "SIZE_LIMIT_EXCEEDED",
                "Regex results are limited to 10,000 matches.",
            ));
        }
    }
    replaced.push_str(&text[last..]);
    let output = json!({"matches":matches, "replacedText":replaced});
    if serde_json::to_vec(&output).map_or(true, |bytes| bytes.len() > MAX_REGEX_OUTPUT_BYTES) {
        return Err(Failure::new(
            "SIZE_LIMIT_EXCEEDED",
            "Regex results exceed the 4 MiB output limit.",
        ));
    }
    Ok(output)
}

fn utf16_offset(text: &str, byte_offset: usize) -> i64 {
    text[..byte_offset].encode_utf16().count() as i64
}

fn expand_replacement(replacement: &str, captures: &Captures<'_, str>, names: &[String]) -> String {
    let mut output = String::new();
    let mut chars = replacement.char_indices().peekable();
    while let Some((_, current)) = chars.next() {
        if current != '$' {
            output.push(current);
            continue;
        }
        match chars.peek().copied() {
            Some((_, '$')) => {
                chars.next();
                output.push('$');
            }
            Some((_, '&')) => {
                chars.next();
                output.push_str(captures.get(0).map_or("", |m| m.as_str()));
            }
            Some((_, '<')) => {
                chars.next();
                let mut name = String::new();
                let mut closed = false;
                for (_, ch) in chars.by_ref() {
                    if ch == '>' {
                        closed = true;
                        break;
                    }
                    name.push(ch);
                }
                if closed {
                    output.push_str(captures.name(&name).map_or("", |m| m.as_str()));
                } else {
                    output.push_str("$<");
                    output.push_str(&name);
                }
            }
            Some((_, digit @ '1'..='9')) => {
                chars.next();
                let index = digit.to_digit(10).unwrap() as usize;
                if index < names.len() {
                    output.push_str(captures.get(index).map_or("", |m| m.as_str()));
                }
            }
            _ => output.push('$'),
        }
    }
    output
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::atomic::AtomicBool;

    #[test]
    fn converts_json_yaml_and_toml_inputs() {
        let json_output =
            convert_data(&json!({"input":"{\"a\":1}","format":"yaml-flow","indentation":"2"}))
                .unwrap();
        assert_eq!(json_output["output"], "{\"a\": 1}\n");
        let yaml_output =
            convert_data(&json!({"input":"a: 1","format":"json","indentation":"2"})).unwrap();
        assert_eq!(yaml_output["output"], "{\n  \"a\": \"1\"\n}\n");
        let toml_output =
            convert_data(&json!({"input":"a = 1","format":"json","indentation":"2"})).unwrap();
        assert_eq!(toml_output["output"], "{\n  \"a\": 1\n}\n");
    }

    #[test]
    fn matches_qt_data_conversion_fixture() {
        let fixture_path =
            Path::new(env!("CARGO_MANIFEST_DIR")).join("../tests/fixtures/tauri-convert-data.json");
        let cases: Vec<Value> = serde_json::from_slice(&fs::read(fixture_path).unwrap()).unwrap();
        for case in cases {
            assert_eq!(convert_data(&case["input"]).unwrap(), case["expected"]);
        }
    }

    #[test]
    fn rejects_yaml_integers_that_exceed_supported_precision() {
        let error = convert_data(&json!({
            "input":"{\"value\":340282366920938463463374607431768211456}",
            "format":"yaml-block",
            "indentation":"2"
        }))
        .unwrap_err();

        assert_eq!(error.code, "INVALID_INPUT");
    }

    #[test]
    fn rejects_toml_integers_outside_signed_64_bit_range() {
        let error = convert_data(&json!({
            "input":"{\"value\":9223372036854775808}",
            "format":"toml",
            "indentation":"2"
        }))
        .unwrap_err();

        assert_eq!(error.code, "INVALID_INPUT");
    }

    #[test]
    fn builds_command_and_qr_content() {
        let command = command_data(&json!({"action":"generate","category":"git","functionIndex":1,"optionIndex":1,"value":"safe message"})).unwrap();
        assert_eq!(command["command"], "git commit -m \"safe message\"");
        let (qr, png) =
            generate_qr(&json!({"type":"text","parameters":{"text_content":"hello"}})).unwrap();
        assert_eq!(qr["content"], "hello");
        assert_eq!(&png[..8], b"\x89PNG\r\n\x1a\n");
    }

    #[test]
    fn generates_commands_using_shared_qt_fixtures() {
        let cases: Vec<Value> =
            serde_json::from_str(include_str!("../../tests/fixtures/tauri-command.json")).unwrap();
        for case in cases {
            match case.get("errorCode") {
                Some(code) => assert_eq!(
                    command_data(&case["input"]).unwrap_err().code,
                    code.as_str().unwrap(),
                    "{}",
                    case["name"]
                ),
                None => assert_eq!(
                    command_data(&case["input"]).unwrap(),
                    case["expected"],
                    "{}",
                    case["name"]
                ),
            }
        }
    }

    #[test]
    fn generates_all_qr_contents_from_shared_qt_fixture() {
        let cases: Vec<Value> =
            serde_json::from_str(include_str!("../../tests/fixtures/tauri-qr-content.json"))
                .unwrap();
        for case in cases {
            assert_eq!(
                generate_qr(&case["input"]).unwrap().0["content"],
                case["expected"]["content"]
            );
        }
    }

    #[test]
    fn evaluates_regex_using_shared_qt_fixture() {
        let cases: Vec<Value> =
            serde_json::from_str(include_str!("../../tests/fixtures/tauri-regex.json")).unwrap();
        for case in cases {
            match case.get("errorCode") {
                Some(code) => assert_eq!(
                    evaluate_regex(&case["input"], &AtomicBool::new(false))
                        .unwrap_err()
                        .code,
                    code.as_str().unwrap(),
                    "{}",
                    case["name"]
                ),
                None => assert_eq!(
                    evaluate_regex(&case["input"], &AtomicBool::new(false)).unwrap(),
                    case["expected"],
                    "{}",
                    case["name"]
                ),
            }
        }
    }

    #[test]
    fn sanitizes_markdown_and_replaces_unicode_regex_matches() {
        let case: Value =
            serde_json::from_str(include_str!("../../tests/fixtures/tauri-markdown.json")).unwrap();
        let html = render_markdown(case["markdown"].as_str().unwrap()).unwrap()["html"]
            .as_str()
            .unwrap()
            .to_owned();
        for value in case["requiredHtml"].as_array().unwrap() {
            assert!(html.contains(value.as_str().unwrap()));
        }
        for value in case["forbiddenHtml"].as_array().unwrap() {
            assert!(!html.contains(value.as_str().unwrap()));
        }
        for value in case["tauriForbiddenHtml"].as_array().unwrap() {
            assert!(!html.contains(value.as_str().unwrap()));
        }
        let regex = evaluate_regex(&json!({"pattern":"(猫)","text":"🍣猫","replacement":"$1","global":true,"caseInsensitive":false,"multiline":false,"dotMatchesEverything":false,"useUnicodeProperties":true,"extendedPatternSyntax":false}), &AtomicBool::new(false)).unwrap();
        assert_eq!(regex["matches"][0]["offset"], 2);
        assert_eq!(regex["matches"][0]["groups"][1]["offset"], 2);
    }

    #[test]
    fn image_transforms_match_shared_qt_fixtures() {
        let cases: Vec<Value> = serde_json::from_str(include_str!(
            "../../tests/fixtures/tauri-image-transforms.json"
        ))
        .unwrap();
        for case in cases {
            let width = case["width"].as_u64().unwrap() as u32;
            let height = case["height"].as_u64().unwrap() as u32;
            let pixels = |name: &str| {
                case[name]
                    .as_array()
                    .unwrap()
                    .iter()
                    .flat_map(|pixel| {
                        pixel
                            .as_array()
                            .unwrap()
                            .iter()
                            .map(|channel| channel.as_u64().unwrap() as u8)
                    })
                    .collect::<Vec<_>>()
            };
            let source = DynamicImage::ImageRgba8(
                ImageBuffer::from_raw(width, height, pixels("sourcePixels")).unwrap(),
            );
            let action = case["action"].as_str().unwrap();
            let mut parameters = case.get("parameters").cloned().unwrap_or_else(|| json!({}));
            if action == "split" {
                let temporary = tempfile::tempdir().unwrap();
                let output = temporary.path().join("output");
                fs::create_dir(&output).unwrap();
                split_image(
                    &source,
                    Path::new("sample.png"),
                    &output,
                    &parameters,
                    &AtomicBool::new(false),
                )
                .unwrap();
                let parts = case["expectedParts"].as_array().unwrap();
                assert_eq!(
                    fs::read_dir(&output).unwrap().count(),
                    parts.len(),
                    "{}",
                    case["name"]
                );
                for part in parts {
                    let image = image::open(output.join(part["name"].as_str().unwrap()))
                        .unwrap()
                        .to_rgba8();
                    assert_eq!(
                        image.dimensions(),
                        (
                            part["width"].as_u64().unwrap() as u32,
                            part["height"].as_u64().unwrap() as u32
                        ),
                        "{}: {}",
                        case["name"],
                        part["name"]
                    );
                    assert_eq!(
                        image.into_raw(),
                        part["pixels"]
                            .as_array()
                            .unwrap()
                            .iter()
                            .flat_map(|pixel| {
                                pixel
                                    .as_array()
                                    .unwrap()
                                    .iter()
                                    .map(|channel| channel.as_u64().unwrap() as u8)
                            })
                            .collect::<Vec<_>>(),
                        "{}: {}",
                        case["name"],
                        part["name"]
                    );
                }
                continue;
            }
            parameters["action"] = json!(action);
            let result = edit_image(&source, &parameters, &AtomicBool::new(false))
                .unwrap()
                .to_rgba8();
            assert_eq!(
                result.dimensions(),
                (
                    case.get("outputWidth")
                        .and_then(Value::as_u64)
                        .unwrap_or(u64::from(width)) as u32,
                    case.get("outputHeight")
                        .and_then(Value::as_u64)
                        .unwrap_or(u64::from(height)) as u32
                ),
                "{}",
                case["name"]
            );
            assert_eq!(
                result.into_raw(),
                pixels("expectedPixels"),
                "{}",
                case["name"]
            );
        }
    }
}
