# Frequently Asked Questions

## General

### What is DevTools?

DevTools is a unified desktop application that combines various development tools into a single interface. It provides QR code generation, image processing, data conversion, API testing, command execution, and phrase generation.

### What platforms are supported?

Currently, DevTools supports:
- **macOS 15.0 or later**
- **Apple Silicon (arm64) only**

Intel Mac support is not currently available.

### Is DevTools free?

Yes, DevTools is open source under the Apache License 2.0. You can use, modify, and distribute it freely.

### Where can I get DevTools?

- **Source code**: [GitHub Repository](https://github.com/LotusAndCompany/devtools)
- **Releases**: [GitHub Releases](https://github.com/LotusAndCompany/devtools/releases)

## Installation

### Do I need to install Qt to use DevTools?

**For users**: No, the distributed application includes all necessary Qt libraries.

**For developers**: Yes, you need Qt 6.x installed to build from source.

### How do I install vcpkg?

```bash
git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh
export PATH="$HOME/vcpkg:$PATH"
```

See [Installation Guide](../getting-started/installation.md) for details.

### Why does the build fail with "Qt not found"?

Qt might not be in your PATH. Set the Qt6_DIR:

```bash
export Qt6_DIR=/opt/homebrew/lib/cmake/Qt6
```

Or install via Homebrew:
```bash
brew install qt@6
```

## Features

### What image formats are supported?

**Input**: PNG, JPEG, BMP, GIF, TIFF
**Output**: PNG, JPEG

### What data formats can I convert?

DevTools supports conversion between:
- JSON
- YAML
- TOML

### Can I generate QR codes for WiFi networks?

Not currently. Supported QR code types:
- Text
- URL
- Email (mailto:)
- Phone (tel:)
- SMS
- Geographic location (geo:)

WiFi support may be added in future versions.

### Are generated QR codes saved automatically?

No, you need to save them manually. Right-click the generated QR code and select "Save Image".

## Usage

### How do I change the language?

1. Go to **DevTools > Preferences**
2. Select the **Language** tab
3. Choose your language
4. Restart the application

### How do I change the theme?

Go to **View > Theme** and select **Light** or **Dark**.

### Can I use keyboard shortcuts?

Yes, common shortcuts:
- `Cmd + Q`: Quit
- `Cmd + ,`: Preferences
- `Cmd + C`: Copy
- `Cmd + V`: Paste

### Why does the API test feature not work with HTTPS?

The API testing feature supports HTTPS. If you're having issues:
1. Check the URL includes `https://`
2. Verify the server's SSL certificate is valid
3. Check your network connection

## Development

### How do I contribute?

1. Fork the repository
2. Create a feature branch
3. Make changes
4. Submit a pull request

See [Contributing Guide](../../CONTRIBUTING.md) for details.

### How do I add a new tool?

See [Adding New Tools](../development/adding-new-tools.md) for a step-by-step guide.

### How do I run tests?

```bash
cmake .. -DENABLE_UNIT_TEST=ON
make
ctest
```

### Where is the API documentation?

- Online: [https://LotusAndCompany.github.io/devtools/](https://LotusAndCompany.github.io/devtools/)
- Generate locally: `make DevTools_docs`

## Troubleshooting

### The application crashes on startup

1. Delete preferences and try again
2. Check macOS version is 15.0 or later
3. Check you're on Apple Silicon
4. Try downloading a fresh copy

### Images aren't loading

1. Check the file format is supported
2. Check file permissions
3. Check file isn't corrupted
4. Try a different file

### Data conversion gives wrong output

1. Validate your input syntax
2. Check you selected the correct input format
3. Note: Comments are lost when converting from YAML/TOML to JSON

### Command execution hangs

Some commands aren't supported:
- Interactive commands (vim, nano)
- Commands requiring TTY
- Commands needing password input

Use Terminal.app for these.

## Future Plans

### Will Windows/Linux be supported?

There are no immediate plans for Windows or Linux support. The current focus is on macOS.

### Will Intel Mac be supported?

There are no immediate plans for Intel Mac support. Apple Silicon (arm64) is the primary target.

### What features are planned?

Check the [GitHub Issues](https://github.com/LotusAndCompany/devtools/issues) for planned features and ongoing development.

## Contact

### How do I report a bug?

Create an issue on [GitHub Issues](https://github.com/LotusAndCompany/devtools/issues) with:
- DevTools version
- macOS version
- Steps to reproduce
- Expected vs actual behavior
- Screenshots if helpful

### How do I request a feature?

Create an issue on [GitHub Issues](https://github.com/LotusAndCompany/devtools/issues) with:
- Description of the feature
- Use case / why it's useful
- Any implementation suggestions

## Related Documentation

- [User Guide](../user-guide/overview.md) - Feature documentation
- [Common Issues](common-issues.md) - Problem solutions
- [Installation](../getting-started/installation.md) - Setup guide
