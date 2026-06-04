# Command Guide

This guide explains what each `bbs` command does, when to use it, and how the common options behave.

## Mental Model

Most of the time you only need a few commands:

- `bbs build`: build
- `bbs run`: build and run
- `bbs test`: run tests
- `bbs dist`: prepare distribution output
- `bbs auto`: watch and rebuild

`bbs update` exists for the cases where you want to regenerate the backend explicitly, inspect resolved state, refresh packages, or force toolchain regeneration.

## Common Rules

Commands operate on the project rooted at the current working directory.

Important shared behavior:

- platform ids use `<os>-<arch>`, for example `windows-x86_64`
- `-t`, `-p`, and `-c` accept `*` on matrix-capable commands
- if a project has one target, `bbs` can often infer `-t`
- if config is omitted, `bbs` resolves `default`
- the backend is initialized automatically when needed
- `run` only executes host-native outputs

## help

```txt
bbs help [command|config]
```

Shows general help or detailed help for one command or config topic.

Examples:

```bat
bbs help
bbs help build
bbs help project
```

## clean

```txt
bbs clean [project|user|local|toolchain]
```

Removes selected `bbs` config files.

Behavior:

- no argument: removes `project.bbs` and `local.bbs`
- `project`: removes `project.bbs`
- `user`: removes `user.bbs`
- `local`: removes `local.bbs`
- `toolchain`: removes `toolchain.bbs`

Use it when you want to reset a specific config file rather than the generated build directories.

## update

```txt
bbs update [-i|--info] [-c config] [--init-toolchain] [--refresh-packages]
```

Parses the project, validates it, resolves packages, and regenerates the derived backend files used by `bbs`.

Use `update` when:

- you want to regenerate backend files without building
- you want to print the resolved project state first
- you want to explicitly refresh the toolchain cache
- you want to refresh repo-backed packages before generation

Options:

- `-i`, `--info`: print resolved project information before generation
- `-c`, `--config`: choose the config used for the info view
- `--init-toolchain`: force regeneration of `toolchain.bbs`
- `--refresh-packages`: refresh repo-backed packages before regeneration

Examples:

```bat
bbs update
bbs update --init-toolchain
bbs update -i -c debug
bbs update --refresh-packages
```

## gen

```txt
bbs gen <format> [-o|--override] [-p platform[,platform...]]
```

Generates utility files in the project root.

Built-in formats:

- `gitignore`
- `github`

Custom formats can be defined in `user.bbs` or `local.bbs` with `gen(...)` entries.

Options:

- `-o`, `--override`: overwrite an existing generated target file
- `-p`, `--platform`: only for the `github` generator, overrides the workflow platform matrix

Examples:

```bat
bbs gen gitignore
bbs gen github
bbs gen github -p windows-x86_64,linux-x86_64
bbs gen .clang-format
```

## cfg

```txt
bbs cfg [-m] [-p] [-u] [-l] [-t]
```

Prints the resolved config file paths used by `bbs`.

Useful when you want to confirm which files are being loaded.

Options:

- `-m`, `--minimal`: print raw paths only
- `-p`, `--project`: only `project.bbs`
- `-u`, `--user`: only `user.bbs`
- `-l`, `--local`: only `local.bbs`
- `-t`, `--toolchain`: only `toolchain.bbs`

Examples:

```bat
bbs cfg
bbs cfg -m
bbs cfg -p -u -l -t
```

## build

```txt
bbs build [-t target|*] [-p platform|*] [-c config|*]
```

Builds selected targets.

Typical usage:

- `bbs build`: build the default selection
- `bbs build -t app`: build one target
- `bbs build -p linux-x86_64`: build for one platform
- `bbs build -c debug`: build one config
- `bbs build -t * -p * -c *`: build the full matrix allowed by the project and toolchain

Examples:

```bat
bbs build
bbs build -t app
bbs build -p linux-x86_64
bbs build -t * -p * -c *
```

## auto

```txt
bbs auto [-t target|*] [-p platform|*] [-c config|*] [--debounce ms]
```

Runs the same selection as `bbs build`, then watches the project for file changes and rebuilds automatically.

Use this for iterative local development.

Options:

- `--debounce`: wait for a quiet period before rebuilding

Behavior notes:

- generated build directories are ignored to avoid rebuild loops
- if your editor saves through multiple file updates, a debounce helps reduce redundant rebuilds

Example:

```bat
bbs auto -t app --debounce 750
```

## run

```txt
bbs run [-t target|*] [-p platform|*] [-c config|*] [program args...]
```

Builds the selected runnable target if needed, then executes it.

Important rule:

- `run` only works for host-native outputs
- cross-built outputs can be built, but not executed locally through `bbs run`

Examples:

```bat
bbs run
bbs run -t app
bbs run -t app sample.txt
bbs run -p windows-x86_64
bbs run -t * -p *
```

## info

```txt
bbs info <project|user|local|toolchain> [attribute] [-m] [--attr=path] [--filter=text] [--values-only]
```

Inspects the parsed config tree for one file.

This is especially useful when:

- learning the file shape
- debugging config resolution
- checking what `toolchain.bbs` contains

Options:

- positional `attribute` or `--attr=...`: focus on one attribute path
- `-m`, `--minimal`: print matching attribute paths only
- `--filter=text`: text filter for matching nodes
- `--values-only`: print only matching values

Examples:

```bat
bbs info project
bbs info project targets.console.output
bbs info user --filter=cmake
bbs info toolchain --values-only
```

## package

```txt
bbs package <list|refresh [name|*]|package_name> [--refresh]
```

Inspects and refreshes package-backed targets.

Forms:

- `bbs package list`: list package targets in the current project
- `bbs package refresh`: refresh all repo/archive-backed packages
- `bbs package refresh <name>`: refresh one package target
- `bbs package <package_name>`: inspect one package target

`--refresh` refreshes repo-backed packages before printing package info.

Examples:

```bat
bbs package list
bbs package raylib_pkg
bbs package raylib_pkg --refresh
bbs package refresh
```

## dist

```txt
bbs dist [-t target|*] [-p platform|*] [-c config|*]
```

Stages the selected output for distribution.

Depending on target and config, this can include:

- copying build outputs into the dist directory
- running pre/post dist hooks
- generating an archive when dist archiving is enabled

Examples:

```bat
bbs dist
bbs dist -t app
bbs dist -p linux-x86_64
```

## test

```txt
bbs test [name] [-t target|*] [-p platform|*] [-c config|*]
```

Runs the project test suite, or one named test when supported by the project.

Examples:

```bat
bbs test
bbs test smoke
bbs test -t * -c debug
```

## bumpver

```txt
bbs bumpver <major|minor|patch|user|all> [-p project_id] [-t target_id]
```

Increments a version inside `project.bbs`.

Rules:

- `major`: increment major and reset lower parts
- `minor`: increment minor and reset lower parts
- `patch`: increment patch
- `user`: increment the 4th version field
- `all`: increment all present parts

Examples:

```bat
bbs bumpver patch
bbs bumpver minor -p my_project
bbs bumpver user -t my_library
```

## Suggested Reading Order

- [3_PROJECT_FOLDER.md](./3_PROJECT_FOLDER.md)
- [4_FILE_FORMAT.md](./4_FILE_FORMAT.md)
- [6_USER_CONFIG.md](./6_USER_CONFIG.md)
- [7_PROJECT_CONFIG.md](./7_PROJECT_CONFIG.md)
- [5_EXAMPLES.md](./5_EXAMPLES.md)
- [11_PACKAGES.md](./11_PACKAGES.md)
