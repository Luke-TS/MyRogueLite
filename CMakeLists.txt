cmake_minimum_required(VERSION 3.20)
project(MyRogueLite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(MyRogueLite src/main.cpp)

find_package(raylib REQUIRED)

target_link_libraries(MyRogueLite raylib)

# Linux extras
if(UNIX AND NOT APPLE)
    target_link_libraries(MyRogueLite dl pthread)
endif()

target_compile_definitions(MyRogueLite PRIVATE 
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)
