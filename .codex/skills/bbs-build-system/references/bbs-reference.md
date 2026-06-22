# BBS Reference

Use this reference when creating or editing `project.bbs`, choosing commands, or generating builder code.

## Core Facts

- Project root: directory containing `project.bbs`
- Main user file: `project.bbs`
- Optional local override: `config.bbs` in the project root
- Shared defaults: `config.bbs` next to the `bbs` executable
- Generated toolchain cache: `toolchain.bbs` next to the `bbs` executable
- Generated output directories: `build/`, `dist/`
- Default asset directory: `assets/`
- Backend generation is automatic; ordinary `bbs` users do not hand-author CMake

## Syntax Model

Use tree syntax:

```txt
name(value)
name(
  child(value)
  child2(
    nested(value)
  )
)
```

Parsing rules:

- indentation is cosmetic
- commas are optional and ignored
- comments may use `# ...` or `// ...`
- strings use double quotes
- lists are sections with repeated scalar children

Recommended minimal app:

```txt
id(my_app)
name("My App")
ver(0.1.0)

targets(
  console(
    output(my_app)
    units(
      src/main.c
    )
  )
)
```

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

Useful defaults:

- CLI app: `console`
- GUI or no-console app: `consoleless`
- reusable compiled library: `static_lib`
- header-only library: `header_lib`
- tests: `test`

## Common Target Fields

Metadata:

- `id`
- `name`
- `authors`
- `ver`
- `license(type(...), file(...))`

Build identity:

- `lang(c|cpp|c++|cuda)`
- `output`
- `stdver`

Sources and relationships:

- `units(...)`
- `include_dirs(...)`
- `link_dirs(...)`
- `dependencies(...)`
- `link_libs(...)`
- `defines(...)`

Compiler and linker tuning:

- `additional_compile_args("...")`
- `additional_link_args("...")`
- `warning_level(default|none|low|medium|high|pedantic)`
- `opt_level(default|none|debug|size|speed_1|speed_2|speed_3|aggressive)`
- `stack_size(<int>)`
- `warnings_as_errors(true|false)`
- `runtime(none|dynamic|static)`

Testing and hooks:

- `testing(true|false)`
- `test_args(...)`
- `pre_build_cmds(...)`
- `post_build_cmds(...)`
- `pre_run_cmds(...)`
- `post_run_cmds(...)`
- `pre_dist_cmds(...)`
- `post_dist_cmds(...)`

Configuration override:

- `filter(config_name ...)`

## Project-Level Fields

Common top-level nodes:

- `id`
- `name`
- `authors`
- `repo`
- `ver`
- `license(...)`
- `configs(...)`
- `filter(...)`
- `targets(...)`
- `builders(...)`
- `find_tool(...)`
- `find_sdk(...)`

## Configuration Pattern

Use `configs(...)` plus `filter(...)` instead of duplicating whole targets:

```txt
id(my_app)

configs(
  debug
  release
)

targets(
  console(
    output(my_app)
    units(src/main.c)

    filter(
      debug
      defines(DEBUG)
      opt_level(debug)
      warning_level(high)
    )

    filter(
      release
      opt_level(speed_3)
      warnings_as_errors(true)
    )
  )
)
```

Rules:

- `default` always exists
- do not reference undefined config names

## Package-Backed Targets

Packages are still declared inside `targets(...)`.

Path package:

```txt
static_lib(
  id(my_dep)
  path("../third_party/my_dep")
  cmake_target(my_dep)
)
```

Repository package:

```txt
static_lib(
  id(raylib_pkg)
  repo(
    link("https://github.com/raysan5/raylib.git")
    tag("5.5")
  )
  cmake_target(raylib)
)
```

Archive package:

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

Consumer target:

```txt
console(
  output(app)
  dependencies(
    my_dep
  )
  units(src/main.c)
)
```

Package invariants:

- exactly one of `path`, `repo(...)`, or `archive(...)`
- the resolved package directory must contain `CMakeLists.txt` or `project.bbs`
- `cmake_target(...)` must match the exported backend target name

## Builder Pattern

Use builders only for dynamic command-time behavior.

`project.bbs`:

```txt
id(my_app)

targets(
  console(
    id(my_app)
    units(src/main.c)
    dependencies(
      preprocessor
    )
  )
)

builders(
  id(preprocessor)
  units(
    src/builder.c
  )
)
```

Builder source:

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

Prefer typed APIs such as:

- `bbs_target_set_text(...)`
- `bbs_target_append_text(...)`
- `bbs_project_set_text(...)`
- `bbs_ctx_set_text(...)`

## Distribution Pattern

```txt
dist(
  name("$OUT-$CFG-$OS-$ARC")
  archive(true)
  copy_assets(true)
  exclude_assets("dev")
  exclude_assets("raw/*.psd")
  precommand("cp README.md '$DGEN/'")
)
```

Supported children:

- `name`
- `archive`
- `copy_assets`
- repeated `exclude_assets(...)`
- repeated `precommand(...)`
- repeated `postcommand(...)`

## Tokens

Important expansion tokens:

- `$CFG`
- `$PLT`
- `$OS`
- `$ARC`
- `$TARGET`
- `$OUT`
- `$VER`
- `$EXE`
- `$DBUILD`
- `$DASSETS`
- `$DDIST`
- `$DGEN`
- `$PROJECT(id)`
- `$CONFIG(path.to.field)`
- `$TOOLCHAIN(path.to.field)`
- `$$` for a literal dollar sign

## Commands

Normal workflow:

```bat
bbs build
bbs run
bbs test
bbs dist
```

Useful explicit forms:

```bat
bbs build -t app -c debug -p windows-x86_64
bbs build -t * -p * -c *
bbs run -t app sample.txt
bbs auto -t app --debounce 750
bbs update
bbs update --init-toolchain
bbs update --refresh-packages
bbs package list
bbs package my_dep
bbs package refresh my_dep
bbs gen gitignore
bbs info project
bbs cfg
```

Rules:

- run commands from the project root
- platform ids use `<os>-<arch>`
- `run` executes host-native outputs only

## Default Layout

When asked to set up a new `bbs` project, prefer:

```txt
project_root/
  project.bbs
  src/
    main.c
  include/
  assets/
```

Do not hand-author `build/` or `dist/`.

## Error Avoidance

Do not:

- assume users must write `CMakeLists.txt` for normal local targets
- emit JSON, YAML, or TOML instead of `.bbs` tree syntax
- use single quotes for `.bbs` strings
- put system libraries in `dependencies(...)`
- use builders for static settings that fit normal target fields
- invent unsupported target types or keys
- assume `bbs run` can execute cross-built binaries locally
- assume hook commands are PowerShell or CMD; docs define them as Bash commands

## Validation Checklist

Before finalizing generated `bbs` content, confirm:

1. `project.bbs` exists in the project root.
2. `targets(...)` contains at least one target.
3. Every target type is supported.
4. Local non-header-only targets have `units(...)` unless package-backed.
5. `dependencies(...)` references `bbs` targets or builder ids.
6. Each package target defines exactly one source mode.
7. Package targets include `cmake_target(...)`.
8. Config names used in `filter(...)` are valid.
9. Strings that need spaces use double quotes.
10. Suggested commands are valid `bbs` commands.
