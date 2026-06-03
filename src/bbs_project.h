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
  const char** selectors;
  int selector_c;
} target_unity_batch;

typedef enum {
  TARGET_HOOK_POST_BUILD,
  TARGET_HOOK_PRE_BUILD,
  TARGET_HOOK_PRE_RUN,
  TARGET_HOOK_POST_RUN,
  TARGET_HOOK_PRE_DIST,
  TARGET_HOOK_POST_DIST,
  TARGET_HOOK_MAX,
} target_hook_kind;

typedef struct {
  target_hook_kind kind;
  const char* attr_name;
  const char* label;
} target_hook_info;

static const target_hook_info TARGET_HOOK_INFOS[] = {
    {.kind = TARGET_HOOK_POST_BUILD, .attr_name = "post_build_cmds", .label = "Post Build Cmds"},
    {.kind = TARGET_HOOK_PRE_BUILD, .attr_name = "pre_build_cmds", .label = "Pre Build Cmds"},
    {.kind = TARGET_HOOK_PRE_RUN, .attr_name = "pre_run_cmds", .label = "Pre Run Cmds"},
    {.kind = TARGET_HOOK_POST_RUN, .attr_name = "post_run_cmds", .label = "Post Run Cmds"},
    {.kind = TARGET_HOOK_PRE_DIST, .attr_name = "pre_dist_cmds", .label = "Pre Dist Cmds"},
    {.kind = TARGET_HOOK_POST_DIST, .attr_name = "post_dist_cmds", .label = "Post Dist Cmds"},
};

typedef struct {
  bool archive;
  const char* archive_name;
} target_dist_config;

typedef struct {
  const char* name;
} project_attr_info;

typedef struct {
  const char* name;
  target_type type;
} project_target_type_alias;

typedef struct {
  const char* name;
  lang value;
} project_lang_alias;

typedef struct {
  const char* name;
  stdlib value;
} project_stdlib_alias;

typedef struct {
  const char* name;
  warning_level value;
} project_warning_level_alias;

typedef struct {
  const char* name;
  opt_level value;
} project_opt_level_alias;

typedef struct {
  const char* name;
  const char* cmake_name;
} project_cmake_config_alias;

static const project_target_type_alias PROJECT_TARGET_TYPE_ALIASES[] = {
    {.name = "console", .type = TARGET_TYPE_CONSOLE},
    {.name = "consoleless", .type = TARGET_TYPE_CONSOLELESS},
    {.name = "gui", .type = TARGET_TYPE_CONSOLELESS},
    {.name = "header_lib", .type = TARGET_TYPE_HEADER_LIB},
    {.name = "header-lib", .type = TARGET_TYPE_HEADER_LIB},
    {.name = "headerlib", .type = TARGET_TYPE_HEADER_LIB},
    {.name = "static_lib", .type = TARGET_TYPE_STATIC_LIB},
    {.name = "static-lib", .type = TARGET_TYPE_STATIC_LIB},
    {.name = "staticlib", .type = TARGET_TYPE_STATIC_LIB},
    {.name = "dyn_lib", .type = TARGET_TYPE_DYN_LIB},
    {.name = "dyn-lib", .type = TARGET_TYPE_DYN_LIB},
    {.name = "shared_lib", .type = TARGET_TYPE_DYN_LIB},
    {.name = "shared-lib", .type = TARGET_TYPE_DYN_LIB},
    {.name = "obj_lib", .type = TARGET_TYPE_OBJ_LIB},
    {.name = "obj-lib", .type = TARGET_TYPE_OBJ_LIB},
    {.name = "object_lib", .type = TARGET_TYPE_OBJ_LIB},
    {.name = "object-lib", .type = TARGET_TYPE_OBJ_LIB},
    {.name = "test", .type = TARGET_TYPE_TEST},
    {.name = "driver", .type = TARGET_TYPE_DRIVER},
};

static const project_lang_alias PROJECT_LANG_ALIASES[] = {
    {.name = "cpp", .value = LANG_CPP},
    {.name = "c++", .value = LANG_CPP},
};

static const project_stdlib_alias PROJECT_STDLIB_ALIASES[] = {
    {.name = "none", .value = STDLIB_NONE},
    {.name = "static", .value = STDLIB_STATIC},
    {.name = "dynamic", .value = STDLIB_DYNAMIC},
};

static const project_warning_level_alias PROJECT_WARNING_LEVEL_ALIASES[] = {
    {.name = "default", .value = WARNING_LEVEL_DEFAULT},
    {.name = "none", .value = WARNING_LEVEL_NONE},
    {.name = "off", .value = WARNING_LEVEL_NONE},
    {.name = "low", .value = WARNING_LEVEL_LOW},
    {.name = "minimal", .value = WARNING_LEVEL_LOW},
    {.name = "medium", .value = WARNING_LEVEL_MEDIUM},
    {.name = "med", .value = WARNING_LEVEL_MEDIUM},
    {.name = "normal", .value = WARNING_LEVEL_MEDIUM},
    {.name = "high", .value = WARNING_LEVEL_HIGH},
    {.name = "all", .value = WARNING_LEVEL_HIGH},
    {.name = "pedantic", .value = WARNING_LEVEL_PEDANTIC},
    {.name = "extra", .value = WARNING_LEVEL_PEDANTIC},
};

static const project_opt_level_alias PROJECT_OPT_LEVEL_ALIASES[] = {
    {.name = "default", .value = OPT_LEVEL_DEFAULT},
    {.name = "none", .value = OPT_LEVEL_NONE},
    {.name = "off", .value = OPT_LEVEL_NONE},
    {.name = "o0", .value = OPT_LEVEL_NONE},
    {.name = "debug", .value = OPT_LEVEL_DEBUG},
    {.name = "og", .value = OPT_LEVEL_DEBUG},
    {.name = "size", .value = OPT_LEVEL_SIZE},
    {.name = "os", .value = OPT_LEVEL_SIZE},
    {.name = "oz", .value = OPT_LEVEL_SIZE},
    {.name = "speed_1", .value = OPT_LEVEL_SPEED_1},
    {.name = "speed-1", .value = OPT_LEVEL_SPEED_1},
    {.name = "o1", .value = OPT_LEVEL_SPEED_1},
    {.name = "speed_2", .value = OPT_LEVEL_SPEED_2},
    {.name = "speed-2", .value = OPT_LEVEL_SPEED_2},
    {.name = "o2", .value = OPT_LEVEL_SPEED_2},
    {.name = "speed_3", .value = OPT_LEVEL_SPEED_3},
    {.name = "speed-3", .value = OPT_LEVEL_SPEED_3},
    {.name = "o3", .value = OPT_LEVEL_SPEED_3},
    {.name = "aggressive", .value = OPT_LEVEL_AGGRESSIVE},
    {.name = "fast", .value = OPT_LEVEL_AGGRESSIVE},
    {.name = "ofast", .value = OPT_LEVEL_AGGRESSIVE},
};

static const project_cmake_config_alias PROJECT_CMAKE_CONFIG_ALIASES[] = {
    {.name = "release", .cmake_name = "Release"},
    {.name = "dist", .cmake_name = "Release"},
    {.name = "shipping", .cmake_name = "Release"},
    {.name = "minsizerel", .cmake_name = "MinSizeRel"},
    {.name = "minsize", .cmake_name = "MinSizeRel"},
    {.name = "relwithdebinfo", .cmake_name = "RelWithDebInfo"},
    {.name = "profile", .cmake_name = "RelWithDebInfo"},
};

static const project_attr_info PROJECT_LICENSE_ATTR_INFOS[] = {
    {.name = "type"},
    {.name = "file"},
};

static const project_attr_info PROJECT_UNITY_ATTR_INFOS[] = {
    {.name = "enabled"},
    {.name = "batch_size"},
    {.name = "batch"},
};

static const project_attr_info PROJECT_TARGET_META_ATTR_INFOS[] = {
    {.name = "id"},
    {.name = "name"},
    {.name = "authors"},
    {.name = "ver"},
    {.name = "license"},
};

static const project_attr_info PROJECT_TARGET_ATTR_INFOS[] = {
    {.name = "lang"},
    {.name = "output"},
    {.name = "path"},
    {.name = "subdir"},
    {.name = "cmake_target"},
    {.name = "repo"},
    {.name = "archive"},
    {.name = "units"},
    {.name = "include_dirs"},
    {.name = "link_dirs"},
    {.name = "dependencies"},
    {.name = "link_libs"},
    {.name = "defines"},
    {.name = "additional_compile_args"},
    {.name = "additional_link_args"},
    {.name = "warning_level"},
    {.name = "opt_level"},
    {.name = "stack_size"},
    {.name = "warnings_as_errors"},
    {.name = "runtime"},
    {.name = "stdver"},
    {.name = "testing"},
    {.name = "test_args"},
    {.name = "post_build_cmds"},
    {.name = "pre_build_cmds"},
    {.name = "pre_run_cmds"},
    {.name = "post_run_cmds"},
    {.name = "pre_dist_cmds"},
    {.name = "post_dist_cmds"},
    {.name = "dist"},
    {.name = "unity"},
};

static const project_attr_info PROJECT_PROJECT_ATTR_INFOS[] = {
    {.name = "id"},
    {.name = "name"},
    {.name = "authors"},
    {.name = "repo"},
    {.name = "ver"},
    {.name = "license"},
    {.name = "configs"},
    {.name = "filter"},
    {.name = "targets"},
};

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
  bool unity_configured;
  bool unity_enabled;
  bool unity_batch_size_set;
  size_t unity_batch_size;
  target_unity_batch* unity_batches;
  int unity_batch_c;

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
  const char** hook_cmds[TARGET_HOOK_MAX];
  int hook_cmd_counts[TARGET_HOOK_MAX];
  target_dist_config dist;
} target;

typedef struct {
  meta meta;
  user user_cfg;
  node* config_tree;
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

static const char** target_hook_cmds(const target* tgt, target_hook_kind kind) {
  return tgt && kind >= 0 && kind < TARGET_HOOK_MAX ? tgt->hook_cmds[kind] : NULL;
}

static int target_hook_cmd_count(const target* tgt, target_hook_kind kind) {
  return tgt && kind >= 0 && kind < TARGET_HOOK_MAX ? tgt->hook_cmd_counts[kind] : 0;
}
