cmake_minimum_required(VERSION 3.20)

project(syncwav)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL)

add_subdirectory(external)

file(GLOB_RECURSE CORE_SOURCES 
	"${PROJECT_SOURCE_DIR}/src/core/*.cpp"
	)
file(GLOB_RECURSE CLI_SOURCES 
	"${PROJECT_SOURCE_DIR}/src/cli/*.cpp"
	)
file(GLOB_RECURSE TEST_SOURCES 
	"${PROJECT_SOURCE_DIR}/src/test/*.cpp"
	)

add_library(syncwav_core SHARED ${CORE_SOURCES})
target_compile_definitions(syncwav_core PRIVATE SWAV_BUILD_DLL)
target_link_libraries(syncwav_core PUBLIC miniaudio spdlog::spdlog uWebSockets ixwebsocket)
target_include_directories(syncwav_core PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(NOT WIN32)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavformat libavcodec libavutil libswresample
  )

  target_link_libraries(syncwav_core PUBLIC
    PkgConfig::FFMPEG
  )
  target_include_directories(syncwav_core PRIVATE
    ${FFMPEG_INCLUDE_DIRS}
  )

else() # WIN32 branch

  set(FFMPEG_ROOT "C:/Program Files/FFmpeg")

  target_include_directories(syncwav_core PUBLIC "${FFMPEG_ROOT}/include")
  target_link_directories(syncwav_core PUBLIC "${FFMPEG_ROOT}/lib")

  target_link_libraries(syncwav_core PUBLIC
    avformat
    avcodec
    avutil
    swresample
  )
  
endif()

add_executable(syncwav_cli ${CLI_SOURCES})
target_link_libraries(syncwav_cli syncwav_core CLI11::CLI11)

add_executable(syncwav_test ${TEST_SOURCES})
target_link_libraries(syncwav_test syncwav_core)
