cmake_minimum_required(VERSION 3.10)

project(wm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(
    SOURCES
    window_manager.cpp
    main.cpp
)

add_link_options("-lxcb")

add_executable(wm ${SOURCES})

target_compile_options(wm PRIVATE -Wall -Wextra -pedantic -Werror)