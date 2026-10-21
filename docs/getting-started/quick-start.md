# Quick Start Guide

Get up and running with DevTools in 5 minutes.

## Prerequisites

Before starting, ensure you have:
- macOS 15.0 or later (Apple Silicon)
- [DevTools installed](installation.md)

## Launching DevTools

### From Terminal

The simplest way is `cmake --build . --target run` from the build directory,
which also shows colored console output and replaces older project-local
processes before starting.

```bash
cd /path/to/devtools/build
cmake --build . --target run
```

Or run the binary directly:

```bash
cd /path/to/devtools/build
./DevTools.app/Contents/MacOS/DevTools
```

Use the terminal command for source validation instead of launching a second
copy from Finder or the Dock. If another DevTools bundle outside the source
tree is running, the target reports it explicitly so it can be closed before
validation. Separately packaged builds are not interchangeable with the
development build in `build/`.

## Application Overview

When you launch DevTools, you'll see:

1. **Side Menu** (left) - Navigate between tools
2. **Main Content Area** (center) - Tool interface
3. **Menu Bar** (top) - File and DevTools menus

## Using the Tools

### QR Code Generation

1. Click **QR Code Generation** in the side menu
2. Select the QR code type (Text, URL, Email, etc.)
3. Enter your content
4. Click **Generate**
5. Click **Save Image** to save the QR code

**Example: Create a URL QR Code**
1. Select **URL** from the QR code type selector
2. Enter: `https://example.com`
3. Click **Generate**
4. Click **Save Image**

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

1. Click **Data/Format Conversion** in the side menu
2. Paste or type your data in the input area
3. Select the output format
4. Select the indentation style
5. Read the converted result in the output area

**Example: Convert JSON to YAML**
1. Paste your JSON:
   ```json
   {"name": "DevTools"}
   ```
2. Set output: **YAML (Block style)**
3. Choose an indentation style
4. Result:
   ```yaml
   name: DevTools
   ```

### HTTP Request

1. Click **HTTP Request** in the side menu
2. Enter the URL
3. Select HTTP method (GET, POST, etc.)
4. For query parameters or Basic authentication, use the corresponding Options tab
5. Add a JSON body in the **Body** tab for POST/PUT requests
6. Click **Send**
7. View the read-only response

**Example: GET Request**
1. Enter: `https://api.github.com`
2. Select **GET**
3. Click **Send**
4. View the JSON response

### Command Generation

1. Click **Command Generation** in the side menu
2. Select a command category and command
3. Select an option and enter a value when requested
4. Click **Generate command**
5. Copy the generated command

The tool generates command text; it does not execute shell commands.

### DB Tool

1. Click **DB Tool** in the side menu
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

1. Click **Phrase Generation** in the side menu
2. Click **Add** to create a template, or select an existing template
3. Enter the title and template text
4. Click **Save**
5. Click **Copy** to copy the template text

### Markdown Preview

1. Click **Markdown Preview** in the side menu
2. Type or paste Markdown in the **Editor** pane
3. Review the rendered result in the **Preview** pane

### Regex Tester

1. Click **Regex Tester** in the side menu
2. Enter a regular expression and test text
3. Choose flags or a preset when needed
4. Review matches and replacement results

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
dark mode, the application updates the qlementine widget theme. Material Symbols are bundled
 as an application font, and their icons follow the active UI palette instead of using separate
 light and dark SVG files.

Bundled qlementine themes are stored in `res/themes/light.json` and
`res/themes/dark.json` and are loaded from the Qt resource prefix `:/themes`.

## Next Steps

- [User Guide](../user-guide/overview.md) - Detailed feature documentation
- [Troubleshooting](../troubleshooting/common-issues.md) - If something doesn't work
- [Contributing](https://github.com/LotusAndCompany/devtools/blob/main/CONTRIBUTING.md) - Help improve DevTools

## Getting Help

- Check the [FAQ](../troubleshooting/faq.md)
- Report issues on [GitHub](https://github.com/LotusAndCompany/devtools/issues)
