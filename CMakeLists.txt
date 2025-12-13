# Trying out a single top-level CMakeLists.txt
cmake_minimum_required(VERSION 3.28)
project("breakdown" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------ #
# Fetch dependencies #
# ------------------ #
include(FetchContent)

# ----- SFML ----- #
FetchContent_Declare(
    sfml_dependency
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.0.2
)

# Configure SFML options
set(SFML_STATIC_LIBRARIES ON CACHE BOOL "Build SFML as static libraries")
set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "Use SFML's internal dependencies")
set(SFML_BUILD_GRAPHICS ON CACHE BOOL "Build the Graphics module")
set(SFML_BUILD_WINDOW ON CACHE BOOL "Build the Window module")
set(SFML_BUILD_AUDIO ON CACHE BOOL "Build the Audio module")
set(SFML_BUILD_NETWORK OFF CACHE BOOL "Do not build the Network module")

# ----- EnTT ----- #
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.15.0
)

# ----- toml++ ----- #
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0  
)

FetchContent_MakeAvailable(sfml_dependency entt tomlplusplus) 

# ------------------ #
# Configure project  #
# ------------------ #

add_executable(breakdown
    "breakdown/src/Main.cpp"
    "breakdown/src/Application.cpp"
    "breakdown/src/State.cpp"
    "breakdown/src/Managers/WindowManager.cpp"
    "breakdown/src/Managers/StateManager.cpp"
    "breakdown/src/Managers/GlobalEventManager.cpp"
    "breakdown/src/Managers/ConfigManager.cpp"
    "breakdown/src/Managers/ResourceManager.cpp"
    "breakdown/src/ECS/EntityFactory.cpp"
    "breakdown/src/ECS/Systems.cpp"
    "breakdown/src/Utilities/RandomMachine.cpp"
    "breakdown/src/Utilities/Utils.cpp"
)

# This runs every time the breakdown target is built.
# It will copy the resources to the build directory
add_custom_command(
    TARGET breakdown
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/breakdown/resources"
            "$<TARGET_FILE_DIR:breakdown>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/breakdown/config"
            "$<TARGET_FILE_DIR:breakdown>/config"
    COMMENT "Copying resources and config to build directory"
)

# This is to copy compile_commands.json to out directory for clangd to find it easily
add_custom_target(
    copy-compile-commands ALL 
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/out/compile_commands.json"
    DEPENDS "${CMAKE_BINARY_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands.json to out directory"
)

# -------------------- #
#  compile definitions #
# -------------------- #

target_compile_definitions(breakdown PRIVATE TOML_EXCEPTIONS=0)

# ------------------ #
# Configure include  #
# ------------------ #

# manually setting entt because clangd was being weird >:/
target_include_directories(breakdown PRIVATE
    "${entt_SOURCE_DIR}/include"
    "breakdown/include"
)

# ------------------ #
#   Link libraries   #
# ------------------ #

target_link_libraries(breakdown PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::Audio
    EnTT::EnTT
    tomlplusplus::tomlplusplus
)

# ---------------------- #
#   Precompile Headers   #
# ---------------------- #
target_precompile_headers(breakdown PRIVATE 
    # STL
    <map>
    <vector>
    <string>
    <string_view>
    <format>
    <print>
    <source_location>
    <memory>
    <optional>
    <functional>
    <random>
    <utility>
    <limits>
    <algorithm>
    <stdexcept> # goal is to get rid of this

    # C STD
    <cstdint>
    <cstdio>
    <cmath>

    # Third party libraries
    <SFML/Graphics.hpp>
    <SFML/Window.hpp>
    <SFML/Audio.hpp>
    <SFML/System.hpp>
    <entt/entt.hpp>
    <toml++/toml.hpp>
)

# ------------------ #
#   MSVC  Settings   #
# ------------------ #

# This should hopefully mitigate any false 'trojan' flags by Windows Defender
if(MSVC)
    target_compile_options(breakdown PRIVATE "/std:c++latest" /Zi /GL /guard:cf /Gy)
    target_link_options(breakdown PRIVATE /LTCG /GUARD:CF /OPT:REF /OPT:ICF)
    set_property(TARGET breakdown PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()