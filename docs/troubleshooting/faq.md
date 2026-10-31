# Frequently Asked Questions

## General

### What is DevTools?

DevTools combines 11 developer tools in one desktop application. The tools cover QR codes, image editing, data conversion, HTTP requests, Markdown preview, command generation, phrase templates, databases, regular expressions, timestamps, and text diffs.

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

**For developers**: Yes. You need Qt 6.9 or later, Bun 1.3.14 or later, and vcpkg to build from source.

### How do I install vcpkg?

```bash
git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh
export PATH="$HOME/vcpkg:$PATH"
export VCPKG_ROOT="$HOME/vcpkg"
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

**Input**: PNG, JPEG, BMP, GIF, and WebP appear in the Open image dialog.
Additional formats depend on the installed Qt image plugins.
**Output**: Formats supported by the installed Qt image plugins

### What data formats can I convert?

DevTools supports conversion between:
- JSON
- YAML
- TOML

### Can I generate QR codes for Wi-Fi networks?

Yes. Select **Wi-Fi**, enter the network name and security details, then click
**Generate**. The QR tool also supports text, URLs, email, phone, SMS, contacts,
calendar events, and locations.

### What databases are supported?

The Database Management tool supports:
- **SQLite** (file-based)
- **MySQL** (server-based)
- **PostgreSQL** (server-based)

Appropriate Qt SQL driver plugins must be installed for MySQL and PostgreSQL.

### Are generated QR codes saved automatically?

No. After generating a code, click **Save image** and choose a destination.

## Usage

### How do I change the language?

1. Click **Settings** at the bottom of the sidebar
2. Choose **English** or **日本語**
3. Click **Save**; the interface changes immediately

### How do I change the theme?

Open **Settings** and choose **Follow system**, **Light**, or **Dark** from
**Theme**.

### Can I use keyboard shortcuts?

Use `Ctrl + W` to close the window and `Cmd/Ctrl + K` to focus tool search.
Standard copy and paste shortcuts are also available in editable fields.

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

See [Contributing Guide](https://github.com/LotusAndCompany/devtools/blob/main/CONTRIBUTING.md) for details.

### How do I add a new tool?

See [Adding New Tools](../development/adding-new-tools.md) for a step-by-step guide.

### How do I run the app from the terminal?

```bash
cd build
cmake --build . --target run
```

This builds and launches the app with colored console output (DEBUG=dim, WARN=yellow, CRIT=red).

### How do I run tests?

Configure the existing build with unit tests enabled, then build and run them:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx \
  -DENABLE_UNIT_TEST=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### Where is the API documentation?

- Online: [https://LotusAndCompany.github.io/devtools/](https://LotusAndCompany.github.io/devtools/)
- Generate locally: `cmake --build build --target DevTools_docs`

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

### Does DevTools execute generated commands?

No. Command Generation builds command text and copies it to the clipboard.
Run the command in a terminal yourself.

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
