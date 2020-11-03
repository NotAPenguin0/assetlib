cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(assetlib C CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(assetlib "")
target_sources(assetlib PRIVATE "src/asset_file.cpp" "src/texture.cpp")
target_include_directories(assetlib PUBLIC "include/")
target_include_directories(assetlib PRIVATE "external/SimpleJSON" "external/lz4/lib/")

add_library(lz4 STATIC)
target_sources(lz4 PRIVATE "external/lz4/lib/lz4.c")
target_include_directories(lz4 PRIVATE "external/lz4/lib")