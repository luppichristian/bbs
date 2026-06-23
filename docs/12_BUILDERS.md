# Builders

`bbs` supports metacompilation through `builders`.

A builder is a small dynamically loaded module that `bbs.exe` compiles, loads, and calls around command execution.
This lets a project inject build logic without forking `bbs` itself and without hardcoding project-specific behavior into the core tool.

In practice, builders are useful for things like:

- injecting compile flags dynamically before a build
- generating or adjusting target settings for one command only
- adding project-local build preprocessing logic
- reacting to build, run, test, dist, update, and auto command phases

By default, builder mutations are command-local.
They affect the current command execution, then disappear when the command ends.
This is intentional: builders are meant to be safe to experiment with.

When you explicitly want a mutation to persist, use the save APIs from `pub/bbs/build.h`.

## Basic Shape

Builders are defined at the top level of `project.bbs` with a `builders(...)` section.

Example:

```txt
id(my_app)
name("My App")
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

In this model:

- the application target is `my_app`
- the builder id is `preprocessor`
- `dependencies(preprocessor)` means that target wants that builder's behavior

Builder ids live in their own namespace, but they still must not conflict with target ids.

## Builder Attributes

Current builder attributes:

- `id(...)`
- `name(...)`
- `authors(...)`
- `ver(...)`
- `license(...)`
- `lang(...)`
- `output(...)`
- `units(...)`
- `include_dirs(...)`
- `defines(...)`
- `undefines(...)`
- `additional_compile_args(...)`
- `additional_link_args(...)`

Notes:

- `id(...)` is required
- `units(...)` is required
- `lang(...)` defaults to `c`
- `lang(...)` accepts `c`, `cpp`, `c++`, `cuda`, and `cu`
- `output(...)` defaults to the builder id
- `output(...)` accepts either an identifier like `output(my_builder)` or a string like `output("Example Output Name")`

## How Builders Are Used

When a command that supports builders runs, `bbs` does the following:

1. parses the project as usual
2. finds the builders declared in `project.bbs`
3. compiles each builder into a dynamic module
4. loads the module into `bbs.exe`
5. finds the exported `bbs_callback(...)` symbol
6. dispatches command lifecycle signals into the builder

Builders currently participate in:

- `bbs build`
- `bbs run`
- `bbs test`
- `bbs dist`
- `bbs auto`
- `bbs update`

## Lifecycle

The entry point is:

```c
bool bbs_callback(bbs_sig signal, bbs_ctx* ctx, bbs_proj* prj, bbs_tgt* tgt);
```

`signal` tells the builder what phase is happening.

Dispatch model:

- `BBS_SIG_INIT` and `BBS_SIG_QUIT` are emitted once with `prj == NULL` and `tgt == NULL`
- every signal tied to a loaded project is emitted once per resolved target in `prj->targets`
- for those project-bound callbacks, `tgt` is the current target being visited
- if a loaded project has no targets, the signal is emitted once with `tgt == NULL`

Common signals:

- `BBS_SIG_INIT`
- `BBS_SIG_QUIT`
- `BBS_SIG_PRE_CMD`
- `BBS_SIG_POST_CMD`
- `BBS_SIG_PRE_BUILD`
- `BBS_SIG_POST_BUILD`
- `BBS_SIG_PRE_RUN`
- `BBS_SIG_POST_RUN`
- `BBS_SIG_PRE_TEST`
- `BBS_SIG_POST_TEST`
- `BBS_SIG_PRE_DIST`
- `BBS_SIG_POST_DIST`
- `BBS_SIG_PRE_UPDATE`
- `BBS_SIG_POST_UPDATE`

Important detail:

- `run`, `test`, `dist`, and `auto` can still need a build phase
- because of that, build-related signals are also emitted where compile-time mutation is needed

If a builder returns `false`, the current command fails.

## Mutation Model

Builders receive the resolved command context and resolved project model.

That means a builder can inspect and modify:

- selected command info through `bbs_ctx`
- project-wide config through `bbs_proj`
- target settings through `bbs_tgt`

Because project-bound signals are target-scoped, most target mutations should work directly with `tgt` instead of iterating `prj->targets` manually.

Typical dynamic changes include:

- appending compile options
- changing target defines
- changing output naming
- adding generated include paths
- switching config-local values for one command

Prefer dedicated properties over raw compiler flags when possible.

Examples:

- use `BBS_TARGET_TEXT_DEFINES` for preprocessor macros
- use `BBS_TARGET_TEXT_UNDEFINES` for macros that should be removed during compilation
- use `BBS_TARGET_TEXT_STDVER` for language standard changes
- use `BBS_TARGET_TEXT_OUTPUT` for output naming

Use `additional_compile_args` only as an escape hatch for flags that do not already have a dedicated property.

Current compiler support is centered on MSVC, Clang, GCC, and CUDA through `nvcc`.
When extra compile args are emitted into the backend, `bbs` treats them as Clang/GCC-style input and translates supported subsets for MSVC on C and C++ targets.
For CUDA builders, `bbs` also normalizes a small common subset for `nvcc` and forwards host-only flags through the active host compiler.

By default these mutations are temporary.

If a builder wants the mutated state to survive future commands, it can call:

- `bbs_save_toolchain(...)`
- `bbs_save_global(...)`
- `bbs_save_local(...)`
- `bbs_save_project(...)`

## Target Selection Through Dependencies

Targets opt into builder behavior through `dependencies(...)`.

Example:

```txt
dependencies(
  preprocessor
)
```

Inside the builder, you can test that relationship with:

```c
bbs_target_has_dependency(tgt, "preprocessor")
```

This is a convenient pattern because it keeps builder selection close to normal target definition syntax.

## Public API Usage

The public API for builders lives in:

```c
#include <bbs/build.h>
```

Useful helpers include:

- `bbs_print(...)`
- `bbs_warn(...)`
- `bbs_error(...)`
- `bbs_target_has_dependency(...)`
- `bbs_target_set_text(...)`
- `bbs_target_append_text(...)`
- `bbs_project_set_text(...)`
- `bbs_ctx_set_text(...)`
- the node inspection helpers for parsed `.bbs` trees

Example pattern:

```c
#include <bbs/build.h>

bool bbs_callback(bbs_sig signal, bbs_ctx* ctx, bbs_proj* prj, bbs_tgt* tgt) {
  (void)ctx;

  if (signal != BBS_SIG_PRE_BUILD || !prj || !tgt)
    return true;

  if (!bbs_target_has_dependency(tgt, "preprocessor"))
    return true;

  if (!bbs_target_set_text(tgt, BBS_TARGET_TEXT_DEFINES, "MY_DEFINE"))
    return false;

  return true;
}
```

The repository example uses the same pattern with a real define check:

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

## Included Example

This repository includes a working example in:

- [`examples/builders/`](../examples/builders/)

That example:

- defines a `preprocessor` builder
- declares `dependencies(preprocessor)` on the app target
- injects `PREPROCESSOR_ACTIVE` through the target `defines` property during the build phase
- proves it worked by requiring that define in `src/main.c`

You can try it with:

```bat
bbs build
bbs run -t my_app
```

Expected output:

```txt
builder define injected successfully
```

## Practical Guidance

Prefer builders when the behavior is:

- project-specific
- dynamic
- tied to one command phase
- awkward to express as a static config value

Prefer plain `project.bbs` attributes when the behavior is:

- static
- always enabled
- easy to describe directly in target settings

Good builder logic is usually narrow and local:

- inspect one builder dependency
- patch one or two target fields
- return

That keeps metacompilation understandable and avoids turning the project into an opaque custom build system.

## Current Status

Builders are already functional, but this area is still young.

Expect it to grow in places like:

- broader service API coverage
- richer persistence controls
- more builder metadata in config
- better ergonomics for composing project-local build pipelines

Even in the current state, builders are one of the strongest `bbs` features because they let you add project-specific build intelligence without modifying the core tool.
