#!/bin/bash
# ============================================================================
# Falcon Installer
# Supports: Linux (native bash), Windows (Git Bash/WSL/MSYS2)
#
# Usage:
#   curl -fsSL https://github.com/falcon-autotuning/falcon/releases/download/v1.1.0/install.sh | bash
# ============================================================================

set -euo pipefail

# Configuration
REPO_OWNER="falcon-autotuning"
REPO_NAME="falcon"

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

# Extract version from GitHub release URL (e.g., v1.1.0)
# If run directly from file, try to detect from environment or GitHub release API
detect_release_version() {
  # Try to get from RELEASE_VERSION env var first
  if [ -n "${RELEASE_VERSION:-}" ]; then
    echo "$RELEASE_VERSION"
    return
  fi

  # Try to detect from GitHub API (latest non-prerelease)
  local api_url="https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/releases"
  local latest=$(curl -fsSL "$api_url?per_page=1" | grep -oP '"tag_name": "\K[^"]+' | head -1 || echo "")

  if [ -n "$latest" ]; then
    echo "$latest"
  else
    echo "❌ Failed to detect release version"
    exit 1
  fi
}

RELEASE_VERSION=$(detect_release_version)
RELEASE_URL="https://github.com/$REPO_OWNER/$REPO_NAME/releases/download/$RELEASE_VERSION"

# Platform-specific configuration
if [ "$PLATFORM" = "windows" ]; then
  PACKAGE_FILE="falcon-${RELEASE_VERSION}-win64.zip"
  INSTALL_DIR="${FALCON_INSTALL_DIR:-C:/falcon}"
else
  PACKAGE_FILE="falcon-${RELEASE_VERSION}-Linux.tar.gz"
  INSTALL_DIR="${FALCON_INSTALL_DIR:-/opt/falcon}"
fi

extract_package() {
  local archive="$1"
  local dest="$2"

  if [ "$PLATFORM" = "windows" ]; then
    unzip -q -o "$archive" -d "$dest"
    if [ -d "$dest/falcon" ]; then
      cp -r "$dest/falcon/"* "$dest/"
      rm -rf "$dest/falcon"
    fi
  else
    tar --overwrite --strip-components=1 -xzf "$archive" -C "$dest"
  fi
}

PACKAGE_URL="$RELEASE_URL/$PACKAGE_FILE"

# Display info
echo "🔧 Falcon Installer"
echo "=========================================="
echo ""
echo "📍 Installation Directory: $INSTALL_DIR"
echo "📦 Platform: $([ "$PLATFORM" = "windows" ] && echo "Windows" || echo "Linux")"
echo "📌 Release: $RELEASE_VERSION"
echo ""

# Check permissions early for Linux/Mac
if [ "$PLATFORM" != "windows" ] && [ "$EUID" -ne 0 ]; then
  if [ -d "$INSTALL_DIR" ] && [ ! -w "$INSTALL_DIR" ]; then
    echo "❌ Permission denied. Please run with 'sudo' to install to $INSTALL_DIR"
    exit 1
  elif [ ! -d "$INSTALL_DIR" ] && [ ! -w "$(dirname "$INSTALL_DIR")" ]; then
    echo "❌ Permission denied. Please run with 'sudo' to install to $INSTALL_DIR"
    exit 1
  fi
fi

# Load Docker image if not present
if ! docker image inspect falcon:latest &>/dev/null; then
  echo "🐳 Docker image falcon:latest not found locally."
  echo "⏳ Downloading Docker image tarball..."

  IMAGE_URL="$RELEASE_URL/falcon-cli-image.tar.gz"
  TEMP_IMAGE="$(mktemp)" || {
    echo "❌ Failed to create temporary file for Docker image"
    exit 1
  }

  echo "📥 Fetching $IMAGE_URL..."
  if curl -fsSL "$IMAGE_URL" -o "$TEMP_IMAGE"; then
    echo "⏳ Loading Docker image into daemon (this may take a while)..."
    if ! docker load -i "$TEMP_IMAGE"; then
      echo "❌ Failed to load Docker image."
      rm -f "$TEMP_IMAGE"
      exit 1
    fi
    rm -f "$TEMP_IMAGE"
    echo "✅ Docker image loaded successfully!"
  else
    echo "❌ Failed to download Docker image tarball from $IMAGE_URL"
    rm -f "$TEMP_IMAGE"
    exit 1
  fi
else
  echo "🐳 Docker image falcon:latest already cached locally."
fi

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
if ! extract_package "$TEMP_FILE" "$INSTALL_DIR"; then
  echo "❌ Extraction failed"
  exit 1
fi

echo "✅ Installation successful!"
echo "📍 Location: $INSTALL_DIR"
echo ""

# Add to PATH
echo "⚙️ Configuring PATH..."

if [ "$PLATFORM" = "windows" ]; then
  WIN_INSTALL_DIR="${INSTALL_DIR//\//\\}"
  WIN_BIN_DIR="${WIN_INSTALL_DIR}\\bin"

  echo "🪟 Adding $WIN_BIN_DIR to Windows User PATH via PowerShell..."
  if powershell.exe -Command "
    \$binDir = '$WIN_BIN_DIR';
    \$currentPath = [Environment]::GetEnvironmentVariable('PATH', 'User');
    if (\$currentPath -notlike '*'\$binDir'*') {
      [Environment]::SetEnvironmentVariable('PATH', \$currentPath + ';\$binDir', 'User');
      exit 0;
    } else {
      exit 1;
    }
  " 2>/dev/null; then
    echo "✅ Added to Windows User PATH."
    echo "ℹ️ Please restart your terminal for changes to take effect."
  else
    echo "ℹ️ $WIN_BIN_DIR is already in Windows User PATH."
  fi
else
  BIN_DIR="$INSTALL_DIR/bin"
  REAL_USER="${SUDO_USER:-$USER}"
  REAL_HOME="$(eval echo "~$REAL_USER")"

  updated=0

  for profile in "$REAL_HOME/.bashrc" "$REAL_HOME/.zshrc" "$REAL_HOME/.bash_profile" "$REAL_HOME/.profile"; do
    if [ -f "$profile" ]; then
      echo "🐧 Adding $BIN_DIR to PATH in $profile..."
      if ! grep -q "$BIN_DIR" "$profile" 2>/dev/null; then
        echo "" >>"$profile"
        echo "# Falcon Toolchain" >>"$profile"
        echo "export PATH=\"$BIN_DIR:\$PATH\"" >>"$profile"
        echo "✅ Added to $profile."
        updated=1
      else
        echo "ℹ️ $BIN_DIR is already in $profile."
      fi
    fi
  done

  if [ "$updated" -eq 1 ]; then
    echo "ℹ️ Please restart your terminal or source your profile file for changes to take effect."
  fi
fi

echo ""
echo "📖 For more info, see: $INSTALL_DIR/FALCON_DEPENDENCIES.txt"
