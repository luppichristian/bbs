#pragma once

#include "bbs_cmd.h"

#define DEF_BUILD_DIR "build"
#define DEF_DIST_DIR  "dist"

typedef struct {
  const char* name;
  const char* copyfile;
} user_gen;

typedef struct {
  const char* builddir;
  const char* distdir;
  unsigned int auto_debounce_ms;
  unsigned int auto_retry_count;
  unsigned int auto_retry_delay_ms;
  const char* dist_archive_format;
  const char* dist_archive_name;
  const char* cmake_args;
  const char* cmake_build_args;
  const char* ctest_args;
  node* merged_scope;
  user_gen* gens;
  int gen_c;
} user;
