# bbs (Better Build System)

`bbs` is a build frontend for C and C++ projects.

It uses a small `.bbs` configuration format, discovers your local toolchain, generates the CMake backend files for you, and gives you one CLI for build, run, test, packaging, and distribution.

## Why Add Another Layer On Top Of CMake

CMake is widely used, but it still leaves a lot of multi-platform build setup in the hands of the project author.
You still need to model targets, platform differences, toolchain choices, and common workflows in a way that stays manageable across environments.

`bbs` exists to provide a smaller, more direct project layer above CMake so that the same project definition can drive builds more consistently across platforms.

## Why Build On Top Of CMake Instead Of Replacing It

Most third-party C and C++ libraries already support CMake.
Building on top of it means `bbs` can stay compatible with the tooling and dependency ecosystem people already use.

If `bbs` replaced CMake entirely, it would be much less practical because it would lose easy access to that existing ecosystem.

## What This Repository Is

This repository contains:

- the `bbs` executable itself
- the `.bbs` file parser and project model
- toolchain discovery logic
- example projects
- end-user documentation

`bbs` is not a replacement for compilers, SDKs, or CMake itself.
It sits above them and makes common project setup and daily use simpler.

## Why Use bbs Instead Of Writing Plain CMake

If you already like CMake as a backend but do not want to hand-author a full CMake project for every small or medium project, `bbs` reduces the amount of work you need to do.

What `bbs` gives you:

- a smaller project format for common C/C++ layouts
- target definitions focused on source files, dependencies, flags, and package sources
- optional unity build batching, including explicit directory-based batches
- automatic generation of the CMake backend files needed to build
- automatic toolchain discovery and caching in `toolchain.bbs`
- simple platform selection with ids like `windows-x86_64` and `linux-arm64`
- file watching with automatic rebuilds through `bbs auto`
- a distribution flow with staging and optional archive creation
- shared user defaults in `user.bbs` and local per-project overrides in `local.bbs`
- helper generators such as `.gitignore` and GitHub workflows

In short:

- plain CMake: you model the whole build system yourself
- `bbs`: you declare the project, `bbs` writes the CMake layer for you

## Quick Start

Minimal `project.bbs`:

```txt
id(my_app)
name("My App")
ver(0.1.0)

targets(
  console(
    units(
      src/main.c
    )
  )
)
```

Build:

```bat
bbs build
```

Build and run:

```bat
bbs run
```

`bbs` initializes the backend automatically when needed.
Use `bbs update --init-toolchain` when you explicitly want to regenerate the cached toolchain state.

## Build This Repository

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

Notes:

- the scripts try to use a static runtime where the toolchain/platform supports it
- on macOS, fully static system runtime linking is not supported, so the shell scripts fall back to a normal dynamic system runtime build with a warning

Alternative with the top-level `CMakeLists.txt`:

```bat
cmake -S . -B build-cmake
cmake --build build-cmake
```

## First Project Setup

1. Build or download `bbs`.
2. Put `bbs.exe` in a stable folder.
3. Add that folder to `PATH`.
4. Install `cmake`.
5. Create a project folder with `project.bbs`.
6. Run `bbs build` or `bbs run`.

Common first commands:

```bat
bbs build
bbs run
bbs test
bbs dist
```

## Core Concepts

- `project.bbs`: shared project definition in the project root
- `user.bbs`: user-wide defaults next to the `bbs` executable
- `local.bbs`: machine-local overrides in one project
- `toolchain.bbs`: generated cache of discovered tools, SDKs, and environments
- `build/`: generated backend files and build outputs by default
- `dist/`: staged distribution outputs by default

For shared default-style settings, precedence is:

1. built-in defaults
2. `user.bbs`
3. `local.bbs`
4. project and target values in `project.bbs`

## Main Commands

- `bbs build`: build selected targets
- `bbs run`: build if needed, then run a runnable target
- `bbs test`: run project tests
- `bbs dist`: stage outputs for distribution and optionally archive them
- `bbs auto`: watch files and rebuild automatically
- `bbs update`: regenerate derived backend files explicitly
- `bbs package`: inspect or refresh package-backed targets
- `bbs info`: inspect parsed config data
- `bbs gen`: generate helper files

Run `bbs help` for the full reference.

## Documentation

The numbered docs are meant to be read in roughly this order:

- [docs/0_INSTALL.md](./docs/0_INSTALL.md)
- [docs/1_BUILDING.md](./docs/1_BUILDING.md)
- [docs/2_COMMAND_GUIDE.md](./docs/2_COMMAND_GUIDE.md)
- [docs/3_FILE_FORMAT.md](./docs/3_FILE_FORMAT.md)
- [docs/4_EXAMPLES.md](./docs/4_EXAMPLES.md)
- [docs/5_USER_CONFIG.md](./docs/5_USER_CONFIG.md)
- [docs/6_PROJECT_CONFIG.md](./docs/6_PROJECT_CONFIG.md)
- [docs/7_CROSSBUILD.md](./docs/7_CROSSBUILD.md)
- [docs/8_TOOLCHAIN.md](./docs/8_TOOLCHAIN.md)
- [docs/9_EXPANSION_TOKENS.md](./docs/9_EXPANSION_TOKENS.md)
- [docs/10_PACKAGES.md](./docs/10_PACKAGES.md)

## Examples

- [docs/4_EXAMPLES.md](./docs/4_EXAMPLES.md)
- [`examples/static_lib/`](./examples/static_lib/)
- [`examples/raylib_example/`](./examples/raylib_example/)
- [`examples/bbs_package_consumer/`](./examples/bbs_package_consumer/) consuming [`examples/bbs_package_dep/`](./examples/bbs_package_dep/) as a nested `project.bbs` package

## Repository Layout

- `src/`: implementation
- `docs/`: end-user documentation
- `examples/`: sample `bbs` projects
- `build_clang.bat`: Windows Clang build
- `build_cl.bat`: Windows MSVC `cl` build
- `build_gcc.bat`: Windows GCC build
- `build_clang.sh`: Unix/macOS Clang build
- `build_gcc.sh`: Unix/macOS GCC build
- `CMakeLists.txt`: simple CMake build for `bbs` itself

## Future Plans

- shader compilation
- custom asset compilation and pipeline support
- metaprogramming features built around the Clang AST

## License

MIT License.

See [LICENSE.md](./LICENSE.md).
