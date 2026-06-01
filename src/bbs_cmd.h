#pragma once

#include "bbs_base.h"

typedef enum {
  CFG_PROJECT,
  CFG_USER,
  CFG_LOCAL,
  CFG_TOOLCHAIN,
  CFG_MAX,
} cfg;

typedef enum {
  CFG_LOC_CWD,
  CFG_LOC_EXE,
} cfg_loc;

typedef struct {
  cfg_loc loc;
  const char* filename;
  const char* desc;
  const char* detailed_desc;
} cfg_info;

static cfg_info CFG_INFOS[] = {
    [CFG_PROJECT] = {.loc = CFG_LOC_CWD,
                     .filename = "project.bbs",
                     .desc = "Main project configuration",
                     .detailed_desc = "The main project configuration file.\n"
                     "Keep it in the project root and declare project metadata, targets, platforms, and build settings there.\n"
                     "bbs reads this file first, then applies defaults from 'user.bbs' and overrides from 'local.bbs' when present."},

    [CFG_USER] = {.loc = CFG_LOC_EXE,
                     .filename = "user.bbs",
                     .desc = "Shared user configuration defaults",
                     .detailed_desc = "Shared user configuration defaults.\n"
                     "Use it to define your preferred defaults across projects.\n"
                     "Keep it next to the bbs executable so those defaults are available everywhere you run bbs.\n"
                     "Values from 'user.bbs' can be overridden by 'local.bbs' in a specific project."                               },

    [CFG_LOCAL] = {.loc = CFG_LOC_CWD,
                      .filename = "local.bbs",
                      .desc = "Machine-specific user overrides",
                      .detailed_desc = "Machine-specific user overrides.\n"
                      "Keep it in the project root directory, next to 'project.bbs'.\n"
                      "Use it for machine-specific overrides to your 'user.bbs' defaults that should not be shared.\n"
                      "Values in 'local.bbs' can override the defaults from 'user.bbs'.\n"
                      "In normal projects this file should usually be added to .gitignore."                                          },

    [CFG_TOOLCHAIN] = {.loc = CFG_LOC_EXE,
                     .filename = "toolchain.bbs",
                     .desc = "Generated toolchain configuration cache",
                     .detailed_desc = "Generated toolchain configuration cache.\n"
                     "bbs creates and refreshes this file while preparing the build environment.\n"
                     "Keep it next to the bbs executable so the detected toolchain state can be reused.\n"
                     "Regenerate it with 'bbs update --init-toolchain' when the local toolchain changes."                           },
};

typedef struct {
  const char* name;
  const char* params;
  const char* desc;
  const char* detailed_desc;
} cmd_info;

typedef struct {
  cmdline* cl;
  const char* cfg_paths[CFG_MAX];
} cmd_ctx;

typedef enum {
  CMD_HELP,
  CMD_CLEAN,
  CMD_UPDATE,
  CMD_CFG,
  CMD_BUILD,
  CMD_RUN,
  CMD_INFO,
  CMD_DIST,
  CMD_TEST,
  CMD_BUMPVER,
  CMD_MAX,
} cmd;

static cmd_info CMD_INFOS[CMD_MAX] = {
    [CMD_HELP] = {   .name = "help",
                  .params = "[command/topic]",
                  .desc = "Show generic help or help for a specific command or config",
                  .detailed_desc = "Without arguments, list all user commands.\n"
                  "Pass a command or config topic to show its syntax, purpose, and any command-specific notes."    },

    [CMD_CLEAN] = {  .name = "clean",
                  .params = "[project|user|local|toolchain]",
                  .desc = "Delete selected bbs config files",
                  .detailed_desc = "Delete one selected bbs config file.\n"
                  "Use 'project', 'user', 'local', or 'toolchain' to clean only that file.\n"
                  "If no argument is provided, bbs cleans both 'project' and 'local'."                             },

    [CMD_UPDATE] = { .name = "update",
                  .params = "[-i|--info] [-c config] [--init-toolchain]",
                  .desc = "Set up derived project folders",
                  .detailed_desc = "Parse and validate the project configuration, then create the derived project directories used by bbs.\n"
                  "Use '-i' or '--info' to print the resolved parsed project state before updating.\n"
                  "Use '-c' or '--config' to resolve the project using a specific named config for the info output.\n"
                  "If the toolchain file is missing, update generates it automatically.\n"
                  "Use '--init-toolchain' to force regeneration instead of using the cached toolchain file."       },

    [CMD_CFG] = {    .name = "cfg",
                  .params = "[-m] [-p] [-u] [-l] [-t]",
                  .desc = "Show the resolved config file paths",
                  .detailed_desc = "Print the resolved config file paths used by bbs.\n"
                  "Use -m or --minimal to print raw paths only.\n"
                  "Use -p or --project to print the project config path.\n"
                  "Use -u or --user to print the shared user config path.\n"
                  "Use -l or --local to print the machine-local config path.\n"
                  "Use -t or --toolchain to print the toolchain config path.\n"
                  "If none of -p, -u, -t or -l is provided, bbs prints all three paths."                           },

    [CMD_BUILD] = {  .name = "build",
                  .params = "[-t target] [-p platform] [-c config]",
                  .desc = "Build and compile the project",
                  .detailed_desc = "Compile the selected target for the requested platform.\n"
                  "If no target is provided, bbs uses the only project target when possible or operates on all targets.\n"
                  "If no platform is provided, the default configured platform selection is used.\n"
                  "If no config is provided, bbs resolves the project using the 'default' config."                 },

    [CMD_RUN] = {    .name = "run",
                  .params = "[-t target] [-p platform] [-c config] | [optional args]",
                  .desc = "Run a built project, automatically building it if required",
                  .detailed_desc = "Build the selected runnable target if needed, then execute it.\n"
                  "Any remaining arguments are forwarded to the program.\n"
                  "The optional platform selects which build output to run when multiple platforms are available.\n"
                  "If no config is provided, bbs resolves the project using the 'default' config."                 },

    [CMD_INFO] = {   .name = "info",
                  .params = "<project|user|local|toolchain> [attribute] [-m] [--attr=path] [--filter=text] [--values-only]",
                  .desc = "Display parsed config or toolchain information",
                  .detailed_desc = "Use 'project', 'user', or 'local' to inspect the parsed nodes from that file.\n"
                  "Use the optional attribute argument or '--attr=path' to show only one attribute.\n"
                  "Use '-m' or '--minimal' to print only matching attribute paths.\n"
                  "Use '--filter=text' to show only matching attributes.\n"
                  "Use '--values-only' to print only matching values.\n"
                  "Use 'toolchain' to inspect the raw parsed toolchain.bbs file just like the other config files." },

    [CMD_DIST] = {   .name = "dist",
                  .params = "[-t target] [-p platform] [-c config]",
                  .desc = "Prepare the built project for distribution",
                  .detailed_desc = "Collect the selected build output and prepare it for distribution.\n"
                  "Use the optional target and platform arguments to dist a specific artifact when the project produces multiple outputs.\n"
                  "If no config is provided, bbs resolves the project using the 'default' config."                 },

    [CMD_TEST] = {   .name = "test",
                  .params = "[name] [-t target] [-p platform] [-c config]",
                  .desc = "Run unit tests for the current project",
                  .detailed_desc = "Run the project's test suite, or a specific named test when provided.\n"
                  "The optional target and platform arguments restrict execution to the matching test target or cross-compilation output.\n"
                  "If no config is provided, bbs resolves the project using the 'default' config."                 },

    [CMD_BUMPVER] = {.name = "bumpver",
                  .params = "<major/minor/patch/user/all> [-p project_id] [-t target_id]",
                  .desc = "Increment a project or target version",
                  .detailed_desc = "Increment the selected version according to the requested part.\n"
                  "Use '-p' or '--project' to choose a specific project node when the file defines more than one.\n"
                  "Use '-t' or '--target' to bump a target-local version instead of the top-level project version."},
};
