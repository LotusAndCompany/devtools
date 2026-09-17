# Command Generation

Build common Git and Docker commands from the DevTools GUI.

## Overview

The Command Generation tool helps construct commands by selecting a command
category, command, and any required option values. It generates the command as
text; it does not execute commands.

## Accessing the Tool

1. Launch DevTools
2. Click **Command Generation** in the side menu

## Interface

- **Category**: Select Git, Docker, or Docker Compose
- **Function**: Select a command from the chosen category
- **Option**: Select an available option when the command provides one
- **Input**: Enter a required value when prompted
- **Generated Result**: Review the generated command
- **Copy**: Copy the generated command to the clipboard
- **Clear**: Clear the generated result

## How to Use

1. Select a category
2. Select a command
3. Select an option if one is available
4. Enter the requested value, if any
5. Click **Generate command**
6. Click **Copy** to use the command in a terminal

Click **Reset** to return all selectors and inputs to their initial state.

## Supported Command Groups

### Git

Includes commands such as `add`, `commit`, `log`, `status`, `diff`, `merge`,
and `remote`.

### Docker

Includes image, container, network, volume, build, pull, push, and log
commands.

### Docker Compose

Includes service build, start, stop, restart, logs, status, and cleanup
commands.

## Limitations

- The tool only generates command text
- Command execution, shell history, and validation are not provided
- Available options depend on the selected command

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
