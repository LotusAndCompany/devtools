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

The sidebar contains Home, tool search, the 11 tools, Settings, and About. The
top bar shows the selected screen and has a button to collapse or restore the
sidebar. The main area shows one selected screen at a time. Settings and About
open as dialogs; tabs remain inside tools that use them, such as HTTP Request
and Database.

## Using the Tools

### QR Code Generation

1. Click **QR Code** in the sidebar
2. Select a type from **QR type** (Text, URL, Email, Wi-Fi, and others)
3. Enter your content
4. Click **Generate**
5. Click **Save image** to save the QR code

**Example: Create a URL QR Code**
1. Select **URL** from the QR code type selector
2. Enter: `https://example.com`
3. Click **Generate**
4. Click **Save image**

### Image Processing

1. Click **Image Editor** in the sidebar
2. Click **Open image** or drop an image into the window
3. Use the Resize, Rotate and flip, Make a color transparent, or Split image card
4. Click the matching Apply button, click a preview pixel to make its color transparent, or choose a folder to split the image
5. Click **Save image**

**Example: Resize an Image**
1. Enter the target width and height in the Resize card
2. Select **Keep aspect ratio** if needed
3. Click **Apply size**
4. Click **Save image**

### Data Conversion

1. Click **Data Conversion** in the sidebar
2. Paste or type your data in the input area
3. Select the output format and indentation
4. Click **Convert**
5. Read or edit the result in the output area

**Example: Convert JSON to YAML**
1. Paste your JSON:
   ```json
   {"name": "DevTools"}
   ```
2. Set **Output format** to **YAML (block)**
3. Choose an indentation style
4. Result:
   ```yaml
   name: DevTools
   ```

### HTTP Request

1. Click **HTTP Request** in the sidebar
2. Enter the URL
3. Select HTTP method (GET, POST, etc.)
4. Use the **Parameters** tab for query parameters or **Authentication** for
   Basic authentication
5. Add a JSON body in the **Body** tab for POST/PUT requests
6. Click **Send**
7. View the read-only response

**Example: GET Request**
1. Enter: `https://api.github.com`
2. Select **GET**
3. Click **Send**
4. View the JSON response

### Command Generation

1. Click **Command Generation** in the sidebar
2. Select a command category and command
3. Select an option and enter a value when requested
4. Click **Generate command**
5. Copy the generated command

The tool generates command text; it does not execute shell commands.

### Database

1. Click **Database** in the sidebar
2. Click **Connection settings** to open the connection dialog
3. Choose a saved connection or enter a new one
4. Click a table name to place a SELECT query in the current SQL tab
5. Click **New SQL** to create another query tab
6. Enter SQL and click **Execute**

**Example: Browse a SQLite Database**
1. Click **Connection settings**
2. Select **SQLite**
3. Click **Choose SQLite file** and select your `.db` file
4. Click **Connect**
5. Click a table name to fill the SQL editor with a SELECT query
6. Click **Execute** to view the rows

### Phrase Generation

1. Click **Phrase Generation** in the sidebar
2. Click **New** to create a template, or select an existing template
3. Enter the title and template text
4. Click **Save template**
5. Click **Copy text** to copy the template text

### Markdown Preview

1. Click **Markdown Preview** in the sidebar
2. Type or paste Markdown in the **Editor** pane
3. Review the rendered result in the **Preview** pane

### Regex Tester

1. Click **Regex Tester** in the sidebar
2. Enter a regular expression and test text
3. Choose flags or a preset when needed
4. Review matches and replacement results

### Timestamp Conversion

1. Click **Timestamp Conversion** in the sidebar
2. Enter a value in any row (Unix Timestamp, Date & Time, or ISO 8601)
3. Click that row's **Convert** button
4. Read the converted value in the other rows

**Example: Convert a Unix timestamp**
1. Enter `1700000000` in **Unix Timestamp > Seconds**
2. Click **Convert** next to that row
3. Read the local date/time, UTC date/time, and ISO 8601 values

### Diff Comparison Tool

1. Click **Text Diff** in the sidebar
2. Paste the original text in the **Original** pane
3. Paste the modified text in the **Modified** pane
4. Choose **Inline** or **Side-by-side** to review the differences

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl + W` | Close window |
| `Cmd/Ctrl + K` | Focus tool search |

## Changing Language

DevTools supports English and Japanese. Open **Settings** at the bottom of the
sidebar, choose a language, and click **Save**. The interface changes
immediately.

## Changing Theme

Choose **Follow system**, **Light**, or **Dark** from **Theme** in Settings.
The application colors update to match the selected theme.


## Next Steps

- [User Guide](../user-guide/overview.md) - Detailed feature documentation
- [Troubleshooting](../troubleshooting/common-issues.md) - If something doesn't work
- [Contributing](https://github.com/LotusAndCompany/devtools/blob/main/CONTRIBUTING.md) - Help improve DevTools

## Getting Help

- Check the [FAQ](../troubleshooting/faq.md)
- Report issues on [GitHub](https://github.com/LotusAndCompany/devtools/issues)
