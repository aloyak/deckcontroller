cmake_minimum_required(VERSION 3.20)
project(driver VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FetchContent)

add_executable(driver
    src/main.cpp
)

target_include_directories(driver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)