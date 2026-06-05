# Config Files

`config` is the merged result of the global and local config files.

`bbs` looks for the same filename in two places:

- global `config.bbs`: next to the `bbs` executable
- local `config.bbs`: in the project root

They intentionally share the same schema.
`config` means the merged result, where local overrides global.

For a broader explanation of how these files relate to the project directory and generated folders, see [3_PROJECT_FOLDER.md](./3_PROJECT_FOLDER.md).

The optional top-level `config(...)` wrapper is allowed.

## Why These Files Exist

Use the global `config.bbs` for settings you want across many projects.

Examples:

- preferred build directory name
- preferred dist directory name
- default extra CMake flags
- default test flags
- reusable custom generators

Use the local `config.bbs` for machine-specific or project-specific overrides that should not be shared.

Examples:

- local CMake flags
- editor-specific debounce tuning
- local SDK or environment quirks

## Precedence

The merge order is:

1. built-in defaults
2. global `config.bbs`
3. local `config.bbs`

So the local `config.bbs` always wins for the same field.

For custom generators, a same-name `gen(...)` in the local `config.bbs` replaces the one from the global `config.bbs`.

## Custom Discovery Hooks

The global and local `config.bbs` files can declare additional toolchain discovery strategies with top-level:

- `find_tool(...)`
- `find_sdk(...)`

These do not return values into other fields.
They extend toolchain initialization so `bbs` can discover tools and SDKs that are not covered by the built-in strategy list.

If a declared tool or SDK is missing from the current cache, `bbs` refreshes that part of `toolchain.bbs` automatically.

`find_tool(...)` fields:

- `id` required
- `exe_name` required
- `target_os` optional: `windows`, `linux`, `macos`, or `any`
- `dir_hints` optional
- `deep_roots` optional
- `version_arg` optional
- `version_regex` optional
- `version_arg_fallback` optional
- `version_regex_fallback` optional

Example:

```txt
find_tool(
  id(ninja)
  exe_name(ninja)
  dir_hints("C:/tools/ninja;{program_files}/ninja")
  version_arg("--version")
  version_regex("([0-9]+\.[0-9]+(\.[0-9]+)?)")
)
```

`find_sdk(...)` fields:

- `id` required
- `target_os` optional: `windows`, `linux`, `macos`, or `any`
- `env_vars` optional
- `root_hints` optional
- `include_rel` optional
- `source_rel` optional
- `lib_rel` optional
- `bin_rel` optional
- `version_file_rel` optional
- `version_regex` optional

At least one of `env_vars` or `root_hints` is required.

Example:

```txt
find_sdk(
  id(my_sdk)
  env_vars(MY_SDK_ROOT)
  root_hints("C:/sdk/my_sdk;{home}/sdk/my_sdk")
  include_rel(include)
  lib_rel(lib)
  bin_rel(bin)
)
```

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

## `assetsdir`

Assets root directory name.

Default:

```txt
assets
```

This directory is created automatically and can be copied into `bbs dist` payloads.

Example:

```txt
assetsdir("resources")
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

This supports the expansion tokens documented in [10_EXPANSION_TOKENS.md](./10_EXPANSION_TOKENS.md).

Example:

```txt
dist_archive_name("$PROJECT(id)-$CFG-$OS-$ARC-$VER")
```

## `cmake_args`

Extra arguments appended to the CMake configure step.

This supports the expansion tokens documented in [10_EXPANSION_TOKENS.md](./10_EXPANSION_TOKENS.md).

Example:

```txt
cmake_args("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
```

## `cmake_build_args`

Extra arguments appended to the CMake build step.

This supports the expansion tokens documented in [10_EXPANSION_TOKENS.md](./10_EXPANSION_TOKENS.md).

Example:

```txt
cmake_build_args("--parallel")
```

## `ctest_args`

Extra arguments appended to the CTest step.

This supports the expansion tokens documented in [10_EXPANSION_TOKENS.md](./10_EXPANSION_TOKENS.md).

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

## Example Global `config.bbs`

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

## Example Local `config.bbs`

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

- [7_PROJECT_CONFIG.md](./7_PROJECT_CONFIG.md)
- [10_EXPANSION_TOKENS.md](./10_EXPANSION_TOKENS.md)
