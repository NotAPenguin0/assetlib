cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(assetlib C CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FetchContent)

FetchContent_Declare(
	plib 
	GIT_REPOSITORY https://github.com/NotAPenguin0/plib
)

FetchContent_MakeAvailable(plib)

add_library(assetlib "")
target_sources(assetlib PRIVATE "src/asset_file.cpp" "src/texture.cpp" "src/mesh.cpp" "src/environment.cpp")
target_include_directories(assetlib PUBLIC "include")
target_include_directories(assetlib PRIVATE "external/SimpleJSON" "external/lz4/lib/" "${plib_SOURCE_DIR}/include")

add_library(lz4 STATIC)
target_sources(lz4 PRIVATE "external/lz4/lib/lz4.c")
target_include_directories(lz4 PRIVATE "external/lz4/lib")

target_link_libraries(assetlib PRIVATE lz4)