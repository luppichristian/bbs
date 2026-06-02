#pragma once
#include "bbs_base.h"
#include "bbs_toolchain.h"
#include "bbs_user.h"

typedef struct {
  ver ver;
  const char* id;
  const char* name;
  const char* repo;
  const char* authors;
  struct {
    const char* type;
    const char* file;
  } license;
} meta;

typedef enum {
  TARGET_TYPE_CONSOLE,
  TARGET_TYPE_CONSOLELESS,
  TARGET_TYPE_HEADER_LIB,
  TARGET_TYPE_STATIC_LIB,
  TARGET_TYPE_DYN_LIB,
  TARGET_TYPE_OBJ_LIB,
  TARGET_TYPE_TEST,
  TARGET_TYPE_DRIVER,
} target_type;

typedef enum {
  LANG_C,
  LANG_CPP,
} lang;

typedef enum {
  STDLIB_NONE,
  STDLIB_DYNAMIC,
  STDLIB_STATIC,
} stdlib;

typedef enum {
  WARNING_LEVEL_DEFAULT,
  WARNING_LEVEL_NONE,
  WARNING_LEVEL_LOW,
  WARNING_LEVEL_MEDIUM,
  WARNING_LEVEL_HIGH,
  WARNING_LEVEL_PEDANTIC,
} warning_level;

typedef enum {
  OPT_LEVEL_DEFAULT,
  OPT_LEVEL_NONE,
  OPT_LEVEL_DEBUG,
  OPT_LEVEL_SIZE,
  OPT_LEVEL_SPEED_1,
  OPT_LEVEL_SPEED_2,
  OPT_LEVEL_SPEED_3,
  OPT_LEVEL_AGGRESSIVE,
} opt_level;

typedef enum {
  PACKAGE_SOURCE_NONE,
  PACKAGE_SOURCE_PATH,
  PACKAGE_SOURCE_REPO,
  PACKAGE_SOURCE_ARCHIVE,
} package_source;

typedef enum {
  PACKAGE_BACKEND_NONE,
  PACKAGE_BACKEND_CMAKE,
  PACKAGE_BACKEND_BBS,
} package_backend;

typedef struct {
  meta meta;
  lang lang;
  target_type type;
  const char* output;

  package_source package_source;
  const char* package_path;
  const char* package_subdir;
  const char* package_repo_link;
  const char* package_repo_tag;
  const char* package_repo_commit;
  package_backend package_backend;
  const char* package_cmake_target;
  const char* package_archive_link;
  const char* package_archive_strip_prefix;
  const char* package_project_cfg_path;
  const char* package_resolved_dir;
  const char* package_cache_dir;
  const char* package_build_dir;

  // Translation units, should support wildcards
  const char** units;
  int unit_c;

  // Include and link dirs
  const char** include_dirs;
  int include_dir_c;
  const char** link_dirs;
  int link_dir_c;
  const char** dependencies;
  int dependency_c;

  // Preprocessor defines
  const char* defines;
  int define_c;

  // Only basenames, extension names are auto detected
  const char** link_libs;
  int link_libs_count;

  // Always specified in clang/gcc format, if calling msvc directly we need to translate
  // manually.
  const char* additional_compile_args;
  const char* additional_link_args;
  warning_level warning_level;
  opt_level opt_level;
  size_t stack_size;
  bool warnings_as_errors;

  // Other language configs
  stdlib runtime;
  const char* stdver;
  bool testing;
  const char** test_args;
  int test_arg_c;

  // In Bash so we are cross platform
  const char** post_build_cmds;
  const char** pre_build_cmds;
  const char** pre_run_cmds;
  const char** post_run_cmds;
  const char** pre_dist_cmds;
  const char** post_dist_cmds;
  int post_build_cmd_c;
  int pre_build_cmd_c;
  int pre_run_cmd_c;
  int post_run_cmd_c;
  int pre_dist_cmd_c;
  int post_dist_cmd_c;
  bool dist_archive;
  const char* dist_archive_name;
} target;

typedef struct {
  meta meta;
  user user_cfg;
  const char* root_dir;
  const char* config_path;
  const char* local_cfg_path;

  const char** configs;
  int config_c;
  const char* active_config;

  target* targets;
  int target_c;
  int target_cap;
} project;
