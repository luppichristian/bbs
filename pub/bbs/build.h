#pragma once

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BBS_ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

#ifdef __cplusplus
extern "C" {
#endif

/* Public builder API version, not the built project's version. */
#define BBS_VERSION_MAJOR     0u
#define BBS_VERSION_MINOR     2u
#define BBS_BUILD_API_VERSION 1u

#if defined(_WIN32) || defined(__CYGWIN__)
#  define BBS_EXPORT __declspec(dllexport)
#  define BBS_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#  define BBS_EXPORT __attribute__((visibility("default")))
#  define BBS_IMPORT __attribute__((visibility("default")))
#else
#  define BBS_EXPORT
#  define BBS_IMPORT
#endif

#ifndef BBS_INTERNAL
#  define BBS_USER_API    extern BBS_EXPORT
#  define BBS_SERVICE_API extern BBS_IMPORT
#else
#  define BBS_USER_API    extern BBS_IMPORT
#  define BBS_SERVICE_API extern BBS_EXPORT
#endif

/*
  Export/import model:

  - Builder modules export `bbs_callback(...)` with `BBS_USER_API`.
  - `bbs.exe` exports host helper functions with `BBS_SERVICE_API`.
  - When building the host itself, `BBS_INTERNAL` flips the direction so the
    same header can be shared by both sides.
*/

/* Commands accepted by `bbs`. */
typedef enum {
  BBS_CMD_UNKNOWN = -1,
  BBS_CMD_HELP = 0,
  BBS_CMD_CLEAN,
  BBS_CMD_UPDATE,
  BBS_CMD_GEN,
  BBS_CMD_CFG,
  BBS_CMD_BUILD,
  BBS_CMD_AUTO,
  BBS_CMD_RUN,
  BBS_CMD_INFO,
  BBS_CMD_PACKAGE,
  BBS_CMD_DIST,
  BBS_CMD_TEST,
  BBS_CMD_BUMPVER,
  BBS_CMD_MAX,
} bbs_cmd;

/* Known `.bbs` config files. */
typedef enum {
  BBS_CFG_PROJECT = 0,
  BBS_CFG_USER,
  BBS_CFG_LOCAL,
  BBS_CFG_TOOLCHAIN,
  BBS_CFG_MAX,
} bbs_cfg;

typedef enum {
  BBS_CFG_LOC_CWD = 0,
  BBS_CFG_LOC_EXE,
} bbs_cfg_loc;

/* Static description of one config file kind. */
/* Simple 3-part or 4-part version value. */
typedef struct {
  bbs_cfg_loc loc;
  const char* filename;
  const char* desc;
  const char* detailed_desc;
} bbs_cfg_info;

/* Static description of one CLI command. */
/* Static toolchain config schema entry. */
typedef struct {
  const char* name;
  const char* params;
  const char* desc;
  const char* detailed_desc;
} bbs_cmd_info;

/*
  Callback lifecycle signals.

  Generic signals wrap the full command.
  Command-specific signals expose important phases such as build/run/dist.
*/
typedef enum {
  BBS_SIG_INIT = 0,
  BBS_SIG_QUIT,
  BBS_SIG_PRE_CMD,
  BBS_SIG_POST_CMD,
  BBS_SIG_PRE_BUILD,
  BBS_SIG_POST_BUILD,
  BBS_SIG_PRE_DIST,
  BBS_SIG_POST_DIST,
  BBS_SIG_PRE_RUN,
  BBS_SIG_POST_RUN,
  BBS_SIG_PRE_TEST,
  BBS_SIG_POST_TEST,
  BBS_SIG_PRE_UPDATE,
  BBS_SIG_POST_UPDATE,
  BBS_SIG_PRE_PACKAGE,
  BBS_SIG_POST_PACKAGE,
  BBS_SIG_PRE_GEN,
  BBS_SIG_POST_GEN,
  BBS_SIG_PRE_CFG,
  BBS_SIG_POST_CFG,
  BBS_SIG_PRE_CLEAN,
  BBS_SIG_POST_CLEAN,
  BBS_SIG_PRE_INFO,
  BBS_SIG_POST_INFO,
  BBS_SIG_PRE_BUMPVER,
  BBS_SIG_POST_BUMPVER,
  BBS_SIG_PRE_AUTO,
  BBS_SIG_POST_AUTO,
  BBS_SIG_MAX,
} bbs_sig;

/* Host / target operating systems known to `bbs`. */
typedef enum {
  BBS_OS_WINDOWS = 0,
  BBS_OS_LINUX,
  BBS_OS_MACOS,
  BBS_OS_MAX,
} bbs_os;

/* Host / target CPU architectures known to `bbs`. */
typedef enum {
  BBS_ARCH_X86_64 = 0,
  BBS_ARCH_X86,
  BBS_ARCH_ARM64,
  BBS_ARCH_MAX,
} bbs_arch;

/* Built-in target kinds supported by `project.bbs`. */
typedef enum {
  BBS_TARGET_CONSOLE = 0,
  BBS_TARGET_CONSOLELESS,
  BBS_TARGET_HEADER_LIB,
  BBS_TARGET_STATIC_LIB,
  BBS_TARGET_DYN_LIB,
  BBS_TARGET_OBJ_LIB,
  BBS_TARGET_TEST,
  BBS_TARGET_DRIVER,
  BBS_TARGET_MAX,
} bbs_target_type;

/* Source language used by a target or builder. */
typedef enum {
  BBS_LANG_C = 0,
  BBS_LANG_CPP,
  BBS_LANG_MAX,
} bbs_lang;

/* Runtime linkage preference for compiled targets. */
typedef enum {
  BBS_STDLIB_NONE = 0,
  BBS_STDLIB_DYNAMIC,
  BBS_STDLIB_STATIC,
  BBS_STDLIB_MAX,
} bbs_stdlib;

/* Warning policy used by generated backend files. */
typedef enum {
  BBS_WARNING_DEFAULT = 0,
  BBS_WARNING_NONE,
  BBS_WARNING_LOW,
  BBS_WARNING_MEDIUM,
  BBS_WARNING_HIGH,
  BBS_WARNING_PEDANTIC,
  BBS_WARNING_MAX,
} bbs_warning_level;

/* Optimization policy used by generated backend files. */
typedef enum {
  BBS_OPT_DEFAULT = 0,
  BBS_OPT_NONE,
  BBS_OPT_DEBUG,
  BBS_OPT_SIZE,
  BBS_OPT_SPEED_1,
  BBS_OPT_SPEED_2,
  BBS_OPT_SPEED_3,
  BBS_OPT_AGGRESSIVE,
  BBS_OPT_MAX,
} bbs_opt_level;

/* External package source kind for a package-backed target. */
typedef enum {
  BBS_PACKAGE_NONE = 0,
  BBS_PACKAGE_PATH,
  BBS_PACKAGE_REPO,
  BBS_PACKAGE_ARCHIVE,
  BBS_PACKAGE_MAX,
} bbs_package_source;

/* Backend used when consuming a package-backed target. */
typedef enum {
  BBS_PACKAGE_BACKEND_NONE = 0,
  BBS_PACKAGE_BACKEND_CMAKE,
  BBS_PACKAGE_BACKEND_BBS,
  BBS_PACKAGE_BACKEND_MAX,
} bbs_package_backend;

/* Internal schema kinds used by toolchain config metadata tables. */
typedef enum {
  BBS_TOOLCHAIN_ATTR_STRING = 0,
  BBS_TOOLCHAIN_ATTR_IDENTIFIER,
  BBS_TOOLCHAIN_ATTR_SECTION,
} bbs_toolchain_attr_kind;

/* Parsed node kind in `.bbs` syntax trees. */
typedef enum {
  BBS_NODE_DEF = 0,
  BBS_NODE_STR,
  BBS_NODE_INT,
  BBS_NODE_FLT,
  BBS_NODE_VER,
  BBS_NODE_IDF,
  BBS_NODE_BOL,
  BBS_NODE_MAX,
} bbs_node_type;

typedef enum {
  BBS_USER_TEXT_BUILD_DIR = 0,
  BBS_USER_TEXT_ASSETS_DIR,
  BBS_USER_TEXT_DIST_DIR,
  BBS_USER_TEXT_DIST_ARCHIVE_FORMAT,
  BBS_USER_TEXT_DIST_ARCHIVE_NAME,
  BBS_USER_TEXT_CMAKE_ARGS,
  BBS_USER_TEXT_CMAKE_BUILD_ARGS,
  BBS_USER_TEXT_CTEST_ARGS,
  BBS_USER_TEXT_MAX,
} bbs_user_text_attr;

typedef enum {
  BBS_USER_UINT_AUTO_DEBOUNCE_MS = 0,
  BBS_USER_UINT_AUTO_RETRY_COUNT,
  BBS_USER_UINT_AUTO_RETRY_DELAY_MS,
  BBS_USER_UINT_MAX,
} bbs_user_uint_attr;

typedef enum {
  BBS_USER_ATTR_BUILD_DIR = 0,
  BBS_USER_ATTR_ASSETS_DIR,
  BBS_USER_ATTR_DIST_DIR,
  BBS_USER_ATTR_AUTO_DEBOUNCE_MS,
  BBS_USER_ATTR_AUTO_RETRY_COUNT,
  BBS_USER_ATTR_AUTO_RETRY_DELAY_MS,
  BBS_USER_ATTR_DIST_ARCHIVE_FORMAT,
  BBS_USER_ATTR_DIST_ARCHIVE_NAME,
  BBS_USER_ATTR_CMAKE_ARGS,
  BBS_USER_ATTR_CMAKE_BUILD_ARGS,
  BBS_USER_ATTR_CTEST_ARGS,
  BBS_USER_ATTR_GEN,
} bbs_user_attr;

typedef enum {
  BBS_USER_ATTR_KIND_TEXT = 0,
  BBS_USER_ATTR_KIND_UINT,
  BBS_USER_ATTR_KIND_ARCHIVE_FORMAT,
  BBS_USER_ATTR_KIND_SECTION,
} bbs_user_attr_kind;

typedef enum {
  BBS_HOOK_POST_BUILD = 0,
  BBS_HOOK_PRE_BUILD,
  BBS_HOOK_PRE_RUN,
  BBS_HOOK_POST_RUN,
  BBS_HOOK_PRE_DIST,
  BBS_HOOK_POST_DIST,
  BBS_HOOK_MAX,
} bbs_hook_kind;

typedef enum {
  BBS_LIST_UNITS = 0,
  BBS_LIST_INCLUDE_DIRS,
  BBS_LIST_LINK_DIRS,
  BBS_LIST_DEPENDENCIES,
  BBS_LIST_LINK_LIBS,
  BBS_LIST_TEST_ARGS,
  BBS_LIST_HOOK_CMDS,
  BBS_LIST_CONFIGS,
  BBS_LIST_CMD_ARGS,
  BBS_LIST_MAX,
} bbs_list_kind;

typedef enum {
  BBS_CTX_TEXT_WORKDIR = 0,
  BBS_CTX_TEXT_SELECTED_TARGET,
  BBS_CTX_TEXT_SELECTED_PLATFORM,
  BBS_CTX_TEXT_SELECTED_CONFIG,
  BBS_CTX_TEXT_MAX,
} bbs_ctx_text_field;

typedef enum {
  BBS_PROJECT_TEXT_ID = 0,
  BBS_PROJECT_TEXT_NAME,
  BBS_PROJECT_TEXT_REPO,
  BBS_PROJECT_TEXT_AUTHORS,
  BBS_PROJECT_TEXT_LICENSE_TYPE,
  BBS_PROJECT_TEXT_LICENSE_FILE,
  BBS_PROJECT_TEXT_ACTIVE_CONFIG,
  BBS_PROJECT_TEXT_BUILD_DIR,
  BBS_PROJECT_TEXT_ASSETS_DIR,
  BBS_PROJECT_TEXT_DIST_DIR,
  BBS_PROJECT_TEXT_DIST_ARCHIVE_FORMAT,
  BBS_PROJECT_TEXT_DIST_ARCHIVE_NAME,
  BBS_PROJECT_TEXT_CMAKE_ARGS,
  BBS_PROJECT_TEXT_CMAKE_BUILD_ARGS,
  BBS_PROJECT_TEXT_CTEST_ARGS,
  BBS_PROJECT_TEXT_MAX,
} bbs_project_text_field;

typedef enum {
  BBS_TARGET_TEXT_ID = 0,
  BBS_TARGET_TEXT_NAME,
  BBS_TARGET_TEXT_REPO,
  BBS_TARGET_TEXT_AUTHORS,
  BBS_TARGET_TEXT_LICENSE_TYPE,
  BBS_TARGET_TEXT_LICENSE_FILE,
  BBS_TARGET_TEXT_OUTPUT,
  BBS_TARGET_TEXT_PACKAGE_PATH,
  BBS_TARGET_TEXT_PACKAGE_SUBDIR,
  BBS_TARGET_TEXT_PACKAGE_REPO_LINK,
  BBS_TARGET_TEXT_PACKAGE_REPO_TAG,
  BBS_TARGET_TEXT_PACKAGE_REPO_COMMIT,
  BBS_TARGET_TEXT_PACKAGE_CMAKE_TARGET,
  BBS_TARGET_TEXT_PACKAGE_ARCHIVE_LINK,
  BBS_TARGET_TEXT_PACKAGE_ARCHIVE_STRIP_PREFIX,
  BBS_TARGET_TEXT_PACKAGE_PROJECT_CFG_PATH,
  BBS_TARGET_TEXT_PACKAGE_RESOLVED_DIR,
  BBS_TARGET_TEXT_PACKAGE_CACHE_DIR,
  BBS_TARGET_TEXT_PACKAGE_BUILD_DIR,
  BBS_TARGET_TEXT_DEFINES,
  BBS_TARGET_TEXT_ADDITIONAL_COMPILE_ARGS,
  BBS_TARGET_TEXT_ADDITIONAL_LINK_ARGS,
  BBS_TARGET_TEXT_STDVER,
  BBS_TARGET_TEXT_DIST_ARCHIVE_NAME,
  BBS_TARGET_TEXT_MAX,
} bbs_target_text_field;

enum {
  BBS_CTXF_HOTBUILD_MODE = 1u << 0,
  BBS_CTXF_COMMAND_LOCAL_MUTATIONS = 1u << 1,
  BBS_CTXF_IN_TARGET_SCOPE = 1u << 2,
  BBS_CTXF_IN_PROJECT_SCOPE = 1u << 3,
};

enum {
  BBS_BUILDERF_NONE = 0u,
  BBS_BUILDERF_MUTATES_CONTEXT = 1u << 0,
  BBS_BUILDERF_MUTATES_PROJECT = 1u << 1,
  BBS_BUILDERF_MUTATES_TARGETS = 1u << 2,
  BBS_BUILDERF_ADDS_TARGETS = 1u << 3,
  BBS_BUILDERF_REMOVES_TARGETS = 1u << 4,
  BBS_BUILDERF_ADDS_CONFIGS = 1u << 5,
  BBS_BUILDERF_RUNS_COMMANDS = 1u << 6,
};

/* Built-in executable discovery strategy. */
typedef struct {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
  uint8_t user;
} bbs_ver;

/* Built-in SDK discovery strategy. */
typedef struct {
  const char* name;
  bbs_toolchain_attr_kind kind;
  bool required;
} bbs_toolchain_attr_info;

/* Small static metadata entry used by public attribute tables. */
typedef struct {
  const char* id;
  bbs_os target_os;
  const char* exe_name;
  const char* dir_hints;
  const char* deep_roots;
  const char* version_arg;
  const char* version_regex;
  const char* version_arg_fallback;
  const char* version_regex_fallback;
} bbs_tool_discover_strat;

/* Generic string-list container used by a few uniform helper APIs. */
typedef struct {
  const char* id;
  bbs_os target_os;
  const char* env_vars;
  const char* root_hints;
  const char* include_rel;
  const char* source_rel;
  const char* lib_rel;
  const char* bin_rel;
  const char* version_file_rel;
  const char* version_regex;
} bbs_sdk_discover_strat;

typedef struct {
  const char* name;
} bbs_project_attr_info;

typedef struct {
  const char* name;
  bbs_target_type type;
} bbs_project_target_type_alias;

typedef struct {
  const char* name;
  bbs_lang value;
} bbs_project_lang_alias;

typedef struct {
  const char* name;
  bbs_stdlib value;
} bbs_project_stdlib_alias;

typedef struct {
  const char* name;
  bbs_warning_level value;
} bbs_project_warning_level_alias;

typedef struct {
  const char* name;
  bbs_opt_level value;
} bbs_project_opt_level_alias;

typedef struct {
  const char* name;
  const char* cmake_name;
} bbs_project_cmake_config_alias;

typedef struct {
  bbs_hook_kind kind;
  const char* attr_name;
  const char* label;
} bbs_hook_info;

typedef struct {
  bbs_user_attr id;
  const char* name;
  bbs_user_attr_kind kind;
} bbs_user_attr_info;

typedef struct {
  const char* name;
} bbs_user_gen_attr_info;

typedef struct {
  const char** items;
  size_t count;
  size_t capacity;
} bbs_strlist;

static const char* BBS_OS_NAMES[BBS_OS_MAX] = {
    [BBS_OS_WINDOWS] = "windows",
    [BBS_OS_LINUX] = "linux",
    [BBS_OS_MACOS] = "macos",
};

static const char* BBS_ARCH_NAMES[BBS_ARCH_MAX] = {
    [BBS_ARCH_X86_64] = "x86_64",
    [BBS_ARCH_X86] = "x86",
    [BBS_ARCH_ARM64] = "arm64",
};

static const char* BBS_NODE_TYPE_NAMES[BBS_NODE_MAX] = {
    [BBS_NODE_DEF] = "def",
    [BBS_NODE_STR] = "str",
    [BBS_NODE_INT] = "int",
    [BBS_NODE_FLT] = "flt",
    [BBS_NODE_VER] = "ver",
    [BBS_NODE_IDF] = "idf",
    [BBS_NODE_BOL] = "bol",
};

static const bbs_cfg_info BBS_CFG_INFOS[BBS_CFG_MAX] = {
    [BBS_CFG_PROJECT] = {.loc = BBS_CFG_LOC_CWD,
                         .filename = "project.bbs",
                         .desc = "Project definition",
                         .detailed_desc = "Defines the project, its targets, supported platforms, and build settings.\n"
                         "Keep it in the project root.\n"
                         "bbs reads this file first, then applies defaults from 'user.bbs' and overrides from 'local.bbs' when present."},
    [BBS_CFG_USER] = {.loc = BBS_CFG_LOC_EXE,
                         .filename = "user.bbs",
                         .desc = "Shared user defaults",
                         .detailed_desc = "Stores your default settings across projects.\n"
                         "Keep it next to the bbs executable.\n"
                         "Values from 'user.bbs' can be overridden by 'local.bbs' in a specific project."                               },
    [BBS_CFG_LOCAL] = {.loc = BBS_CFG_LOC_CWD,
                         .filename = "local.bbs",
                         .desc = "Machine-local overrides",
                         .detailed_desc = "Stores machine-specific overrides for a single project.\n"
                         "Keep it in the project root next to 'project.bbs'.\n"
                         "Use it for values that should not be shared.\n"
                         "In most projects this file should be added to .gitignore."                                                    },
    [BBS_CFG_TOOLCHAIN] = {.loc = BBS_CFG_LOC_EXE,
                         .filename = "toolchain.bbs",
                         .desc = "Generated toolchain cache",
                         .detailed_desc = "Caches detected toolchain state.\n"
                         "bbs creates and refreshes this file while preparing the build environment.\n"
                         "Keep it next to the bbs executable.\n"
                         "Regenerate it with 'bbs update --init-toolchain' after local toolchain changes."                              },
};

static const bbs_cmd_info BBS_CMD_INFOS[BBS_CMD_MAX] = {
    [BBS_CMD_HELP] = {   .name = "help",                                                                               .params = "[command/topic]",         .desc = "Show help for commands and configs",                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          .detailed_desc = "Without arguments, lists all available commands and config topics.\nPass a command or config name to show its usage and details."},
    [BBS_CMD_CLEAN] = {  .name = "clean",                                                                .params = "[project|user|local|toolchain]",           .desc = "Remove selected bbs config files",                                                                                                                                                                                                                                                                                                                                                                                                                                        .detailed_desc = "Removes one selected bbs config file.\nUse 'project', 'user', 'local', or 'toolchain' to clean only that file.\nIf no argument is provided, bbs cleans both 'project' and 'local'."},
    [BBS_CMD_UPDATE] = { .name = "update",                               .params = "[-i|--info] [-c config] [--init-toolchain] [--refresh-packages]",             .desc = "Generate derived project files",                                                    .detailed_desc = "Parses and validates the project configuration, then generates the derived files and directories used by bbs.\nUse '-i' or '--info' to print the resolved project state before updating.\nUse '-c' or '--config' to resolve the project with a specific config for the info output.\nIf the toolchain file is missing, bbs generates it automatically.\nUse '--init-toolchain' to force regeneration instead of reusing the cached toolchain file.\nUse '--refresh-packages' to re-fetch repo-backed packages into the shared cache before regenerating backend files."},
    [BBS_CMD_GEN] = {    .name = "gen",                                          .params = "<format> [-o|--override] [-p platform[,platform...]]", .desc = "Generate utility files in the project root", .detailed_desc = "Generates one supported utility file in the current project root.\nUse 'gitignore' to generate a complete '.gitignore' template for bbs-based C/C++ projects.\nUse 'github' to generate a multi-platform GitHub Actions workflow for build, test, run, and dist commands.\nUse '-p' or '--platform' with a comma-separated list such as 'windows-x86_64,linux-x86_64' to override the workflow matrix.\nCustom generators can also be defined in 'user.bbs' or 'local.bbs' with 'gen(name(...), copyfile(...))'.\nIf the target file already exists, bbs refuses to overwrite it unless '-o' or '--override' is provided."},
    [BBS_CMD_CFG] = {    .name = "cfg",                                                                      .params = "[-m] [-p] [-u] [-l] [-t]",                 .desc = "Show resolved config paths",                                                                                                                                                                                                                .detailed_desc = "Prints the resolved config file paths used by bbs.\nUse -m or --minimal to print raw paths only.\nUse -p or --project to print the project config path.\nUse -u or --user to print the shared user config path.\nUse -l or --local to print the machine-local config path.\nUse -t or --toolchain to print the toolchain config path.\nIf none of -p, -u, -t or -l is provided, bbs prints all four paths."},
    [BBS_CMD_BUILD] = {  .name = "build",                                                   .params = "[-t target|*] [-p platform|*] [-c config|*]",                     .desc = "Build selected targets",                                                                                                                                                                                                     .detailed_desc = "Builds the selected target for the requested platform.\nIf no target is provided, bbs uses the only project target when possible or operates on all targets.\nIf no platform is provided, the default platform selection is used.\nUse '*' with -t, -p, or -c to execute the command for all matching targets, platforms, or configs.\nIf no config is provided, bbs resolves the project using the 'default' config."},
    [BBS_CMD_AUTO] = {   .name = "auto",                                   .params = "[-t target|*] [-p platform|*] [-c config|*] [--debounce ms]",      .desc = "Watch files and rebuild automatically",                                                                                                                          .detailed_desc = "Runs the same build selection as 'bbs build', then keeps watching the project for file changes.\nWhen a source or config file changes, bbs rebuilds using the same target, platform, and config arguments.\nUse '--debounce' to wait for a quiet period before rebuilding when editors save through multiple file updates.\nUse '*' with -t, -p, or -c to execute the command for all matching targets, platforms, or configs.\nGenerated output directories are ignored to avoid rebuild loops."},
    [BBS_CMD_RUN] = {    .name = "run",                                 .params = "[-t target|*] [-p platform|*] [-c config|*] | [optional args]",        .desc = "Run a target, building it if needed",                                                                                                                                          .detailed_desc = "Builds the selected runnable target if needed, then executes it.\nAny remaining arguments are forwarded to the program.\nThe optional platform selects which build output to run when multiple platforms are available.\nUse '*' with -t or -c to execute the command for all matching runnable targets or configs.\nUse '-p *' to run all host-native runnable outputs for the current machine.\nIf no config is provided, bbs resolves the project using the 'default' config."},
    [BBS_CMD_INFO] = {   .name = "info", .params = "<project|user|local|toolchain> [attribute] [-m] [--attr=path] [--filter=text] [--values-only]",                 .desc = "Inspect parsed config data",                                                                                                                                                                                   .detailed_desc = "Use 'project', 'user', or 'local' to inspect the parsed nodes from that file.\nUse the optional attribute argument or '--attr=path' to show only one attribute.\nUse '-m' or '--minimal' to print only matching attribute paths.\nUse '--filter=text' to show only matching attributes.\nUse '--values-only' to print only matching values.\nUse 'toolchain' to inspect the parsed toolchain.bbs file just like the other config files."},
    [BBS_CMD_PACKAGE] = {.name = "package",                                              .params = "<list|refresh [name|*]|package_name> [--refresh]",                .desc = "Inspect configured packages",                                                                                                                                                                                      .detailed_desc = "Use 'bbs package list' to print all package-backed targets in the current project.\nUse 'bbs package refresh' to refresh all repo/archive-backed packages, or 'bbs package refresh <name>' to refresh one package.\nUse 'bbs package <package_name>' to inspect one configured package, including its source, cache location, and resolution status.\nUse '--refresh' to re-fetch repo-backed packages before printing package info."},
    [BBS_CMD_DIST] = {   .name = "dist",                                                   .params = "[-t target|*] [-p platform|*] [-c config|*]",      .desc = "Package build output for distribution",                                                                                                                                                                                                                                       .detailed_desc = "Collects the selected build output and prepares it for distribution.\nUse the optional target and platform arguments to package a specific artifact when the project produces multiple outputs.\nUse '*' with -t, -p, or -c to execute the command for all matching targets, platforms, or configs.\nIf no config is provided, bbs resolves the project using the 'default' config."},
    [BBS_CMD_TEST] = {   .name = "test",                                            .params = "[name] [-t target|*] [-p platform|*] [-c config|*]",                          .desc = "Run project tests",                                                                                                                                                                                                                                   .detailed_desc = "Runs the project's test suite, or a specific named test when provided.\nThe optional target and platform arguments restrict execution to the matching test target or cross-compilation output.\nUse '*' with -t, -p, or -c to execute the command for all matching test targets, platforms, or configs.\nIf no config is provided, bbs resolves the project using the 'default' config."},
    [BBS_CMD_BUMPVER] = {.name = "bumpver",                                   .params = "<major/minor/patch/user/all> [-p project_id] [-t target_id]",      .desc = "Increment a project or target version",                                                                                                                                                                                                                                                                                                                                                         .detailed_desc = "Increments the selected version according to the requested part.\nUse '-p' or '--project' to choose a specific project node when the file defines more than one.\nUse '-t' or '--target' to bump a target-local version instead of the top-level project version."},
};

static const bbs_hook_info BBS_HOOK_INFOS[BBS_HOOK_MAX] = {
    {.kind = BBS_HOOK_POST_BUILD, .attr_name = "post_build_cmds", .label = "Post Build Cmds"},
    { .kind = BBS_HOOK_PRE_BUILD,  .attr_name = "pre_build_cmds",  .label = "Pre Build Cmds"},
    {   .kind = BBS_HOOK_PRE_RUN,    .attr_name = "pre_run_cmds",    .label = "Pre Run Cmds"},
    {  .kind = BBS_HOOK_POST_RUN,   .attr_name = "post_run_cmds",   .label = "Post Run Cmds"},
    {  .kind = BBS_HOOK_PRE_DIST,   .attr_name = "pre_dist_cmds",   .label = "Pre Dist Cmds"},
    { .kind = BBS_HOOK_POST_DIST,  .attr_name = "post_dist_cmds",  .label = "Post Dist Cmds"},
};

static const bbs_user_attr_info BBS_USER_ATTR_INFOS[] = {
    {          BBS_USER_ATTR_BUILD_DIR,            "builddir",           BBS_USER_ATTR_KIND_TEXT},
    {         BBS_USER_ATTR_ASSETS_DIR,           "assetsdir",           BBS_USER_ATTR_KIND_TEXT},
    {           BBS_USER_ATTR_DIST_DIR,             "distdir",           BBS_USER_ATTR_KIND_TEXT},
    {   BBS_USER_ATTR_AUTO_DEBOUNCE_MS,    "auto_debounce_ms",           BBS_USER_ATTR_KIND_UINT},
    {   BBS_USER_ATTR_AUTO_RETRY_COUNT,    "auto_retry_count",           BBS_USER_ATTR_KIND_UINT},
    {BBS_USER_ATTR_AUTO_RETRY_DELAY_MS, "auto_retry_delay_ms",           BBS_USER_ATTR_KIND_UINT},
    {BBS_USER_ATTR_DIST_ARCHIVE_FORMAT, "dist_archive_format", BBS_USER_ATTR_KIND_ARCHIVE_FORMAT},
    {  BBS_USER_ATTR_DIST_ARCHIVE_NAME,   "dist_archive_name",           BBS_USER_ATTR_KIND_TEXT},
    {         BBS_USER_ATTR_CMAKE_ARGS,          "cmake_args",           BBS_USER_ATTR_KIND_TEXT},
    {   BBS_USER_ATTR_CMAKE_BUILD_ARGS,    "cmake_build_args",           BBS_USER_ATTR_KIND_TEXT},
    {         BBS_USER_ATTR_CTEST_ARGS,          "ctest_args",           BBS_USER_ATTR_KIND_TEXT},
    {                BBS_USER_ATTR_GEN,                 "gen",        BBS_USER_ATTR_KIND_SECTION},
};

static const bbs_user_gen_attr_info BBS_USER_GEN_ATTR_INFOS[] = {
    {"name"},
    {"copyfile"},
};

static const bbs_project_target_type_alias BBS_PROJECT_TARGET_TYPE_ALIASES[] = {
    {    .name = "console",     .type = BBS_TARGET_CONSOLE},
    {.name = "consoleless", .type = BBS_TARGET_CONSOLELESS},
    {        .name = "gui", .type = BBS_TARGET_CONSOLELESS},
    { .name = "header_lib",  .type = BBS_TARGET_HEADER_LIB},
    { .name = "header-lib",  .type = BBS_TARGET_HEADER_LIB},
    {  .name = "headerlib",  .type = BBS_TARGET_HEADER_LIB},
    { .name = "static_lib",  .type = BBS_TARGET_STATIC_LIB},
    { .name = "static-lib",  .type = BBS_TARGET_STATIC_LIB},
    {  .name = "staticlib",  .type = BBS_TARGET_STATIC_LIB},
    {    .name = "dyn_lib",     .type = BBS_TARGET_DYN_LIB},
    {    .name = "dyn-lib",     .type = BBS_TARGET_DYN_LIB},
    { .name = "shared_lib",     .type = BBS_TARGET_DYN_LIB},
    { .name = "shared-lib",     .type = BBS_TARGET_DYN_LIB},
    {    .name = "obj_lib",     .type = BBS_TARGET_OBJ_LIB},
    {    .name = "obj-lib",     .type = BBS_TARGET_OBJ_LIB},
    { .name = "object_lib",     .type = BBS_TARGET_OBJ_LIB},
    { .name = "object-lib",     .type = BBS_TARGET_OBJ_LIB},
    {       .name = "test",        .type = BBS_TARGET_TEST},
    {     .name = "driver",      .type = BBS_TARGET_DRIVER},
};

static const bbs_project_lang_alias BBS_PROJECT_LANG_ALIASES[] = {
    {.name = "cpp", .value = BBS_LANG_CPP},
    {.name = "c++", .value = BBS_LANG_CPP},
};

static const bbs_project_stdlib_alias BBS_PROJECT_STDLIB_ALIASES[] = {
    {   .name = "none",    .value = BBS_STDLIB_NONE},
    { .name = "static",  .value = BBS_STDLIB_STATIC},
    {.name = "dynamic", .value = BBS_STDLIB_DYNAMIC},
};

static const bbs_project_warning_level_alias BBS_PROJECT_WARNING_LEVEL_ALIASES[] = {
    { .name = "default",  .value = BBS_WARNING_DEFAULT},
    {    .name = "none",     .value = BBS_WARNING_NONE},
    {     .name = "off",     .value = BBS_WARNING_NONE},
    {     .name = "low",      .value = BBS_WARNING_LOW},
    { .name = "minimal",      .value = BBS_WARNING_LOW},
    {  .name = "medium",   .value = BBS_WARNING_MEDIUM},
    {     .name = "med",   .value = BBS_WARNING_MEDIUM},
    {  .name = "normal",   .value = BBS_WARNING_MEDIUM},
    {    .name = "high",     .value = BBS_WARNING_HIGH},
    {     .name = "all",     .value = BBS_WARNING_HIGH},
    {.name = "pedantic", .value = BBS_WARNING_PEDANTIC},
    {   .name = "extra", .value = BBS_WARNING_PEDANTIC},
};

static const bbs_project_opt_level_alias BBS_PROJECT_OPT_LEVEL_ALIASES[] = {
    {   .name = "default",    .value = BBS_OPT_DEFAULT},
    {      .name = "none",       .value = BBS_OPT_NONE},
    {       .name = "off",       .value = BBS_OPT_NONE},
    {        .name = "o0",       .value = BBS_OPT_NONE},
    {     .name = "debug",      .value = BBS_OPT_DEBUG},
    {        .name = "og",      .value = BBS_OPT_DEBUG},
    {      .name = "size",       .value = BBS_OPT_SIZE},
    {        .name = "os",       .value = BBS_OPT_SIZE},
    {        .name = "oz",       .value = BBS_OPT_SIZE},
    {   .name = "speed_1",    .value = BBS_OPT_SPEED_1},
    {   .name = "speed-1",    .value = BBS_OPT_SPEED_1},
    {        .name = "o1",    .value = BBS_OPT_SPEED_1},
    {   .name = "speed_2",    .value = BBS_OPT_SPEED_2},
    {   .name = "speed-2",    .value = BBS_OPT_SPEED_2},
    {        .name = "o2",    .value = BBS_OPT_SPEED_2},
    {   .name = "speed_3",    .value = BBS_OPT_SPEED_3},
    {   .name = "speed-3",    .value = BBS_OPT_SPEED_3},
    {        .name = "o3",    .value = BBS_OPT_SPEED_3},
    {.name = "aggressive", .value = BBS_OPT_AGGRESSIVE},
    {      .name = "fast", .value = BBS_OPT_AGGRESSIVE},
    {     .name = "ofast", .value = BBS_OPT_AGGRESSIVE},
};

static const bbs_project_cmake_config_alias BBS_PROJECT_CMAKE_CONFIG_ALIASES[] = {
    {       .name = "release",        .cmake_name = "Release"},
    {          .name = "dist",        .cmake_name = "Release"},
    {      .name = "shipping",        .cmake_name = "Release"},
    {    .name = "minsizerel",     .cmake_name = "MinSizeRel"},
    {       .name = "minsize",     .cmake_name = "MinSizeRel"},
    {.name = "relwithdebinfo", .cmake_name = "RelWithDebInfo"},
    {       .name = "profile", .cmake_name = "RelWithDebInfo"},
};

static const bbs_project_attr_info BBS_PROJECT_LICENSE_ATTR_INFOS[] = {{.name = "type"}, {.name = "file"}};
static const bbs_project_attr_info BBS_PROJECT_UNITY_ATTR_INFOS[] = {{.name = "enabled"}, {.name = "batch_size"}, {.name = "batch"}};
static const bbs_project_attr_info BBS_PROJECT_TARGET_META_ATTR_INFOS[] = {{.name = "id"}, {.name = "name"}, {.name = "authors"}, {.name = "ver"}, {.name = "license"}};
static const bbs_project_attr_info BBS_PROJECT_TARGET_ATTR_INFOS[] = {{.name = "lang"}, {.name = "output"}, {.name = "path"}, {.name = "subdir"}, {.name = "cmake_target"}, {.name = "repo"}, {.name = "archive"}, {.name = "units"}, {.name = "include_dirs"}, {.name = "link_dirs"}, {.name = "dependencies"}, {.name = "link_libs"}, {.name = "defines"}, {.name = "additional_compile_args"}, {.name = "additional_link_args"}, {.name = "warning_level"}, {.name = "opt_level"}, {.name = "stack_size"}, {.name = "warnings_as_errors"}, {.name = "runtime"}, {.name = "stdver"}, {.name = "testing"}, {.name = "test_args"}, {.name = "post_build_cmds"}, {.name = "pre_build_cmds"}, {.name = "pre_run_cmds"}, {.name = "post_run_cmds"}, {.name = "pre_dist_cmds"}, {.name = "post_dist_cmds"}, {.name = "dist"}, {.name = "unity"}};
static const bbs_project_attr_info BBS_PROJECT_PROJECT_ATTR_INFOS[] = {{.name = "id"}, {.name = "name"}, {.name = "authors"}, {.name = "repo"}, {.name = "ver"}, {.name = "license"}, {.name = "configs"}, {.name = "filter"}, {.name = "targets"}, {.name = "builders"}};

static const bbs_toolchain_attr_info BBS_TOOLCHAIN_HOST_ATTR_INFOS[] = {
    {.name = "arch", .kind = BBS_TOOLCHAIN_ATTR_IDENTIFIER, .required = false},
    {  .name = "os", .kind = BBS_TOOLCHAIN_ATTR_IDENTIFIER, .required = false}
};
static const bbs_toolchain_attr_info BBS_TOOLCHAIN_TOOL_ATTR_INFOS[] = {
    {     .name = "id", .kind = BBS_TOOLCHAIN_ATTR_STRING, .required = false},
    {   .name = "path", .kind = BBS_TOOLCHAIN_ATTR_STRING,  .required = true},
    {.name = "version", .kind = BBS_TOOLCHAIN_ATTR_STRING, .required = false}
};
static const bbs_toolchain_attr_info BBS_TOOLCHAIN_SDK_ATTR_INFOS[] = {
    {     .name = "name", .kind = BBS_TOOLCHAIN_ATTR_STRING, .required = false},
    {  .name = "version", .kind = BBS_TOOLCHAIN_ATTR_STRING, .required = false},
    {.name = "base_path", .kind = BBS_TOOLCHAIN_ATTR_STRING, .required = false},
    { .name = "inc_path", .kind = BBS_TOOLCHAIN_ATTR_STRING, .required = false},
    { .name = "src_path", .kind = BBS_TOOLCHAIN_ATTR_STRING, .required = false},
    { .name = "lib_path", .kind = BBS_TOOLCHAIN_ATTR_STRING, .required = false},
    { .name = "bin_path", .kind = BBS_TOOLCHAIN_ATTR_STRING, .required = false}
};
static const bbs_toolchain_attr_info BBS_TOOLCHAIN_PROBE_ATTR_INFOS[] = {
    {.name = "docker_buildx_platforms", .kind = BBS_TOOLCHAIN_ATTR_STRING, .required = false}
};
static const bbs_toolchain_attr_info BBS_TOOLCHAIN_ENV_ATTR_INFOS[] = {
    {      .name = "id",  .kind = BBS_TOOLCHAIN_ATTR_STRING,  .required = true},
    {.name = "provider",  .kind = BBS_TOOLCHAIN_ATTR_STRING,  .required = true},
    {    .name = "name",  .kind = BBS_TOOLCHAIN_ATTR_STRING,  .required = true},
    {    .name = "host", .kind = BBS_TOOLCHAIN_ATTR_SECTION, .required = false},
    {  .name = "probes", .kind = BBS_TOOLCHAIN_ATTR_SECTION, .required = false},
    {   .name = "tools", .kind = BBS_TOOLCHAIN_ATTR_SECTION, .required = false},
    {    .name = "sdks", .kind = BBS_TOOLCHAIN_ATTR_SECTION, .required = false}
};
static const bbs_toolchain_attr_info BBS_TOOLCHAIN_ROOT_ATTR_INFOS[] = {
    {        .name = "host", .kind = BBS_TOOLCHAIN_ATTR_SECTION, .required = false},
    {       .name = "tools", .kind = BBS_TOOLCHAIN_ATTR_SECTION, .required = false},
    {        .name = "sdks", .kind = BBS_TOOLCHAIN_ATTR_SECTION, .required = false},
    {.name = "environments", .kind = BBS_TOOLCHAIN_ATTR_SECTION, .required = false}
};

static const bbs_arch BBS_TOOLCHAIN_MSVC_SUPPORTED_ARCHES[] = {BBS_ARCH_X86_64, BBS_ARCH_X86, BBS_ARCH_ARM64};
static const bbs_arch BBS_TOOLCHAIN_XCODE_SUPPORTED_ARCHES[] = {BBS_ARCH_X86_64, BBS_ARCH_ARM64};

/* Btw you can define these yourself in .bbs configurations with find_tool()*/
static const bbs_tool_discover_strat BBS_TOOL_DISCOVER_STRATS[] = {
    {.id = "cmake", .target_os = BBS_OS_MAX, .exe_name = "cmake", .dir_hints = "{program_files}\\CMake\\bin;{program_files_x86}\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Professional\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Enterprise\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{user_profile}\\scoop\\apps\\cmake\\current\\bin;C:\\ProgramData\\chocolatey\\bin", .deep_roots = "{program_files};{program_files_x86}", .version_arg = "--version", .version_regex = "cmake version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)"},
    {.id = "ctest", .target_os = BBS_OS_MAX, .exe_name = "ctest", .dir_hints = "{program_files}\\CMake\\bin;{program_files_x86}\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Professional\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Enterprise\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{user_profile}\\scoop\\apps\\cmake\\current\\bin;C:\\ProgramData\\chocolatey\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin", .deep_roots = "{program_files};{program_files_x86};/usr;/usr/local", .version_arg = "--version", .version_regex = "ctest version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)"},
    {.id = "docker", .target_os = BBS_OS_MAX, .exe_name = "docker", .dir_hints = "{program_files}\\Docker\\Docker\\resources\\bin;{local_app_data}\\Programs\\Docker\\Docker\\resources\\bin;C:\\Program Files\\Docker\\Docker\\resources\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin", .deep_roots = "{program_files};{local_app_data};/usr;/usr/local;/opt/homebrew", .version_arg = "--version", .version_regex = "Docker version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)"},
    {.id = "bash", .target_os = BBS_OS_MAX, .exe_name = "bash", .dir_hints = "{program_files}\\Git\\bin;{program_files}\\Git\\usr\\bin;{program_files_x86}\\Git\\bin;{program_files_x86}\\Git\\usr\\bin;{msys2_root}\\usr\\bin;C:\\msys64\\usr\\bin;C:\\Program Files\\Git\\bin;C:\\Program Files\\Git\\usr\\bin;/bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin", .deep_roots = "{program_files};{program_files_x86};{msys2_root};/usr;/usr/local;/opt/homebrew", .version_arg = "--version", .version_regex = "version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)"},
    {.id = "git", .target_os = BBS_OS_MAX, .exe_name = "git", .dir_hints = "{program_files}\\Git\\cmd;{program_files}\\Git\\bin;{program_files_x86}\\Git\\cmd;{program_files_x86}\\Git\\bin;{msys2_root}\\usr\\bin;C:\\msys64\\usr\\bin;C:\\Program Files\\Git\\cmd;C:\\Program Files\\Git\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin", .deep_roots = "{program_files};{program_files_x86};{msys2_root};/usr;/usr/local;/opt/homebrew", .version_arg = "--version", .version_regex = "version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)"},
    {.id = "wsl", .target_os = BBS_OS_WINDOWS, .exe_name = "wsl", .dir_hints = "C:\\Windows\\System32;{user_profile}\\AppData\\Local\\Microsoft\\WindowsApps", .deep_roots = "", .version_arg = "--version", .version_regex = "WSL version: ([0-9]+\\.[0-9]+(\\.[0-9]+)?)", .version_arg_fallback = "--status", .version_regex_fallback = "Default Version: ([0-9]+)"},
    {.id = "vcvarsall", .target_os = BBS_OS_WINDOWS, .exe_name = "vcvarsall.bat", .dir_hints = "{program_files}\\Microsoft Visual Studio;{program_files_x86}\\Microsoft Visual Studio", .deep_roots = "{program_files};{program_files_x86}", .version_arg = "", .version_regex = ""},
};

/* Btw you can define these yourself in .bbs configurations with find_sdk()*/
static const bbs_sdk_discover_strat BBS_SDK_DISCOVER_STRATS[] = {
    {.id = "windows_sdk", .target_os = BBS_OS_WINDOWS,         .env_vars = "WindowsSdkDir;WindowsSdkVerBinPath",                                                                                                                                                                                                                                                                                                                                         .root_hints = "{program_files_x86}\\Windows Kits\\10;{program_files_x86}\\Windows Kits\\11;C:\\Program Files (x86)\\Windows Kits\\10;C:\\Program Files (x86)\\Windows Kits\\11",                            .include_rel = "Include",              .source_rel = "Source",                                         .lib_rel = "Lib",                                         .bin_rel = "bin",               .version_file_rel = "Include\\**\\um\\Windows.h",                               .version_regex = "([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)"},
    {   .id = "ucrt_sdk", .target_os = BBS_OS_WINDOWS,             .env_vars = "UniversalCRTSdkDir;UCRTVersion",                                                                                                                                                                                                                                                                                                                                                                                                                         .root_hints = "{program_files_x86}\\Windows Kits\\10;C:\\Program Files (x86)\\Windows Kits\\10",                            .include_rel = "Include",              .source_rel = "Source",                                         .lib_rel = "Lib",                                         .bin_rel = "bin",             .version_file_rel = "Include\\**\\ucrt\\corecrt.h",                               .version_regex = "([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)"},
    {       .id = "msvc", .target_os = BBS_OS_WINDOWS,             .env_vars = "VCToolsInstallDir;VCINSTALLDIR", .root_hints = "{program_files}\\Microsoft Visual Studio\\*\\*\\VC\\Tools\\MSVC\\*;{program_files_x86}\\Microsoft Visual Studio\\*\\*\\VC\\Tools\\MSVC\\*;C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC;C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Tools\\MSVC;C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Tools\\MSVC;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\MSVC;C:\\BuildTools\\VC\\Tools\\MSVC",                            .include_rel = "include",                    .source_rel = "",                                         .lib_rel = "lib",                           .bin_rel = "bin\\Hostx64\\x64",                    .version_file_rel = "include\\yvals_core.h",                                            .version_regex = "_MSC_FULL_VER ([0-9]+)"},
    {      .id = "xcode",   .target_os = BBS_OS_MACOS,                              .env_vars = "DEVELOPER_DIR",                                                                                                                                                                                                                                                                                                                                                                                                                          .root_hints = "/Applications/Xcode.app/Contents/Developer;/Library/Developer/CommandLineTools",                                   .include_rel = "",                    .source_rel = "", .lib_rel = "Toolchains/XcodeDefault.xctoolchain/usr/lib", .bin_rel = "Toolchains/XcodeDefault.xctoolchain/usr/bin",                         .version_file_rel = "../version.plist", .version_regex = "<key>CFBundleShortVersionString</key>\\s*<string>([^<]+)</string>"},
    { .id = "vulkan_sdk",     .target_os = BBS_OS_MAX,                                 .env_vars = "VULKAN_SDK",                                                                                                                                                                                                                                                                                                                                                                      .root_hints = "{program_files}\\VulkanSDK\\*;{program_files_x86}\\VulkanSDK\\*;{local_app_data}\\VulkanSDK\\*;C:\\VulkanSDK\\*;{home}/VulkanSDK/*",                            .include_rel = "Include",                    .source_rel = "",                                         .lib_rel = "Lib",                                         .bin_rel = "Bin", .version_file_rel = "README.txt;config\\vk_layer_settings.txt",                            .version_regex = "([0-9]+\\.[0-9]+\\.[0-9]+(\\.[0-9]+)?)"},
    {.id = "android_ndk",     .target_os = BBS_OS_MAX, .env_vars = "ANDROID_NDK_ROOT;ANDROID_NDK_HOME;NDK_ROOT",                                                                                                                                                                                                          .root_hints = "{home}\\AppData\\Local\\Android\\Sdk\\ndk\\*;{home}\\AppData\\Local\\Android\\Sdk\\ndk-bundle;{local_app_data}\\Android\\Sdk\\ndk\\*;{home}/Android/Sdk/ndk/*;{home}/Android/Sdk/ndk-bundle;/opt/android-ndk;/opt/android-sdk/ndk/*;{program_files}/Android/Android Studio/plugins/android-ndk",           .include_rel = "toolchains/llvm/prebuilt",             .source_rel = "sources",                    .lib_rel = "toolchains/llvm/prebuilt",                    .bin_rel = "toolchains/llvm/prebuilt",                        .version_file_rel = "source.properties",                .version_regex = "Pkg.Revision\\s*=\\s*([0-9]+\\.[0-9]+(\\.[0-9]+)?)"},
    {      .id = "emsdk",     .target_os = BBS_OS_MAX,                                      .env_vars = "EMSDK",                                                                                                                                                                                                                                                                                                                                                                                                                                  .root_hints = "{home}\\emsdk;{home}/emsdk;{program_files}\\emsdk;C:\\emsdk;/opt/emsdk", .include_rel = "upstream/emscripten/system/include", .source_rel = "upstream/emscripten",       .lib_rel = "upstream/emscripten/cache/sysroot/lib",                         .bin_rel = "upstream/emscripten",  .version_file_rel = ".emscripten;upstream/emscripten/emcc.py",                                        .version_regex = "([0-9]+\\.[0-9]+\\.[0-9]+)"},
    {       .id = "musl",   .target_os = BBS_OS_LINUX,                                  .env_vars = "MUSL_ROOT",                                                                                                                                                                                                                                                                                                                                                                                                                                                        .root_hints = "/usr;/usr/local;/opt/musl;/opt/homebrew/opt/musl",                            .include_rel = "include",                 .source_rel = "src",                                         .lib_rel = "lib",                                         .bin_rel = "bin",           .version_file_rel = "include/features.h;lib/libc.so",                                     .version_regex = "([0-9]+\\.[0-9]+(\\.[0-9]+)?)"},
    {      .id = "glibc",   .target_os = BBS_OS_LINUX,                                 .env_vars = "GLIBC_ROOT",                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         .root_hints = "/usr;/usr/local",                            .include_rel = "include",                 .source_rel = "src",                                   .lib_rel = "lib;lib64",                                         .bin_rel = "bin",         .version_file_rel = "include/features.h;lib/libc.so.6",                                     .version_regex = "([0-9]+\\.[0-9]+(\\.[0-9]+)?)"},
};

static inline const char* bbs_cmd_name(bbs_cmd command) {
  return command >= 0 && command < BBS_CMD_MAX ? BBS_CMD_INFOS[command].name : NULL;
}

static inline const char* bbs_sig_name(bbs_sig signal) {
  static const char* names[BBS_SIG_MAX] = {
      [BBS_SIG_INIT] = "init",
      [BBS_SIG_QUIT] = "quit",
      [BBS_SIG_PRE_CMD] = "pre_cmd",
      [BBS_SIG_POST_CMD] = "post_cmd",
      [BBS_SIG_PRE_BUILD] = "pre_build",
      [BBS_SIG_POST_BUILD] = "post_build",
      [BBS_SIG_PRE_DIST] = "pre_dist",
      [BBS_SIG_POST_DIST] = "post_dist",
      [BBS_SIG_PRE_RUN] = "pre_run",
      [BBS_SIG_POST_RUN] = "post_run",
      [BBS_SIG_PRE_TEST] = "pre_test",
      [BBS_SIG_POST_TEST] = "post_test",
      [BBS_SIG_PRE_UPDATE] = "pre_update",
      [BBS_SIG_POST_UPDATE] = "post_update",
      [BBS_SIG_PRE_PACKAGE] = "pre_package",
      [BBS_SIG_POST_PACKAGE] = "post_package",
      [BBS_SIG_PRE_GEN] = "pre_gen",
      [BBS_SIG_POST_GEN] = "post_gen",
      [BBS_SIG_PRE_CFG] = "pre_cfg",
      [BBS_SIG_POST_CFG] = "post_cfg",
      [BBS_SIG_PRE_CLEAN] = "pre_clean",
      [BBS_SIG_POST_CLEAN] = "post_clean",
      [BBS_SIG_PRE_INFO] = "pre_info",
      [BBS_SIG_POST_INFO] = "post_info",
      [BBS_SIG_PRE_BUMPVER] = "pre_bumpver",
      [BBS_SIG_POST_BUMPVER] = "post_bumpver",
      [BBS_SIG_PRE_AUTO] = "pre_auto",
      [BBS_SIG_POST_AUTO] = "post_auto",
  };
  return signal >= 0 && signal < BBS_SIG_MAX ? names[signal] : NULL;
}

static inline const char* bbs_os_name(bbs_os os) {
  return os >= 0 && os < BBS_OS_MAX ? BBS_OS_NAMES[os] : NULL;
}

static inline const char* bbs_arch_name(bbs_arch arch) {
  return arch >= 0 && arch < BBS_ARCH_MAX ? BBS_ARCH_NAMES[arch] : NULL;
}

static inline const char* bbs_node_type_name(bbs_node_type type) {
  return type >= 0 && type < BBS_NODE_MAX ? BBS_NODE_TYPE_NAMES[type] : NULL;
}

static inline const char* bbs_target_type_name(bbs_target_type type) {
  switch (type) {
    case BBS_TARGET_CONSOLE:
      return "console";
    case BBS_TARGET_CONSOLELESS:
      return "consoleless";
    case BBS_TARGET_HEADER_LIB:
      return "header_lib";
    case BBS_TARGET_STATIC_LIB:
      return "static_lib";
    case BBS_TARGET_DYN_LIB:
      return "dyn_lib";
    case BBS_TARGET_OBJ_LIB:
      return "obj_lib";
    case BBS_TARGET_TEST:
      return "test";
    case BBS_TARGET_DRIVER:
      return "driver";
    default:
      return NULL;
  }
}

static inline const char* bbs_lang_name(bbs_lang lang) {
  switch (lang) {
    case BBS_LANG_C:
      return "c";
    case BBS_LANG_CPP:
      return "cpp";
    default:
      return NULL;
  }
}

static inline const char* bbs_hook_name(bbs_hook_kind kind) {
  return kind >= 0 && kind < BBS_HOOK_MAX ? BBS_HOOK_INFOS[kind].attr_name : NULL;
}

typedef struct bbs_node bbs_node;
/*
  Parsed `.bbs` syntax node.

  Builders mainly use these for inspection. The host also uses the source offset
  fields for diagnostics and targeted text edits.
*/
struct bbs_node {
  bbs_node* next;
  bbs_node* parent;
  bbs_node* children;

  const char* name;
  size_t name_dim;
  bbs_node_type type;
  union {
    const char* _str;
    int64_t _int;
    double _flt;
    bbs_ver _ver;
    const char* _idf;
    bool _bol;
  } value;

  size_t txt_offset;
  size_t txt_dim;
  size_t src_txt_offset;
  size_t src_txt_dim;
  uint8_t ver_parts;
};

/* One `(os, arch)` pair. */
typedef struct {
  bbs_arch arch;
  bbs_os os;
} bbs_ptf;

/* Common metadata shared by projects, targets, and builders. */
typedef struct {
  bbs_ver ver;
  const char* id;
  const char* name;
  const char* repo;
  const char* authors;
  struct {
    const char* type;
    const char* file;
  } license;
} bbs_meta;

/* One explicit unity-build selector batch. */
typedef struct {
  const char** selectors;
  int selector_c;
} bbs_unity_batch;

/* Distribution settings stored on a resolved target. */
typedef struct {
  bool archive;
  bool copy_assets;
  const char* archive_name;
  const char** exclude_assets;
  int exclude_asset_c;
} bbs_dist;

typedef struct bbs_tgt {
  /* Identity and high-level build mode. */
  bbs_meta meta;
  bbs_lang lang;
  bbs_target_type type;
  const char* output;

  /* Package-backed target configuration. */
  bbs_package_source package_source;
  const char* package_path;
  const char* package_subdir;
  const char* package_repo_link;
  const char* package_repo_tag;
  const char* package_repo_commit;
  bbs_package_backend package_backend;
  const char* package_cmake_target;
  const char* package_archive_link;
  const char* package_archive_strip_prefix;
  const char* package_project_cfg_path;
  const char* package_resolved_dir;
  const char* package_cache_dir;
  const char* package_build_dir;

  /* Compile inputs. */
  const char** units;
  int unit_c;
  bool unity_configured;
  bool unity_enabled;
  bool unity_batch_size_set;
  size_t unity_batch_size;
  bbs_unity_batch* unity_batches;
  int unity_batch_c;

  /* Include / link / dependency inputs. */
  const char** include_dirs;
  int include_dir_c;
  const char** link_dirs;
  int link_dir_c;
  const char** dependencies;
  int dependency_c;
  const char* defines;
  int define_c;
  const char** link_libs;
  int link_libs_count;

  /* Backend-facing text flags. */
  const char* additional_compile_args;
  const char* additional_link_args;
  bbs_warning_level warning_level;
  bbs_opt_level opt_level;
  size_t stack_size;
  bool warnings_as_errors;

  /* Language/runtime/test settings. */
  bbs_stdlib runtime;
  const char* stdver;
  bool testing;
  const char** test_args;
  int test_arg_c;

  /* Command hooks and distribution settings. */
  const char** hook_cmds[BBS_HOOK_MAX];
  int hook_cmd_counts[BBS_HOOK_MAX];
  bbs_dist dist;

  /* Reserved for builder or host-side bookkeeping. */
  void* user_data;
} bbs_tgt;

/* Resolved build target as seen by builders. */

/* One builder declared in `project.bbs -> builders(...)`. */
typedef struct {
  bbs_meta meta;
  bbs_lang lang;
  const char* output;
  const char** units;
  int unit_c;
  const char** include_dirs;
  int include_dir_c;
  const char* defines;
  int define_c;
  const char* additional_compile_args;
  const char* additional_link_args;
  void* user_data;
} bbs_builder;

/* One custom generator from `user.bbs` / `local.bbs`. */
typedef struct {
  const char* name;
  const char* copyfile;
} bbs_gen;

/* Resolved merged user/local config view. */
typedef struct {
  const char* text_values[BBS_USER_TEXT_MAX];
  unsigned int uint_values[BBS_USER_UINT_MAX];
  bbs_node* merged_scope;
  bbs_gen* gens;
  int gen_c;
} bbs_user;

/* One discovered tool executable. */
typedef struct {
  const char* id;
  const char* path;
  const char* version;
} bbs_tool;

/* One discovered SDK root. */
typedef struct {
  const char* name;
  const char* version;
  const char* base_path;
  const char* inc_path;
  const char* src_path;
  const char* lib_path;
  const char* bin_path;
} bbs_sdk;

/* One discovered toolchain environment/provider. */
typedef struct {
  const char* id;
  const char* provider;
  const char* name;
  bbs_arch host_arch;
  bbs_os host_os;
  const char* probe_docker_buildx_platforms;
  bool supported[BBS_OS_MAX][BBS_ARCH_MAX];
  const char* support_source[BBS_OS_MAX][BBS_ARCH_MAX];
  bbs_tool* tools;
  int tool_c;
  int tool_cap;
  bbs_sdk* sdks;
  int sdk_c;
  int sdk_cap;
} bbs_toolchain_env;

/* Resolved toolchain cache and discovered environment set. */
typedef struct {
  bbs_arch host_arch;
  bbs_os host_os;
  bool supported[BBS_OS_MAX][BBS_ARCH_MAX];
  const char* support_source[BBS_OS_MAX][BBS_ARCH_MAX];
  bbs_node* config_tree;
  const char* project_cfg_path;
  const char* user_cfg_path;
  const char* local_cfg_path;
  const char* toolchain_cfg_path;
  bbs_toolchain_env* envs;
  int env_c;
  int env_cap;
} bbs_toolchain;

/* Resolved project view passed to builders. */
typedef struct {
  bbs_meta meta;
  bbs_user user_cfg;
  bbs_node* config_tree;

  const char* root_dir;
  const char* config_path;
  const char* local_cfg_path;

  const char** configs;
  int config_c;
  const char* active_config;

  const char* build_dir;
  const char* assets_dir;
  const char* dist_dir;
  const char* dist_archive_format;
  const char* dist_archive_name;
  const char* cmake_args;
  const char* cmake_build_args;
  const char* ctest_args;
  unsigned int auto_debounce_ms;
  unsigned int auto_retry_count;
  unsigned int auto_retry_delay_ms;

  bbs_gen* gens;
  int gen_c;

  bbs_tgt* targets;
  int target_c;
  int target_cap;

  bbs_builder* builders;
  int builder_c;
  int builder_cap;

  void* user_data;
} bbs_proj;

/* Runtime command context passed to builders. */
typedef struct {
  uint32_t api_version;
  uint32_t struct_size;

  bbs_sig signal;
  bbs_cmd command;
  uint32_t flags;

  bool hotbuild_mode;
  bool command_failed;
  int command_result;

  bbs_ptf host;
  bbs_ptf selected;

  const char* exe_path;
  const char* cwd;
  const char* workdir;

  const char* project_cfg_path;
  const char* user_cfg_path;
  const char* local_cfg_path;
  const char* toolchain_cfg_path;

  const char* command_name;
  bbs_strlist command_args;

  const char* selected_target;
  const char* selected_platform;
  const char* selected_config;

  const bbs_toolchain* toolchain;

  void* user_data;
} bbs_ctx;

static inline const char* bbs_user_text(const bbs_user* user, bbs_user_text_attr attr) {
  return user && attr >= 0 && attr < BBS_USER_TEXT_MAX ? user->text_values[attr] : NULL;
}

/* Read one resolved text value from merged user config. */

static inline unsigned int bbs_user_uint(const bbs_user* user, bbs_user_uint_attr attr) {
  return user && attr >= 0 && attr < BBS_USER_UINT_MAX ? user->uint_values[attr] : 0u;
}

/* Read one resolved integer value from merged user config. */

static inline bbs_node* bbs_node_get_child(bbs_node* node, const char* name) {
  bbs_node* child = node ? node->children : NULL;
  for (; child; child = child->next) {
    size_t i = 0;
    if (!child->name || !name)
      continue;
    for (; i < child->name_dim && name[i] != '\0'; ++i) {
      char a = child->name[i];
      char b = name[i];
      if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
      if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
      if (a != b)
        break;
    }
    if (i == child->name_dim && name[i] == '\0')
      return child;
  }
  return NULL;
}

/* Lookup a direct named child on a parsed `.bbs` node. Case-insensitive. */

static inline const char* bbs_node_name(const bbs_node* node) {
  return node ? node->name : NULL;
}
static inline bbs_node_type bbs_node_type_of(const bbs_node* node) {
  return node ? node->type : BBS_NODE_DEF;
}
static inline const char* bbs_node_str(const bbs_node* node) {
  return node && node->type == BBS_NODE_STR ? node->value._str : NULL;
}
static inline const char* bbs_node_idf(const bbs_node* node) {
  return node && node->type == BBS_NODE_IDF ? node->value._idf : NULL;
}
static inline int64_t bbs_node_int(const bbs_node* node) {
  return node && node->type == BBS_NODE_INT ? node->value._int : 0;
}
static inline double bbs_node_flt(const bbs_node* node) {
  return node && node->type == BBS_NODE_FLT ? node->value._flt : 0.0;
}
static inline bbs_ver bbs_node_ver(const bbs_node* node) {
  return node && node->type == BBS_NODE_VER ? node->value._ver : (bbs_ver) {0};
}
static inline bool bbs_node_bol(const bbs_node* node) {
  return node && node->type == BBS_NODE_BOL ? node->value._bol : false;
}

static inline const char** bbs_target_hook_cmds(const bbs_tgt* tgt, bbs_hook_kind kind) {
  return tgt && kind >= 0 && kind < BBS_HOOK_MAX ? tgt->hook_cmds[kind] : NULL;
}

/* Access one hook command list stored on a resolved target. */

static inline int bbs_target_hook_cmd_count(const bbs_tgt* tgt, bbs_hook_kind kind) {
  return tgt && kind >= 0 && kind < BBS_HOOK_MAX ? tgt->hook_cmd_counts[kind] : 0;
}

static inline int bbs_stricmp(const char* a, const char* b) {
  while (*a && *b) {
    int ca = tolower((unsigned char)*a);
    int cb = tolower((unsigned char)*b);

    if (ca != cb)
      return ca - cb;

    a++;
    b++;
  }

  return tolower((unsigned char)*a) -
         tolower((unsigned char)*b);
}
/* Access the count for one hook command list. */

static inline bool bbs_target_has_dependency(const bbs_tgt* tgt, const char* name) {
  if (!tgt || !name || !name[0])
    return false;
  for (int i = 0; i < tgt->dependency_c; ++i)
    if (tgt->dependencies[i] && bbs_stricmp(tgt->dependencies[i], name) == 0)
      return true;
  return false;
}

/* Check whether a resolved target depends on another target or builder id. */

/*
  All pointers passed to builders are owned by bbs.

  Builders may mutate `bbs_ctx`, `bbs_proj`, and `bbs_tgt` during a command.
  Those mutations are command-local: bbs must preserve them for the remainder of
  the current command execution and then restore the original state once the
  command fully completes.

  Builders must not directly replace pointer-bearing fields inside these
  structs. Use the service APIs below for string and list mutations so bbs keeps
  ownership of all stored data.

  Rough rule:

  - read directly from the public structs
  - mutate text through `bbs_*_set_text` / `bbs_target_append_text`
  - mutate collections through the host helper APIs below
*/

/* Host-side logging helpers. These mirror the normal bbs console style. */
BBS_SERVICE_API void bbs_print(const char* fmt, ...);
BBS_SERVICE_API void bbs_warn(const char* fmt, ...);
BBS_SERVICE_API void bbs_error(const char* fmt, ...);

/* Replace one mutable text field on a context/project/target. */
BBS_SERVICE_API bool bbs_ctx_set_text(bbs_ctx* ctx, bbs_ctx_text_field field, const char* value);
BBS_SERVICE_API bool bbs_project_set_text(bbs_proj* proj, bbs_project_text_field field, const char* value);
BBS_SERVICE_API bool bbs_target_set_text(bbs_tgt* tgt, bbs_target_text_field field, const char* value);

/* Append text to a target text field, inserting `separator` when needed. */
BBS_SERVICE_API bool bbs_target_append_text(bbs_tgt* tgt, bbs_target_text_field field, const char* value, const char* separator);

/* Generic string-list mutation helpers for host-owned `bbs_strlist` values. */
BBS_SERVICE_API bool bbs_strlist_reserve(bbs_strlist* list, size_t min_capacity);
BBS_SERVICE_API bool bbs_strlist_append(bbs_strlist* list, const char* value);
BBS_SERVICE_API bool bbs_strlist_append_n(bbs_strlist* list, const char* value, size_t len);
BBS_SERVICE_API bool bbs_strlist_insert(bbs_strlist* list, size_t index, const char* value);
BBS_SERVICE_API bool bbs_strlist_remove_at(bbs_strlist* list, size_t index);
BBS_SERVICE_API bool bbs_strlist_remove_value(bbs_strlist* list, const char* value, bool ignore_case);
BBS_SERVICE_API void bbs_strlist_clear(bbs_strlist* list);
BBS_SERVICE_API ptrdiff_t bbs_strlist_find(const bbs_strlist* list, const char* value, bool ignore_case);
BBS_SERVICE_API bool bbs_strlist_contains(const bbs_strlist* list, const char* value, bool ignore_case);

/* Target helpers for host-created or host-managed target values. */
BBS_SERVICE_API void bbs_target_init(bbs_tgt* tgt);
BBS_SERVICE_API void bbs_target_copy(bbs_tgt* dst, const bbs_tgt* src);

/* Uniform list access helper for APIs that want one list view. */
BBS_SERVICE_API bbs_strlist* bbs_target_list(bbs_tgt* tgt, bbs_list_kind kind, bbs_hook_kind hook_kind);
BBS_SERVICE_API const bbs_strlist* bbs_target_list_const(const bbs_tgt* tgt, bbs_list_kind kind, bbs_hook_kind hook_kind);

/* Structural project mutation helpers. */
BBS_SERVICE_API bool bbs_project_reserve_targets(bbs_proj* proj, size_t min_capacity);
BBS_SERVICE_API bbs_tgt* bbs_project_add_target(bbs_proj* proj);
BBS_SERVICE_API bbs_tgt* bbs_project_insert_target(bbs_proj* proj, size_t index);
BBS_SERVICE_API bool bbs_project_remove_target_at(bbs_proj* proj, size_t index);
BBS_SERVICE_API ptrdiff_t bbs_project_find_target_index(const bbs_proj* proj, const char* id);
BBS_SERVICE_API bbs_tgt* bbs_project_find_target(bbs_proj* proj, const char* id);
BBS_SERVICE_API const bbs_tgt* bbs_project_find_target_const(const bbs_proj* proj, const char* id);

/* Config name mutation helpers on the resolved project. */
BBS_SERVICE_API bool bbs_project_reserve_configs(bbs_proj* proj, size_t min_capacity);
BBS_SERVICE_API bool bbs_project_add_config(bbs_proj* proj, const char* config_name);
BBS_SERVICE_API bool bbs_project_remove_config(bbs_proj* proj, const char* config_name);
BBS_SERVICE_API ptrdiff_t bbs_project_find_config_index(const bbs_proj* proj, const char* config_name);
BBS_SERVICE_API bool bbs_project_has_config(const bbs_proj* proj, const char* config_name);

/* Custom generator mutation helpers. */
BBS_SERVICE_API bool bbs_project_add_gen(bbs_proj* proj, const char* name, const char* copyfile);
BBS_SERVICE_API ptrdiff_t bbs_project_find_gen_index(const bbs_proj* proj, const char* name);

/* Execute one bash-compatible command through the active host toolchain. */
BBS_SERVICE_API int bbs_run_bash(bbs_ctx* ctx, const char* workdir, const char* command);

/*
  Run one discovery strategy directly against the active host toolchain.

  Example:

  bbs_tool_discover_strat cmake = {
      .id = "cmake",
      .target_os = BBS_OS_MAX,
      .exe_name = "cmake",
      .dir_hints = "/usr/bin;/usr/local/bin",
      .version_arg = "--version",
      .version_regex = "cmake version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
  };
  const bbs_tool* tool = bbs_find_tool(ctx, &cmake);

  bbs_sdk_discover_strat vulkan = {
      .id = "vulkan_sdk",
      .target_os = BBS_OS_MAX,
      .env_vars = "VULKAN_SDK",
      .root_hints = "{home}/VulkanSDK/<version>",
      .include_rel = "Include",
      .lib_rel = "Lib",
      .bin_rel = "Bin",
      .version_file_rel = "README.txt",
      .version_regex = "([0-9]+\\.[0-9]+\\.[0-9]+)",
  };
  const bbs_sdk* sdk = bbs_find_sdk(ctx, &vulkan);
*/
BBS_SERVICE_API const bbs_tool* bbs_find_tool(bbs_ctx* ctx, const bbs_tool_discover_strat* strat);
BBS_SERVICE_API const bbs_sdk* bbs_find_sdk(bbs_ctx* ctx, const bbs_sdk_discover_strat* strat);

/*
  Persist the current in-memory state to the corresponding config file.

  Builders can use these to keep selected command-local mutations instead of
  letting them disappear at process exit.

  If you do not call these APIs, builder-side mutations are expected to remain
  temporary.
*/
BBS_SERVICE_API bool bbs_save_toolchain(const bbs_ctx* ctx);
BBS_SERVICE_API bool bbs_save_user(const bbs_ctx* ctx, const bbs_proj* proj);
BBS_SERVICE_API bool bbs_save_local(const bbs_ctx* ctx, const bbs_proj* proj);
BBS_SERVICE_API bool bbs_save_project(const bbs_ctx* ctx, const bbs_proj* proj);

/* Required export implemented by every builder module. */
BBS_USER_API bool bbs_callback(
    bbs_sig signal,
    bbs_ctx* ctx,
    bbs_proj* prj,
    bbs_tgt* tgt);

#ifdef __cplusplus
}
#endif
