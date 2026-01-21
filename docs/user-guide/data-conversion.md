# Data Conversion

Convert between JSON, YAML, and TOML data formats with syntax validation and formatting.

## Overview

The Data Conversion tool helps developers quickly convert configuration files and data structures between different formats without manual rewriting.

## Accessing the Tool

1. Launch DevTools
2. Click **Data Conversion** in the side menu

## Supported Formats

| Format | Extension | Description |
|--------|-----------|-------------|
| JSON | .json | JavaScript Object Notation - widely used for APIs and configs |
| YAML | .yaml, .yml | YAML Ain't Markup Language - human-readable configuration |
| TOML | .toml | Tom's Obvious, Minimal Language - configuration files |

## How to Use

### Basic Conversion

1. Paste or type your data in the **Input** area
2. Select the **Input Format** (JSON, YAML, or TOML)
3. Select the **Output Format**
4. Click **Convert**
5. View the result in the **Output** area
6. Copy or save the result

### Loading from File

1. Click **Open File**
2. Select a file (.json, .yaml, .yml, or .toml)
3. The content loads into the Input area
4. Format is auto-detected based on extension

### Saving Results

1. After conversion, click **Save**
2. Choose filename and location
3. Select appropriate extension

## Format Examples

### JSON

```json
{
  "name": "DevTools",
  "version": "0.2.0",
  "features": ["qr-code", "image", "data-conversion"],
  "settings": {
    "theme": "dark",
    "language": "en"
  }
}
```

### YAML

```yaml
name: DevTools
version: "0.2.0"
features:
  - qr-code
  - image
  - data-conversion
settings:
  theme: dark
  language: en
```

### TOML

```toml
name = "DevTools"
version = "0.2.0"
features = ["qr-code", "image", "data-conversion"]

[settings]
theme = "dark"
language = "en"
```

## Conversion Rules

### JSON to YAML
- Objects become mappings
- Arrays become sequences
- Strings, numbers, booleans preserved
- `null` becomes `null` or `~`

### JSON to TOML
- Objects become tables
- Nested objects become nested tables
- Arrays of objects become array of tables
- Note: TOML has limitations with complex nested structures

### YAML to JSON
- Mappings become objects
- Sequences become arrays
- All types converted to JSON equivalents
- Comments are lost (JSON doesn't support comments)

### YAML to TOML
- Similar to JSON to TOML
- Comments are lost

### TOML to JSON/YAML
- Tables become objects/mappings
- Array of tables become arrays
- All types converted appropriately

## Syntax Validation

DevTools validates syntax before conversion:

- **Valid**: Conversion proceeds
- **Invalid**: Error message displayed with details

### Common Errors

| Error | Cause | Solution |
|-------|-------|----------|
| Unexpected token | Invalid JSON syntax | Check brackets, quotes, commas |
| Invalid YAML | Indentation or syntax error | Fix indentation (use spaces, not tabs) |
| Invalid TOML | Syntax error | Check key=value format |

## Formatting Options

### JSON Formatting
- Indentation: 2 or 4 spaces
- Compact or pretty-printed output

### YAML Formatting
- Standard indentation: 2 spaces
- Flow style or block style

### TOML Formatting
- Standard formatting
- Inline tables where appropriate

## Use Cases

### Configuration Migration

Convert configuration files between formats:
```
config.json → config.yaml
```

### API Response Processing

Convert API JSON responses to more readable YAML:
```json
{"status":"ok","data":{"id":1,"name":"Test"}}
```
to:
```yaml
status: ok
data:
  id: 1
  name: Test
```

### Documentation

Convert examples between formats for documentation.

## Limitations

### TOML Limitations
- Limited support for deeply nested structures
- Array of tables syntax differs from JSON/YAML arrays
- Some complex structures may not convert cleanly

### Data Type Handling
- Date/time formats may vary between formats
- Large numbers may lose precision
- Binary data is not supported

## Best Practices

1. **Validate first**: Ensure input is valid before converting
2. **Review output**: Check converted data for accuracy
3. **Preserve originals**: Keep original files before overwriting
4. **Test configurations**: Test converted configs in their target applications

## Troubleshooting

### Conversion Failed

1. **Check syntax**: Validate input format is correct
2. **Check format selection**: Ensure input format is correctly selected
3. **Simplify**: Try with simpler data first

### Output Looks Wrong

1. **Check data types**: Some types convert differently
2. **Check nesting**: Deep nesting may cause issues
3. **Manual adjustment**: Some manual editing may be needed

### Special Characters Issues

1. **Escape properly**: Special characters need escaping in JSON
2. **Use quotes**: Quote strings in YAML/TOML when needed

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
