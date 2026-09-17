# Regular Expression Tester

Test regular expressions, inspect matches, and preview replacement results in
DevTools.

## Accessing the Tool

1. Launch DevTools
2. Click **Regex Tester** in the side menu

## Interface

- **Preset pattern**: Start with a built-in email, URL, IPv4, date, phone, or
  UUID pattern
- **Regular expression pattern**: Enter the pattern to test
- **Flags**: Toggle global, case-insensitive, multiline, singleline, Unicode,
  and extended matching
- **Test text**: Enter the text to search
- **Replacement string**: Enter replacement text
- **Matches**: Review each match and its position; capture groups are included
  when available
- **Quick Reference**: Browse common regular-expression tokens and
  double-click a token to insert it into the pattern

## How to Use

1. Enter a pattern or choose a preset
2. Enter test text
3. Enable the flags required by the pattern
4. Review the match results
5. Enter replacement text to preview substitutions

Use **Copy Pattern**, **Copy Matches**, or **Copy Result** to copy the
corresponding content.

## Supported Flags

| Flag | Meaning |
|------|---------|
| `g` | Match all occurrences |
| `i` | Ignore case |
| `m` | Treat the text as multiple lines |
| `s` | Let `.` match newline characters |
| `u` | Enable Unicode mode |
| `x` | Ignore unescaped whitespace in the pattern |

## Limitations

- The tester does not execute or save regular-expression files
- Invalid patterns are shown as validation errors
- Evaluation is stopped when catastrophic backtracking exceeds the safety
  timeout

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
