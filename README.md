# MEGASTRUCTURES: Prototypes

This repository contains the code and assets of the prototypes of the game [**MEGASTRUCTURES**](https://klaim.itch.io/megastructures)

Presentation and links

see : https://docs.google.com/presentation/d/1YOBf4iuNcZKgCnOEpWVkoMfEHL0yWTOpm8IMhf159i4/edit?usp=sharing

## Prototype 1 - Overview

For this prototype the goal is to evaluate the C++-based techs/tools stack for the game by building a minimalistic roguelike game with the following elements being used for evaluation:
- Bleeding-edge C++ (beyond C++ 23 where possible)
- [`build2`](https://build2.org) as build-system and dependency manager
- View separate from model (`proto1` is the executable acting as a view, `proto1-model` is the model library)
- Entity Component System setup on the model (using [`entt`](https://github.com/skypjack/entt) as implementation)
- C++ coroutines to implement the action-turn logic (using `std::generator`)

The view is implemented using [`SFML`](https://sfml-dev.org) and [`Dear IMGUI`](https://github.com/ocornut/imgui).

See the presenttion above for more details.

## Prototye 2 - Overview

*NOTE: this prototype was not started yet.*

For this prototype we will consider using an hexagonal representation of space (knowing that the final game will have a 3D grid ).

## Prototype 3 - Overview

*NOTE: this prototype was not started yet.*

For this prototype we will experiment with implementing a special action-turn system based on time granularity with the goal of allowing high-speed combat (including hacking).

## Prototype 4 - Overview

*NOTE: this prototype was not started yet.*

For this prototype we will use Godot as the "view" implementation and a C++ library as the "model". The goal is mainly to evaluate how difficult it makes working on the game. Depending on the resuts, we might decide to go with Godot, or try with Unreal or switch to a C++ graphic rendering library instead.

# Development

As these are prototypes I (the author) will not spend too much time setting up stuffs to help with participating to this project, sorry in advance. If I decide to make the commercial game open-source I will however make all the necessary setup to ease helping the project.

However if you still want to build the project yourself, here are some instructions:

## Requirements:

You need to install:
- a toolchain: Visual Studio C++/MSVC, LLVM/clang or gcc depending on your platform and choice:
    - **clang must be v15.x** (or higher if you modify the build configs);
    - **msvc must be the last available** version from Visual Studio 2022 (or preview)
    - **gcc must be 11.2 or higher**
- **on Windows** I recommend using `Git-Bash` (which is installed with `git`) to run the commands and scripts below;
- [`build2`](https://build2.org) as buildsystem and dependency manager.
    If you are not familiar with `build2` I recommend to read at least the [toolchain introduction](https://build2.org/build2-toolchain/doc/build2-toolchain-intro.xhtml), however you don't need to read it to proceed with the following instructions - you might need to read it if you want to submit a PR.
    - we might be using the stage (aka not finished/bleeding-edge) version of `build2` for which you can find the install script there: https://stage.build2.org/0/

Then clone this repository and go with a console in the root of the clone. Then decide which way to go below.

## I Just want to build and run the prototypes:

Once `build2` is installed, if you just want to build and be able to run the projects:
```bash
bdep init -C build-clang-release @clang-release cc config.config.load=build2-configs/clang-release.config config.install.root=./install/
b install
```
If nothing goes wrong you will find the executable in `./install/`.
These commands gather the dependencies, build the projects and install the whole thing into that directory.

*Note that due to a missing filtering feature in `build2`, all the headers from the libraries used by the executables will also be installed this way. Once the filtering feature is made available we will just install the executables and it's dependencies, not it's dependencies headers.*

## I want to develop:

1. To create the build configurations for development and initialize all the projects in them:
    ```bash
    ./init-configs.sh <toolchain>
    ```
    Replace `<toolchain>` by `msvc`, `clang` or `gcc` (see the `build2-configs/`directory for a list of possible configs).
    For example:
    ```bash
    ./init-configs.sh clang
    ./init-configs.sh msvc
    ```
    This needs to be done only once per toolchain.

    If you want to erase all current configurations and restart from scratch, add `--clean`. For example:
    ```bash
    ./init-configs.sh clang --clean
    ```
    
    This script create the configurations for the toolchain and initialize all the projects in it.
    The first toolchain's debug configuration will be set as the default when using `build2` without specifying a configuration.

    *NOTE: for simplicity, we only initialize default toolchains in this script, if you want to setup a more specific way or use a specific version of a toolchain, feel free to tweak the script or use `build2` commands to achieve your goal.*

2. To build the projects:
    ```bash
    bdep update
    ```
    This will only build in the default configuration (debug).
    To build in all the initialized configurations, add `--all` or `-a`. To build in specific configurations, use their alias as `@<toolchain>-debug` or `@<toolchain>-release` after replaceing `<toolchain>` by the toolchain name. See the `build2-configs/` directory for all the possibilities.
    ```bash
    bdep update @msvc-debug @clang-debug  # Build in debug configurations using msvc and clang
    bdep update -a # Build in all configurations initialized
    ```
    
3. To build and run the tests:
    ```bash
    bdep test -a # build and run all the tests of all the configurations
    ```

4. To build and install (in `./install/`) the projects:
    - Using the default build:
        ```bash
        b install
        ```
    - Using a specific configuration:
        ```bash
        b install: build-<toolchain>-<buildmode>
        ```
        For example:
        ```bash
        b install: build-clang-release
        ```

Read the [`build2`](https://build2.org) documentation for details.