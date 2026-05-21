#!/usr/bin/env bash
set -e

# --- Default Configuration ---
NDK_HOME="${ANDROID_NDK_HOME:-/opt/android-sdk/ndk/27.1.12297006}"
BUILD_DIR="build-android"
ABI="arm64-v8a"
PLATFORM="android-24"
CLEAN=0
CMAKE_ARGS=()

# --- Help Menu ---
show_help() {
    cat << EOF
SyncWave Android Build Script

Usage: ./build_android.sh [OPTIONS] [CMAKE_ARGS...]

Options:
  -h, --help            Show this help message and exit
  -c, --clean           Delete the build directory before configuring
  --ndk <path>          Path to the Android NDK (Overrides ANDROID_NDK_HOME)
                        Default: $NDK_HOME
  --build-dir <dir>     Directory for build artifacts
                        Default: $BUILD_DIR
  --abi <abi>           Target Android ABI (e.g., arm64-v8a, armeabi-v7a, x86_64)
                        Default: $ABI
  --platform <level>    Android API level (e.g., android-24)
                        Default: $PLATFORM

Any additional arguments (e.g., -DCMAKE_BUILD_TYPE=Debug) are passed directly to CMake.

Example:
  ./build_android.sh --clean --abi x86_64 -DSWAV_ENABLE_LOGGING=OFF
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
    --ndk)
      NDK_HOME="$2"
      shift 2
      ;;
    --build-dir)
      BUILD_DIR="$2"
      shift 2
      ;;
    --abi)
      ABI="$2"
      shift 2
      ;;
    --platform)
      PLATFORM="$2"
      shift 2
      ;;
    *)
      # Collect all other arguments to pass to CMake
      CMAKE_ARGS+=("$1")
      shift
      ;;
  esac
done

echo "======================================================="
echo "Compiling SyncWave for Android ($ABI)"
echo "======================================================="

# Sanity check: Ensure NDK path actually exists
if [ ! -d "$NDK_HOME" ]; then
    echo "Error: NDK directory not found at: $NDK_HOME"
    echo "Use --ndk to specify the correct path."
    exit 1
fi

# Clean only if explicitly requested
if [ "$CLEAN" -eq 1 ] && [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build cache..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Configuring Build System with CMake..."
cmake -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_HOME/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="$PLATFORM" \
    -DCMAKE_BUILD_TYPE="Release" \
    -DSWAV_BUILD_SHARED=OFF \
    -DSWAV_ENABLE_FFMPEG=OFF \
    -DSWAV_BUILD_TUI=OFF \
    "${CMAKE_ARGS[@]}" \
    ..

echo "Compiling targets with Ninja..."
ninja

echo "======================================================="
echo "Build Complete: $BUILD_DIR/syncwav_cli"
echo "======================================================="
