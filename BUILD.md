# Build Instructions

[日本語版](docs/ja/BUILD.md)

This document describes how to build DevTools from source.

## Requirements

### Platform
- **macOS 15.0 or later** (Apple Silicon / arm64 only)

### Tools
- **CMake**: 3.21.1 or later
- **C++ Compiler**: C++17 compatible (Clang recommended)
- **Qt**: 6.9.3 (Widgets, LinguistTools, Network modules)
- **vcpkg**: For dependency management

### Dependencies (via vcpkg)
- toml11
- yaml-cpp

### Optional
- **Doxygen**: For generating API documentation
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
Download from [qt.io](https://www.qt.io/download) and install Qt 6.x with the following components:
- Qt 6.x for macOS
- Qt Creator (optional, but recommended)

### 2. Install vcpkg

```bash
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap
./bootstrap-vcpkg.sh

# Add to PATH (add to your shell profile for persistence)
export PATH="$PWD:$PATH"
```

### 3. Clone the Repository

```bash
git clone https://github.com/LotusAndCompany/devtools.git
cd devtools
```

### 4. Install Dependencies

```bash
vcpkg install
```

## Building

### Using CMake (Command Line)

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DVCPKG_TARGET_TRIPLET=arm64-osx

# Build
make

# Or with parallel jobs
make -j$(sysctl -n hw.ncpu)
```

### Using Qt Creator

1. Open `CMakeLists.txt` in Qt Creator
2. Enable the vcpkg plugin in Qt Creator settings
3. Add the following CMake argument:
   ```
   -DVCPKG_TARGET_TRIPLET=arm64-osx
   ```
4. Configure the project
5. Build using the Build button or `Cmd+B`

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_UNIT_TEST` | OFF | Enable unit tests |
| `VCPKG_TARGET_TRIPLET` | auto | Set to `arm64-osx` for Apple Silicon |

Example with options:
```bash
cmake .. -DENABLE_UNIT_TEST=ON -DVCPKG_TARGET_TRIPLET=arm64-osx
```

## Running Tests

```bash
# Enable tests first
cmake .. -DENABLE_UNIT_TEST=ON

# Build
make

# Run tests
ctest
```

## Generating Documentation

Doxygen is required to generate API documentation.

### Install Doxygen
```bash
brew install doxygen
```

### Generate Documentation
```bash
cd build
make DevTools_docs
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
   rm -rf build && mkdir build && cd build && cmake ..
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

### Qt Not Found

1. **Set Qt6_DIR** environment variable:
   ```bash
   export Qt6_DIR=/opt/homebrew/lib/cmake/Qt6
   ```
2. **Or pass it to CMake**:
   ```bash
   cmake .. -DQt6_DIR=/opt/homebrew/lib/cmake/Qt6
   ```
