# Quick Start Guide

Get up and running with DevTools in 5 minutes.

## Prerequisites

Before starting, ensure you have:
- macOS 15.0 or later (Apple Silicon)
- [DevTools installed](installation.md)

## Launching DevTools

### From Terminal

The simplest way is `cmake --build . --target run` from the build directory,
which also shows colored console output.

```bash
cd /path/to/devtools/build
cmake --build . --target run
```

Or run the binary directly:

```bash
cd /path/to/devtools/build
./DevTools.app/Contents/MacOS/DevTools
```

### From Finder

Navigate to the build directory and double-click `DevTools.app`.

## Application Overview

When you launch DevTools, you'll see:

1. **Side Menu** (left) - Navigate between tools
2. **Main Content Area** (center) - Tool interface
3. **Menu Bar** (top) - File and DevTools menus

## Using the Tools

### QR Code Generation

1. Click **QR Code** in the side menu
2. Select the QR code type (Text, URL, Email, etc.)
3. Enter your content
4. Click **Generate**
5. Save the QR code image

**Example: Create a URL QR Code**
1. Select **URL** tab
2. Enter: `https://example.com`
3. Click **Generate**
4. Right-click to save the image

### Image Processing

1. Click **Image** in the side menu
2. Choose a tool: Resize, Rotate, Split, or Transparency
3. Load an image
4. Adjust settings
5. Process and save

**Example: Resize an Image**
1. Select **Resize** tab
2. Click **Open Image**
3. Set width and height
4. Check **Keep Aspect Ratio** if needed
5. Click **Resize**
6. Save the result

### Data Conversion

1. Click **Data Conversion** in the side menu
2. Paste or type your data in the input area
3. Select input format (JSON, YAML, or TOML)
4. Select output format
5. Click **Convert**

**Example: Convert JSON to YAML**
1. Paste your JSON:
   ```json
   {"name": "DevTools"}
   ```
2. Set input: **JSON**
3. Set output: **YAML**
4. Click **Convert**
5. Result:
   ```yaml
   name: DevTools
   ```

### API Testing

1. Click **API** in the side menu
2. Enter the URL
3. Select HTTP method (GET, POST, etc.)
4. Add headers if needed
5. Add body for POST/PUT requests
6. Click **Send**
7. View the response

**Example: GET Request**
1. Enter: `https://api.github.com`
2. Select **GET**
3. Click **Send**
4. View the JSON response

### Command Execution

1. Click **Command** in the side menu
2. Enter a shell command
3. Click **Execute**
4. View the output

**Example: List Files**
1. Enter: `ls -la`
2. Click **Execute**
3. View the directory listing

### Database Management

1. Click **DB** in the side menu
2. Click **DB Connection Settings** to open connection settings
3. Create a new connection or select from history
4. Browse tables in the left panel
5. Click **SQL** to open a query tab
6. Enter SQL and click **Execute** to execute

**Example: Browse a SQLite Database**
1. Click **DB Connection Settings** > **New Connection**
2. Select **SQLite**
3. Click **Browse...** and select your `.db` file
4. Click **Connect**
5. Click a table name to view its data

### Phrase Generation

1. Click **Phrase** in the side menu
2. Configure generation options
3. Click **Generate**
4. Copy the generated text

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl + W` | Close window |

## Changing Language

DevTools supports English and Japanese:

1. Go to **DevTools > Settings**
2. Select **Language**
3. Choose your preferred language
4. Click **Apply** or **OK**; the UI changes immediately

## Changing Theme

DevTools follows the macOS system appearance. When the system switches between light and
dark mode, the application updates both the qlementine widget theme and the icon theme.

Bundled qlementine themes are stored in `res/themes/light.json` and
`res/themes/dark.json` and are loaded from the Qt resource prefix `:/themes`.

## Next Steps

- [User Guide](../user-guide/overview.md) - Detailed feature documentation
- [Troubleshooting](../troubleshooting/common-issues.md) - If something doesn't work
- [Contributing](https://github.com/LotusAndCompany/devtools/blob/main/CONTRIBUTING.md) - Help improve DevTools

## Getting Help

- Check the [FAQ](../troubleshooting/faq.md)
- Report issues on [GitHub](https://github.com/LotusAndCompany/devtools/issues)
