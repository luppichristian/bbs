# Toolchain

`toolchain.bbs` is the generated cache of discovered tools, SDKs, environments, and platform support.

This file is how `bbs` remembers what it found on your machine.

## Location

`toolchain.bbs` lives next to the `bbs` executable.

## Why The File Exists

Tool discovery can involve:

- scanning `PATH`
- checking known install directories
- reading environment variables
- probing SDK roots
- checking WSL and Docker capabilities

Instead of doing a full discovery every time, `bbs` writes the results into `toolchain.bbs` and reuses them.

## When It Is Generated

`bbs` creates or refreshes `toolchain.bbs` when:

- the file does not exist
- you run `bbs update --init-toolchain`

Normal build and run commands can also trigger initialization when the cache is missing.
Custom discovery declarations from `project.bbs`, the global `config.bbs`, and the local `config.bbs` can also trigger targeted refreshes when a declared tool or SDK is not present in the current cache.

## Generation Flow

High-level flow:

1. detect host OS and arch
2. discover host tools
3. discover host SDKs
4. snapshot the host environment
5. discover extra environments such as WSL
6. probe Docker Buildx support when Docker is available
7. derive aggregate platform support
8. write the result as `toolchain.bbs`

If an old cache exists, `bbs` preserves previously known environments that were not rediscovered in the current run.

## Tools That Can Be Discovered

Current tool discovery includes:

- `cmake`
- `ctest`
- `docker`
- `bash`
- `git`
- `wsl`
- `vcvarsall`

## SDKs That Can Be Discovered

Current SDK discovery includes:

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

- discovery means “found on your machine”
- `bbs` does not install these tools for you

## Custom Discovery Declarations

`project.bbs`, the global `config.bbs`, and the local `config.bbs` can add custom discovery strategies with:

- `find_tool(...)`
- `find_sdk(...)`

These declarations are read during toolchain initialization.
They use the same discovery pipeline as the built-in strategies:

- system lookup
- hint directories
- deep root scans
- version probing

Behavior:

1. load `toolchain.bbs`
2. scan config files for custom `find_tool(...)` and `find_sdk(...)` declarations
3. if a declared tool or SDK is missing from the cache, run the matching discovery strategy
4. update `toolchain.bbs` with the discovered result

Example custom tool declaration:

```txt
find_tool(
  id(ninja)
  exe_name(ninja)
  dir_hints("C:/tools/ninja")
  version_arg("--version")
  version_regex("([0-9]+\.[0-9]+(\.[0-9]+)?)")
)
```

Example custom SDK declaration:

```txt
find_sdk(
  id(my_sdk)
  env_vars(MY_SDK_ROOT)
  root_hints("C:/sdk/my_sdk")
  include_rel(include)
  lib_rel(lib)
  bin_rel(bin)
)
```

## How Discovery Works

### Tools

For tools, `bbs` searches in roughly this order:

1. system lookup such as `PATH` / `where` / `which`
2. known hint directories
3. deeper directory scans under known roots

It then tries to probe a version from the executable output or the path.

### SDKs

For SDKs, `bbs` searches in roughly this order:

1. environment variables
2. known root hints
3. version files and version-like path matches

## File Shape

Generated files use `environments(...)` with one or more `environment(...)` entries.

Each environment can contain:

- `id`
- `provider`
- `name`
- `host(...)`
- `probes(...)`
- `tools(...)`
- `sdks(...)`

Typical example:

```txt
environments(
  environment(
    id("host:current:windows:x86_64")
    provider("host")
    name("current")
    host(
      arch(x86_64)
      os(windows)
    )
    tools(
      tool(
        id("cmake")
        path("C:/Program Files/CMake/bin/cmake.exe")
        version("3.31.4")
      )
    )
  )
)
```

## Environments

An environment is a discovered execution context.

Examples:

- your current host machine
- a WSL distro such as Ubuntu

Each environment carries:

- its own OS and architecture
- discovered tools
- discovered SDKs
- support information for buildable target platforms

## Support Matrix

The toolchain file is also the basis for the platform support matrix used by `bbs`.

That matrix answers questions like:

- can this machine build `windows-x86_64`?
- can it build `linux-arm64`?
- did that support come from `msvc`, `xcode`, `wsl`, or `docker-buildx`?

Inspect it with:

```bat
bbs info toolchain
```

## Editing The File

Manual edits are allowed.

This is useful when:

- a tool exists but discovery missed it
- you want to override a tool path
- you want to add a custom SDK root
- you need a quick local fix without changing discovery code

Typical edits:

- add or fix a `tool(...)` entry
- add or fix an `sdk(...)` entry
- add or adjust an `environment(...)` entry

Example tool entry:

```txt
tool(
  id("bash")
  path("C:/Program Files/Git/bin/bash.exe")
  version("5.2.37")
)
```

Example SDK entry:

```txt
sdk(
  name("emsdk")
  base_path("C:/emsdk")
  inc_path("C:/emsdk/upstream/emscripten/system/include")
  bin_path("C:/emsdk/upstream/emscripten")
)
```

## Important Warning About Manual Edits

Manual edits are not permanent if you regenerate the file.

Running:

```bat
bbs update --init-toolchain
```

can replace or rewrite parts of `toolchain.bbs` based on fresh discovery.

Use this rule of thumb:

- machine changed: regenerate
- discovery is incomplete and you need a quick fix: edit manually

## Useful Inspection Commands

```bat
bbs info toolchain
bbs info toolchain --filter=cmake
bbs info toolchain --values-only
```

## Related Reads

- [8_CROSSBUILD.md](./8_CROSSBUILD.md)
- [2_COMMAND_GUIDE.md](./2_COMMAND_GUIDE.md)
