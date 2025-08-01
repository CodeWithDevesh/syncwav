cmake_minimum_required(VERSION 3.5)

project(syncwav)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)

add_subdirectory(external)

file(GLOB_RECURSE CORE_SOURCES 
	"${PROJECT_SOURCE_DIR}/src/core/*.cpp"
	)
file(GLOB_RECURSE CLI_SOURCES 
	"${PROJECT_SOURCE_DIR}/src/cli/*.cpp"
	)

add_library(syncwav_core SHARED ${CORE_SOURCES})
target_compile_definitions(syncwav_core PRIVATE SWAV_BUILD_DLL)
target_link_libraries(syncwav_core PUBLIC spdlog::spdlog uWebSockets)
target_include_directories(syncwav_core PUBLIC ${PROJECT_SOURCE_DIR}/include)


add_executable(syncwav_cli ${CLI_SOURCES})
target_link_libraries(syncwav_cli syncwav_core CLI11::CLI11)
