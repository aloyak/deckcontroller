cmake_minimum_required(VERSION 3.20)
project(deckcontroller VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(WIN32)
	add_subdirectory(target)
else()
	add_subdirectory(deck)
	add_subdirectory(target)
endif()
