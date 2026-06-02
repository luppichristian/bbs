# Expansion Tokens

Some `bbs` string fields support token expansion before they are used.

This is mainly relevant for:

- Bash hook commands
- distribution archive naming

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

## Basic Rules

- tokens start with `$`
- token names use uppercase letters and `_`
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

## `$PRJ`

Project id.

## `$PROJECT`

Absolute path to `project.bbs`.

## `$TOOLCHAIN`

Absolute path to `toolchain.bbs`.

## `$USER`

Absolute path to `user.bbs`.

## `$LOCAL`

Absolute path to `local.bbs`.

## `$DBUILD`

Absolute path to the resolved build output directory for the selected config and platform.

## `$DDIST`

Absolute path to the resolved dist config directory.

## `$DGEN`

Absolute path to the generated dist payload directory.
This is usually `<dist>/<config>-<platform>/gen`.

## `$TARGET`

Target id.

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

## Practical Examples

## Archive naming

```txt
dist_archive_name("$PRJ-$CFG-$OS-$ARC-$VER")
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

## Good Usage Advice

- use quotes around path-like expansions in shell commands
- prefer `$DDIST` and `$DGEN` instead of hardcoding dist paths
- prefer `$CFG`, `$OS`, `$ARC`, and `$VER` for archive names instead of repeating fixed strings

## Related Reads

- [6_PROJECT_CONFIG.md](./6_PROJECT_CONFIG.md)
- [4_EXAMPLES.md](./4_EXAMPLES.md)
