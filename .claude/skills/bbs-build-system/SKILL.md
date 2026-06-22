---
name: BBS Build System
description: Create, explain, validate, and repair projects that use the `bbs` build system and its `.bbs` syntax.
when_to_use: Use when working with `project.bbs`, choosing valid `bbs` target types and fields, emitting `bbs build/run/test/dist/update/package/gen/info/cfg` commands, modeling package-backed targets or `builders(...)`, or avoiding generic CMake assumptions for `bbs`-based C, C++, or CUDA projects.
---

Use `bbs` as the source of truth. Do not fall back to generic CMake advice unless the user explicitly asks for raw CMake.

## Quick Start

1. Detect whether you are working on a `bbs` project or on the `bbs` tool itself.
2. If the task involves `project.bbs`, read [references/bbs-reference.md](references/bbs-reference.md).
3. Emit valid `.bbs` tree syntax and valid `bbs` commands.
4. Prefer minimal, idiomatic `bbs` structures over verbose generated configs.
5. Validate that target ids, dependencies, configs, and command examples are internally consistent.

## Working Modes

### Working on a `bbs` Project

Assume:

- the project root is the directory containing `project.bbs`
- `bbs` generates the backend automatically
- `project.bbs` is the main authored file
- `build/` and `dist/` are generated output directories

Prefer these defaults unless the user requests otherwise:

- `console(...)` for a runnable CLI app
- `src/main.c` for the first source file
- `output(<project_id>)` when the binary name matters
- `configs(...)` plus `filter(...)` for debug or release differences
- `dependencies(...)` for other `bbs` targets
- `link_libs(...)` only for external or system libraries

Do not invent unsupported fields or target kinds. When the request is ambiguous, keep the output small and conventional.

### Working on the `bbs` Source Repository

If the repository contains `src/bbs.c`, `pub/bbs/build.h`, and top-level build scripts such as `build_clang.bat`, assume you are editing the `bbs` implementation itself.

In that case:

- prefer `build_clang.bat` from the repository root for validation
- preserve the single-translation-unit style where `.c` files are included as implementation units
- expect many `.c` files to use `#pragma once`
- treat `src/bbs.c` as the main aggregation point
- keep `pub/bbs/build.h` stable, clear, and documented
- do not add declarations to headers unless there is a concrete need

## Authoring Rules

When creating or updating `project.bbs`:

1. Use direct top-level nodes such as `id(...)`, `name(...)`, `ver(...)`, `targets(...)`.
2. Always include at least `id(...)` and `targets(...)`.
3. Use double-quoted strings where quoting is needed.
4. Put repeated items inside sections such as `units(...)`, `dependencies(...)`, `configs(...)`.
5. Keep target definitions focused on valid `bbs` concepts instead of CMake implementation details.

For detailed target types, fields, examples, and error-avoidance rules, read [references/bbs-reference.md](references/bbs-reference.md).

## Builders

Use `builders(...)` only for dynamic command-time logic that must mutate project or target state before or after build phases.

When generating builder code:

- include `#include <bbs/build.h>`
- export `bool bbs_callback(bbs_sig signal, bbs_ctx* ctx, bbs_proj* prj, bbs_tgt* tgt)`
- return `false` only when the command should fail
- prefer typed builder APIs over raw string rewriting

## Command Emission

Emit `bbs` commands from the project root. Prefer:

```bat
bbs build
bbs run
bbs test
bbs dist
```

Use explicit flags only when needed, such as `-t`, `-c`, `-p`, `--init-toolchain`, or `--refresh-packages`.

## Final Check

Before finishing:

1. Confirm the syntax is valid `.bbs` tree syntax, not JSON, YAML, or CMake.
2. Confirm every dependency name refers to a `bbs` target or builder id unless it belongs in `link_libs(...)`.
3. Confirm every suggested command is a real `bbs` command.
4. Confirm any repo edits still fit the `bbs` source layout and build flow.
