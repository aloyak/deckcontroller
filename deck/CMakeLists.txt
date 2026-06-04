cmake_minimum_required(VERSION 3.20)
project(deck VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FetchContent)

include(SDL3)

add_executable(deck
    src/main.cpp
)

target_link_libraries(deck PRIVATE SDL3::SDL3)

target_include_directories(deck PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# TODO: check for complete windows support
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()