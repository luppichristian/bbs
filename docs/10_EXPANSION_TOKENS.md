# Expansion Tokens

Some `bbs` string fields support token expansion before they are used.

This is mainly relevant for:

- Bash hook commands
- distribution archive naming
- CMake and CTest extra argument strings

## Where Tokens Are Used

In practice, the most important token-aware places are:

- `pre_build_cmds(...)`
- `post_build_cmds(...)`
- `pre_run_cmds(...)`
- `post_run_cmds(...)`
- `pre_dist_cmds(...)`
- `post_dist_cmds(...)`
- `dist(...)` `precommand(...)`
- `dist(...)` `postcommand(...)`
- `dist_archive_name`
- target `dist(...)` `name(...)`
- `cmake_args`
- `cmake_build_args`
- `ctest_args`

## Basic Rules

- tokens start with `$`
- token names use uppercase letters and `_`
- scoped lookups use `$SCOPE(path.to.value)`
- unknown tokens are left unchanged
- `$$` becomes a literal `$`

Example:

```txt
"echo $$HOME"
```

becomes a script containing:

```txt
echo $HOME
```

## Supported Tokens

## `$CWD`

Current working directory.

## `$CFG`

Resolved active config.
Usually `default` unless another config was selected.

## `$PLT`

Resolved platform id, for example `windows-x86_64`.

## `$OS`

Platform OS part, for example `windows`.

## `$ARC`

Platform architecture part, for example `x86_64`.

## `$PROJECT`

Scoped lookup into the parsed `project.bbs` tree.

Examples:

```txt
$PROJECT(id)
$PROJECT(license.type)
```

## `$TOOLCHAIN`

Scoped lookup into the parsed `toolchain.bbs` tree.

## `$CONFIG`

Scoped lookup into the effective merged config tree.
This reflects global `config.bbs` with local `config.bbs` overrides applied.

## `$LOCAL`

Absolute path to the local `config.bbs`.

## `$PROJECT_FILE`

Absolute path to `project.bbs`.

## `$TOOLCHAIN_FILE`

Absolute path to `toolchain.bbs`.

## `$GLOBAL_FILE`

Absolute path to the global `config.bbs`.

## `$LOCAL_FILE`

Absolute path to the local `config.bbs`.

## `$DBUILD`

Absolute path to the resolved build output directory for the selected config and platform.

## `$DASSETS`

Absolute path to the resolved assets directory.

## `$DDIST`

Absolute path to the resolved dist config directory.

## `$DGEN`

Absolute path to the generated dist payload directory.
This is usually `<dist>/<config>-<platform>/gen`.

## `$TARGET`

Target id.

When used as a scoped lookup, it can also read resolved fields from the current target.

Examples:

```txt
$TARGET(output)
$TARGET(lang)
$TARGET(package.resolved_dir)
$TARGET(include_dirs.0)
$TARGET(exe)
```

## `$OUT`

Target output name.

## `$VER`

Resolved version text.

Resolution order:

1. target version, if present
2. project version
3. `0.0.0`

## `$EXE`

Absolute path to the target executable output when available.

## `$WORKDIR`

Working directory used for the script being executed.

## `$DEP(...)` / `$DEPENDENCY(...)`

Scoped lookup into another resolved target by id.
The first path segment is the dependency target id, and the remaining path is resolved the same way as `$TARGET(...)`.

Examples:

```txt
$DEP(raylib.package.resolved_dir)
$DEPENDENCY(enet.package.build_dir)
$DEP(my_codegen.exe)
```

## Scoped Path Rules

- use `.` for nested fields, for example `$PROJECT(license.type)`
- path segments are matched case-insensitively
- scalar values are expanded as text
- non-scalar nodes are left unchanged
- numeric path segments can index repeated children in source order, for example `$PROJECT(targets.0.output)`
- `$TARGET(...)` supports resolved scalar target fields such as `id`, `name`, `lang`, `type`, `output`, `runtime`, `stdver`, `warning_level`, `opt_level`, `stack_size`, `warnings_as_errors`, `defines`, `undefines`, `additional_compile_args`, `additional_link_args`, `platform`, `os`, `arch`, `workdir`, `exe`, `build_dir`, `dist_dir`, `gen_dir`, and `assets_dir`
- `$TARGET(...)` also supports nested groups such as `meta.*`, `license.type`, `license.file`, `package.*`, and `dist.*`
- list-style target fields can be indexed with numeric suffixes, for example `$TARGET(include_dirs.0)`, `$TARGET(dependencies.1)`, `$TARGET(package.cmake_args.0)`, and `$TARGET(package.cmake_options.0)`
- `$DEP(...)` and `$DEPENDENCY(...)` first resolve the referenced target id, then apply the same lookup rules as `$TARGET(...)`
- bare `$PROJECT`, `$CONFIG`, and `$TOOLCHAIN` were previously file-path tokens; prefer `$PROJECT_FILE`, `$GLOBAL_FILE`, and `$TOOLCHAIN_FILE` for that use

## Practical Examples

## Archive naming

```txt
dist_archive_name("$PROJECT(id)-$CFG-$OS-$ARC-$VER")
```

Example result:

```txt
my_app-debug-windows-x86_64-0.1.0
```

## Copy built executable into dist output

```txt
post_build_cmds(
  "cp '$EXE' '$DDIST'"
)
```

## Put generated files into the dist staging area

```txt
dist(
  archive(true)
  precommand("cp README.md '$DGEN/'")
)
```

## Emit a literal `$`

```txt
post_build_cmds(
  "echo $$HOME"
)
```

## Query config values in command args

```txt
cmake_args("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DPROJECT_ID=$PROJECT(id)")
ctest_args("--output-on-failure --label-regex $PROJECT(id)")
```

## Query resolved target and dependency fields

```txt
pre_build_cmds(
  "echo target output: $TARGET(output)"
  "echo first include dir: $TARGET(include_dirs.0)"
  "echo enet package dir: $DEP(enet.package.resolved_dir)"
)
```

## Refer to config file paths explicitly

```txt
post_build_cmds(
  "echo project file: $PROJECT_FILE"
  "echo toolchain file: $TOOLCHAIN_FILE"
)
```

## Good Usage Advice

- use quotes around path-like expansions in shell commands
- prefer `$DDIST` and `$DGEN` instead of hardcoding dist paths
- prefer `$CFG`, `$OS`, `$ARC`, and `$VER` for archive names instead of repeating fixed strings
- prefer `$PROJECT_FILE`, `$GLOBAL_FILE`, and `$TOOLCHAIN_FILE` over the deprecated bare file-path forms

## Related Reads

- [7_PROJECT_CONFIG.md](./7_PROJECT_CONFIG.md)
- [5_EXAMPLES.md](./5_EXAMPLES.md)
