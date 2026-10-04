# Phrase Generation

Create and reuse named text templates in DevTools.

## Overview

The Phrase Generation tool stores templates as text files under the
application's `content/` directory. It is intended for reusable snippets,
test data, and frequently used messages.

## Accessing the Tool

1. Launch DevTools
2. Click **Phrase Generation** in the side menu

## Interface

- **Editor**: Enter a template title and its text
- **Templates**: Select an existing template or create a new one
- **Add**: Start a new template
- **Save**: Create or update the selected template
- **Copy**: Copy the editor text to the clipboard
- **Delete**: Remove the selected template

## How to Use

### Create a Template

1. Click **Add** (or press `Cmd + N`)
2. Enter a title
3. Enter the template text
4. Click **Save** (or press `Cmd + S`)

The saved template appears in the **Templates** list.

### Edit a Template

1. Select a template from the list
2. Change its title or text
3. Click **Save**

If you select another template while changes are unsaved, DevTools asks
whether to save or discard them.

### Copy or Delete a Template

- Click **Copy** to copy the current editor text
- Select a template and click **Delete** to remove it
- Confirm the delete operation when prompted

## Storage

- Templates are saved as `.txt` files in `content/`
- The first line stores the title
- The remaining text stores the template body
- The list is rebuilt from the files when the tool is opened

## Limitations

- Templates are local to the current application working directory
- A title and non-empty body are required to save
- There is no search, tagging, or cloud synchronization

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
