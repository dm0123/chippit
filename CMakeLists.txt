cmake_minimum_required(VERSION 3.29)

set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES ${CMAKE_SOURCE_DIR}/conan_provider.cmake)

project(chippit)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

find_package(SDL2 REQUIRED)

add_executable(chippit src/main.cpp src/application.cpp src/emulation.cpp)
target_sources(chippit PUBLIC FILE_SET CXX_MODULES 
    FILES
        src/core.cppm
        src/emulation.cppm
        src/graphics.cppm
        src/input.cppm
        src/rom.cppm
        src/application.cppm
)

target_link_libraries(chippit PUBLIC SDL2::SDL2)