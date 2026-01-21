# Command Execution

Execute shell commands from the DevTools GUI and view output in real-time.

## Overview

The Command Execution tool provides a simple interface for running shell commands without switching to a terminal application.

## Accessing the Tool

1. Launch DevTools
2. Click **Command** in the side menu

## Interface

- **Command Input**: Text field for entering commands
- **Execute Button**: Run the command
- **Output Area**: Display command output
- **Clear Button**: Clear the output area

## Basic Usage

### Running a Command

1. Enter a command in the input field
2. Click **Execute** (or press Enter)
3. View the output in the output area

### Example Commands

```bash
# List files
ls -la

# Show current directory
pwd

# Display system info
uname -a

# Show disk usage
df -h

# Display memory info
top -l 1 | head -n 10
```

## Command Examples

### File Operations

```bash
# List files in current directory
ls

# List all files including hidden
ls -la

# Show file contents
cat filename.txt

# Count lines in file
wc -l filename.txt
```

### System Information

```bash
# macOS version
sw_vers

# CPU info
sysctl -n machdep.cpu.brand_string

# Memory info
vm_stat

# Disk space
df -h
```

### Development Tasks

```bash
# Check Git status
git status

# Show Git branch
git branch

# Run npm commands
npm --version

# Python version
python3 --version
```

### Network

```bash
# Check connectivity
ping -c 3 google.com

# Show IP address
ifconfig en0 | grep inet

# DNS lookup
nslookup example.com
```

## Working Directory

Commands execute in the context of DevTools' working directory. To work in a specific directory:

```bash
# Change directory and run command
cd /path/to/directory && ls

# Or use absolute paths
ls /path/to/directory
```

## Environment

- Commands run in a standard shell environment
- User environment variables are available
- PATH is inherited from the system

## Output Handling

### Standard Output

Normal command output is displayed in the output area.

### Error Output

Error messages (stderr) are also captured and displayed.

### Long Output

- Output is displayed as it's received
- Scroll to see all output
- Use **Clear** to reset

## Limitations

### Not Supported

- Interactive commands requiring user input
- Commands that need a TTY (e.g., `vim`, `nano`)
- Long-running background processes
- Commands requiring sudo password

### Recommended Alternatives

For unsupported operations:
- Use Terminal.app for interactive commands
- Use appropriate GUI applications

## Security Considerations

### Safe Commands

- Read-only commands (ls, cat, pwd)
- Information queries (git status, npm --version)
- Network diagnostics (ping, nslookup)

### Use Caution

- Commands that modify files
- Commands that delete data
- Commands affecting system configuration

### Avoid

- Commands requiring elevated privileges
- Destructive commands without backups
- Commands from untrusted sources

## Tips

### Command Chaining

Use `&&` to run multiple commands:
```bash
cd project && npm install && npm test
```

### Redirecting Output

Save output to a file:
```bash
ls -la > file_list.txt
```

### Piping

Use pipes for data processing:
```bash
cat file.txt | grep "search" | wc -l
```

## Troubleshooting

### Command Not Found

1. **Check spelling**: Command name must be exact
2. **Check PATH**: Command may not be in PATH
3. **Use full path**: `/usr/bin/command`

### Permission Denied

1. **Check file permissions**: May need chmod
2. **Avoid sudo**: Not supported in this interface
3. **Use Terminal**: For privileged operations

### No Output

1. **Command successful**: Some commands produce no output
2. **Check stderr**: Error may have occurred
3. **Add verbose flag**: Try `-v` flag if supported

### Command Hangs

1. **Timeout**: Long commands may timeout
2. **Interactive**: Command may be waiting for input
3. **Cancel**: Close and reopen the tool

## Best Practices

1. **Test commands**: Know what a command does before running
2. **Use safe flags**: Prefer `-n` (dry-run) when available
3. **Keep backups**: Before modifying files
4. **Review output**: Check results before proceeding

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
