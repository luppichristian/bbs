#pragma once

#include "bbs_cmd.h"

#define DEF_BUILD_DIR "build"
#define DEF_DIST_DIR  "dist"

typedef struct {
  const char* name;
  const char* copyfile;
} user_gen;

typedef enum {
  USER_ATTR_BUILDDIR,
  USER_ATTR_DISTDIR,
  USER_ATTR_AUTO_DEBOUNCE_MS,
  USER_ATTR_AUTO_RETRY_COUNT,
  USER_ATTR_AUTO_RETRY_DELAY_MS,
  USER_ATTR_DIST_ARCHIVE_FORMAT,
  USER_ATTR_DIST_ARCHIVE_NAME,
  USER_ATTR_CMAKE_ARGS,
  USER_ATTR_CMAKE_BUILD_ARGS,
  USER_ATTR_CTEST_ARGS,
  USER_ATTR_GEN,
} user_attr;

typedef enum {
  USER_TEXT_BUILDDIR,
  USER_TEXT_DISTDIR,
  USER_TEXT_DIST_ARCHIVE_FORMAT,
  USER_TEXT_DIST_ARCHIVE_NAME,
  USER_TEXT_CMAKE_ARGS,
  USER_TEXT_CMAKE_BUILD_ARGS,
  USER_TEXT_CTEST_ARGS,
  USER_TEXT_MAX,
} user_text_attr;

typedef enum {
  USER_UINT_AUTO_DEBOUNCE_MS,
  USER_UINT_AUTO_RETRY_COUNT,
  USER_UINT_AUTO_RETRY_DELAY_MS,
  USER_UINT_MAX,
} user_uint_attr;

typedef enum {
  USER_ATTR_KIND_TEXT,
  USER_ATTR_KIND_UINT,
  USER_ATTR_KIND_ARCHIVE_FORMAT,
  USER_ATTR_KIND_SECTION,
} user_attr_kind;

typedef struct {
  user_attr id;
  const char* name;
  user_attr_kind kind;
} user_attr_info;

static const user_attr_info USER_ATTR_INFOS[] = {
    {USER_ATTR_BUILDDIR, "builddir", USER_ATTR_KIND_TEXT},
    {USER_ATTR_DISTDIR, "distdir", USER_ATTR_KIND_TEXT},
    {USER_ATTR_AUTO_DEBOUNCE_MS, "auto_debounce_ms", USER_ATTR_KIND_UINT},
    {USER_ATTR_AUTO_RETRY_COUNT, "auto_retry_count", USER_ATTR_KIND_UINT},
    {USER_ATTR_AUTO_RETRY_DELAY_MS, "auto_retry_delay_ms", USER_ATTR_KIND_UINT},
    {USER_ATTR_DIST_ARCHIVE_FORMAT, "dist_archive_format", USER_ATTR_KIND_ARCHIVE_FORMAT},
    {USER_ATTR_DIST_ARCHIVE_NAME, "dist_archive_name", USER_ATTR_KIND_TEXT},
    {USER_ATTR_CMAKE_ARGS, "cmake_args", USER_ATTR_KIND_TEXT},
    {USER_ATTR_CMAKE_BUILD_ARGS, "cmake_build_args", USER_ATTR_KIND_TEXT},
    {USER_ATTR_CTEST_ARGS, "ctest_args", USER_ATTR_KIND_TEXT},
    {USER_ATTR_GEN, "gen", USER_ATTR_KIND_SECTION},
};

typedef enum {
  USER_GEN_ATTR_NAME,
  USER_GEN_ATTR_COPYFILE,
} user_gen_attr;

typedef struct {
  user_gen_attr id;
  const char* name;
} user_gen_attr_info;

static const user_gen_attr_info USER_GEN_ATTR_INFOS[] = {
    {USER_GEN_ATTR_NAME, "name"},
    {USER_GEN_ATTR_COPYFILE, "copyfile"},
};

typedef struct {
  const char* text_values[USER_TEXT_MAX];
  unsigned int uint_values[USER_UINT_MAX];
  node* merged_scope;
  user_gen* gens;
  int gen_c;
} user;

static const char* user_text(const user* u, user_text_attr attr) {
  return u && attr >= 0 && attr < USER_TEXT_MAX ? u->text_values[attr] : NULL;
}

static unsigned int user_uint(const user* u, user_uint_attr attr) {
  return u && attr >= 0 && attr < USER_UINT_MAX ? u->uint_values[attr] : 0;
}
