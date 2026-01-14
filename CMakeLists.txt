cmake_minimum_required(VERSION 3.25)

project(Izotrox CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(ANDROID)
    add_definitions(-D__ANDROID__)
    file(GLOB_RECURSE PLATFORM_SOURCES "src/platform/android/*.cpp")
else()
    add_definitions(-D__LINUX__)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    file(GLOB_RECURSE PLATFORM_SOURCES "src/platform/linux/*.cpp")
endif()


file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*/*.cpp")

file(GLOB_RECURSE HEADERS
     "include/**/*.h"
     "include/**/*.hpp")

add_executable(izotrox ${SOURCES} ${HEADERS} ${PLATFORM_SOURCES})

target_include_directories(izotrox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(izotrox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(izotrox PRIVATE LUA_USE_POSIX)
target_compile_options(izotrox PRIVATE -O3)

target_link_libraries(izotrox PRIVATE m)

if(NOT ANDROID)
    target_link_libraries(izotrox PRIVATE ${SDL2_LIBRARIES})
endif()

if(ANDROID)
    install(TARGETS izotrox DESTINATION /data/adb/izotrox.dir/)
else()
    install(TARGETS izotrox DESTINATION /usr/local/bin/)
endif()
