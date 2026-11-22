cmake_minimum_required(VERSION 3.20)
project(los_santos_lens CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ScriptHookV SDK (headers and import library), committed in ./sdk.
set(SHV_SDK "${CMAKE_SOURCE_DIR}/sdk" CACHE PATH "ScriptHookV SDK root")

# Release version, set by the release workflow (-DLSL_VERSION=1.2.3). Local builds are "dev".
set(LSL_VERSION "dev" CACHE STRING "Plugin version (X.Y.Z), or dev")
set(LSL_VERSION_MAJOR 0)
set(LSL_VERSION_MINOR 0)
set(LSL_VERSION_PATCH 0)
if(LSL_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
    set(LSL_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(LSL_VERSION_MINOR ${CMAKE_MATCH_2})
    set(LSL_VERSION_PATCH ${CMAKE_MATCH_3})
elseif(NOT LSL_VERSION STREQUAL "dev")
    message(FATAL_ERROR "LSL_VERSION must be X.Y.Z or dev, got '${LSL_VERSION}'")
endif()
configure_file(src/version.h.in "${CMAKE_BINARY_DIR}/generated/version.h" @ONLY)

add_library(los_santos_lens SHARED
    src/main.cpp
    src/ui.cpp
    src/settings.cpp
    src/esp.cpp
    src/aim.cpp
    src/world.cpp
    src/garage.cpp
    src/photo.cpp
    src/menu.cpp
    src/version.rc)
target_include_directories(los_santos_lens PRIVATE "${SHV_SDK}/inc" "${CMAKE_BINARY_DIR}/generated")
target_link_libraries(los_santos_lens PRIVATE "${SHV_SDK}/lib/ScriptHookV.lib" windowscodecs ole32)
target_compile_definitions(los_santos_lens PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
set_target_properties(los_santos_lens PROPERTIES
    OUTPUT_NAME "LosSantosLens"
    SUFFIX ".asi"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
