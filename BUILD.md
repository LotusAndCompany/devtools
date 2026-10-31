# Build Instructions

[日本語版](docs/ja/BUILD.md)

This document describes how to build DevTools from source.

## Requirements

### Platform
- **macOS 15.0 or later** (Apple Silicon / arm64 only)

### Tools
- **CMake**: 3.21.1 or later
- **C++ Compiler**: C++17 compatible (Clang recommended)
- **Qt**: 6.9+ (6.9.3 tested; Core, Gui, Widgets, LinguistTools, Sql, Network, Svg,
  Concurrent, WebChannel, and WebEngineWidgets modules)
- **vcpkg**: For dependency management
- **Bun**: 1.3.14 (required to build the React frontend)

### Dependencies (via vcpkg)
- toml11
- yaml-cpp

### Dependencies (via CMake FetchContent)
- qlementine v1.4.2 (modern Qt Widgets `QStyle`, MIT license)

Qlementine is fetched during CMake configure from
`https://github.com/oclero/qlementine.git`. It requires Qt 6.8+ and CMake 3.21+.
DevTools links it into the Qt Widgets support library. The main application
screen is React, bundled by Vite and hosted by Qt WebEngine.

### Optional
- **Doxygen**: 1.18+ (API documentation generation)
- **GraphViz**: For class diagrams and dependency graphs in documentation (optional)
- **Qt Creator**: 18.0.1+ (IDE with Qt integration)
- **Ninja**: 1.12.1+ (Fast build tool)
- **Qt Installer Framework**: 4.10 (Application packaging)

## Installation

### 1. Install Qt6

#### Using Homebrew (recommended for macOS)
```bash
brew install qt@6
```

#### Using Qt Online Installer
Download from [qt.io](https://www.qt.io/download) and install Qt 6.9 or later with the following components:
- Qt 6.9 or later for macOS
- Core, Gui, Widgets, LinguistTools, Sql, Network, Svg, Concurrent, WebChannel, and WebEngineWidgets modules
- Qt Creator (optional, but recommended)

### 2. Install vcpkg

```bash
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap
./bootstrap-vcpkg.sh

# Configure the vcpkg root and add it to PATH.
export VCPKG_ROOT="$PWD"
export PATH="$VCPKG_ROOT:$PATH"
```

### 3. Clone the Repository

```bash
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools
```

### 4. Install Bun

Install Bun 1.3.14 or later from the
[official installation guide](https://bun.sh/docs/installation), then verify
the version:

```bash
bun --version
```

### 5. Install Dependencies

```bash
vcpkg install
```

## Building

### Using CMake (Command Line)

```bash
# Create build directory
mkdir build && cd build

# Configure. This installs vcpkg manifest dependencies and downloads qlementine.
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx

# Build
cmake --build . --parallel
```

### Running DevTools

After building, launch the application:

```bash
# Build and run with colored console output
cmake --build . --target run

# Or run the binary directly
./DevTools.app/Contents/MacOS/DevTools

# Or open the app bundle
open DevTools.app
```

The `run` target replaces older DevTools processes from this source tree. If a
separately installed DevTools app is running, quit it first; the target stops
with an error rather than selecting that process.

Log output is color-coded by severity: DEBUG=dim, INFO=default, WARN=yellow, CRIT=red.

### Using Qt Creator

1. Open `CMakeLists.txt` in Qt Creator
2. Enable the vcpkg plugin in Qt Creator settings
3. Add the following CMake argument:
   ```
   -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
   -DVCPKG_TARGET_TRIPLET=arm64-osx
   ```
4. Configure the project
5. Build using the Build button or `Cmd+B`

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_UNIT_TEST` | OFF | Enable unit tests |
| `VCPKG_TARGET_TRIPLET` | auto | Set to `arm64-osx` for Apple Silicon |

Qlementine's example applications are disabled in the project configuration
(`QLEMENTINE_SANDBOX=OFF`, `QLEMENTINE_SHOWCASE=OFF`) so only the style library is built.

Example with options:
```bash
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx \
  -DENABLE_UNIT_TEST=ON
```

## Running Tests

```bash
# Enable tests first
cmake .. -DENABLE_UNIT_TEST=ON

# Build
cmake --build . --parallel

# Run tests
ctest
```

## Generating Documentation

Doxygen 1.18+ is required to generate API documentation.

### Install Doxygen
```bash
brew install doxygen
```

### Install GraphViz (Optional)

GraphViz enables class inheritance diagrams, collaboration diagrams, and include dependency graphs.

```bash
brew install graphviz
```

If GraphViz is not installed, documentation will be generated without diagrams.

### Generate Documentation
```bash
cmake --build build --target DevTools_docs
```

The generated documentation will be in `build/doxygen/html/`.

## Troubleshooting

### CMake Configuration Errors

1. **Check for syntax errors** in CMakeLists.txt
2. **Delete CMake cache**:
   ```bash
   rm -rf build/CMakeCache.txt
   ```
3. **Clean build directory**:
   ```bash
   rm -rf build
   cmake -S . -B build \
     -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
     -DVCPKG_TARGET_TRIPLET=arm64-osx
   ```

### vcpkg Issues

1. **Ensure vcpkg is properly installed**:
   ```bash
   vcpkg version
   ```
2. **Update vcpkg**:
   ```bash
   cd /path/to/vcpkg
   git pull
   ./bootstrap-vcpkg.sh
   ```

### Qt or Qt WebEngine Not Found

1. **Set Qt6_DIR** environment variable:
   ```bash
   export Qt6_DIR=/opt/homebrew/lib/cmake/Qt6
   ```
2. **Or pass it to CMake**:
   ```bash
   cmake .. -DQt6_DIR=/opt/homebrew/lib/cmake/Qt6
   ```

### qlementine FetchContent Issues

If configuration fails while downloading qlementine:

1. Check network access to `https://github.com/oclero/qlementine.git`.
2. Delete the FetchContent cache and configure again:
   ```bash
   rm -rf build/_deps/qlementine-*
   cmake -S . -B build \
     -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
     -DVCPKG_TARGET_TRIPLET=arm64-osx
   ```
3. Verify that the active Qt installation is Qt 6.9 or later and includes the Svg module.
