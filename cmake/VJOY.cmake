include(FetchContent)

FetchContent_Declare(
    vjoy
    GIT_REPOSITORY https://github.com/shauleiz/vJoy
    GIT_TAG        master  
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(vjoy)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_VJOY_LIB_DIR "${vjoy_SOURCE_DIR}/SDK/lib/amd64")
else()
    set(_VJOY_LIB_DIR "${vjoy_SOURCE_DIR}/SDK/lib")
endif()

add_library(vjoy STATIC IMPORTED GLOBAL)

set_target_properties(vjoy PROPERTIES
    IMPORTED_LOCATION             "${_VJOY_LIB_DIR}/vJoyInterface.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${vjoy_SOURCE_DIR}/SDK/inc"
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_VJOY_INSTALL_DIR "C:/Program Files/vJoy/x64")
else()
    set(_VJOY_INSTALL_DIR "C:/Program Files/vJoy/x86")
endif()
set(VJOY_DLL "${_VJOY_INSTALL_DIR}/vJoyInterface.dll" CACHE INTERNAL "")