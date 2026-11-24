# M1 timestamp conversion service record

## Boundary

`TimestampConversionGUI` owns input fields, local and UTC date-time editors,
copy/convert buttons, the current-time action and the localized invalid-input
message. `TimestampConversion` in `features/timestamp_conversion/core/`
converts values through a canonical `QDateTime` instant and does not depend on
Widgets or application UI state.

## Input and output contract

- `fromUnixTimestamp(text, unit)` trims the text, parses a signed 64-bit
  integer, interprets it as seconds or milliseconds since the Unix epoch, and
  returns a UTC `QDateTime`. Invalid integer text returns an invalid date-time.
- `fromIso8601(text)` trims and parses Qt ISO 8601 date-time text, including
  optional milliseconds and an explicit zone/offset, and returns the instant
  in UTC. Parse failure returns an invalid date-time.
- `toUnixTimestamp(instant, unit)` returns the epoch value as decimal text;
  an invalid date-time returns an empty string.
- `toIso8601(instant)` returns a UTC ISO 8601 string with milliseconds and `Z`.
- `toIso8601Local(instant)` returns ISO 8601 text with milliseconds and the
  system time-zone offset. An invalid date-time returns an empty string.

The core API uses Qt Core `QDateTime`, `QTimeZone` and `QString`. It has no
filesystem, network or persistent side effects. The date-time editors display
whole seconds; the Unix-millisecond and ISO rows retain millisecond precision.
When an invalid value is submitted, the UI displays a localized status and
leaves the other fields unchanged.

## Tests and limits

`test_timestamp_conversion` compiles the service source directly as a
`CORE_ONLY` Qt Test. It covers seconds/milliseconds, invalid inputs, ISO text
with and without milliseconds, UTC/local output and a round trip across the
representations.

Parsing, date range and daylight-saving behavior follow the configured Qt
version and its time-zone database. Local ISO output also depends on the
machine's system time zone. A deterministic host Bridge should accept an
explicit time-zone identifier for local output rather than silently inheriting
the host setting. Define the supported epoch range and invalid-value response
before exposing it as a cross-language API.

## Future host contract

Candidate requests: `{unixTimestamp, unit}` or `{iso8601}`. Candidate response:
`{unixSeconds, unixMilliseconds, iso8601Utc, iso8601Local, valid}`. The Bridge
should keep parsing and time-zone policy explicit. Integer parsing and DTO
mapping are possible Rust candidates after the accepted range, ISO behavior,
and Qt parity requirements are settled; time-zone conversion should remain
behind a tested platform adapter until parity is demonstrated.
