# AGENTS.md

## Build

- Build the project with `build_clang.bat` from the repository root.
- Prefer `build_clang.bat` for validation after code changes.

## Source Layout

- This project uses `.c` files as implementation includes, similar to headers.
- Many `.c` files use `#pragma once` and are included from other `.c` files.
- `src/bbs.c` is the main aggregation point and includes the implementation units directly.
- `pub/` contains the public API surface intended for external users and builder modules.
- `pub/bbs/build.h` is the main public reference header and should stay clear, documented, and stable-minded.

## Function Declarations

- Do not add function declarations to headers unless there is a specific need.
- Functions are typically made available by including the relevant `.c` implementation file.
- When adding a new implementation unit, follow the existing pattern: add `#pragma once`, include dependencies, and include the file where needed.

## Editing Guidance

- Preserve the existing single-translation-unit style.
- Prefer minimal changes that match the current include-based architecture.
- If you add a new `.c` file with shared functionality, include it from the implementation units that need it.
