# Common Issues

This document lists common issues and their solutions.

## Build Issues

### CMake Configuration Errors

#### Qt Not Found

**Error:**
```
Could not find a package configuration file provided by "Qt6"
```

**Solution:**

1. Install Qt6:
   ```bash
   brew install qt@6
   ```

2. Set Qt6_DIR:
   ```bash
   export Qt6_DIR=/opt/homebrew/lib/cmake/Qt6
   # Apple Silicon
   cmake .. -DQt6_DIR=/opt/homebrew/lib/cmake/Qt6
   # Intel Mac
   cmake .. -DQt6_DIR=/usr/local/lib/cmake/Qt6
   ```

#### vcpkg Packages Not Found

**Error:**
```
Could not find a package configuration file provided by "yaml-cpp"
```

**Solution:**

1. Install vcpkg packages:
   ```bash
   vcpkg install
   ```

2. Check vcpkg is in PATH:
   ```bash
   vcpkg version
   ```

3. Specify toolchain file:
   ```bash
   cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
   ```

### Compilation Errors

#### UI File Not Found

**Error:**
```
The user interface file "*.ui" could not be found
```

**Solution:**

Add path to `CMAKE_AUTOUIC_SEARCH_PATHS` in CMakeLists.txt:

```cmake
set(CMAKE_AUTOUIC_SEARCH_PATHS
    gui/image/basic/
    gui/your_path/
)
```

#### Linker Errors

**Error:**
```
Undefined symbols for architecture arm64
```

**Solution:**

1. Check module is in MODULE_LIST
2. Check dependencies are linked:
   ```cmake
   target_link_libraries(${PROJECT_NAME}_module PUBLIC
       required_library
   )
   ```

### Build Performance

#### Slow Compilation

**Solution:**

Use parallel builds:
```bash
make -j$(sysctl -n hw.ncpu)
```

#### Frequent Rebuilds

**Solution:**

Clean and rebuild:
```bash
rm -rf build
mkdir build && cd build
cmake ..
make
```

## Runtime Issues

### Application Won't Start

#### Missing Qt Libraries

**Error:**
```
Library not loaded: @rpath/QtCore.framework
```

**Solution:**

Run macdeployqt:
```bash
macdeployqt DevTools.app
```

#### Wrong Architecture

**Error:**
```
Cannot be opened because the developer cannot be verified
```

**Solution:**

1. Check architecture matches:
   ```bash
   file DevTools.app/Contents/MacOS/DevTools
   ```

2. Rebuild (vcpkg triplet is auto-detected based on architecture):
   ```bash
   cmake ..
   ```

### Feature-Specific Issues

#### QR Code Generation Fails

**Symptoms:**
- QR code not generated
- Empty image

**Solutions:**

1. Check input is not empty
2. Check input is not too long (QR codes have size limits)
3. Try simpler content first

#### Image Processing Fails

**Symptoms:**
- Image not loaded
- Processing produces black image

**Solutions:**

1. Check file format is supported (PNG, JPEG, BMP)
2. Check file is not corrupted
3. Check file permissions

#### Data Conversion Fails

**Symptoms:**
- Conversion error message
- Empty output

**Solutions:**

1. Validate input syntax:
   - JSON: Check brackets, quotes, commas
   - YAML: Check indentation (use spaces, not tabs)
   - TOML: Check key=value format

2. Check for special characters
3. Try simpler input first

#### API Testing No Response

**Symptoms:**
- Request hangs
- Connection refused

**Solutions:**

1. Check URL is correct (include https://)
2. Check internet connection
3. Check firewall settings
4. Verify the API server is running

#### Command Execution Hangs

**Symptoms:**
- Command doesn't return
- No output

**Solutions:**

1. Command may be interactive (not supported)
2. Command may need input (not supported)
3. Try simpler commands first
4. Use terminal for complex operations

## UI Issues

### Text Truncation

**Symptoms:**
- Labels cut off
- Buttons too small

**Solutions:**

1. Resize window
2. Check display scaling settings
3. Report bug with screenshot

### Theme Not Applied

**Symptoms:**
- Icons wrong color
- Incorrect appearance

**Solutions:**

1. Restart application after theme change
2. Check View > Theme setting
3. Reset preferences

### Language Not Changed

**Symptoms:**
- UI still in old language

**Solutions:**

1. Restart application after language change
2. Check all windows are closed before restart

## Installation Issues

### vcpkg Bootstrap Fails

**Error:**
```
bootstrap-vcpkg.sh: Permission denied
```

**Solution:**

```bash
chmod +x bootstrap-vcpkg.sh
./bootstrap-vcpkg.sh
```

### Homebrew Qt Issues

**Error:**
```
qt@6 is keg-only
```

**Solution:**

Link or set path:
```bash
brew link qt@6
# Or
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
```

## Test Issues

### Tests Not Found

**Symptoms:**
- ctest shows no tests

**Solution:**

Enable tests:
```bash
cmake .. -DENABLE_UNIT_TEST=ON
make
```

### Qt Tests Crash

**Symptoms:**
- SIGSEGV in Qt test

**Solution:**

Ensure QApplication is created in test fixture:
```cpp
static QApplication* app_;
```

## Getting More Help

If your issue is not listed:

1. Search [GitHub Issues](https://github.com/LotusAndCompany/devtools/issues)
2. Check [FAQ](faq.md)
3. Create a new issue with:
   - DevTools version
   - macOS version
   - Steps to reproduce
   - Error messages
   - Screenshots if applicable

## Related Documentation

- [Installation](../getting-started/installation.md) - Setup guide
- [FAQ](faq.md) - Frequently asked questions
- [Build Instructions](../../BUILD.md) - Detailed build guide
