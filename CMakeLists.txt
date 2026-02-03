cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(Izotrox CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(ANDROID)
    add_definitions(-D__ANDROID__)
    file(GLOB_RECURSE PLATFORM_SOURCES "src/Platform/Android/*.cpp")
else()
    add_definitions(-D__LINUX__)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    file(GLOB_RECURSE PLATFORM_SOURCES "src/Platform/Linux/*.cpp")
    # Include AndroidDevice even on Linux as it's used in main.cpp (with #ifdefs inside)
    list(APPEND PLATFORM_SOURCES "src/Platform/Android/AndroidDevice.cpp")
endif()


file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*/Platform/.*")

file(GLOB_RECURSE HEADERS "include/**/*.h" "include/**/*.hpp")

add_executable(izotrox ${SOURCES} ${HEADERS} ${PLATFORM_SOURCES})

target_include_directories(izotrox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(izotrox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(izotrox PRIVATE LUA_USE_POSIX)
target_compile_options(izotrox PRIVATE)

target_link_libraries(izotrox PRIVATE m)

if(NOT ANDROID)
    target_link_libraries(izotrox PRIVATE ${SDL2_LIBRARIES})
endif()

if(ANDROID)
    install(TARGETS izotrox DESTINATION /data/adb/izotrox.dir/)
endif()
