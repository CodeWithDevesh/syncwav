cmake_minimum_required(VERSION 3.20)
project(
  syncwav
  VERSION 0.1.0
  LANGUAGES CXX C) # C required for some low-level audio headers

# --- Global Compiler Settings ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clangd/LSP
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ==============================================================================
# FEATURE FLAGS (Toggle via CLI: cmake -DSWAV_BUILD_TUI=OFF ..)
# ==============================================================================
option(SWAV_ENABLE_LOGGING "Enable spdlog for internal logging" ON)
option(SWAV_ENABLE_WEBSOCKETS "Enable ixWebSocket remote control/streaming" ON)
option(SWAV_BUILD_CLI "Build the Command Line Interface executable" ON)
option(SWAV_BUILD_TUI "Build the Terminal UI executable" ON)

# ==============================================================================
# SYSTEM DEPENDENCIES
# ==============================================================================
find_package(Threads REQUIRED)
find_package(ZLIB)

if(SWAV_ENABLE_WEBSOCKETS)
  find_package(OpenSSL REQUIRED)
endif()

# Load the external directory (which relies on the feature flags above)
add_subdirectory(external)

# ==============================================================================
# CORE LIBRARY
# ==============================================================================
# Strict Warnings Target
add_library(syncwav_warnings INTERFACE)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(syncwav_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
     "${PROJECT_SOURCE_DIR}/src/core/*.cpp")

add_library(syncwav_core SHARED ${CORE_SOURCES})
add_library(syncwav::core ALIAS syncwav_core) # Namespace alias for cleaner
                                              # linking

target_compile_definitions(syncwav_core PRIVATE SWAV_BUILD_DLL)
target_include_directories(
  syncwav_core PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

# Base Links (Always required)
target_link_libraries(syncwav_core PUBLIC miniaudio Threads::Threads
                                          syncwav_warnings)

# --- Apply Core Feature Macros & Links ---
if(SWAV_ENABLE_LOGGING)
  target_link_libraries(syncwav_core PUBLIC spdlog::spdlog)
  target_compile_definitions(syncwav_core PUBLIC SWAV_USE_SPDLOG)
endif()

if(SWAV_ENABLE_WEBSOCKETS)
  target_link_libraries(syncwav_core PUBLIC ixwebsocket)
  target_compile_definitions(syncwav_core PUBLIC SWAV_USE_WEBSOCKETS)
endif()

if(ZLIB_FOUND)
  target_link_libraries(syncwav_core PUBLIC ZLIB::ZLIB)
  target_compile_definitions(syncwav_core PUBLIC SWAV_USE_ZLIB)
endif()

# --- Graceful FFmpeg Implementation ---
if(NOT WIN32)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(FFMPEG IMPORTED_TARGET libavformat libavcodec libavutil
                    libswresample)

  if(FFMPEG_FOUND)
    target_link_libraries(syncwav_core PUBLIC PkgConfig::FFMPEG)
    target_compile_definitions(syncwav_core PUBLIC SWAV_USE_FFMPEG)
  else()
    message(WARNING "FFmpeg not found via pkg-config. File input disabled.")
  endif()
else()
  set(FFMPEG_ROOT
      "C:/Program Files/FFmpeg"
      CACHE PATH "Path to FFmpeg installation")
  if(EXISTS "${FFMPEG_ROOT}/include")
    target_include_directories(syncwav_core PUBLIC "${FFMPEG_ROOT}/include")
    target_link_directories(syncwav_core PUBLIC "${FFMPEG_ROOT}/lib")
    target_link_libraries(syncwav_core PUBLIC avformat avcodec avutil
                                              swresample)
    target_compile_definitions(syncwav_core PUBLIC SWAV_USE_FFMPEG)
  else()
    message(WARNING "FFmpeg not found at ${FFMPEG_ROOT}. File input disabled.")
  endif()
endif()

# ==============================================================================
# EXECUTABLES
# ==============================================================================
if(SWAV_BUILD_CLI)
  file(GLOB_RECURSE CLI_SOURCES CONFIGURE_DEPENDS
       "${PROJECT_SOURCE_DIR}/src/cli/*.cpp")
  add_executable(syncwav_cli ${CLI_SOURCES})
  target_link_libraries(syncwav_cli PRIVATE syncwav::core CLI11::CLI11
                                            syncwav_warnings)
endif()

if(SWAV_BUILD_TUI)
  file(GLOB_RECURSE TUI_SOURCES CONFIGURE_DEPENDS
       "${PROJECT_SOURCE_DIR}/src/tui/*.cpp")
  add_executable(syncwav_tui ${TUI_SOURCES})
  target_link_libraries(
    syncwav_tui PRIVATE syncwav::core ftxui::component ftxui::dom ftxui::screen
                        syncwav_warnings)
endif()


# ==============================================================================
# BUILD CONFIGURATION SUMMARY
# ==============================================================================
message(STATUS "")
message(STATUS "=======================================================")
message(STATUS " syncwav v${PROJECT_VERSION} - Build Configuration Summary")
message(STATUS "=======================================================")
message(STATUS " Compiler        : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " Build Type      : ${CMAKE_BUILD_TYPE}")
message(STATUS "")
message(STATUS " --- Core Features ---")
message(STATUS " Internal Logging (spdlog) : ${SWAV_ENABLE_LOGGING}")
message(STATUS " WebSockets (ixWebSocket)  : ${SWAV_ENABLE_WEBSOCKETS}")
message(STATUS "")
message(STATUS " --- Executables ---")
message(STATUS " Build CLI (syncwav_cli)   : ${SWAV_BUILD_CLI}")
message(STATUS " Build TUI (syncwav_tui)   : ${SWAV_BUILD_TUI}")
message(STATUS "")
message(STATUS " --- Optional Dependencies ---")

# Evaluate FFmpeg status based on the platform checks we did earlier
if(SWAV_USE_FFMPEG)
    message(STATUS " FFmpeg (File Input)       : ON")
else()
    message(STATUS " FFmpeg (File Input)       : OFF")
endif()

if(ZLIB_FOUND)
    message(STATUS " ZLIB (Compression)        : ON")
else()
    message(STATUS " ZLIB (Compression)        : OFF")
endif()

message(STATUS "=======================================================")
message(STATUS " Configure finished. Ready to build!")
message(STATUS "")
