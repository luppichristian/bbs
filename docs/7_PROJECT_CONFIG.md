# Project Config

`project.bbs` defines the project itself: metadata, configs, targets, package sources, hooks, and distribution behavior.

This is the main file you edit when creating and growing a `bbs` project.

## Location

`project.bbs` lives in the project root.

For a broader explanation of how the full project directory is handled, see [3_PROJECT_FOLDER.md](./3_PROJECT_FOLDER.md).

The optional top-level `project(...)` wrapper is accepted, but most examples use the direct form.

## Mental Model

A `project.bbs` file usually has these parts:

1. project metadata
2. optional config list
3. project-level filters
4. target list
5. optional builder list

Smallest valid shape:

```txt
id(my_app)

targets(
  console(
    units(src/main.c)
  )
)
```

## Custom Discovery Hooks

`project.bbs` can also declare extra discovery strategies with top-level `find_tool(...)` and `find_sdk(...)` sections.

This is useful when a single project depends on a tool or SDK that should be discovered differently from the global defaults.

Example:

```txt
find_tool(
  id(ninja)
  exe_name(ninja)
  dir_hints("C:/tools/ninja;{home}/tools/ninja")
)

find_sdk(
  id(proprietary_sdk)
  root_hints("C:/vendor/sdk;{home}/vendor/sdk")
  include_rel(include)
  lib_rel(lib)
)
```

These sections are top-level discovery declarations only. They are not valid inside target sections and they are not inline expressions.

## Top-Level Options

## `id`

Project identifier.

Used for project identity and in some token expansions.

Example:

```txt
id(my_app)
```

## `name`

Human-readable project name.

Example:

```txt
name("My App")
```

## `authors`

Free-form author string.

## `repo`

Project repository URL or label.

At project scope, `repo` is a scalar string or identifier, not a nested section.

Example:

```txt
repo("https://github.com/example/my_app")
```

## `ver`

Project version.

Examples:

```txt
ver(1.2.3)
ver(1.2.3.4)
```

## `license(...)`

Optional license section with:

- `type`
- `file`

Example:

```txt
license(
  type("MIT")
  file("LICENSE.md")
)
```

## `configs(...)`

List of selectable config names.

Example:

```txt
configs(
  debug
  release
)
```

Important:

- config names may contain letters, digits, `_`, `-`, and `.`
- `default` is always available even if omitted
- duplicate config names are rejected

## `filter(...)`

Project-level config-specific overrides.

The first item must be the config name.
The remaining named entries are target-style override fields applied to all targets.

Example:

```txt
filter(
  debug
  warning_level(high)
  opt_level(debug)
)
```

## `targets(...)`

Container for target definitions.

Projects must define at least one target.

## `builders(...)`

Optional top-level metacompilation section.

Builders are project-local modules that `bbs` compiles and loads dynamically, then calls before and after command phases.
They are useful for dynamic build logic such as appending compile options, injecting defines, or adjusting resolved target settings for one command only.

Example:

```txt
builders(
  id(preprocessor)
  units(
    src/builder.c
  )
)
```

Targets can opt into builder behavior through `dependencies(...)`:

```txt
console(
  id(my_app)
  units(src/main.c)
  dependencies(
    preprocessor
  )
)
```

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

For full lifecycle details and the shipped example, see [12_BUILDERS.md](./12_BUILDERS.md).

## Target Types

Supported target section names:

- `console`
- `consoleless`
- `header_lib`
- `static_lib`
- `dyn_lib`
- `obj_lib`
- `test`
- `driver`

Accepted aliases include:

- `gui`
- `header-lib`
- `headerlib`
- `static-lib`
- `staticlib`
- `dyn-lib`
- `shared_lib`
- `shared-lib`
- `obj-lib`
- `object_lib`
- `object-lib`

## Target Metadata

These fields are valid inside a target section:

- `id`
- `name`
- `authors`
- `ver`
- `license(...)`

Fallback behavior:

- missing target metadata inherits from project metadata where appropriate
- if there is only one target and it has no id, `bbs` can synthesize one from output or target type
- if `output` is omitted, it falls back to target id

## Build Identity Fields

## `lang`

Allowed values:

- `c`
- `cpp`
- `c++`
- `cuda`
- `cu`

Default is `c`.

Notes:

- `lang(cuda)` enables generated CMake CUDA support for the target
- CUDA targets may still list units such as `src/main.c`; `bbs` marks those sources as CUDA in the generated backend

## `output`

Logical output name for the target.

## `stdver`

Language standard string forwarded into the generated backend.

Examples:

- `c11`
- `gnu11`
- `c++20`
- `cuda17`
- `cuda20`

For CUDA targets, `stdver(...)` is forwarded as `CUDA_STANDARD` in the generated CMake backend.

## `cuda_architectures`

Optional CUDA architecture list forwarded into generated CMake as `CUDA_ARCHITECTURES`.

Examples:

- `cuda_architectures(75)`
- `cuda_architectures(75 89)`

Notes:

- this can be set at project scope to provide a default for all targets
- targets and filters may override it with their own `cuda_architectures(...)`
- `bbs` normalizes the list to the semicolon-separated form CMake expects

## Source And Dependency Fields

## `units(...)`

Source files or patterns for the target.

Example:

```txt
units(
  src/main.c
  src/app.c
)
```

## `unity(...)`

Optional unity-build configuration for compiled targets.

Example:

```txt
unity(
  enabled(true)
  batch_size(8)

  batch(
    src/core
    src/platform
  )

  batch(
    src/feature/*.c
  )
)
```

Supported children:

- `enabled(true|false)` turns unity builds on or off for the target. When a `unity(...)` section is present, the default is `true` unless you explicitly set `enabled(false)`.
- `batch_size(<positive-int>)` forwards the fallback unity batch size into CMake.
- `batch(...)` declares an explicit unity batch. You can repeat `batch(...)` multiple times.

Batch matching rules:

- batch items are matched against the resolved unit paths relative to the project root
- plain paths match either that exact file or any file under that directory prefix
- wildcard entries such as `src/feature/*.c` or `src/**/win_*.c` are also supported
- every explicit batch must match at least one unit
- one source file may belong to only one explicit batch

Execution model:

- explicit `batch(...)` groups are emitted as generated unity wrapper source files in the build directory
- remaining units still stay in the target source list
- if unity is enabled, CMake native unity mode still applies to the remaining non-explicit units

Notes:

- `unity(...)` is only valid on compiled local targets
- header libraries cannot use unity builds

## `include_dirs(...)`

Include directories added to the target.

## `link_dirs(...)`

Link search directories.

## `dependencies(...)`

Dependencies on other `bbs` targets by name.

These are project-internal target links, not raw system libraries.

They can also be used to opt into builder behavior when the dependency name matches a builder id declared in `builders(...)`.

## `link_libs(...)`

External libraries by basename.

## `defines(...)`

Preprocessor definitions.

This is parsed as a scalar list and joined into a space-separated string.

Example:

```txt
defines(
  DEBUG
  FEATURE_X=1
)
```

## Compiler And Linker Tuning

## `additional_compile_args`

Extra compile flags as a single string.

Notes:

- for C and C++ targets, `bbs` treats this as Clang/GCC-style input and translates a supported subset for MSVC when generating the backend
- for CUDA targets, `bbs` normalizes a small common subset for `nvcc`, including standard selection, warning flags, optimization flags, and host-compiler forwarding
- use this field for CUDA-specific flags that do not already have a dedicated property such as `cuda_architectures(...)`

## `additional_link_args`

Extra link flags as a single string.

## `warning_level`

Allowed values:

- `default`
- `none`
- `low`
- `medium`
- `high`
- `pedantic`

Accepted aliases include `off`, `minimal`, `normal`, `all`, and `extra`.

This setting currently applies to C and C++ backend emission. Use `additional_compile_args` for CUDA-specific warning flags.

## `opt_level`

Allowed values:

- `default`
- `none`
- `debug`
- `size`
- `speed_1`
- `speed_2`
- `speed_3`
- `aggressive`

Accepted aliases include `o0`, `og`, `os`, `oz`, `o1`, `o2`, `o3`, `fast`, and `ofast`.

This setting currently applies to C and C++ backend emission. Use `additional_compile_args` for CUDA-specific optimization flags.

## `stack_size`

Non-negative integer stack size override.

## `warnings_as_errors`

Boolean.

## `runtime`

Allowed values:

- `none`
- `dynamic`
- `static`

Default is `dynamic`.

## Testing Fields

## `testing`

Boolean. `test` targets default to `true`.

## `test_args(...)`

Extra test arguments for the target.

## Script Hook Fields

All hook commands are Bash commands.

- `pre_build_cmds(...)`
- `post_build_cmds(...)`
- `pre_run_cmds(...)`
- `post_run_cmds(...)`
- `pre_dist_cmds(...)`
- `post_dist_cmds(...)`

These support the tokens documented in [10_EXPANSION_TOKENS.md](./10_EXPANSION_TOKENS.md).

## Distribution Fields

There are two ways to define dist behavior.

### Flat Hook Form

You can use only the normal dist hook lists:

- `pre_dist_cmds(...)`
- `post_dist_cmds(...)`

### `dist(...)` Section Form

```txt
dist(
  name("$OUT-$CFG-$OS-$ARC")
  archive(true)
  copy_assets(true)
  exclude_assets("dev")
  exclude_assets("raw/*.psd")
  precommand("...")
  postcommand("...")
)
```

Supported fields inside `dist(...)`:

- `name`: archive name pattern override
- `archive`: boolean
- `copy_assets`: boolean
- repeated `exclude_assets(...)`: asset paths relative to the assets root
- repeated `precommand(...)`
- repeated `postcommand(...)`

`dist(...)` is the only place where repeated `precommand(...)` and `postcommand(...)` entries are used directly.

When `copy_assets(true)` is enabled, each `exclude_assets(...)` entry removes either one exact asset path or an entire nested subtree from the copied assets payload.

## Package Target Fields

Targets can point at external package sources.

### Path-backed package

Fields:

- `path`
- `subdir`
- `cmake_target`

The resolved package directory may provide either `CMakeLists.txt` or `project.bbs`.

If both are present, `bbs` prefers `CMakeLists.txt`.

Example:

```txt
static_lib(
  id(my_dep)
  path("../third_party/my_dep")
  subdir("src")
  cmake_target(my_dep)
)
```

### Repo-backed package

```txt
static_lib(
  id(my_dep)
  repo(
    link("https://example.com/my_dep.git")
    tag("1.0.0")
  )
  cmake_target(my_dep)
)
```

Supported `repo(...)` fields:

- `link`
- `tag`
- `commit`

### Archive-backed package

```txt
static_lib(
  id(my_dep)
  archive(
    link("https://example.com/my_dep.zip")
    strip_prefix("my_dep-1.0.0")
  )
  cmake_target(my_dep)
)
```

Supported `archive(...)` fields:

- `link`
- `strip_prefix`

Important package rule:

- only one package source mode should be active at a time
- setting `path`, `repo(...)`, or `archive(...)` replaces the other package-source settings for that target
- `cmake_target` names the target that the package backend exports, whether that backend comes from native CMake or from a nested `project.bbs`

## Config Filters

`filter(...)` works at both project scope and target scope.

Rules:

- first item is the config name
- remaining named entries override normal target fields
- the referenced config must exist in `configs(...)` or be `default`

Target-level example:

```txt
console(
  output(app)
  units(src/main.c)

  filter(
    debug
    defines(DEBUG)
    opt_level(debug)
  )

  filter(
    release
    opt_level(speed_3)
  )
)
```

## Minimal But Realistic Example

```txt
id(my_app)
name("My App")
ver(0.1.0)

configs(
  debug
  release
)

targets(
  console(
    output(my_app)
    include_dirs(
      src
    )
    units(
      src/main.c
      src/app.c
    )

    filter(
      debug
      defines(DEBUG)
      opt_level(debug)
    )

    filter(
      release
      opt_level(speed_3)
      warnings_as_errors(true)
    )
  )
)
```

## Related Reads

- [4_FILE_FORMAT.md](./4_FILE_FORMAT.md)
- [5_EXAMPLES.md](./5_EXAMPLES.md)
- [6_CONFIG.md](./6_CONFIG.md)
- [10_EXPANSION_TOKENS.md](./10_EXPANSION_TOKENS.md)
- [11_PACKAGES.md](./11_PACKAGES.md)
