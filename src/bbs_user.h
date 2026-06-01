#pragma once

#include "bbs_cmd.h"

#define DEF_BUILD_DIR "build"
#define DEF_DIST_DIR  "dist"

typedef struct {
  const char* builddir;
  const char* distdir;
} user;
