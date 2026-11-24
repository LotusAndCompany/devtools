# Diff Comparison Tool

Compare two texts and review the differences inline or side by side.

## Accessing the Tool

1. Launch DevTools
2. Click **Diff Comparison Tool** in the side menu

## Interface

- **Original**: Enter the text to treat as the baseline
- **Modified**: Enter the text to compare against the baseline
- **View mode**: Switch between **Inline** and **Side-by-side**
- **Diff**: Review the computed differences

The diff is recalculated automatically after each input change.

## How to Use

### Inline View

1. Paste or type the original text in the **Original** pane
2. Paste or type the modified text in the **Modified** pane
3. Select **Inline** from the view-mode selector
4. Removed lines are shown with a `-` prefix, added lines with a `+` prefix

### Side-by-side View

1. Enter both texts as above
2. Select **Side-by-side** from the view-mode selector
3. The original and modified lines are aligned in two columns, with removed
   lines on the left and added lines on the right

## Comparison Behavior

- Comparisons are made **line by line**, not character by character
- Line endings are normalized; CRLF, CR, and LF are treated as equivalent
- A difference consisting only of a trailing newline is not reported
- Leading, consecutive, and trailing whitespace is preserved in the diff
- The diff updates automatically as you type

## Limitations

- The tool uses a longest-common-subsequence algorithm. Comparing large texts
  takes O(n×m) time and memory, where n and m are the line counts of the two
  inputs, so very large inputs can be slow
- The tool does not save or load files; use copy and paste to transfer text

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
