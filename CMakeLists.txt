cmake_minimum_required(VERSION 3.20)
project(los_santos_lens CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Extract the ScriptHookV SDK (dev-c.com) into ./sdk so that sdk/inc/main.h and sdk/lib/ScriptHookV.lib exist.
set(SHV_SDK "${CMAKE_SOURCE_DIR}/sdk" CACHE PATH "ScriptHookV SDK root")

add_library(los_santos_lens SHARED
    src/main.cpp
    src/ui.cpp
    src/settings.cpp
    src/esp.cpp
    src/aim.cpp
    src/world.cpp
    src/garage.cpp
    src/photo.cpp
    src/menu.cpp)
target_include_directories(los_santos_lens PRIVATE "${SHV_SDK}/inc")
target_link_libraries(los_santos_lens PRIVATE "${SHV_SDK}/lib/ScriptHookV.lib" windowscodecs ole32)
target_compile_definitions(los_santos_lens PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
set_target_properties(los_santos_lens PROPERTIES
    OUTPUT_NAME "LosSantosLens"
    SUFFIX ".asi"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
