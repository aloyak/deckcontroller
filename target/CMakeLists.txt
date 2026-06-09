include(FetchContent)

include(SDL3)
include(IMGUI)

if(WIN32)
    include(VJOY)
endif()

add_executable(driver
    src/main.cpp
    src/network.cpp
    src/ui.cpp
)

if(UNIX)
    target_sources(driver PRIVATE
        src/linux/driver.cpp
    )
elseif(WIN32)
    target_sources(driver PRIVATE
        src/windows/driver.cpp
    )
endif()

if(WIN32)
    target_link_libraries(driver PRIVATE SDL3::SDL3 SDL3_image::SDL3_image imgui vjoy)
else()
    target_link_libraries(driver PRIVATE SDL3::SDL3 SDL3_image::SDL3_image imgui)
endif()

target_include_directories(driver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# copy the resources folder to the output directory
add_custom_command(TARGET driver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:driver>/resources
)

if(WIN32)
    add_custom_command(TARGET driver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:driver>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3_image::SDL3_image>
            $<TARGET_FILE_DIR:driver>
    )
endif()