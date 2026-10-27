# M1 diff tool service record

## Boundary

`DiffToolGUI` owns the two text editors, the 150 ms render debounce, the
inline/side-by-side selector, HTML presentation and change highlighting.
`DiffTool` in `features/diff_tool/core/` owns line splitting, line-level
comparison and side-by-side row grouping. It has no Widget or display-HTML
dependency. It inherits `Tool` only to retain the application's tool identity;
`Tool` is a Qt Core `QObject` type.

## Input and output contract

- `splitLines(text)` normalizes LF, CRLF and CR to LF. A trailing newline does
  not produce an additional final row; interior empty lines are retained.
- `diffLines(left, right)` returns `Equal`, `Delete` or `Insert` records. Line
  numbers are one-based; the missing side is numbered zero. Equal-length LCS
  choices prefer deletion, which makes output stable for repeated lines.
- `diff(leftText, rightText)` composes those operations after line splitting.
- `groupSideBySide(lines)` emits equal rows unchanged, pairs an immediately
  adjacent Delete followed by Insert as one changed row, and pads unpaired
  rows with empty text.

The public values are `DiffLine` and `SideBySideRow`, using `QString`,
`QStringList` and `QVector`. The service has no I/O or persistent side effects.
The GUI escapes user text before it creates display HTML.

## Tests and limits

`test_diff_tool` compiles the service source directly as a `CORE_ONLY` test and
links `DevTools_core` for the inherited `Tool` symbols. It covers newline
normalization, empty/interior lines, equal/insert/delete/replace operations,
line numbers and row grouping.

The LCS implementation uses O(n*m) time and memory for n and m input lines.
There is no input-size guard or cancellation path, so a future host Bridge must
set a size limit before accepting untrusted or very large input. Qt value
types and the `Tool` QObject identity remain part of the C++ API; neither is a
Widget dependency.

## Future host contract

Candidate request: `{leftText, rightText}`. Candidate response:
`{lines:[{operation, leftLine, rightLine, text}], sideBySideRows}`. A Bridge
must version the operation names, preserve line-number and newline rules, set
an input limit, and avoid returning rendered HTML as the data contract. The
LCS and DTO mapping could be implemented in Rust after those rules and tie
breaking are fixed by parity tests.
