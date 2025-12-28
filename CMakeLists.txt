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

# This will copy the resources to the build directory
add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/breakdown/resources"
            "$<TARGET_FILE_DIR:breakdown>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/breakdown/config"
            "$<TARGET_FILE_DIR:breakdown>/config"
    COMMENT "Syncing resources and config files..."
)
ADD_DEPENDENCIES(breakdown CopyAssets)

# This is to copy compile_commands.json to out directory for clangd
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
#  Platform Config   #
# ------------------ #

# ----- MSVC  Settings ----- #
# This should hopefully mitigate any false 'trojan' flags by Windows Defender
if(MSVC)
    target_compile_options(breakdown PRIVATE "/std:c++latest" /Zi /GL /guard:cf /Gy)
    target_link_options(breakdown PRIVATE /LTCG /GUARD:CF /OPT:REF /OPT:ICF)
    set_property(TARGET breakdown PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

# ----- Windows Settings ----- #
# Default to OFF (No console) for final game. 
# Turn this ON to enable console window on Windows Release builds to see log output
option(SHOW_CONSOLE "Show console window on Windows Release builds" OFF)

if(WIN32)
    if(SHOW_CONSOLE)
        # 1. CONSOLE MODE
        message(STATUS "Windows Build: Console Window ENABLED")
    else()
        # 2. GUI MODE
        set_target_properties(breakdown PROPERTIES WIN32_EXECUTABLE ON)
        
        # Link SFML::Main to provide the 'WinMain' entry point required by GUI apps
        target_link_libraries(breakdown PRIVATE SFML::Main)
        
        message(STATUS "Windows Build: Console Window DISABLED (GUI Mode)")
    endif()
endif()

# ----- Linux Packaging ----- #

# ------------------ #
#    Installation    #
# ------------------ #
# This isn't the correct way to do this but it works for now.
# It is going to put resources and config into the same directory
# as the executable rather than in a separate share/breakdown or ~/.config/breakdown
install(TARGETS breakdown DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/breakdown/resources" DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/breakdown/config" DESTINATION .)

# ------------------ #
#      Packaging     #
# ------------------ #

# Set package metadata
set(CPACK_PACKAGE_NAME "breakdown")
set(CPACK_PACKAGE_VENDOR "nantr0nic")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A configurable breakout game where the bricks descend.")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "nantr0nic")

# Set the Generator (TGZ = .tar.gz)
set(CPACK_GENERATOR "TGZ")

# Configure the structure inside the tarball
# This makes it so when they extract it, they get a folder named "breakdown-1.0.0-Linux"
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

# This includes CPack and generates the packaging targets
include(CPack)
