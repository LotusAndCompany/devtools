# Licensing review

Reviewed 2026-11-13.

## Decision and scope

The company approved distribution, as communicated by the user on 2026-11-13.
This review checks the locked dependency inventory, license metadata and text,
About rendering, and bundle configuration. The upstream `objc2` note about
bindings derived from Apple SDKs remains documented for transparency; this
review does not treat it as a release gate under the company's approval.

The Rust inventory uses the `cargo tree --locked --target
aarch64-apple-darwin --edges normal` graph, excluding the app package itself.
This graph can include proc-macro dependencies that are used during the build
and may not ship in the app binary, so the inventory is conservative. The
frontend inventory uses production dependencies and runtime peers resolved
from `bun.lock`, excluding type-only development peers.

## Findings

- The app declares `Apache-2.0` in `src-tauri/Cargo.toml`; the full app license
  is in [LICENSE](../LICENSE).
- `getAboutInfo` supplies the app license and third-party notices to About.
  The app license is wrapped in a Markdown code block to preserve its original
  line breaks. About renders the sanitized Markdown in a scrollable React
  region; HTTPS links open in the system browser.
- `src-tauri/tauri.conf.json` includes `LICENSE` and
  `THIRD-PARTY-NOTICES.md` in the app bundle.
- The notice inventory has 365 Rust dependency rows and 21 production
  frontend package rows. The lockfile hashes recorded in the notice file
  match the current lockfiles:
  - `src-tauri/Cargo.lock`:
    `0a7fca841b7673909f6fe8502d41488a41d14305f993e03daac695b4229a1274`
  - `frontend/bun.lock`:
    `eae0f6e21317e407947a7f5a6d80e3af1e5f3fa65fc05bfb1dd584f0fdb8a587`
- License expressions match Cargo metadata for all 365 Rust rows and package
  metadata for all 21 frontend rows.
- Text checks found no mismatches after normalizing line endings and trailing
  whitespace:
  - 654 Rust license and notice files matched their crate package files,
    including the opener and `open` crates used for external license links.
  - 12 additional Rust license files matched the pinned upstream source
    commits referenced in the inventory.
  - All four distinct SPDX texts used for metadata-only license files matched
    the pinned SPDX License List revision.
  - All 23 frontend license-text references matched package files or the
    locked `@babel/runtime@8.0.8` npm archive. Its SHA-512 integrity value
    matches `bun.lock`.
- The six MPL-2.0 package rows link to their exact-version crate source
  archives. Their downloaded archive SHA-256 values match `Cargo.lock`.
- The notice document is 988,558 bytes. The combined Markdown input for the
  About renderer is 999,387 bytes, leaving 49,189 bytes below its 1 MiB
  bridge-value limit.

The notice file deduplicates byte-identical normalized license texts and
retains package-specific copyright and notice text. MIT requires retaining its
copyright and permission notice; BSD-3-Clause requires its notice, conditions,
and disclaimer in binary distributions; Apache 2.0 requires a license copy and
applicable upstream NOTICE text; MPL 2.0 requires source-code availability
and a way for recipients to obtain it. These terms specify recipient notices
and source access, not a particular screen layout. The full notice can be
bundled and presented on a dedicated License tab.

Primary license terms:

- [MIT](https://opensource.org/license/mit)
- [BSD-3-Clause](https://opensource.org/license/bsd-3-clause)
- [Apache License 2.0, Section 4](https://www.apache.org/licenses/LICENSE-2.0)
- [Mozilla Public License 2.0, Sections 3.1–3.4](https://www.mozilla.org/en-US/MPL/2.0/)

## Excluded Rust lockfile entries

The prior inventory included 22 Rust rows outside the selected target's
normal dependency graph. `phf_generator 0.14.0` is selected only through a
build edge; the other entries below are absent from the current target's
normal/build graph. They were removed from the distributed notice inventory.

`bs58 0.5.1`, `chacha20 0.10.2`, `jiff 0.2.42`, `jiff-core 0.1.4`,
`lru-slab 0.1.3`, `objc2-io-surface 0.3.2`, `phf_generator 0.14.0`,
`phf_macros 0.14.0`, `quinn 0.11.12`, `quinn-proto 0.11.19`,
`quinn-udp 0.5.16`, `rand 0.10.3`, `rand_core 0.10.1`, `rand_pcg 0.10.2`,
`ref-cast 1.0.27`, `ref-cast-impl 1.0.27`, `schemars 0.9.0`,
`schemars 1.2.3`, `sqlx-macros 0.8.6`, `sqlx-macros-core 0.8.6`,
`tray-icon 0.25.1`, and `zlib-rs 0.6.8`.

## Unconfirmed items

- The final Xcode Archive was not rebuilt after the current notice and About
  changes. Source configuration includes both notice resources, but the next
  release archive should be checked for the updated files and Markdown links.
- The 365-package Rust list is based on Cargo's normal-edge graph, not a link
  map of code actually present in the final binary. It may include
  build-time proc-macro dependencies. The notice keeps those rows for
  traceability; pruning them requires artifact-level evidence.
- The upstream `objc2` license note says these bindings derive from Apple SDKs
  and that redistribution implications are unclear. Its legal interpretation
  is not independently determined here. Company distribution approval is
  recorded; this informational item is not a release gate.

## Validation

- Frontend lint, format check, typecheck, and production build passed. The
  production build reports a chunk-size warning for the existing 742.66 kB
  JavaScript bundle.
- `cargo check --locked` and a local Tauri dev compile passed. The UI tool
  attached to the installed `/Applications/DevTools.app`, so the current dev
  UI's scrolling, link handling, and initial render could not be confirmed.
- The debug app-bundle build failed in the environment while compiling
  `aws-lc-sys`; no updated `.app` or Xcode Archive was produced.
- The app-level test suites were not run in this review.
