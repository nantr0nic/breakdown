![Breakdown GIF](screenshots/breakdowngif.gif)

# Breakdown
A breakout game but the bricks descend.

First prototype of an SFML game made with [SFML 3 Game Template](https://www.github.com/nantr0nic/sfml3-game-template/).

Classic breakdown game but with a descending mechanic. **Fully configurable**: the level design, brick properties (value, health, color), paddle & ball properties, and descent speeds are all configurable in a set of TOML files. Now with music and some cool sounds!

## Complete features
* Collision system
* Game state transitions
* Scoring system
* Settings Menu
* Data loaded from easy to read TOML files and can be modified
* Using ECS system to construct game and G/UI entities

## Keymap
* Shoot the ball with the ``spacebar``.
* Move the paddle with ``A`` and ``D``.
* Turn music on/off with ``M``.
* Terminate the game with ``Escape``.

## Next features
Kinda done with this for now (12/2025), but when I come back to it:

* [[maybe]] Powerups

## Config files
* ``Ball.toml`` Set ball properties and color.
* ``Player.toml`` Set paddle properties and color.
* ``Bricks.toml`` Set brick score values, health, and colors. This includes the built-in Normal, Strong, and Gold bricks and also Custom_1 and Custom_2 for custom bricks.
* ``Levels.toml`` Used to generate the levels in the game. Used to determine brick size and type per level. Code key for brick types is in the file. Can modify total number of levels (doesn't have to be the default 4).
* ``AssetsManifest.toml`` Used to load game assets: fonts, textures, sounds, and music. Assets can be changed
without compilation (don't change the id, just the path). If you want to add new assets and recompile, there 
is an ``AssetKeys.hpp`` file that can be modified to add new assets for convenience; make sure the id is unique
and the id in the manifest matches the string name in the keys file.

## How to use 
**Windows:** download the latest [release](https://github.com/nantr0nic/breakdown/releases), unzip, and run the executable. That's it!

**Linux:** a .tar.gz file is provided in the latest [release](https://github.com/nantr0nic/breakdown/releases). Extract the file and run the executable. If you are missing dependencies, they are listed below in [Prerequisites](#Prerequisites).

## Project dependencies

The project is built using CMake with a single `CMakeLists.txt` in the root directory. Currently relies on the following libraries:

*   [**SFML 3.0.2**](https://github.com/SFML/SFML): Used for graphics, windowing, and audio.
*   [**EnTT**](https://github.com/skypjack/entt): For the ECS architecture.
*   [**toml++**](https://github.com/marzer/tomlplusplus): For parsing TOML configuration files.

Thank you for making this project possible!

## Prerequisites & Linux Build Instructions

### Prerequisites

While this project automatically downloads SFML, you must install the system dependencies required to build SFML on Linux.

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev libfreetype6-dev libopenal-dev libflac-dev libvorbis-dev
```
**Fedora:**
```bash
sudo dnf install gcc-c++ cmake libX11-devel libXrandr-devel libXcursor-devel libXi-devel libudev-devel mesa-libGL-devel freetype-devel openal-soft-devel flac-devel libvorbis-devel
```
**Arch Linux:**
```bash
sudo pacman -S base-devel cmake git libx11 libxrandr libxcursor libxi mesa freetype2 openal flac libvorbis
```

### Build options
#### Using CMake Presets

CMake presets are provided in CMakePresets.json. I personally prefer using ```ninja``` and ```clang``` so they are configured to use those tools. Either install ninja and clang, or modify the presets to use your preferred tools.

**1. Configure:**
```bash
cmake --preset linux-release
```
**2. Build:**
```bash
cmake --build --preset linux-release
```
**3. Run:**
```bash
cd out/build/linux-release/
./breakdown
```

#### Standard build

This works with GCC and Make (no ninja/clang required):

**1. Configure:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
```
**2. Build:**
```bash
cmake --build build
```
**3. Run:**
```bash
cd build/
./breakdown
```
