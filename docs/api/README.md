# API Documentation

This document provides an overview of the DevTools API and links to detailed documentation.

## Generated Documentation

DevTools uses Doxygen for API documentation generation. The full API reference is available at:

**[https://LotusAndCompany.github.io/devtools/](https://LotusAndCompany.github.io/devtools/)**

## Generating Documentation Locally

### Prerequisites

Install Doxygen:

```bash
brew install doxygen
```

### Generate

```bash
cd build
cmake ..
make DevTools_docs
```

The generated documentation will be in `build/doxygen/html/`. Open `index.html` in a browser.

## Code Architecture Overview

### Core Modules

#### Application Framework

| Class | File | Description |
|-------|------|-------------|
| `GuiApplication` | `gui/gui_application.h` | Main application class |
| `ApplicationMixin` | `core/application_mixin.h` | Application utilities mixin |
| `MainWindow` | `gui/main_window.h` | Main window implementation |

#### Tool System

| Class | File | Description |
|-------|------|-------------|
| `Tool` | `core/tool/tool.h` | Base class for tools |
| `GuiTool` | `gui/gui_tool.h` | Base class for GUI tools |
| `ToolId` | `core/tool/tool_id_fields.h` | Tool identification enum |

#### Navigation

| Class | File | Description |
|-------|------|-------------|
| `SideMenu` | `gui/sidemenu.h` | Side menu navigation |
| `ContentsArea` | `gui/contents_area.h` | Main content area |
| `SideMenuItem` | `gui/sidemenu_item.h` | Individual menu item |

### Feature Modules

#### QR Code Generation

| Class | File | Description |
|-------|------|-------------|
| `QRCodeGenerationGui` | `gui/qr_code/qr_code_generation_gui.h` | QR code UI |
| `ContentGenerator` | `core/qr_tool/content_generator.h` | QR content generation |
| `qrcodegen` | `core/qr_tool/qrcodegen.hpp` | QR code library |

#### Image Processing

| Class | File | Description |
|-------|------|-------------|
| `BasicImageIO` | `core/image/basic_image_io.h` | Image I/O operations |
| `BasicImageEditInterface` | `core/image/basic_image_edit_interface.h` | Edit interface |
| `ImageResize` | `core/image/resize/image_resize.h` | Resize algorithm |
| `ImageRotation` | `core/image/rotation/image_rotation.h` | Rotation algorithm |
| `ImageDivision` | `core/image/division/image_division.h` | Division algorithm |
| `ImageTransparent` | `core/image/transparent/image_transparent.h` | Transparency |

#### Data Conversion

| Class | File | Description |
|-------|------|-------------|
| `DataConversion` | `core/data_conversion/data_conversion.h` | Conversion coordinator |
| `JsonParser` | `core/data_conversion/parser/json_parser.h` | JSON parsing |
| `YamlParser` | `core/data_conversion/parser/yaml_parser.h` | YAML parsing |
| `TomlParser` | `core/data_conversion/parser/toml_parser.h` | TOML parsing |
| `JsonEmitter` | `core/data_conversion/emitter/json_emitter.h` | JSON output |
| `YamlEmitter` | `core/data_conversion/emitter/yaml_emitter.h` | YAML output |
| `TomlEmitter` | `core/data_conversion/emitter/toml_emitter.h` | TOML output |

### Exception Classes

| Class | File | Description |
|-------|------|-------------|
| `CommonException` | `core/exception/common_exception.h` | Base exception |
| `InvalidArgumentException` | `core/exception/invalid_argument_exception.h` | Invalid argument |
| `OutOfRangeException` | `core/exception/out_of_range_exception.h` | Out of range |
| `InvalidStateException` | `core/exception/invalid_state_exception.h` | Invalid state |
| `UnderDevelopmentException` | `core/exception/under_development_exception.h` | Not implemented |

## Key Interfaces

### Tool Interface

All tools implement the base `GuiTool` interface:

```cpp
class GuiTool : public QWidget {
    Q_OBJECT
public:
    explicit GuiTool(QWidget* parent = nullptr);
    virtual ~GuiTool();
    
    // Override in subclasses for initialization
    virtual void initialize();
};
```

### Parser Interface

Data parsers implement `BasicParser`:

```cpp
template<typename OutputType>
class BasicParser {
public:
    virtual ~BasicParser() = default;
    virtual std::optional<OutputType> parse(const QString& input) = 0;
};
```

### Emitter Interface

Data emitters implement `BasicEmitter`:

```cpp
template<typename InputType>
class BasicEmitter {
public:
    virtual ~BasicEmitter() = default;
    virtual QString emit(const InputType& data) = 0;
};
```

## Usage Examples

### Creating a Tool

```cpp
#include "gui/gui_tool.h"

class MyTool : public GuiTool {
    Q_OBJECT
public:
    explicit MyTool(QWidget* parent = nullptr);
    
    void initialize() override {
        // Setup UI
    }
};
```

### Using Data Conversion

```cpp
#include "core/data_conversion/parser/json_parser.h"
#include "core/data_conversion/emitter/yaml_emitter.h"

JsonParser parser;
auto data = parser.parse(jsonString);

if (data) {
    YamlEmitter emitter;
    QString yaml = emitter.emit(*data);
}
```

### Using Image Processing

```cpp
#include "core/image/resize/image_resize.h"

ImageResize resizer;
QImage input("image.png");
QImage output = resizer.resize(input, 200, 150, true);  // keep aspect ratio
output.save("resized.png");
```

## Documentation Comments

All public APIs should be documented using Doxygen format:

```cpp
/**
 * @brief Process the input image.
 * 
 * Detailed description of what this method does.
 * 
 * @param input The input image to process
 * @param options Processing options
 * @return The processed image
 * @throws InvalidArgumentException if input is null
 * 
 * @example
 * @code
 * QImage result = processor.process(image, options);
 * @endcode
 */
QImage process(const QImage& input, const Options& options);
```

## Related Documentation

- [Architecture](../development/architecture.md) - System architecture
- [Adding New Tools](../development/adding-new-tools.md) - Creating modules
- [Coding Standards](../development/coding-standards.md) - Code style
