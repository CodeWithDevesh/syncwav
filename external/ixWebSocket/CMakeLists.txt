include(FetchContent)
FetchContent_Declare(
  ixwebsocket_content
  GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
  GIT_TAG v11.4.6
)

FetchContent_MakeAvailable(ixwebsocket_content)
