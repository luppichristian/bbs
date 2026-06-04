#!/usr/bin/env sh
set -eu

mkdir -p build

uname_s=$(uname -s)
static_flags=""
if [ "$uname_s" = "Linux" ]; then
  static_flags="-static -static-libgcc"
elif [ "$uname_s" = "Darwin" ]; then
  printf '%s\n' "warning: macOS does not support fully static system runtime linking; building without -static" >&2
fi

gcc -std=gnu11 -O3 -DNDEBUG $static_flags -Wall -Wpedantic -Wno-unused-function src/bbs.c -o build/bbs
