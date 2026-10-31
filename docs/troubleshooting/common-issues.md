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
   cmake .. \
     -DQt6_DIR=/opt/homebrew/lib/cmake/Qt6 \
     -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
     -DVCPKG_TARGET_TRIPLET=arm64-osx
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
   cmake .. \
     -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
     -DVCPKG_TARGET_TRIPLET=arm64-osx
   ```

### Compilation Errors

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
cmake --build build --parallel
```

#### Frequent Rebuilds

**Solution:**

Clean and rebuild:
```bash
rm -rf build
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx
cmake --build build --parallel
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

2. Rebuild with correct triplet:
   ```bash
   cmake -S . -B build \
     -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
     -DVCPKG_TARGET_TRIPLET=arm64-osx
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

1. Check that Qt can read the file format (the Open image dialog lists PNG,
   JPEG, BMP, GIF, and WebP)
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

#### Command Generation Does Not Produce a Result

**Symptoms:**
- The generated command area is empty
- The **Generate command** button is unavailable

**Solutions:**

1. Select a command category and command
2. Select an option when one is available
3. Enter the requested value when the input field is shown
4. The tool generates command text; execute it in a terminal yourself

#### Database Connection Failed

**Symptoms:**
- "Connection Failed" error message
- Cannot connect to database

**Solutions:**

1. **SQLite**: Verify the file path is correct and the file is a valid SQLite database
2. **MySQL/PostgreSQL**: Check hostname, database name, username, password,
   and that the server accepts connections
3. **Server not running**: Ensure the database server is running and accessible
4. **Driver not found**: Ensure the Qt SQL driver plugin (QSQLITE, QMYSQL, QPSQL) is installed

#### Database Tables Not Showing

**Symptoms:**
- Tables panel is empty after connecting

**Solutions:**

1. Click the **Refresh** button in the toolbar
2. Verify the database contains tables
3. Check that the connection is active (**Execute** and **Refresh** should be enabled)

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
- Icons have the wrong color
- Controls do not match the selected theme

**Solutions:**

1. Open **Settings** and select **Follow system**, **Light**, or **Dark**.
2. Restart DevTools if the selected appearance does not update.
3. If the issue remains, rebuild the application so CMake embeds the current frontend assets.

### Language Not Changed

**Symptoms:**
- UI still in old language

**Solutions:**

1. Click **Save** after changing the language in **Settings**.
2. If the UI does not update, close and reopen the application.

## Installation Issues

### Bun Not Found

The CMake configure step requires Bun 1.3.14 or later to build the embedded
React frontend. Install Bun, confirm `bun --version`, then configure again.

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
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx \
  -DENABLE_UNIT_TEST=ON
cmake --build build --parallel
```

### Qt Tests Crash

**Symptoms:**
- SIGSEGV in Qt test

**Solution:**

When running widget tests in an environment without a display, use Qt's
offscreen platform plugin:

```bash
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
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
- [Build Instructions](https://github.com/LotusAndCompany/devtools/blob/main/BUILD.md) - Detailed build guide
