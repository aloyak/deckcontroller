include(FetchContent)

include(SDL3)

include(IMGUI)

add_executable(driver
    src/main.cpp
)

target_link_libraries(driver PRIVATE SDL3::SDL3)

target_include_directories(driver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# TODO: check for complete windows support
if(WIN32)
    add_custom_command(TARGET driver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:driver>
    )
endif()