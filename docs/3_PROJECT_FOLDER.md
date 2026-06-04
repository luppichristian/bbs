# Project Folder

This page explains how `bbs` treats the project root, which files are expected to exist there, and which directories or files may be created during normal use.

## Project Root

The project root is the directory containing `project.bbs`.

`bbs` resolves relative project paths from that directory.

Typical layout:

```txt
my_project/
  project.bbs
  src/
  include/
  assets/
  local.bbs
  build/
  dist/
```

Not every entry is required.

## Files You Usually Write

### `project.bbs`

Required project definition.

This declares targets, configs, packages, builders, dist settings, and so on.

### `local.bbs`

Optional machine-local override file in the project root.

Use it for per-machine overrides such as custom arguments or paths that should not be shared.

This file is not generated automatically, but `bbs` looks for it.

### Source Files And Project Assets

Everything else in the project root is up to your project layout.

Common examples:

- `src/`
- `include/`
- `tests/`
- `assets/`
- `README.md`
- license files

## Directories Managed By `bbs`

### `builddir(...)`

Default: `build`

This is the generated backend and build-output root.

`bbs` creates it automatically.

Typical contents include:

- generated `CMakeLists.txt`
- generated `CMakePresets.json`
- generated toolchain helper files
- per-config/per-platform build directories
- compiled binaries, libraries, and intermediate backend output

### `assetsdir(...)`

Default: `assets`

This is the project asset root.

`bbs` creates it automatically so the folder is always available as part of the project layout.

Unlike `builddir(...)` and `distdir(...)`, this is not treated as disposable generated output. It is meant to store project-owned runtime assets.

If a target uses:

```txt
dist(
  copy_assets(true)
)
```

then `bbs dist` copies this directory recursively into the staged distribution payload.

You can remove specific asset paths from the copied payload with repeated `exclude_assets("...")` entries inside `dist(...)`.

### `distdir(...)`

Default: `dist`

This is the distribution output root.

`bbs` creates it automatically.

Typical contents include:

- per-config/per-platform dist directories
- a `gen/` staging folder containing the assembled payload
- optional archives created from that payload

## Dist Layout

For a target distributed with `bbs dist`, the output typically looks like this:

```txt
dist/
  default-windows-x86_64/
    gen/
      my_app.exe
      assets/
    my_project-default-windows-x86_64-0.1.0.zip
```

Inside `gen/`, `bbs` may place:

- the built executable
- copied runtime libraries when needed
- copied assets when `copy_assets(true)` is enabled
- any extra files added by `precommand(...)` or `postcommand(...)`

## Files Outside The Project Root

Some `bbs` files are not stored in the project folder:

### `user.bbs`

Shared user defaults file.

This lives next to the `bbs` executable, not in the project root.

### `toolchain.bbs`

Generated toolchain cache.

This also lives next to the `bbs` executable, not in the project root.

### Package Cache

Fetched package content is stored in the shared `packages/` area next to the `bbs` executable.

That cache is shared across projects.

## What `bbs clean` Removes

`bbs clean` removes the configured `builddir(...)` and `distdir(...)` from the project root when they exist.

It does not remove the assets directory.

## What You Usually Commit

Usually commit:

- `project.bbs`
- source files
- headers
- tests
- project assets
- documentation

Usually do not commit:

- `build/`
- `dist/`
- `local.bbs`

`bbs gen gitignore` can generate a starter `.gitignore` for the generated output.

## Related Docs

- [4_FILE_FORMAT.md](./4_FILE_FORMAT.md)
- [6_USER_CONFIG.md](./6_USER_CONFIG.md)
- [7_PROJECT_CONFIG.md](./7_PROJECT_CONFIG.md)
- [10_EXPANSION_TOKENS.md](./10_EXPANSION_TOKENS.md)
