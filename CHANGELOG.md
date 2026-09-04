# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.8.0](https://github.com/LotusAndCompany/devtools/compare/v0.7.0...v0.8.0) (2026-09-04)


### Features

* add Markdown preview feature with GUI support ([#98](https://github.com/LotusAndCompany/devtools/issues/98)) ([43fe411](https://github.com/LotusAndCompany/devtools/commit/43fe4112bc284c50f6585374bd47ae481d9a5624))
* **build,logging:** add make run target and colored console output ([#84](https://github.com/LotusAndCompany/devtools/issues/84)) ([215428d](https://github.com/LotusAndCompany/devtools/commit/215428da1ae3525b8a200b8b951a99fec8314c19))
* **gui:** adopt material symbols icon font ([#118](https://github.com/LotusAndCompany/devtools/issues/118)) ([5cd29ee](https://github.com/LotusAndCompany/devtools/commit/5cd29eedde2a68490d9ea36eec55adab10aec470))
* **image:** replace legacy image tools with unified editor ([#69](https://github.com/LotusAndCompany/devtools/issues/69)) ([ad4a4da](https://github.com/LotusAndCompany/devtools/commit/ad4a4daa45d62ec30609a27c089f5ab89b5cc0b5))
* **phrase_generation:** restructure UI layout and improve usability ([#97](https://github.com/LotusAndCompany/devtools/issues/97)) ([59507c0](https://github.com/LotusAndCompany/devtools/commit/59507c01a80a1a3eefe5570c2439a0f294035990))
* **style:** apply qlementine modern Qt6 QStyle theme ([#91](https://github.com/LotusAndCompany/devtools/issues/91)) ([0ec4876](https://github.com/LotusAndCompany/devtools/commit/0ec4876d20619d6163a72a35c5c953c8cf74ec51))


### Bug Fixes

* **build:** avoid duplicate static library links ([#76](https://github.com/LotusAndCompany/devtools/issues/76)) ([aff6c1d](https://github.com/LotusAndCompany/devtools/commit/aff6c1dbe8307f1839ea5b7a84fe7aae29cb30c2))
* **ci:** add build-mode: none to CodeQL C++ init step ([#101](https://github.com/LotusAndCompany/devtools/issues/101)) ([9197f0e](https://github.com/LotusAndCompany/devtools/commit/9197f0e28d98d26667aea4fc3315ee6de2ff8c02))
* **ci:** disable vcpkg binary cache, bump run-vcpkg to v11.6, mirror Doxygen ([#102](https://github.com/LotusAndCompany/devtools/issues/102)) ([2d7f6a9](https://github.com/LotusAndCompany/devtools/commit/2d7f6a94097548fac0df64513e007ba8b58494c8))
* **ci:** pin codeql-action to v4 tag and add dependabot grouping ([#112](https://github.com/LotusAndCompany/devtools/issues/112)) ([18b51a4](https://github.com/LotusAndCompany/devtools/commit/18b51a47da44f4601772588800b2268c5aa38841))
* **cmake:** change Doxygen not found message from WARNING to STATUS ([#100](https://github.com/LotusAndCompany/devtools/issues/100)) ([17b5434](https://github.com/LotusAndCompany/devtools/commit/17b5434da2c9581d30da159d52bbfacbf5a1c9d3))
* **data_conversion:** resolve 4 ctest failures in parser and outdated flag ([#78](https://github.com/LotusAndCompany/devtools/issues/78)) ([e71a3bd](https://github.com/LotusAndCompany/devtools/commit/e71a3bd51d18f9ef014c71918c934b9de9f5ff3a))
* **image-tool:** change section checkbox default value to false. ([#88](https://github.com/LotusAndCompany/devtools/issues/88)) ([aa2d010](https://github.com/LotusAndCompany/devtools/commit/aa2d010f24ec3d4f6faa1ed3157c02620c1f82ea))
* **phrase_generation:** improve save button position in phrase generation UI ([#43](https://github.com/LotusAndCompany/devtools/issues/43)) ([5db7227](https://github.com/LotusAndCompany/devtools/commit/5db7227ba405c05db06394e850e1764a050de58a))
* **ui:** manually draw right border on sidemenu to fix qlementine issue ([#99](https://github.com/LotusAndCompany/devtools/issues/99)) ([c9b909a](https://github.com/LotusAndCompany/devtools/commit/c9b909a8db5042b982a41442b39a34abc6c60670))


### Build System

* **i18n:** make translation updates explicit ([#71](https://github.com/LotusAndCompany/devtools/issues/71)) ([cfb5745](https://github.com/LotusAndCompany/devtools/commit/cfb5745fde64f293378fb28a45366b457d2d4bcb))

## [0.7.0](https://github.com/LotusAndCompany/devtools/compare/v0.6.0...v0.7.0) (2026-05-01)


### Features

* **design:** add UI design files for DevTools application ([#25](https://github.com/LotusAndCompany/devtools/issues/25)) ([3b4381e](https://github.com/LotusAndCompany/devtools/commit/3b4381e0dbd8c167093fcc6faeb51e1eb7890c34))

## [0.6.0](https://github.com/LotusAndCompany/devtools/compare/v0.5.3...v0.6.0) (2026-04-22)


### Features

* add drag-and-drop file loading support ([#27](https://github.com/LotusAndCompany/devtools/issues/27)) ([0d83461](https://github.com/LotusAndCompany/devtools/commit/0d83461569c68ba595be64820c74da89039a5829))


### Documentation

* add cross-tool AI agent harness ([#29](https://github.com/LotusAndCompany/devtools/issues/29)) ([c28a0e9](https://github.com/LotusAndCompany/devtools/commit/c28a0e976e96231d83e19c9737a818cd5f2ccad4))
* improve Doxygen configuration and page layout ([#23](https://github.com/LotusAndCompany/devtools/issues/23)) ([4ca77ba](https://github.com/LotusAndCompany/devtools/commit/4ca77ba9273c8e5b95409486700f36fcfd26c59e))

## [0.5.3](https://github.com/LotusAndCompany/devtools/compare/v0.5.2...v0.5.3) (2026-04-07)


### Documentation

* add database management tool documentation ([#20](https://github.com/LotusAndCompany/devtools/issues/20)) ([56b0ee9](https://github.com/LotusAndCompany/devtools/commit/56b0ee93d51825d22033f8bce0b83da0baf609f7))

## [0.5.2](https://github.com/LotusAndCompany/devtools/compare/v0.5.1...v0.5.2) (2026-02-09)


### Documentation

* add specific version info for Qt and related tools ([#13](https://github.com/LotusAndCompany/devtools/issues/13)) ([c86e102](https://github.com/LotusAndCompany/devtools/commit/c86e102445f3ba6e3d59d8fd639c934ce4b72703))

## [0.5.1](https://github.com/LotusAndCompany/devtools/compare/v0.5.0...v0.5.1) (2026-01-21)


### Bug Fixes

* add job-level permissions for release-please ([#8](https://github.com/LotusAndCompany/devtools/issues/8)) ([0f33cfc](https://github.com/LotusAndCompany/devtools/commit/0f33cfcec3fa2ef741fff623e02fb8c2222cb56f))
* auto-create tags when release PR is merged ([#7](https://github.com/LotusAndCompany/devtools/issues/7)) ([2d1b577](https://github.com/LotusAndCompany/devtools/commit/2d1b577422a7571653582224e92b009db7d6113a))
* restore release-please workflow to initial implementation ([#9](https://github.com/LotusAndCompany/devtools/issues/9)) ([e5bd4f2](https://github.com/LotusAndCompany/devtools/commit/e5bd4f2e437ad165927eab1f26074e27d8eef8b3))

## [0.5.0](https://github.com/LotusAndCompany/devtools/compare/v0.4.0...v0.5.0) (2026-01-21)


### Features

* Make Release Please workflow manual-only and restrict to owners ([#3](https://github.com/LotusAndCompany/devtools/issues/3)) ([5ad446e](https://github.com/LotusAndCompany/devtools/commit/5ad446e915a55b6a58f55e0bd0a42803b8f78bcd))

## [0.4.0](https://github.com/LotusAndCompany/devtools/compare/v0.3.0...v0.4.0) (2026-01-21)


### Features

* Initial commit of the DevTools application ([3a1a3d6](https://github.com/LotusAndCompany/devtools/commit/3a1a3d63bc8ec4d074aa2f13fd9c58fe1d34022f))

## [Unreleased]

### Added
- Open source documentation (README, BUILD, CONTRIBUTING, etc.)
- Apache License 2.0

### Changed
- License changed from LGPL v3 to Apache License 2.0

## [0.2.0] - 2025

### Added
- QR Code generation tool
  - Text QR codes
  - URL QR codes
  - Email (mailto:) QR codes
- Data conversion tool (JSON, YAML, TOML)
- Image processing tools
  - Resize
  - Rotation
  - Division
  - Transparency
- API testing tool
- Command execution tool
- Phrase generation tool
- Multi-language support (Japanese/English)

### Changed
- Updated application icon

## [0.1.0] - 2024

### Added
- Initial release
- Basic application framework
- Core module architecture

[Unreleased]: https://github.com/LotusAndCompany/devtools/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/LotusAndCompany/devtools/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/LotusAndCompany/devtools/releases/tag/v0.1.0
