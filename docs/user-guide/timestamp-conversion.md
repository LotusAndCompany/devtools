# Timestamp Conversion

Convert between Unix timestamps, local and UTC date/time, and ISO 8601 in
DevTools.

## Accessing the Tool

1. Launch DevTools
2. Click **Timestamp Conversion** in the sidebar

## Interface

The tool shows six rows, grouped into three sections, that all represent the
same instant in different formats:

- **Unix Timestamp**
  - **Seconds**: Unix timestamp in seconds
  - **Milliseconds**: Unix timestamp in milliseconds
- **Date & Time**
  - **Local**: date and time in the system's local timezone (seconds
    precision)
  - **UTC**: date and time in UTC (seconds precision)
- **ISO 8601**
  - **UTC**: ISO 8601 string in UTC with a `Z` suffix and milliseconds, e.g.
    `2026-09-13T20:32:37.497Z`
  - **Local**: ISO 8601 string with the local UTC offset and milliseconds,
    e.g. `2026-09-14T05:32:37.497+09:00`

Each row has its own **Convert** and **Copy** button.

## How to Use

1. Enter a value in any one row
2. Click that row's **Convert** button
3. The other five rows update to the same instant

Editing a row's text does not update the other rows by itself; you must press
**Convert** on that row. Click **Now** to set every row to the current time.
Click a row's **Copy** button to copy that row's displayed value to the
clipboard.

**Example: Convert a Unix timestamp to a date**
1. Enter `1700000000` in **Unix Timestamp > Seconds**
2. Click **Convert** next to that row
3. Read the corresponding value in **Date & Time > Local** (or any other row)

## Notes

- Unix timestamps must be entered as integers
- **Local** rows use the system's timezone
- Date & Time rows display seconds precision; ISO 8601 rows include
  milliseconds
- An invalid value shows an error message below the rows and leaves every
  row unchanged

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
