# bbs tester

This directory contains an isolated end-to-end test harness for `bbs`.

## Goals

- run the shipped examples without mutating `examples/` in-place
- keep the test toolchain cache, package cache, copied workspaces, and logs inside `tester/`
- cover additional negative and generator edge cases beyond the examples
- provide one command that CI can run after every push / pull request

## How isolation works

`tester/run.py` does **not** run the examples from the repository source tree.

Instead it:

1. copies the built `bbs` binary into `tester/.artifacts/runtime/bin/`
2. copies the example projects into `tester/.artifacts/work/examples/`
3. runs all commands from those copied sandboxes
4. writes command logs to `tester/.artifacts/logs/`

Because `bbs` stores its global cache next to the executable, copying the binary into `tester/.artifacts/runtime/bin/` keeps generated `toolchain.bbs`, package cache data, and related files out of the repository root.

## Local usage

Build `bbs` first, then run:

```bash
python tester/run.py --bbs build/bbs.exe
```

On Unix-like runners:

```bash
python tester/run.py --bbs build/ci-bbs/bbs
```

Useful options:

- `--list` to list cases
- `--filter <substring>` to run only a subset
- `--fail-fast` to stop on the first failure
- `--show-log-paths` to print the artifact/log directories even on success

## Coverage

The harness exercises:

- all shipped example projects under `examples/`
- builder examples
- config/dist examples
- package examples, including repo-backed and nested `project.bbs` dependencies
- CUDA example when the runner has a working CUDA environment
- failure-path checks for missing projects, unknown targets, and unknown platforms
- generator checks for `.gitignore`, GitHub workflow generation, and VSCode files

CUDA is treated as an environment-dependent case: the tester skips it when the runner cannot successfully execute the CUDA example, including missing toolkit detection and runtime/PTX toolchain mismatches.
