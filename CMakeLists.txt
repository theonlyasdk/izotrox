cmake_minimum_required(VERSION 3.25)

project(Izotrox LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Let's disable GNU extensions and stick to standard C++
set(CMAKE_CXX_EXTENSIONS OFF)

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
    # Need to be available on desktop to prevent linker errors.
    # These functions do nothing on desktop, but we still need them
    # to satisfy our linker.
    list(APPEND PLATFORM_SOURCES "src/Platform/Android/AndroidDevice.cpp")
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*/*.cpp")
# Exclude platform-specific sources from the main source list
# We will add them separately to ensure they are only compiled 
# for the appropriate platform
list(FILTER SOURCES EXCLUDE REGEX ".*/Platform/.*")
file(GLOB_RECURSE HEADERS "include/**/*.h" "include/**/*.hpp")

add_executable(izotrox ${SOURCES} ${HEADERS} ${PLATFORM_SOURCES})

target_include_directories(izotrox PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(izotrox PRIVATE LUA_USE_POSIX)

set(COMPILE_OPTS)
list(APPEND COMPILE_OPTS -O3 -flto)

option(IZO_ENABLE_SANITIZERS "Enable Address/UB sanitizers" OFF)
if(IZO_ENABLE_SANITIZERS)
    list(APPEND COMPILE_OPTS -fsanitize=address,undefined -O0)
    set(LINK_OPTS -fsanitize=address,undefined)
else()
    set(LINK_OPTS -flto)
endif()

target_compile_options(izotrox PRIVATE ${COMPILE_OPTS})

# We precompile some of the commonly used heavy headers
# to speed up the compilation.
set(PCH_HEADERS
    <vector>
    <string>
    <unordered_map>
    <memory>
    <iostream>
    <algorithm>
    <cmath>
)

# We don't use SDL on Android so we ignore SDL.h
if(NOT ANDROID)
    list(APPEND PCH_HEADERS <SDL.h>)
endif()

target_precompile_headers(izotrox PRIVATE ${PCH_HEADERS})

set(LINK_LIBS m)

if(NOT ANDROID)
    list(APPEND LINK_LIBS SDL2::SDL2)
endif()

target_link_libraries(izotrox PRIVATE ${LINK_LIBS} ${LINK_OPTS})

# On Android we can provide a location to install the binary
# We chose /data/adb/izotrox.dir/ because it's a common location 
# for Magisk modules to place their binaries. And also, we can access
# it from adb. Pretty cool!
if(ANDROID)
    install(TARGETS izotrox DESTINATION /data/adb/izotrox.dir/)
endif()
# On Linux, u just gotta run it from the build directory
