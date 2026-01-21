# DevTools

[日本語版 README](docs/ja/README.md)

A unified desktop application that combines various development tools into a single, convenient interface. DevTools eliminates the need for multiple web services by providing all essential developer utilities in one place.

<!-- TODO: Add screenshot -->
<!-- ![DevTools Screenshot](docs/images/screenshot.png) -->

## Features

### QR Code Generation
- **Text**: Generate QR codes from plain text
- **URL**: Create QR codes for website links
- **Email**: Generate mailto: QR codes
- **Phone**: Create tel: QR codes for phone numbers
- **SMS**: Generate SMS QR codes
- **Geo**: Create location-based QR codes

### Image Processing
- **Resize**: Scale images to specific dimensions
- **Rotation**: Rotate images by any angle
- **Division**: Split images into multiple parts
- **Transparency**: Make specific colors transparent

### Data Conversion
- Convert between **JSON**, **YAML**, and **TOML** formats
- Syntax validation and formatting

### API Testing
- Send HTTP requests (GET, POST, PUT, DELETE, etc.)
- View responses with syntax highlighting

### Command Execution
- Execute shell commands from the GUI
- View command output in real-time

### Phrase Generation
- Generate random phrases and text

## Requirements

- **Platform**: macOS 15.0+ (Apple Silicon / arm64 only)
- **Qt**: 6.x
- **CMake**: 3.21.1+
- **C++ Compiler**: C++17 compatible

## Quick Start

```bash
# Clone the repository
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools

# Install dependencies via vcpkg
vcpkg install

# Configure with CMake
mkdir build && cd build
cmake ..

# Build
make
```

For detailed build instructions, see [BUILD.md](BUILD.md).

## Documentation

### Quick Links
- [Documentation Index](docs/index.md)
- [Quick Start Guide](docs/getting-started/quick-start.md)
- [Installation Guide](docs/getting-started/installation.md)

### Detailed Documentation
- [User Guide](docs/user-guide/overview.md) - How to use each feature
- [Development Guide](docs/development/architecture.md) - For contributors
- [API Documentation](https://LotusAndCompany.github.io/devtools/) (Doxygen)

### Other Resources
- [Build Instructions](BUILD.md) / [ビルド手順](docs/ja/BUILD.md)
- [Contributing Guide](CONTRIBUTING.md) / [貢献ガイド](docs/ja/CONTRIBUTING.md)
- [Troubleshooting](docs/troubleshooting/common-issues.md) / [FAQ](docs/troubleshooting/faq.md)

## Contributing

We welcome contributions! Please read our [Contributing Guide](CONTRIBUTING.md) for details on:

- How to report bugs
- How to request features
- How to submit pull requests
- Code style guidelines

## Security

For security vulnerabilities, please see our [Security Policy](SECURITY.md).

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

### Third-Party Licenses

DevTools uses the following third-party libraries:

| Library | License |
|---------|---------|
| Qt6 | LGPL v3 |
| toml11 | MIT |
| yaml-cpp | MIT |
| qrcodegen | MIT |
| Material Icons | Apache 2.0 |
| Remix Icon | Apache 2.0 |

## Acknowledgments

- [Qt Project](https://www.qt.io/) for the excellent cross-platform framework
- [Project Nayuki](https://www.nayuki.io/) for the QR code generator library
- All contributors and users of DevTools

---

Made with by [Lotus&Company Inc.](https://lotusandcompanyinc.com/)
