# Installation Guide

This guide provides detailed instructions for setting up the DevTools development environment.

## System Requirements

### Platform
- **macOS 15.0 or later** (Apple Silicon / arm64 only)

### Required Tools
| Tool | Version | Purpose |
|------|---------|---------|
| CMake | 3.21.1+ | Build system |
| C++ Compiler | C++17 compatible | Compilation (Clang recommended) |
| Qt | 6.9.0+ | Native host; CI pins 6.9.0 |
| vcpkg | Latest | Package management |
| Bun | 1.3.14 | Installs frontend dependencies and builds the embedded React UI |

### Optional Tools
| Tool | Version | Purpose |
|------|---------|---------|
| Doxygen | 1.18+ | API documentation generation |
| Qt Creator | 18.0.1+ | IDE with Qt integration |
| Ninja | 1.12.1+ | Fast build tool |

## Step-by-Step Installation

### 1. Install Qt6

Qt6 provides the GUI framework for DevTools.

#### Option A: Qt Online Installer (Recommended)

Use the Qt Online Installer to install the pinned Qt version:

1. Download and run the [Qt Online Installer](https://www.qt.io/download).
2. In a custom installation, select **Qt 6.9.0 > macOS (clang_64)**.
3. Install the desktop modules used by this project: Core, Gui, Widgets,
   LinguistTools, Sql, Network, Svg, and Concurrent.
4. Qt Creator is optional. The React frontend uses Apple's WKWebView, so Qt
   WebEngine and WebChannel are not application dependencies.

The Qt Maintenance Tool included with the SDK can add or update components
later. See the [Qt component guide](https://doc.qt.io/qt-6/add-update-remove-components.html).

#### Option B: Using Homebrew

```bash
brew install qt@6
```

Homebrew may provide a Qt version older than the project minimum. Check the
installed version before configuring, and use Qt 6.9.0 or later.

### 2. Install vcpkg

vcpkg manages C++ library dependencies.

```bash
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg

# Bootstrap vcpkg
./bootstrap-vcpkg.sh

# Add to PATH (add to your shell profile)
export VCPKG_ROOT="$HOME/vcpkg"
export PATH="$VCPKG_ROOT:$PATH"
```

Verify installation:

```bash
vcpkg version
```

### 3. Install Additional Tools

#### Doxygen (Optional, 1.18+)

```bash
brew install doxygen
```

#### GraphViz (Optional, for documentation diagrams)

```bash
brew install graphviz
```

#### clang-format and clang-tidy (For development)

```bash
brew install llvm
```

Install Bun 1.3.14 or later from the
[official installation guide](https://bun.sh/docs/installation). CMake invokes
Bun to install the locked frontend dependencies and build the React assets.

### 4. Clone the Repository

```bash
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools
```

### 5. Install Dependencies

vcpkg will automatically install required libraries:

```bash
vcpkg install
```

This installs:
- **toml11** - TOML parser (header-only)
- **yaml-cpp** - YAML parser

Qlementine is not installed by vcpkg in this project. CMake downloads qlementine v1.4.2
with FetchContent during the configure step and builds it against the active Qt6
installation.

### 6. Configure and Build

```bash
# Configure with CMake. This installs manifest dependencies and downloads qlementine.
cmake -S . -B build \
  -DQt6_DIR="$HOME/Qt/6.9.0/macos/lib/cmake/Qt6" \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx

# Build (using all available cores)
cmake --build build --parallel
```

### 7. Run DevTools

After building, run the application:

```bash
# Build and run with colored console output (recommended).
# This replaces older project-local processes before starting.
cmake --build build --target run

# Or run the binary directly
./build/DevTools.app/Contents/MacOS/DevTools
```

Use the `run` target when iterating on the source tree. It replaces older
project-local DevTools processes before launching the canonical binary. If a
DevTools bundle outside the source tree is running, the target stops with an
explicit error instead of allowing an ambiguous application-name selection.
Separately packaged builds, such as `build-appstore`, are not used for source
validation.

## IDE Setup

### Qt Creator

1. Open Qt Creator
2. Go to **Preferences > Kits**
3. Ensure a Qt 6.9.0 or later kit is configured
4. Enable the vcpkg plugin in **Preferences > CMake > vcpkg**
5. Open `CMakeLists.txt` from the project root
6. Add CMake argument: `-DVCPKG_TARGET_TRIPLET=arm64-osx`
7. Configure and build

### VS Code

1. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools
   - clangd (optional, for better intellisense)

2. Create `.vscode/settings.json`:
   ```json
   {
     "cmake.configureArgs": [
       "-DVCPKG_TARGET_TRIPLET=arm64-osx"
     ]
   }
   ```

3. Use the CMake Tools extension to configure and build

### CLion

1. Open the project folder
2. Go to **Preferences > Build, Execution, Deployment > CMake**
3. Add CMake option: `-DVCPKG_TARGET_TRIPLET=arm64-osx`
4. Reload CMake project

## Verifying Installation

After building, verify everything works:

```bash
# Run the application
./build/DevTools.app/Contents/MacOS/DevTools

# Run tests (if enabled). Keep the vcpkg toolchain and triplet from configuration.
cmake -S . -B build \
  -DQt6_DIR="$HOME/Qt/6.9.0/macos/lib/cmake/Qt6" \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx \
  -DENABLE_UNIT_TEST=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Next Steps

- [Quick Start Guide](quick-start.md) - Learn the basics
- [Architecture Overview](../development/architecture.md) - Understand the codebase
- [Contributing Guide](https://github.com/LotusAndCompany/devtools/blob/main/CONTRIBUTING.md) - Start contributing

## Troubleshooting

If you encounter issues, see [Common Issues](../troubleshooting/common-issues.md) or check the [FAQ](../troubleshooting/faq.md).
