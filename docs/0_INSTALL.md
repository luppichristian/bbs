# Install

This guide explains how to get `bbs` working on your machine and what external tools it expects to find.

## What bbs Installs

`bbs` is a single executable.

It does not install compilers, SDKs, or CMake for you.
Instead, it discovers tools already available on your machine and stores the result in `toolchain.bbs`.

That means installation has two parts:

1. place `bbs` somewhere stable
2. make sure the external tools you want to use are installed

## Minimum Requirements

Required:

- `bbs.exe`
- `cmake`

Strongly recommended:

- `ctest`
- `bash`
- `git`

Platform-specific toolchains depend on what you want to build.

Examples:

- Windows native builds: MSVC + Windows SDK
- macOS native builds: Xcode or Command Line Tools
- Linux native builds: host compiler toolchain with CMake available
- Windows-to-Linux workflows: WSL and/or Docker Buildx

## Build Or Download bbs

### Build From Source

Windows build scripts:

```bat
build_clang.bat
build_cl.bat
build_gcc.bat
```

Unix and macOS build scripts:

```sh
./build_clang.sh
./build_gcc.sh
```

These build `build/bbs.exe` on Windows or `build/bbs` on Unix-like systems.

Static runtime notes:

- `build_clang.bat` uses Clang with the static MSVC runtime
- `build_cl.bat` uses MSVC `cl` with `/MT`
- `build_gcc.bat` uses GCC with static runtime flags
- `build_clang.sh` and `build_gcc.sh` use `-static` on Linux
- on macOS, fully static system runtime linking is not supported, so the shell scripts fall back to a normal system runtime build and print a warning

Alternative:

```bat
cmake -S . -B build-cmake
cmake --build build-cmake
```

### Install The Executable

Move `bbs.exe` to a stable location, for example:

```txt
C:\tools\bbs\bbs.exe
```

`bbs` stores `user.bbs` and `toolchain.bbs` next to the executable, so pick a location you control.

## Add bbs To PATH

On Windows:

1. put `bbs.exe` in a folder such as `C:\tools\bbs`
2. add that folder to your user `PATH`
3. open a new terminal
4. verify:

```bat
bbs help
```

## Install CMake

`cmake` is the one hard requirement for normal `bbs` operation.
If `cmake` is missing, toolchain generation fails.

Common Windows install options:

- official installer
- Scoop: `scoop install cmake`
- Chocolatey: `choco install cmake`

Verify:

```bat
cmake --version
ctest --version
```

## Install Bash If You Want Script Hooks

`bbs` runs build, run, and dist hooks through a Bash-compatible shell.

You should install one if you plan to use:

- `pre_build_cmds`
- `post_build_cmds`
- `pre_run_cmds`
- `post_run_cmds`
- `pre_dist_cmds`
- `post_dist_cmds`
- `dist(...)` `precommand(...)` and `postcommand(...)`

Common Windows choice:

- Git for Windows

Verify:

```bat
bash --version
```

## Tools bbs Discovers

The current tool discovery list includes:

- `cmake`
- `ctest`
- `docker`
- `bash`
- `git`
- `wsl` on Windows
- `vcvarsall.bat` on Windows

The current SDK discovery list includes:

- `windows_sdk`
- `ucrt_sdk`
- `msvc`
- `xcode`
- `vulkan_sdk`
- `android_ndk`
- `emsdk`
- `musl`
- `glibc`

Important:

- `bbs` discovers these tools
- `bbs` does not download them
- `toolchain.bbs` is only a cache of what was found

## First Run In A Project

Create a `project.bbs` file in your project root.

Minimal example:

```txt
id(my_app)

targets(
  console(
    units(
      src/main.c
    )
  )
)
```

Then you can immediately do:

```bat
bbs build
```

Or:

```bat
bbs run
```

Those commands initialize the backend automatically if needed.

## When To Run `bbs update --init-toolchain`

This command is useful when:

- you just installed or removed build tools
- you changed SDKs or shell tools
- you want to refresh `toolchain.bbs` explicitly
- you want to inspect the discovered toolchain before building

Example:

```bat
bbs update --init-toolchain
```

That will:

- regenerate `toolchain.bbs`
- reload `user.bbs` and `local.bbs`
- regenerate the derived backend files under the build directory

## Where Files Live

- `project.bbs`: project root
- `local.bbs`: project root
- `user.bbs`: next to `bbs.exe`
- `toolchain.bbs`: next to `bbs.exe`

## Next Reads

- [1_BUILDING.md](./1_BUILDING.md)
- [2_COMMAND_GUIDE.md](./2_COMMAND_GUIDE.md)
- [3_FILE_FORMAT.md](./3_FILE_FORMAT.md)
- [6_PROJECT_CONFIG.md](./6_PROJECT_CONFIG.md)
