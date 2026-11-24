# M1 HTTP request service

## Scope and current behavior

- Objective: move HTTP request construction, transport, response data and query
  parameter encoding out of the Widget while retaining existing UI behavior.
- Scope: HTTP core service, existing HTTP Widget, CMake registration, and
  core-only and Widget integration tests. No new dependency or UI strings.
- Input: GET, POST, PUT or DELETE, URL, username, password and body. Requests
  retain the JSON content type; credentials are sent as Basic auth only when
  both fields are nonempty, using the same local-byte encoding as before.
  POST and PUT send the UTF-8 body; GET and DELETE do not send it.
- Output: request ID, raw response bytes, status and reason, Qt network error,
  error text and elapsed milliseconds. The Widget retains its existing
  translated status/error formatting and UTF-8 response display. An unsupported
  method returns ID 0 without a request. Query encoding strips the existing
  query and ignores empty keys, preserving ordering and duplicate keys.
- Qt dependencies: Core and Network. The service owns its network manager on
  its creating thread and emits results on that thread. It exposes cancellation
  by ID; the existing UI has no cancel button. Destroying the service destroys
  the manager and pending replies. The existing UI still displays every result
  in completion order, including an older request finishing after a newer one.

## Compatibility limits and remaining work

The previous UI used one shared start time for all outstanding requests; the
service deliberately retains that behavior, so overlapping requests can report
an elapsed time measured from the newest request. This is a known defect and
should be corrected as a separately reviewed behavior change. There is no
application-configured timeout or explicit timeout UI; Qt's default behavior
is retained. Results are fully buffered, and the service has not been tested
against external DNS, TLS, proxies, large responses or application shutdown
with the real native event loop. Credentials must not be logged or placed in
future bridge diagnostics. A bridge must preserve request ID and error fields,
validate its input, and decide cancellation and late-response policy explicitly.

The remaining M1 work includes the rest of the feature/UI audit, services,
contracts, integration and existing UI regression checks. This extraction does
not establish complete issue #132 parity.

## Verification

Environment: macOS 26.6.2 arm64, Qt 6.9.2, existing `build` directory,
`ENABLE_UNIT_TEST=ON`, Homebrew Qt/yaml-cpp/toml11. All HTTP test servers bind
only to the local loopback interface and use synthetic data.

- `cmake --build build --target format`: passed.
- `cmake --build build --target test_http_service DevTools_http_request -j 8`:
  passed.
- `ctest --test-dir build -R '^test_http_service$' --output-on-failure`:
  passed. The tests cover methods, request body and authorization, query
  encoding, invalid method and URL, 404 response, reversed completion order,
  cancellation and destruction with a request in flight.
- `ctest --test-dir build -R '^test_api_tool$' --output-on-failure`: passed.
  The Widget test sends a request through the real service, checks the query,
  response and status display, and destroys a Widget with a pending request.
- A sandboxed invocation with `QT_QPA_PLATFORM=offscreen` failed to bind a
  loopback socket with permission denied; direct CTest invocations passed.
  The restricted run is not evidence of an application network failure.
- `cmake --build build -j 8`: passed; `cmake --build build --target
  format-check`: passed.
- `ctest --test-dir build --output-on-failure`: 32/33 passed in 11.47 seconds.
  `test_db_connection_window` failed its two pre-existing path assertions due
  to `/var` versus `/private/var` spelling, as recorded in the M0 audit. This
  invocation had a different temporary-directory spelling from the earlier
  offscreen run that passed all 31 then-registered tests. No test was disabled.
- `otool -L build/test_http_service` listed Qt Core, Network and Test without
  Qt Widgets or Gui.
- No manual UI visual review was performed.

## Execution record

At 2026-10-27 10:29 JST the goal's recorded elapsed time was 443 seconds,
including the preceding extraction work and waits. Per-command model execution
time and human intervention time were not separately measured. No commit or
PR was created in this slice; the checkout remains on main with uncommitted
changes, and the existing untracked migration and article files were retained.
The next work is to continue extraction and parity testing for the remaining
features, including persistence and database operations.
