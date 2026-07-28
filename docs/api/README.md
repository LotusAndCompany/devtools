# API Documentation

This document provides an overview of the DevTools API and links to detailed documentation.

## Generated Documentation

DevTools uses Doxygen for API documentation generation. The full API reference is available at:

**[https://LotusAndCompany.github.io/devtools/](https://LotusAndCompany.github.io/devtools/)**

## Generating Documentation Locally

### Prerequisites

Install Doxygen (1.16+):

```bash
brew install doxygen
```

Optionally, install GraphViz to enable class diagrams and dependency graphs:

```bash
brew install graphviz
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

The application framework lives under `features/framework/` and is compiled as
the `DevTools_core` library.

#### Application Framework

| Class | File | Description |
|-------|------|-------------|
| `GuiApplication` | `features/framework/gui/gui_application.h` | Main application class |
| `ApplicationMixin` | `features/framework/core/application_mixin.h` | Application utilities mixin |
| `MainWindow` | `features/framework/gui/main_window.h` | Main window implementation |

#### Tool System

| Class | File | Description |
|-------|------|-------------|
| `Tool` | `features/framework/core/tool/tool.h` | Base class for tools |
| `GuiTool` | `features/framework/gui/gui_tool.h` | Base class for GUI tools |
| `ToolId` | `features/framework/core/tool/tool_id_fields.h` | Tool identification enum |

#### Navigation

| Class | File | Description |
|-------|------|-------------|
| `SideMenu` | `features/framework/gui/sidemenu.h` | Side menu navigation |
| `ContentsArea` | `features/framework/gui/contents_area.h` | Main content area |
| `SideMenuItem` | `features/framework/gui/sidemenu_item.h` | Individual menu item |

### Feature Modules

#### QR Code Generation

| Class | File | Description |
|-------|------|-------------|
| `QRCodeGenerationGui` | `features/qr_code/gui/qr_code_generation_gui.h` | QR code UI |
| `ContentGenerator` | `features/qr_code/core/content_generator.h` | QR content generation |
| `qrcodegen` | `features/qr_code/core/qrcodegen.hpp` | QR code library (bundled) |

#### Image Processing

Image-processing tools (resize, rotation, division, transparency) are unified
into a single GUI module. Core classes provide I/O and the edit interface that
all image tools share.

| Class | File | Description |
|-------|------|-------------|
| `ImageIO` | `features/image/core/basic_image_io.h` | Image I/O operations |
| `BasicImageEditInterface` | `features/image/core/basic_image_edit_interface.h` | Shared edit interface |
| `ImageToolsUnifiedGui` | `features/image/gui/basic/image_tools_unified_gui.h` | Unified image tools UI |

#### Data Conversion

| Class | File | Description |
|-------|------|-------------|
| `DataConversion` | `features/data_conversion/core/data_conversion.h` | Conversion coordinator |
| `JsonParser` | `features/data_conversion/core/parser/json_parser.h` | JSON parsing |
| `YamlParser` | `features/data_conversion/core/parser/yaml_parser.h` | YAML parsing |
| `TomlParser` | `features/data_conversion/core/parser/toml_parser.h` | TOML parsing |
| `JsonEmitter` | `features/data_conversion/core/emitter/json_emitter.h` | JSON output |
| `YamlEmitter` | `features/data_conversion/core/emitter/yaml_emitter.h` | YAML output |
| `TomlEmitter` | `features/data_conversion/core/emitter/toml_emitter.h` | TOML output |

### Exception Classes

| Class | File | Description |
|-------|------|-------------|
| `CommonException` | `features/framework/core/exception/common_exception.h` | Base exception |
| `InvalidArgumentException` | `features/framework/core/exception/invalid_argument_exception.h` | Invalid argument |
| `OutOfRangeException` | `features/framework/core/exception/out_of_range_exception.h` | Out of range |
| `InvalidStateException` | `features/framework/core/exception/invalid_state_exception.h` | Invalid state |
| `UnderDevelopmentException` | `features/framework/core/exception/under_development_exception.h` | Not implemented |

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
#include "features/framework/gui/gui_tool.h"

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
#include "features/data_conversion/core/parser/json_parser.h"
#include "features/data_conversion/core/emitter/yaml_emitter.h"

JsonParser parser;
auto data = parser.parse(jsonString);

if (data) {
    YamlEmitter emitter;
    QString yaml = emitter.emit(*data);
}
```

### Using Image I/O

Image processing tools are unified into `ImageToolsUnifiedGui`. The standalone
`ImageResize` / `ImageRotation` / `ImageDivision` / `ImageTransparent` classes
no longer exist; use the shared I/O and edit interface to operate on images at
the API level.

```cpp
#include "features/image/core/basic_image_io.h"

QImage image;
if (ImageIO::save("output.png", image, "PNG")) {
    // image saved successfully
}
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
