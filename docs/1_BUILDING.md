# Building

This guide explains the different ways to build the `bbs` executable itself.

There are two main approaches:

- manual compiler scripts
- the top-level `CMakeLists.txt`

Both build the same program from `src/bbs.c`.

## Which Build Path Should You Use

Use the manual scripts when:

- you want the shortest local build path
- you already know which compiler you want to use
- you want an explicit compiler-specific command line
- you want the repository's static-runtime defaults

Use the top-level `CMakeLists.txt` when:

- you want a standard CMake workflow
- you want to integrate this repository into another CMake-oriented environment
- you want IDE generator support through CMake

## Manual Build Scripts

## Windows

### `build_clang.bat`

Builds with Clang:

```bat
build_clang.bat
```

Current behavior:

- compiles `src\bbs.c`
- outputs `build\bbs.exe`
- uses static MSVC runtime through `-fms-runtime-lib=static`

### `build_cl.bat`

Builds with MSVC `cl`:

```bat
build_cl.bat
```

Current behavior:

- compiles `src\bbs.c`
- outputs `build\bbs.exe`
- uses static MSVC runtime through `/MT`

### `build_gcc.bat`

Builds with GCC:

```bat
build_gcc.bat
```

Current behavior:

- compiles `src\bbs.c`
- outputs `build\bbs.exe`
- uses static runtime flags such as `-static` and `-static-libgcc`

## Unix And macOS

### `build_clang.sh`

```sh
./build_clang.sh
```

Current behavior:

- compiles `src/bbs.c`
- outputs `build/bbs`
- uses `-static` on Linux
- warns and falls back to a normal system-runtime build on macOS

### `build_gcc.sh`

```sh
./build_gcc.sh
```

Current behavior:

- compiles `src/bbs.c`
- outputs `build/bbs`
- uses `-static -static-libgcc` on Linux
- warns and falls back to a normal system-runtime build on macOS

## Why The macOS Scripts Behave Differently

On macOS, fully static linking of the system runtime is not supported the way it commonly is on Linux or Windows static-runtime builds.

Because of that:

- the shell scripts still build on macOS
- they print a warning
- they do not force a fully static system runtime build there

## CMake Build

The repository also ships a simple top-level `CMakeLists.txt` for building `bbs` itself.

Configure and build:

```bat
cmake -S . -B build-cmake
cmake --build build-cmake
```

What this CMake build currently does:

- defines a single executable target: `bbs`
- builds from `src/bbs.c`
- writes runtime outputs to the build directory root
- enables static MSVC runtime on MSVC builds through `MSVC_RUNTIME_LIBRARY`

## Manual Scripts Vs CMake

### Manual scripts

Pros:

- minimal
- fast to run
- compiler-specific and explicit
- easy to inspect

Cons:

- one script per compiler family
- fewer generator/IDE conveniences
- less flexible for larger host-build customization

### CMake

Pros:

- standard workflow
- IDE and generator integration
- easier to extend if repository self-build grows

Cons:

- more moving parts than a one-line compiler script
- slower startup than direct compiler invocation

## Recommended Choice

For day-to-day work in this repository:

- on Windows, prefer `build_clang.bat`
- on Linux, prefer `./build_clang.sh` or `./build_gcc.sh`
- use the CMake build when you specifically want a CMake workflow

## Output Paths

Manual scripts:

- Windows: `build/bbs.exe`
- Unix/macOS: `build/bbs`

CMake build:

- `build-cmake/bbs.exe` on Windows
- `build-cmake/bbs` on Unix-like systems

depending on generator and platform conventions.

## Related Reads

- [0_INSTALL.md](./0_INSTALL.md)
- [9_TOOLCHAIN.md](./9_TOOLCHAIN.md)
