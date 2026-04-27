#!/bin/bash
# ============================================================================
# Falcon Installer
# Supports: Linux (native bash), Windows (Git Bash/WSL/MSYS2)
#
# Usage:
#   curl -fsSL https://github.com/falcon-autotuning/falcon/releases/download/v1.0.0/install.sh | bash
#   Or with custom version:
#   bash install.sh v1.0.1
# ============================================================================

set -euo pipefail

# Configuration
RELEASE_VERSION="${1:-v1.0.0}"
REPO_OWNER="falcon-autotuning"
REPO_NAME="falcon"
RELEASE_URL="https://github.com/$REPO_OWNER/$REPO_NAME/releases/download/$RELEASE_VERSION"

# Detect platform
detect_platform() {
  local uname_s
  uname_s="$(uname -s 2>/dev/null || echo "Unknown")"

  case "$uname_s" in
  Linux)
    echo "linux"
    ;;
  MINGW* | MSYS* | CYGWIN*)
    echo "windows"
    ;;
  *)
    echo "unsupported"
    ;;
  esac
}

PLATFORM="$(detect_platform)"

if [ "$PLATFORM" = "unsupported" ]; then
  echo "❌ Unsupported platform"
  exit 1
fi

# Platform-specific configuration
if [ "$PLATFORM" = "windows" ]; then
  PACKAGE_FILE="falcon-${RELEASE_VERSION}-win64.zip"
  INSTALL_DIR="${FALCON_INSTALL_DIR:-C:/falcon}"
  EXTRACT_CMD="unzip -q -o"
else
  PACKAGE_FILE="falcon-${RELEASE_VERSION}-Linux.tar.gz"
  INSTALL_DIR="${FALCON_INSTALL_DIR:-/opt/falcon}"
  EXTRACT_CMD="tar --strip-components=1 -xzf"
fi

PACKAGE_URL="$RELEASE_URL/$PACKAGE_FILE"

# Display info
echo "🔧 Falcon Installer"
echo "=========================================="
echo ""
echo "📍 Installation Directory: $INSTALL_DIR"
echo "📦 Platform: $([ "$PLATFORM" = "windows" ] && echo "Windows" || echo "Linux")"
echo "📥 Package: $PACKAGE_FILE"
echo ""

# Create install directory
mkdir -p "$INSTALL_DIR" || {
  echo "❌ Failed to create installation directory: $INSTALL_DIR"
  exit 1
}

# Download and extract in one step
TEMP_FILE="$(mktemp)" || {
  echo "❌ Failed to create temporary file"
  exit 1
}

trap "rm -f '$TEMP_FILE'" EXIT

echo "⏳ Downloading and extracting..."
if ! curl -fsSL "$PACKAGE_URL" -o "$TEMP_FILE"; then
  echo "❌ Download failed: $PACKAGE_URL"
  exit 1
fi

# Extract based on platform
if ! $EXTRACT_CMD "$TEMP_FILE" -C "$INSTALL_DIR"; then
  echo "❌ Extraction failed"
  exit 1
fi

echo "✅ Installation successful!"
echo "📍 Location: $INSTALL_DIR"
echo ""

# Platform-specific next steps
if [ "$PLATFORM" = "windows" ]; then
  echo "📋 Next steps (PowerShell):"
  echo ""
  echo "  Add to PATH:"
  echo "    \$env:PATH = \"$INSTALL_DIR\\bin;\" + \$env:PATH"
  echo ""
  echo "  Or add CMake prefix:"
  echo "    \$env:CMAKE_PREFIX_PATH = \"$INSTALL_DIR\\lib\\cmake;\" + \$env:CMAKE_PREFIX_PATH"
else
  echo "📋 Next steps:"
  echo ""
  echo "  Add to ~/.bashrc or ~/.zshrc:"
  echo "    export PATH=\"$INSTALL_DIR/bin:\$PATH\""
  echo "    export LD_LIBRARY_PATH=\"$INSTALL_DIR/lib:\$LD_LIBRARY_PATH\""
  echo "    export PKG_CONFIG_PATH=\"$INSTALL_DIR/lib/pkgconfig:\$PKG_CONFIG_PATH\""
  echo ""
  echo "  Then reload:"
  echo "    source ~/.bashrc"
fi

echo ""
echo "📖 For more info, see: $INSTALL_DIR/FALCON_DEPENDENCIES.txt"
