cmake_minimum_required(VERSION 3.20)
include(FetchContent)

FetchContent_Declare(
	spdlog
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_TAG v1.15.3
)

FetchContent_MakeAvailable(spdlog)
