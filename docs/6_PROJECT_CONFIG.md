# Project Config

`project.bbs` defines the project itself: metadata, configs, targets, package sources, hooks, and distribution behavior.

This is the main file you edit when creating and growing a `bbs` project.

## Location

`project.bbs` lives in the project root.

The optional top-level `project(...)` wrapper is accepted, but most examples use the direct form.

## Mental Model

A `project.bbs` file usually has these parts:

1. project metadata
2. optional config list
3. project-level filters
4. target list

Smallest valid shape:

```txt
id(my_app)

targets(
  console(
    units(src/main.c)
  )
)
```

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

Default is `c`.

## `output`

Logical output name for the target.

## `stdver`

Language standard string forwarded into the generated backend.

Examples:

- `c11`
- `gnu11`
- `c++20`

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

## `include_dirs(...)`

Include directories added to the target.

## `link_dirs(...)`

Link search directories.

## `dependencies(...)`

Dependencies on other `bbs` targets by name.

These are project-internal target links, not raw system libraries.

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

These support the tokens documented in [9_EXPANSION_TOKENS.md](./9_EXPANSION_TOKENS.md).

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
  precommand("...")
  postcommand("...")
)
```

Supported fields inside `dist(...)`:

- `name`: archive name pattern override
- `archive`: boolean
- repeated `precommand(...)`
- repeated `postcommand(...)`

`dist(...)` is the only place where repeated `precommand(...)` and `postcommand(...)` entries are used directly.

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

- [3_FILE_FORMAT.md](./3_FILE_FORMAT.md)
- [4_EXAMPLES.md](./4_EXAMPLES.md)
- [5_USER_CONFIG.md](./5_USER_CONFIG.md)
- [9_EXPANSION_TOKENS.md](./9_EXPANSION_TOKENS.md)
- [10_PACKAGES.md](./10_PACKAGES.md)
