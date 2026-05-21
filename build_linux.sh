#!/usr/bin/env bash
set -e

# --- Default Configuration ---
BUILD_DIR="build"
BUILD_TYPE="Release"
CLEAN=0
CMAKE_ARGS=()

# --- Default Feature States ---
# These match the defaults in your CMakeLists.txt
ENABLE_LOGGING="ON"
ENABLE_WEBSOCKETS="ON"
ENABLE_FFMPEG="ON"
ENABLE_LOOPBACK="ON"
BUILD_CLI="ON"
BUILD_TUI="ON"
STRICT_WARNINGS="ON"
BUILD_SHARED="ON"

# --- Help Menu ---
show_help() {
    cat << EOF
SyncWave Linux Build Script

Usage: ./build_linux.sh [OPTIONS] [CMAKE_ARGS...]

Options:
  -h, --help               Show this help message and exit
  -c, --clean              Delete the build directory before configuring
  --build-dir <dir>        Directory for build artifacts (Default: $BUILD_DIR)
  --build-type <type>      CMake build type [Release, Debug, RelWithDebInfo] (Default: $BUILD_TYPE)

Build Configuration Options (All default to ON):
  --disable-logging        Disable spdlog for internal logging
  --disable-websockets     Disable ixWebSocket remote control/streaming
  --disable-ffmpeg         Disable FFmpeg for file IO
  --disable-loopback       Disable loopback input mode (Windows only anyway)
  --disable-cli            Do not build the Command Line Interface (syncwav_cli)
  --disable-tui            Do not build the Terminal UI (syncwav_tui)
  --disable-warnings       Disable strict compiler warnings
  --static                 Build syncwav_core statically (Sets SWAV_BUILD_SHARED=OFF)

Any additional arguments (e.g., -DCMAKE_EXPORT_COMPILE_COMMANDS=OFF) are passed directly to CMake.

Example:
  ./build_linux.sh --clean --build-type Debug --disable-ffmpeg --static
EOF
    exit 0
}

# --- Argument Parsing ---
while [[ $# -gt 0 ]]; do
  case $1 in
    -h|--help)
      show_help
      ;;
    -c|--clean)
      CLEAN=1
      shift
      ;;
    --build-dir)
      BUILD_DIR="$2"
      shift 2
      ;;
    --build-type)
      BUILD_TYPE="$2"
      shift 2
      ;;
    --disable-logging)
      ENABLE_LOGGING="OFF"
      shift
      ;;
    --disable-websockets)
      ENABLE_WEBSOCKETS="OFF"
      shift
      ;;
    --disable-ffmpeg)
      ENABLE_FFMPEG="OFF"
      shift
      ;;
    --disable-loopback)
      ENABLE_LOOPBACK="OFF"
      shift
      ;;
    --disable-cli)
      BUILD_CLI="OFF"
      shift
      ;;
    --disable-tui)
      BUILD_TUI="OFF"
      shift
      ;;
    --disable-warnings)
      STRICT_WARNINGS="OFF"
      shift
      ;;
    --static)
      BUILD_SHARED="OFF"
      shift
      ;;
    *)
      # Collect all other arguments to pass to CMake
      CMAKE_ARGS+=("$1")
      shift
      ;;
  esac
done

echo "======================================================="
echo "Compiling SyncWave for Linux"
echo "======================================================="

# Clean only if explicitly requested
if [ "$CLEAN" -eq 1 ] && [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build cache..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Configuring Build System with CMake..."
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DSWAV_ENABLE_LOGGING="$ENABLE_LOGGING" \
    -DSWAV_ENABLE_WEBSOCKETS="$ENABLE_WEBSOCKETS" \
    -DSWAV_ENABLE_FFMPEG="$ENABLE_FFMPEG" \
    -DSWAV_ENABLE_LOOPBACK="$ENABLE_LOOPBACK" \
    -DSWAV_BUILD_CLI="$BUILD_CLI" \
    -DSWAV_BUILD_TUI="$BUILD_TUI" \
    -DSWAV_STRICT_WARNINGS="$STRICT_WARNINGS" \
    -DSWAV_BUILD_SHARED="$BUILD_SHARED" \
    "${CMAKE_ARGS[@]}" \
    ..

echo "Compiling targets with Ninja..."
ninja

echo "======================================================="
echo "Build Complete!"
if [ "$BUILD_CLI" == "ON" ]; then
    echo " CLI: $BUILD_DIR/syncwav_cli"
fi
if [ "$BUILD_TUI" == "ON" ]; then
    echo " TUI: $BUILD_DIR/syncwav_tui"
fi
echo "======================================================="
