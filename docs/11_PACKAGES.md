# Packages

This guide explains package-backed targets in `bbs`.

In `bbs`, a package is still a target in `project.bbs`, but its implementation comes from an external source instead of local `units(...)`.

## What A Package Target Is

A package target is a target that uses one of these source modes:

- local path
- git repository
- archive download

Once resolved, `bbs` expects the package to provide either:

- a native `CMakeLists.txt`, or
- a `project.bbs` file that `bbs` can translate into an embedded CMake backend

In both cases, `bbs` links the package into the generated backend with the configured `cmake.target`.

## Why Packages Exist

Package-backed targets are useful when:

- you want to consume a third-party library without writing custom fetch logic
- you want that dependency to behave like a normal target dependency inside `bbs`
- you want one consistent workflow for local and external dependencies

## Package Source Modes

Only one package source mode should be active for a target.

The three supported modes are:

- `path`
- `repo(...)`
- `archive(...)`

If more than one is declared for the same target, the project is invalid.

## 1. Path Package

Use a local directory as the package source.

Example:

```txt
static_lib(
  id(my_dep)
  path("../third_party/my_dep")
  cmake(
    target(my_dep)
  )
)
```

Optional field:

- `subdir`

Example:

```txt
static_lib(
  id(my_dep)
  path("../third_party/my_dep")
  subdir("src")
  cmake(
    target(my_dep)
  )
)
```

Behavior:

- `bbs` does not fetch anything
- the directory must exist locally
- the resolved directory must contain either `CMakeLists.txt` or `project.bbs`

## 2. Repo Package

Use a Git repository as the package source.

Example using a tag:

```txt
static_lib(
  id(raylib_pkg)
  repo(
    link("https://github.com/raysan5/raylib.git")
    tag("5.5")
  )
  cmake(
    target(raylib)
  )
)
```

Supported fields inside `repo(...)`:

- `link`
- `tag`
- `commit`

Important rules:

- `link` is required for repo-backed packages
- `tag` and `commit` are optional
- you cannot declare both `tag` and `commit` on the same package

Behavior:

- the package is cloned into the shared package cache
- if `tag` is set, `bbs` clones that tag
- if `commit` is set, `bbs` clones and checks out that exact commit
- if neither is set, `bbs` uses the repository head

## 3. Archive Package

Use a downloadable archive as the package source.

Example:

```txt
static_lib(
  id(my_dep)
  archive(
    link("https://example.com/my_dep.zip")
    strip_prefix("my_dep-1.0.0")
  )
  cmake(
    target(my_dep)
  )
)
```

Supported fields inside `archive(...)`:

- `link`
- `strip_prefix`

Behavior:

- the archive is downloaded into the shared package cache
- `bbs` extracts it
- `strip_prefix` lets you point at the real package root inside the extracted archive tree
- the resolved directory must contain either `CMakeLists.txt` or `project.bbs`

## Backend Detection

`bbs` checks the resolved package directory in this order:

- `CMakeLists.txt`
- `project.bbs`

If both files are present, `bbs` uses the native `CMakeLists.txt`.

If only `project.bbs` is present, `bbs` loads that package as its own `bbs` project, generates an internal CMakeLists for it, and includes that generated backend in the parent build.

Nested `project.bbs` packages use their own default package config. They do not inherit the parent project's selected `bbs` config name.

## `cmake(...)`

`cmake.target` is the expected target name exported by the package backend.

Example:

```txt
cmake(
  target(raylib)
)
```

Optional children:

- `target(...)`
- `args(...)`
- `option(...)`

`args(...)` lets you predefine CMake variables such as `-DNAME=VALUE` or `NAME=VALUE` before the package subdirectory is added.

`option(...)` is a boolean-focused shorthand. Each entry may be `NAME` or `NAME=ON/OFF`.

Why it matters:

- for native CMake packages, `bbs` adds the package directory as a CMake subdirectory
- for `project.bbs` packages, `bbs` generates an internal CMake backend first and then adds that generated directory as a subdirectory
- then it expects that package backend to define the target named by `cmake.target`
- if that target does not exist, the build fails

## Package Cache

Repo and archive packages are resolved into a shared cache directory managed by `bbs`.

The exact cache path is internal, but you can inspect it with:

```bat
bbs package <package_name>
```

That output includes fields such as:

- source
- cache location
- resolved directory
- resolution status

## Package Resolution Status

Depending on the source mode and cache state, package info may show statuses such as:

- `ready`
- `missing`
- `cached`
- `up-to-date`
- `outdated`
- `invalid`

Typical meaning:

- `ready`: path package exists and looks usable
- `missing`: package is not present yet
- `cached`: archive or repo exists in cache
- `up-to-date`: repo package local ref matches remote/tag/commit resolution
- `outdated`: repo package exists but does not match the expected remote ref
- `invalid`: package was found or extracted but does not look usable, usually because both `CMakeLists.txt` and `project.bbs` are missing

## Using Packages From Other Targets

Once declared, use the package target like any other target dependency.

Example:

```txt
targets(
  static_lib(
    id(raylib_pkg)
    repo(
      link("https://github.com/raysan5/raylib.git")
      tag("5.5")
    )
    cmake(
      target(raylib)
    )
  )

  console(
    output(game)
    dependencies(
      raylib_pkg
    )
    units(src/main.c)
  )
)
```

## Package Commands

The package command has three main uses.

## List Packages

```bat
bbs package list
```

Shows all package-backed targets in the current project.

## Inspect One Package

```bat
bbs package raylib_pkg
```

Shows details for one package target.

## Refresh Packages

Refresh one package:

```bat
bbs package refresh raylib_pkg
```

Refresh all packages:

```bat
bbs package refresh
```

You can also force refresh before inspection:

```bat
bbs package raylib_pkg --refresh
bbs package list --refresh
```

## What Refresh Means

Refresh behavior depends on source mode.

- path package: nothing is fetched
- repo package: cached clone may be deleted and re-cloned
- archive package: cached extracted directory may be deleted and re-downloaded

Use refresh when:

- the remote dependency changed
- the cache is broken
- you want a clean re-fetch

## Build Integration

Package resolution usually happens as part of normal project preparation.

That means you can often just run:

```bat
bbs build
```

and let `bbs` resolve packages automatically.

Use `bbs package ...` when you want to inspect or refresh packages explicitly.

## Common Failure Cases

Typical package problems are:

- missing `git` for repo-backed packages
- bad repo/archive URL
- invalid `tag` or `commit`
- missing both `CMakeLists.txt` and `project.bbs` in the resolved package directory
- wrong `cmake.target`
- wrong `strip_prefix`

If a package fails to resolve, inspect it with:

```bat
bbs package <package_name>
```

## Examples

See:

- [5_EXAMPLES.md](./5_EXAMPLES.md)
- [`../examples/raylib_example/`](../examples/raylib_example/)

## Related Reads

- [2_COMMAND_GUIDE.md](./2_COMMAND_GUIDE.md)
- [7_PROJECT_CONFIG.md](./7_PROJECT_CONFIG.md)
