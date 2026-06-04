# Crossbuild

This guide explains how `bbs` thinks about platforms, where platform support comes from, and what cross-build behavior to expect.

## Platform Model

`bbs` uses platform ids in the form:

```txt
<os>-<arch>
```

Examples:

- `windows-x86_64`
- `windows-x86`
- `windows-arm64`
- `linux-x86_64`
- `linux-arm64`
- `macos-x86_64`
- `macos-arm64`

Supported OS names:

- `windows`
- `linux`
- `macos`

Supported arch names:

- `x86_64`
- `x86`
- `arm64`

## Where Platform Support Comes From

During toolchain initialization, `bbs` builds a support matrix.

That matrix is not hard-coded from your operating system alone.
It depends on what tools and environments were discovered.

Typical support sources are:

- `msvc` for Windows targets on Windows
- `xcode` for macOS targets on macOS
- `host-toolchain` for host-native builds where CMake and the native toolchain are available
- `wsl:<distro>` for Linux builds reachable through WSL
- `docker-buildx` for Linux platform support provided by Docker Buildx

You can inspect the result with:

```bat
bbs info toolchain
```

## What Happens When You Pick `-p`

When you pass `-p`, `bbs`:

1. parses the platform id
2. checks whether that platform is supported by the current toolchain
3. generates or uses backend files for that platform
4. builds using the selected config and target set

If the platform is not supported, the command fails early.

## Build Selection Examples

```bat
bbs build -p windows-x86_64
bbs build -p linux-arm64
bbs build -p *
```

`-p *` means “all supported matching platforms” for commands that support matrix execution.

## Cross-Build vs Run

Building and running are different.

You may be able to build for a platform that is not the host platform.
That does not mean you can run the produced binary locally.

Example:

- on Windows, `bbs build -p linux-x86_64` may be valid through WSL or Docker support
- `bbs run -p linux-x86_64` is not valid on Windows because the output is not host-native

Important rule:

- `bbs run` only supports host-native runnable outputs
- `bbs run -p *` expands only to host-native runnable outputs

## Generated Backend Files

`bbs` writes derived backend files under the build root, including:

- `CMakeLists.txt`
- `bbs-toolchain.cmake`
- `CMakePresets.json`

Those files carry the resolved platform-specific values used by the generated backend.

## Typical Workflows

### Native build

```bat
bbs build
```

### Explicit host build

```bat
bbs build -p windows-x86_64
```

### Build every supported platform for one target

```bat
bbs build -t app -p *
```

### Build and package one cross target

```bat
bbs build -p linux-x86_64
bbs dist -p linux-x86_64
```

## When To Regenerate Toolchain Support

Run:

```bat
bbs update --init-toolchain
```

when:

- you installed a new compiler or SDK
- you enabled WSL or Docker Buildx
- you changed shell tools or system paths
- the cached support matrix is stale

## Related Reads

- [9_TOOLCHAIN.md](./9_TOOLCHAIN.md)
- [2_COMMAND_GUIDE.md](./2_COMMAND_GUIDE.md)
