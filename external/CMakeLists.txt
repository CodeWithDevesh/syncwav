# ==============================================================================
# PREVENT THIRD-PARTY BLOAT (Globally disable tests and examples)
# ==============================================================================
set(SPDLOG_BUILD_TESTS
    OFF
    CACHE INTERNAL "Disable spdlog tests")
set(SPDLOG_BUILD_EXAMPLE
    OFF
    CACHE INTERNAL "Disable spdlog examples")

set(CLI11_BUILD_TESTS
    OFF
    CACHE INTERNAL "Disable CLI11 tests")
set(CLI11_BUILD_EXAMPLES
    OFF
    CACHE INTERNAL "Disable CLI11 examples")

set(FTXUI_BUILD_TESTS
    OFF
    CACHE INTERNAL "Disable FTXUI tests")
set(FTXUI_BUILD_EXAMPLES
    OFF
    CACHE INTERNAL "Disable FTXUI examples")
set(FTXUI_BUILD_DOCS
    OFF
    CACHE INTERNAL "Disable FTXUI docs")

set(IXWEBSOCKET_BUILD_TESTS
    OFF
    CACHE INTERNAL "Disable ixWebSocket tests")

set(MINIAUDIO_BUILD_EXAMPLES
    OFF
    CACHE INTERNAL "Disable miniaudio examples")
set(MINIAUDIO_BUILD_TESTS
    OFF
    CACHE INTERNAL "Disable miniaudio tests")
set(MINIAUDIO_BUILD_TOOLS
    OFF
    CACHE INTERNAL "Disable miniaudio tools")
set(MINIAUDIO_INSTALL
    OFF
    CACHE INTERNAL "Disable miniaudio install targets")

# ==============================================================================
# IXWEBSOCKET TRANSITIVE DEPENDENCY INJECTION
# ==============================================================================
# Force ixWebSocket to respect the system state found by the root CMake script
if(ZLIB_FOUND)
  set(USE_ZLIB
      ON
      CACHE INTERNAL "Enable zlib compression in ixWebSocket")
else()
  set(USE_ZLIB
      OFF
      CACHE INTERNAL "Disable zlib compression in ixWebSocket")
endif()

if(OPENSSL_FOUND)
  set(USE_TLS
      ON
      CACHE INTERNAL "Enable TLS in ixWebSocket")
  set(USE_OPEN_SSL
      ON
      CACHE INTERNAL "Force ixWebSocket to use OpenSSL")
else()
  set(USE_TLS
      OFF
      CACHE INTERNAL "Disable TLS in ixWebSocket")
  set(USE_OPEN_SSL
      OFF
      CACHE INTERNAL "Disable OpenSSL in ixWebSocket")
endif()

# ==============================================================================
# LOAD SUBMODULES CONDITIONALLY
# ==============================================================================
# We only add the directories if the root script feature flags are turned ON
if(SWAV_ENABLE_LOGGING)
  add_subdirectory(spdlog EXCLUDE_FROM_ALL)
endif()

if(SWAV_ENABLE_WEBSOCKETS)
  add_subdirectory(ixwebsocket EXCLUDE_FROM_ALL)
endif()

if(SWAV_BUILD_CLI)
  add_subdirectory(cli11 EXCLUDE_FROM_ALL)
endif()

if(SWAV_BUILD_TUI)
  add_subdirectory(ftxui EXCLUDE_FROM_ALL)
endif()

# Miniaudio is typically a header-only library. We create an interface target
# here so the root script can link it cleanly like the others.
add_subdirectory(miniaudio EXCLUDE_FROM_ALL)
