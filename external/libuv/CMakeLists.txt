include(FetchContent)
FetchContent_Declare(
  libuv_content
  GIT_REPOSITORY https://github.com/libuv/libuv
  GIT_TAG v1.51.0
)

FetchContent_MakeAvailable(libuv_content)
