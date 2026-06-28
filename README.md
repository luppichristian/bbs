# bbs (my Better Build System)

`bbs` is a build frontend for C, C++, and CUDA projects.

It uses a small `.bbs` configuration format, discovers your local toolchain, generates the CMake backend files for you, and gives you one CLI for build, run, test, packaging, and distribution.

It also supports project-local metacompilation through `builders`: small dynamically loaded modules that can modify targets, flags, and command behavior before and after build phases.

Current compiler support: `MSVC`, `Clang`, `GCC`, and `NVCC` for CUDA targets.

Current supported target platforms: `windows`, `linux`, `macos`

Current supported target architectures: `x86_64`, `x86`, `arm64`

For escape-hatch flags, `bbs` treats `additional_compile_args` as Clang/GCC-style input and translates supported subsets to MSVC when generating the backend. Use dedicated fields such as `undefines(...)` when they exist instead of spelling those through raw compile args.

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
- the public builder/reference API in `pub/`
- example projects
- end-user documentation

`bbs` is not a replacement for compilers, SDKs, or CMake itself.
It sits above them and makes common project setup and daily use simpler.

## Why Use bbs Instead Of Writing Plain CMake

If you already like CMake as a backend but do not want to hand-author a full CMake project for every small or medium project, `bbs` reduces the amount of work you need to do.

What `bbs` gives you:

- a smaller project format for common C/C++/CUDA layouts
- target definitions focused on source files, dependencies, flags, and package sources
- optional unity build batching, including explicit directory-based batches
- automatic generation of the CMake backend files needed to build
- automatic toolchain discovery and caching in `toolchain.bbs`
- simple platform selection with ids like `windows-x86_64` and `linux-arm64`
- file watching with automatic rebuilds through `bbs auto`
- a distribution flow with staging and optional archive creation
- global config in `config.bbs` next to `bbs`, with local config overrides in `config.bbs`
- helper generators such as `.gitignore`, VSCode workspace files, and GitHub workflows
- metacompilation through `builders(...)`, so projects can inject dynamic prebuild/post-command logic without forking `bbs`

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
bbs run -a sample.txt
```

Minimal CUDA `project.bbs`:

```txt
id(cuda_app)
name("CUDA App")
ver(0.1.0)
cuda_architectures(75 89)

targets(
  console(
    output(cuda_app)
    lang(cuda)
    units(
      src/main.c
    )
  )
)
```

Notes:

- `lang(cuda)` enables CMake CUDA generation
- `cuda_architectures(...)` maps to CMake `CUDA_ARCHITECTURES` and can be set per project or per target
- `units(src/main.c)` works even when the source file is not named `.cu`
- run `bbs update --init-toolchain` if you just installed CUDA and want to refresh discovery

See `examples/cuda_console/` and [docs/5_EXAMPLES.md](./docs/5_EXAMPLES.md) for a working sample.

## Project Layout Overview

A typical `bbs` project directory looks like this:

```txt
my_project/
  project.bbs
  src/
  include/
  assets/
  config.bbs
  build/
  dist/
```

What these mean:

- `project.bbs`: main project definition
- `src/`, `include/`, `assets/`: project-owned files and folders
- `config.bbs`: optional machine-local override file for this project
- `build/`: generated backend and build output directory
- `dist/`: generated distribution staging and archive output directory

Related files outside the project root:

- global `config.bbs`: defaults next to the `bbs` executable
- `toolchain.bbs`: generated toolchain cache next to the `bbs` executable
- `packages/`: shared fetched package cache next to the `bbs` executable

The configured assets directory is created automatically and can be copied into dist payloads with `dist(copy_assets(true))`.

For the full explanation, see [docs/3_PROJECT_FOLDER.md](./docs/3_PROJECT_FOLDER.md).

With builders:

```txt
id(my_app)
name("My App")
ver(0.1.0)

targets(
  console(
    id(my_app)
    units(
      src/main.c
    )
    dependencies(
      preprocessor
    )
  )
)

builders(
  id(preprocessor)
  units(
    src/builder.c
  )
)
```

This lets a project compile and load a small builder module that can, for example, inject compile flags dynamically before a target is built.

Minimal builder source:

```c
#include <bbs/build.h>

bool bbs_callback(bbs_sig signal, bbs_ctx* ctx, bbs_proj* prj, bbs_tgt* tgt) {
  (void)ctx;

  if (signal != BBS_SIG_PRE_BUILD || !prj || !tgt)
    return true;

  if (!bbs_target_has_dependency(tgt, "preprocessor"))
    return true;

  if (!bbs_target_set_text(tgt, BBS_TARGET_TEXT_DEFINES, "PREPROCESSOR_ACTIVE"))
    return false;

  return true;
}
```

See [docs/12_BUILDERS.md](./docs/12_BUILDERS.md) for the full builders guide.

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
5. Install the language toolchain you want to use, such as a C/C++ compiler or the CUDA toolkit.
6. Create a project folder with `project.bbs`.
7. Run `bbs build` or `bbs run`.

Common first commands:

```bat
bbs build
bbs run
bbs test
bbs dist
```

## Core Concepts

- `project.bbs`: shared project definition in the project root
- global `config.bbs`: defaults next to the `bbs` executable
- local `config.bbs`: machine-local override config in one project
- `toolchain.bbs`: generated cache of discovered tools, SDKs, and environments
- `build/`: generated backend files and build outputs by default
- `dist/`: staged distribution outputs by default
- `builders(...)`: optional project-local metacompilation modules compiled and loaded by `bbs`

For default-style settings, precedence is:

1. built-in defaults
2. global `config.bbs`
3. local `config.bbs`
4. project and target values in `project.bbs`

## Main Commands

- `bbs build`: build selected targets
- `bbs run`: build if needed, then run a runnable target; use `-a` or `--args` to forward program arguments
- `bbs test`: run project tests
- `bbs dist`: stage outputs for distribution and optionally archive them
- `bbs auto`: watch files and rebuild automatically
- `bbs update`: regenerate derived backend files explicitly
- `bbs package`: inspect or refresh package-backed targets
- `bbs info`: inspect parsed config data
- `bbs gen`: generate helper files

Config command model:

- `config`: merged global + local view
- `global`: `config.bbs` next to `bbs`
- `local`: `config.bbs` in the project root

Run `bbs help` for the full reference.

## Documentation

The numbered docs are meant to be read in roughly this order:

- [docs/0_INSTALL.md](./docs/0_INSTALL.md)
- [docs/1_BUILDING.md](./docs/1_BUILDING.md)
- [docs/2_COMMAND_GUIDE.md](./docs/2_COMMAND_GUIDE.md)
- [docs/3_PROJECT_FOLDER.md](./docs/3_PROJECT_FOLDER.md)
- [docs/4_FILE_FORMAT.md](./docs/4_FILE_FORMAT.md)
- [docs/5_EXAMPLES.md](./docs/5_EXAMPLES.md)
- [docs/6_CONFIG.md](./docs/6_CONFIG.md)
- [docs/7_PROJECT_CONFIG.md](./docs/7_PROJECT_CONFIG.md)
- [docs/8_CROSSBUILD.md](./docs/8_CROSSBUILD.md)
- [docs/9_TOOLCHAIN.md](./docs/9_TOOLCHAIN.md)
- [docs/10_EXPANSION_TOKENS.md](./docs/10_EXPANSION_TOKENS.md)
- [docs/11_PACKAGES.md](./docs/11_PACKAGES.md)
- [docs/12_BUILDERS.md](./docs/12_BUILDERS.md)

## Examples

- [docs/5_EXAMPLES.md](./docs/5_EXAMPLES.md)
- [`examples/static_lib/`](./examples/static_lib/)
- [`examples/raylib_example/`](./examples/raylib_example/)
- [`examples/bbs_package_consumer/`](./examples/bbs_package_consumer/) consuming [`examples/bbs_package_dep/`](./examples/bbs_package_dep/) as a nested `project.bbs` package
- [`examples/builders/`](./examples/builders/) showing dynamic metacompilation through a builder dependency
- [`examples/builder_target_properties/`](./examples/builder_target_properties/) showing how a builder can rewrite target properties before compilation

## Repository Layout

- `src/`: implementation
- `pub/`: public headers for builders and external consumers
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
- deeper metaprogramming features built around the Clang AST

## AI Skills

This repository includes project-local skills for both Codex and Claude so they can work with `bbs` without defaulting to generic CMake assumptions.

- Codex skill: [`.codex/skills/bbs-build-system/`](./.codex/skills/bbs-build-system/)
- Claude skill: [`.claude/skills/bbs-build-system/`](./.claude/skills/bbs-build-system/)

These skills teach the assistants how to:

- write and edit valid `project.bbs` files
- choose supported target types and fields
- emit the right `bbs` commands
- model package-backed targets and `builders(...)`
- respect this repository's `bbs` source layout when editing the tool itself

## License

MIT License.

See [LICENSE.md](./LICENSE.md).
