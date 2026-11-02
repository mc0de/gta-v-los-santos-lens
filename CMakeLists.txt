cmake_minimum_required(VERSION 3.20)
project(gta_esp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Extract the ScriptHookV SDK (dev-c.com) into ./sdk so that sdk/inc/main.h and sdk/lib/ScriptHookV.lib exist.
set(SHV_SDK "${CMAKE_SOURCE_DIR}/sdk" CACHE PATH "ScriptHookV SDK root")

add_library(gta_esp SHARED
    src/main.cpp
    src/ui.cpp
    src/settings.cpp
    src/esp.cpp
    src/aim.cpp
    src/world.cpp
    src/menu.cpp)
target_include_directories(gta_esp PRIVATE "${SHV_SDK}/inc")
target_link_libraries(gta_esp PRIVATE "${SHV_SDK}/lib/ScriptHookV.lib")
target_compile_definitions(gta_esp PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
set_target_properties(gta_esp PROPERTIES
    OUTPUT_NAME "SPESP"
    SUFFIX ".asi"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
