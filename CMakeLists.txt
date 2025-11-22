cmake_minimum_required(VERSION 3.20)
project(bible_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(bible-app
    src/main.cpp
    src/cli.cpp
    src/validation.cpp
    src/listing.cpp
    src/fs_utils.cpp
    src/print.cpp
    src/reader.cpp
    src/format_utils.cpp
    src/config.cpp
)
 
# Headers live in include/
target_include_directories(bible-app PRIVATE ${CMAKE_SOURCE_DIR}/include)

# RapidJSON (header-only). Try CMake package; otherwise allow system includes.
find_package(RapidJSON QUIET)
if (RapidJSON_FOUND)
    target_include_directories(bible-app PRIVATE ${RapidJSON_INCLUDE_DIRS})
else()
    message(STATUS "RapidJSON package not found; assuming headers available in default include path")
endif()


