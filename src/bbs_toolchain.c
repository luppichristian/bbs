#pragma once
#include "bbs_toolchain.h"
#include "bbs_base.c"

static toolchain* toolchain_init(const char* path) {
  toolchain* tc = push(sizeof(toolchain));
  memset(tc, 0, sizeof(toolchain));
  return tc;
}
