set(ZLIB_USE_STATIC_LIBS ON)
find_package(ZLIB REQUIRED)

add_subdirectory(miniaudio)
add_subdirectory(spdlog)
add_subdirectory(cli11)
add_subdirectory(ixWebSocket)
