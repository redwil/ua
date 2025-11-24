#!/usr/bin/env bash
set -euo pipefail

# Create a GitHub release tarball containing built binaries and metadata.
#
# By default this packages binaries from ./build (CMake) or the repo root
# (autotools). Provide --build-dir to override.
#
# Example:
#   ./scripts/make-release-tarball.sh --version v1.0.0
#   ./scripts/make-release-tarball.sh --build-dir ./build-cmake

usage() {
    cat <<'EOF'
Usage: make-release-tarball.sh [--version VERSION] [--build-dir DIR]

Options:
  --version VERSION   Version string to embed in the tarball name. If omitted,
                      git describe is used, then __UA_VERSION fallback.
  --build-dir DIR     Directory containing built binaries (ua, kua).
  -h, --help          Show this help.
EOF
}

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIST_DIR="${ROOT}/dist"
BUILD_DIR=""
VERSION=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --version)
            VERSION="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage
            exit 1
            ;;
    esac
done

if [[ -z "$VERSION" ]]; then
    if VERSION=$(git -C "$ROOT" describe --tags --always 2>/dev/null); then
        :
    else
        VERSION=$(grep -oP '__UA_VERSION "\\K[^"]+' "$ROOT/src/ua.cc" 2>/dev/null || echo "dev")
    fi
fi

if [[ -z "$BUILD_DIR" ]]; then
    if [[ -d "$ROOT/build" ]]; then
        BUILD_DIR="$ROOT/build"
    else
        BUILD_DIR="$ROOT"
    fi
else
    BUILD_DIR="$(cd "$BUILD_DIR" && pwd)"
fi

copy_bin() {
    local src="$1"
    local dst_dir="$2"
    if [[ -x "$src" ]]; then
        cp "$src" "$dst_dir"/
        return 0
    fi
    return 1
}

PLATFORM="$(uname -s | tr '[:upper:]' '[:lower:]')"
ARCH="$(uname -m)"
PREFIX="ua-${VERSION}-${PLATFORM}-${ARCH}"
STAGE_DIR="${DIST_DIR}/${PREFIX}"
TARBALL="${DIST_DIR}/${PREFIX}.tar.gz"

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR/bin"

found_bin=false
copy_bin "$BUILD_DIR/ua" "$STAGE_DIR/bin" && found_bin=true
copy_bin "$BUILD_DIR/kua" "$STAGE_DIR/bin" && found_bin=true
copy_bin "$ROOT/ua" "$STAGE_DIR/bin" && found_bin=true
copy_bin "$ROOT/kua" "$STAGE_DIR/bin" && found_bin=true

if [[ "$found_bin" = false ]]; then
    echo "No binaries found (expected ua/kua). Build first, or point to --build-dir." >&2
    exit 1
fi

# Include docs/license metadata
for f in README NEWS ChangeLog AUTHORS COPYING INSTALL; do
    if [[ -f "$ROOT/$f" ]]; then
        cp "$ROOT/$f" "$STAGE_DIR"/
    fi
done

# Package
mkdir -p "$DIST_DIR"
tar -C "$DIST_DIR" -czf "$TARBALL" "$(basename "$STAGE_DIR")"
echo "Created tarball: $TARBALL"
