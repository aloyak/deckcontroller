include(FetchContent)

include(SDL3)

add_executable(deck
    src/main.cpp
    src/network.cpp
    src/input.cpp
)

target_link_libraries(deck PRIVATE SDL3::SDL3)

target_include_directories(deck PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# TODO: check for complete windows support
if(WIN32)
    add_custom_command(TARGET deck POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:deck>
    )
endif()