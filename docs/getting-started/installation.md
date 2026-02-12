# Installation Guide

This guide provides detailed instructions for setting up the DevTools development environment.

## System Requirements

### Platform
- **macOS 15.0 or later** (Apple Silicon / arm64 and Intel / x86_64)

### Required Tools
| Tool | Version | Purpose |
|------|---------|---------|
| CMake | 3.21.1+ | Build system |
| C++ Compiler | C++17 compatible | Compilation (Clang recommended) |
| Qt | 6.9.3 | GUI framework |
| vcpkg | Latest | Package management |

### Optional Tools
| Tool | Version | Purpose |
|------|---------|---------|
| Doxygen | - | API documentation generation |
| Qt Creator | 18.0.1+ | IDE with Qt integration |
| Ninja | 1.12.1+ | Fast build tool |
| Qt Installer Framework | 4.10 | Application packaging |

## Step-by-Step Installation

### 1. Install Qt6

Qt6 provides the GUI framework for DevTools.

#### Option A: Using Homebrew (Recommended)

```bash
brew install qt@6
```

After installation, add Qt to your PATH:

```bash
# Add to ~/.zshrc or ~/.bash_profile
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
```

#### Option B: Using Qt Online Installer

1. Download the installer from [qt.io](https://www.qt.io/download)
2. Run the installer
3. Select Qt 6.x for macOS
4. Optionally install Qt Creator

### 2. Install vcpkg

vcpkg manages C++ library dependencies.

```bash
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg

# Bootstrap vcpkg
./bootstrap-vcpkg.sh

# Add to PATH (add to your shell profile)
export PATH="$HOME/vcpkg:$PATH"
export VCPKG_ROOT="$HOME/vcpkg"
```

Verify installation:

```bash
vcpkg version
```

### 3. Install Additional Tools

#### Doxygen (Optional)

```bash
brew install doxygen
```

#### clang-format and clang-tidy (For development)

```bash
brew install llvm
```

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

### 6. Configure and Build

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake (vcpkg triplet is auto-detected based on architecture)
cmake ..

# Build (using all available cores)
make -j$(sysctl -n hw.ncpu)
```

### 7. Run DevTools

After building, run the application:

```bash
./DevTools.app/Contents/MacOS/DevTools
```

Or open the app bundle:

```bash
open DevTools.app
```

## IDE Setup

### Qt Creator

1. Open Qt Creator
2. Go to **Preferences > Kits**
3. Ensure a Qt 6.x kit is configured
4. Enable the vcpkg plugin in **Preferences > CMake > vcpkg**
5. Open `CMakeLists.txt` from the project root
6. Configure and build (vcpkg triplet is auto-detected)

### VS Code

1. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools
   - clangd (optional, for better intellisense)

2. Use the CMake Tools extension to configure and build (vcpkg triplet is auto-detected)

### CLion

1. Open the project folder
2. Go to **Preferences > Build, Execution, Deployment > CMake**
3. Reload CMake project (vcpkg triplet is auto-detected)

## Verifying Installation

After building, verify everything works:

```bash
# Run the application
./DevTools.app/Contents/MacOS/DevTools

# Run tests (if enabled)
cmake .. -DENABLE_UNIT_TEST=ON
make
ctest --output-on-failure
```

## Next Steps

- [Quick Start Guide](quick-start.md) - Learn the basics
- [Architecture Overview](../development/architecture.md) - Understand the codebase
- [Contributing Guide](../../CONTRIBUTING.md) - Start contributing

## Troubleshooting

If you encounter issues, see [Common Issues](../troubleshooting/common-issues.md) or check the [FAQ](../troubleshooting/faq.md).
