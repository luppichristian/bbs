# User Config

`user.bbs` stores your shared defaults across projects.
`local.bbs` uses the same schema, but applies only to the current project and overrides `user.bbs`.

This guide covers both files because they intentionally share the same structure.

## Where These Files Live

- `user.bbs`: next to the `bbs` executable
- `local.bbs`: in the project root

The optional top-level `user(...)` wrapper is allowed.

## Why These Files Exist

Use `user.bbs` for settings you want across many projects.

Examples:

- preferred build directory name
- preferred dist directory name
- default extra CMake flags
- default test flags
- reusable custom generators

Use `local.bbs` for machine-specific or project-specific overrides that should not be shared.

Examples:

- local CMake flags
- editor-specific debounce tuning
- local SDK or environment quirks

## Precedence

The merge order is:

1. built-in defaults
2. `user.bbs`
3. `local.bbs`

So `local.bbs` always wins over `user.bbs` for the same field.

For custom generators, a same-name `gen(...)` in `local.bbs` replaces the one from `user.bbs`.

## Supported Options

## `builddir`

Build root directory name.

Default:

```txt
build
```

This is the root where derived backend files and build outputs are written.

Example:

```txt
builddir("out")
```

## `distdir`

Distribution root directory name.

Default:

```txt
dist
```

This is where `bbs dist` stages outputs.

Example:

```txt
distdir("release")
```

## `auto_debounce_ms`

Default debounce delay for `bbs auto`.

Default:

```txt
500
```

Use a larger value if your editor writes multiple file updates during save.

Example:

```txt
auto_debounce_ms(750)
```

## `auto_retry_count`

Automatic retry count used by the build flow.

Default:

```txt
3
```

Example:

```txt
auto_retry_count(5)
```

## `auto_retry_delay_ms`

Delay between automatic retries.

Default:

```txt
250
```

Example:

```txt
auto_retry_delay_ms(500)
```

## `dist_archive_format`

Default archive format used by `bbs dist` when archive creation is enabled.

Allowed values:

- `zip`
- `tar`
- `rar`

Default:

```txt
zip
```

Example:

```txt
dist_archive_format(zip)
```

## `dist_archive_name`

Default archive name pattern.

Default:

```txt
$CFG-$OS-$ARC--$VER
```

This supports the expansion tokens documented in [9_EXPANSION_TOKENS.md](./9_EXPANSION_TOKENS.md).

Example:

```txt
dist_archive_name("$PRJ-$CFG-$OS-$ARC-$VER")
```

## `cmake_args`

Extra arguments appended to the CMake configure step.

Typical use cases:

- `compile_commands.json`
- custom cache variables
- generator-specific tuning

Example:

```txt
cmake_args("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
```

## `cmake_build_args`

Extra arguments appended to the CMake build step.

Example:

```txt
cmake_build_args("--parallel")
```

## `ctest_args`

Extra arguments appended to the CTest step.

Example:

```txt
ctest_args("--output-on-failure")
```

## `gen(...)`

Defines a custom generator used by `bbs gen <name>`.

Required fields:

- `name`
- `copyfile`

Example:

```txt
gen(
  name(".clang-format")
  copyfile("C:/templates/.clang-format")
)
```

Then:

```bat
bbs gen .clang-format
```

## Example `user.bbs`

```txt
builddir("build")
distdir("dist")
auto_debounce_ms(750)
auto_retry_count(3)
auto_retry_delay_ms(250)
dist_archive_format(zip)
dist_archive_name("$PRJ-$CFG-$OS-$ARC-$VER")
cmake_args("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
cmake_build_args("--parallel")
ctest_args("--output-on-failure")

gen(
  name(".clang-format")
  copyfile("C:/templates/.clang-format")
)
```

## Example `local.bbs`

```txt
cmake_args("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DMY_MACHINE=ON")
auto_debounce_ms(900)
```

## What Does Not Belong Here

Do not put project structure here.

These belong in `project.bbs` instead:

- targets
- source files
- dependencies between targets
- package source declarations
- project metadata

## Related Reads

- [6_PROJECT_CONFIG.md](./6_PROJECT_CONFIG.md)
- [9_EXPANSION_TOKENS.md](./9_EXPANSION_TOKENS.md)
