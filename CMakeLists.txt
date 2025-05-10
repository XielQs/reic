cmake_minimum_required(VERSION 3.16)
project(reic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(inc)

file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

add_compile_options(-Wall -Wextra)

if(DEBUG)
    add_compile_options(-O0 -g3 -ggdb3)
else()
    add_compile_options(-O2)
endif()

add_executable(reic ${SRC_FILES})
