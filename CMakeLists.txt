cmake_minimum_required(VERSION 3.20)
project(Delve)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(Delve src/main.cpp)
target_include_directories(Delve PRIVATE src)

find_package(raylib REQUIRED)

target_link_libraries(Delve raylib)

# Linux extras
if(UNIX AND NOT APPLE)
    target_link_libraries(Delve dl pthread)
endif()

target_compile_definitions(Delve PRIVATE 
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
    LEVELS_DIR="${CMAKE_SOURCE_DIR}/levels"
)
