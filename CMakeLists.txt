cmake_minimum_required(VERSION 3.25)

project(Izotrox LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Using ccache")
    set(CMAKE_C_COMPILER_LAUNCHER ccache)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()

if(ANDROID)
    add_definitions(-D__ANDROID__)
    file(GLOB_RECURSE PLATFORM_SOURCES "src/Platform/Android/*.cpp")
else()
    add_definitions(-D__LINUX__)
    find_package(SDL2 REQUIRED)
    file(GLOB_RECURSE PLATFORM_SOURCES "src/Platform/Linux/*.cpp")
    list(APPEND PLATFORM_SOURCES "src/Platform/Android/AndroidDevice.cpp")
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*/Platform/.*")

file(GLOB_RECURSE HEADERS "include/**/*.h" "include/**/*.hpp")

add_executable(izotrox ${SOURCES} ${HEADERS} ${PLATFORM_SOURCES})

target_include_directories(izotrox PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(izotrox PRIVATE LUA_USE_POSIX)

# Fast debug flags
target_compile_options(izotrox PRIVATE -O3)

# Precompiled headers (big win)
target_precompile_headers(izotrox PRIVATE
    <vector>
    <string>
    <unordered_map>
    <memory>
    <iostream>
    <algorithm>
    <cmath>
    <SDL.h>
)

target_link_libraries(izotrox PRIVATE m)

if(NOT ANDROID)
    target_link_libraries(izotrox PRIVATE SDL2::SDL2)
endif()

if(ANDROID)
    install(TARGETS izotrox DESTINATION /data/adb/izotrox.dir/)
endif()
