include(FetchContent)
FetchContent_Declare(
  uSockets_content
  GIT_REPOSITORY https://github.com/uNetworking/uSockets
  GIT_TAG v0.8.8
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(uSockets_content)

file(GLOB_RECURSE USOCKETS_SOURCES ${usockets_content_SOURCE_DIR}/src/*.c ${usockets_content_SOURCE_DIR}/src/*.cpp)
add_library(uSockets STATIC ${USOCKETS_SOURCES})
target_compile_definitions(uSockets PUBLIC LIBUS_USE_OPENSSL)
target_link_libraries(uSockets PUBLIC zlibstatic uv_a OpenSSL::SSL)
target_include_directories(uSockets PUBLIC ${usockets_content_SOURCE_DIR}/src)

set_property(TARGET uSockets PROPERTY CXX_STANDARD 20)
