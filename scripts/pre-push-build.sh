#!/bin/bash
# Pre-push hook: Verify CMake build succeeds
# Used by pre-commit framework

REPO_ROOT=$(git rev-parse --show-toplevel)
BUILD_DIR="${REPO_ROOT}/build"
LOG_FILE=$(mktemp)

cleanup() {
    rm -f "$LOG_FILE"
}
trap cleanup EXIT

run_build() {
    if [ ! -d "$BUILD_DIR" ]; then
        echo "Build directory not found. Creating and configuring..."
        mkdir -p "$BUILD_DIR"
        cmake -B "$BUILD_DIR" -S "$REPO_ROOT" || return 1
    fi

    if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
        echo "CMake not configured. Running configuration..."
        cmake -B "$BUILD_DIR" -S "$REPO_ROOT" || return 1
    fi

    echo "Building project..."
    cmake --build "$BUILD_DIR" --parallel || return 1

    # Restore translation files modified by lupdate during build
    git checkout -- "${REPO_ROOT}/res/"*.ts 2>/dev/null || true

    return 0
}

# Run build and capture output
if run_build > "$LOG_FILE" 2>&1; then
    exit 0
else
    echo "Build failed! Output:"
    echo "----------------------------------------"
    cat "$LOG_FILE"
    echo "----------------------------------------"
    exit 1
fi
