# Examples

This guide shows practical `bbs` setups, from the smallest project to package-backed targets and distribution hooks.

## 1. Small Console App

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

Build:

```bat
bbs build
```

Build and run:

```bat
bbs run
```

Why this works:

- there is only one target, so `bbs` can infer it
- the target is runnable because it is `console`

## 2. Multiple Configs

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

Useful commands:

```bat
bbs build -c debug
bbs build -c release
bbs run -c debug
```

What this demonstrates:

- `configs(...)` declares selectable build configs
- `filter(...)` applies config-specific overrides

## 3. Static Library Plus App

This follows `examples/static_lib/project.bbs`.

```txt
configs(
  debug
)

id(static_lib_example)
name("Static Library Example")
ver(0.1.0)

targets(
  static_lib(
    id(greeter)
    output(greeter)
    include_dirs(
      src
    )
    units(
      src/greeter.c
    )
  )

  console(
    output(static_lib_example)
    include_dirs(
      src
    )
    dependencies(
      greeter
    )
    units(
      src/main.c
    )
  )
)
```

Why it is useful:

- `greeter` is built once as a reusable target
- the app links it through `dependencies(...)`
- you keep library and executable structure inside one project file

## 4. Test Target

```txt
targets(
  static_lib(
    id(core)
    units(src/core.c)
  )

  test(
    output(core_tests)
    dependencies(
      core
    )
    units(
      tests/test_main.c
    )
  )
)
```

Run tests:

```bat
bbs test
```

This is the simplest pattern for keeping reusable code in one target and test code in another.

## 5. Package From Git Repository

This follows `examples/raylib_example/project.bbs`.

```txt
id(raylib_example)
name("Raylib Package Example")
ver(0.1.0)

targets(
  static_lib(
    id(raylib_pkg)
    name("raylib package")
    repo(
      link("https://github.com/raysan5/raylib.git")
      tag("5.5")
    )
    cmake_target(raylib)
  )

  console(
    output(raylib_example)
    dependencies(
      raylib_pkg
    )
    units(
      src/main.c
    )
  )
)
```

Useful commands:

```bat
bbs package list
bbs package raylib_pkg
bbs package refresh raylib_pkg
bbs build
```

What this demonstrates:

- package targets can come from a repository
- the package target still behaves like a normal dependency for your own targets

## 6. Package From Another `project.bbs`

This follows `examples/bbs_package_consumer/project.bbs` and `examples/bbs_package_dep/project.bbs`.

Consumer project:

```txt
id(bbs_package_consumer)
name("BBS Package Consumer")
ver(0.1.0)

targets(
  static_lib(
    id(greeter_pkg)
    path("../bbs_package_dep")
    cmake_target(bbs_greeter)
  )

  console(
    output(bbs_package_consumer)
    dependencies(
      greeter_pkg
    )
    units(
      src/main.c
    )
  )
)
```

Package project:

```txt
id(bbs_package_dep)

targets(
  static_lib(
    id(bbs_greeter)
    include_dirs(
      src
    )
    units(
      src/greeter.c
    )
  )
)
```

Useful commands:

```bat
bbs package list
bbs package greeter_pkg
bbs build
bbs run
```

## 7. Builder-Driven Metacompilation

This follows `examples/builders/project.bbs`.

```txt
id(builders_example)
name("Builders Example")
ver(0.1.0)

targets(
  console(
    id(my_app)
    units(
      src/main.c
    )
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
  (void)tgt;

  if (signal != BBS_SIG_PRE_BUILD || !prj)
    return true;

  for (int i = 0; i < prj->target_c; ++i) {
    bbs_tgt* current = &prj->targets[i];
    if (!bbs_target_has_dependency(current, "preprocessor"))
      continue;

    if (!bbs_target_append_text(current, BBS_TARGET_TEXT_ADDITIONAL_COMPILE_ARGS, "-DPREPROCESSOR_ACTIVE", " "))
      return false;
  }

  return true;
}
```

Useful commands:

```bat
bbs build
bbs run -t my_app
```

What this demonstrates:

- builders are defined in `project.bbs`, not in the global tool
- targets opt into builder behavior through `dependencies(...)`
- builders can mutate compile-time settings dynamically before the target is built
- the mutation is local to the current command unless the builder explicitly saves it

What this demonstrates:

- a package target can point at a directory that has its own `project.bbs`
- `bbs` generates an embedded backend for that nested package project automatically
- `cmake_target(...)` still names the exported target that the parent project links

## 7. Local Machine Overrides

`user.bbs`

```txt
builddir("build")
distdir("dist")
ctest_args("--output-on-failure")
```

`local.bbs`

```txt
cmake_args("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
auto_debounce_ms(750)
```

Why split them:

- `user.bbs` is shared across your projects
- `local.bbs` is specific to one project on one machine
- `local.bbs` overrides `user.bbs`

## 8. Pre/Post Build Hooks

```txt
targets(
  console(
    output(my_app)
    units(src/main.c)
    pre_build_cmds(
      "echo building $OUT for $PLT"
    )
    post_build_cmds(
      "echo executable: $EXE"
    )
  )
)
```

What this demonstrates:

- hook commands run in Bash
- `$OUT`, `$PLT`, and `$EXE` are expanded before execution

See [9_EXPANSION_TOKENS.md](./9_EXPANSION_TOKENS.md) for the full token list.

## 9. Distribution Archive

`user.bbs`

```txt
dist_archive_format(zip)
dist_archive_name("$PROJECT(id)-$CFG-$OS-$ARC-$VER")
```

`project.bbs`

```txt
targets(
  console(
    output(my_app)
    units(src/main.c)
    dist(
      archive(true)
      name("$OUT-$CFG")
      precommand("cp README.md $DGEN/")
    )
  )
)
```

Then:

```bat
bbs dist
```

What this demonstrates:

- user config can define default archive format and naming
- a target can override the dist archive name
- the dist flow has its own hooks and staging directory

## 10. Custom Generator

`user.bbs`

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

This is a small but useful way to share project bootstrap files across many projects.

## 11. Inspecting Parsed Config

Useful while learning or debugging:

```bat
bbs info project
bbs info project --filter=dist
bbs info user
bbs info toolchain --values-only
```

This is especially helpful when you are unsure whether a value came from:

- the project file
- user defaults
- local overrides
- generated toolchain state

## 12. Full Beginner Flow

Create:

- `src/main.c`
- `project.bbs`

Minimal `project.bbs`:

```txt
id(hello)

targets(
  console(
    units(src/main.c)
  )
)
```

Then the most common first commands are:

```bat
bbs build
bbs run
```

After that, you usually add configs, tests, hooks, or distribution rules as the project grows.

## 13. Unity Batches By Directory

This follows `examples/unity_batches/project.bbs`.

```txt
id(unity_batches)

targets(
  console(
    output(unity_batches)
    units(
      src/main.c
      src/common/*.c
      src/feature/*.c
    )
    unity(
      enabled(true)
      batch_size(4)

      batch(
        src/common
      )

      batch(
        src/feature
      )
    )
  )
)
```

What this demonstrates:

- `units(...)` still defines the full source set
- `unity.batch(...)` groups files by directory without changing the source list
- `batch_size(...)` still controls fallback batching for any units left outside explicit batches
