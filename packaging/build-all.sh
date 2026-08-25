#!/bin/bash
# build-all.sh — Build FreeEffect for the current platform with packaging
# Usage: ./packaging/build-all.sh [platform]
# Platforms: macos, linux, windows (auto-detected if omitted)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build-release"
VERSION="0.1.0"

# Auto-detect platform
if [ -n "${1:-}" ]; then
    PLATFORM="$1"
else
    case "$(uname -s)" in
        Darwin*)  PLATFORM="macos" ;;
        Linux*)   PLATFORM="linux" ;;
        MINGW*|MSYS*|CYGWIN*) PLATFORM="windows" ;;
        *) echo "Unknown platform"; exit 1 ;;
    esac
fi

echo "=========================================="
echo " FreeEffect $VERSION — Building for $PLATFORM"
echo "=========================================="

# Clean previous build
rm -rf "$BUILD_DIR"

case "$PLATFORM" in
    macos)
        echo "[1/4] Configuring (Universal Binary)..."
        cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
            -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
            -DCPACK_GENERATOR="DragNDrop" \
            "$PROJECT_DIR"

        echo "[2/4] Building..."
        cmake --build "$BUILD_DIR" --config Release -j$(sysctl -n hw.ncpu)

        echo "[3/4] Testing..."
        cd "$BUILD_DIR" && ctest --output-on-failure && cd "$PROJECT_DIR"

        echo "[4/4] Creating DMG..."
        cd "$BUILD_DIR" && cpack -G DragNDrop && cd "$PROJECT_DIR"
        echo ""
        echo "✓ DMG created in: $BUILD_DIR/"
        ls -lh "$BUILD_DIR"/*.dmg 2>/dev/null || true
        ;;

    linux)
        echo "[1/5] Configuring..."
        cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX=/usr \
            -DCPACK_GENERATOR="DEB;RPM" \
            "$PROJECT_DIR"

        echo "[2/5] Building..."
        cmake --build "$BUILD_DIR" --config Release -j$(nproc)

        echo "[3/5] Testing..."
        cd "$BUILD_DIR" && ctest --output-on-failure && cd "$PROJECT_DIR"

        echo "[4/5] Creating DEB..."
        cd "$BUILD_DIR" && cpack -G DEB && cd "$PROJECT_DIR"

        echo "[5/5] Creating RPM..."
        cd "$BUILD_DIR" && cpack -G RPM && cd "$PROJECT_DIR"
        echo ""
        echo "✓ Packages created in: $BUILD_DIR/"
        ls -lh "$BUILD_DIR"/*.deb "$BUILD_DIR"/*.rpm 2>/dev/null || true
        ;;

    windows)
        echo "[1/4] Configuring..."
        cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release \
            -DCPACK_GENERATOR="NSIS" \
            "$PROJECT_DIR"

        echo "[2/4] Building..."
        cmake --build "$BUILD_DIR" --config Release

        echo "[3/4] Testing..."
        cd "$BUILD_DIR" && ctest --output-on-failure && cd "$PROJECT_DIR"

        echo "[4/4] Creating NSIS Installer..."
        cd "$BUILD_DIR" && cpack -G NSIS -C Release && cd "$PROJECT_DIR"
        echo ""
        echo "✓ Installer created in: $BUILD_DIR/"
        ls -lh "$BUILD_DIR"/*.exe 2>/dev/null || true
        ;;
esac

echo ""
echo "=========================================="
echo " Build complete!"
echo "=========================================="
