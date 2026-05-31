#pragma once

#define PROJ_FILENAME      "project.bbs"
#define USER_FILENAME      "user.bbs"
#define LOCAL_FILENAME     "local.bbs"
#define TOOLCHAIN_FILENAME "toolchain.bbs"

#include "bbs_base.h"

typedef struct {
  const char* name;
  const char* params;
  const char* desc;
  const char* detailed_desc;
} cmd_info;

typedef struct {
  cmdline* cl;

  bool debug;

  const char* user;
  const char* project;
  const char* local;
  const char* toolchain;
} cmd_ctx;

typedef enum {

  /////////////////////////////////////////
  // HELPER COMMANDS
  /////////////////////////////////////////

  CMD_DEFAULT,
  CMD_BUILDIR,
  CMD_CFGFILE,
  CMD_USERCFG,
  CMD_LOCALCFG,
  CMD_HELP_END,

  /////////////////////////////////////////
  // USER COMMANDS
  /////////////////////////////////////////

  CMD_CLEAN = CMD_HELP_END,
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
    [CMD_DEFAULT] = {    .name = "help",
                     .params = "[command/topic]",
                     .desc = "Show generic help or help for a specific command",
                     .detailed_desc = "Without arguments, list all user commands.\n"
                     "Pass a command or help topic to show its syntax, purpose, and any command-specific notes."      },

    [CMD_BUILDIR] = {.name = "builddir",
                     .params = "",
                     .desc = "Show info about the generated files in the build directory",
                     .detailed_desc = "Explain how the generated build directory is used.\n"
                     "Describe what files are expected to appear there for configured targets and platforms."         },

    [CMD_CFGFILE] = { .name = "project",
                     .params = "",
                     .desc = "Show help on how to write a config file for your project",
                     .detailed_desc = "Describe the main project configuration file format.\n"
                     "Explain which attributes can be declared and how bbs reads the file from the project root."     },

    [CMD_USERCFG] = {    .name = "user",
                     .params = "",
                     .desc = "Show help on the shared user config file",
                     .detailed_desc = "Explain how to use the shared user config file to define default settings.\n"
                     "These defaults can be reused across projects and overridden locally when needed."               },

    [CMD_LOCALCFG] = {   .name = "local",
                     .params = "",
                     .desc = "Show help on the machine-specific local config file",
                     .detailed_desc = "Explain how to use the local config file for machine-specific settings.\n"
                     "Store local paths and other overrides there when they should not be shared with the project."   },

    [CMD_CLEAN] = {   .name = "clean",
                     .params = "[project|user|local|toolchain]",
                     .desc = "Delete selected bbs config files",
                     .detailed_desc = "Delete one selected bbs config file.\n"
                     "Use 'project', 'user', 'local', or 'toolchain' to clean only that file.\n"
                     "If no argument is provided, bbs cleans both 'project' and 'local'."                             },

    [CMD_UPDATE] = {  .name = "update",
                     .params = "[-i|--info] [-c config] [--init-toolchain]",
                     .desc = "Set up derived project folders",
                     .detailed_desc = "Parse and validate the project configuration, then create the derived project directories used by bbs.\n"
                     "Use '-i' or '--info' to print the resolved parsed project state before updating.\n"
                     "Use '-c' or '--config' to resolve the project using a specific named config for the info output.\n"
                     "If the toolchain file is missing, update generates it automatically.\n"
                     "Use '--init-toolchain' to force regeneration instead of using the cached toolchain file."       },

    [CMD_CFG] = {     .name = "cfg",
                     .params = "[-m] [-p] [-u] [-l] [-t]",
                     .desc = "Show the resolved config file paths",
                     .detailed_desc = "Print the resolved config file paths used by bbs.\n"
                     "Use -m or --minimal to print raw paths only.\n"
                     "Use -p or --project to print the project config path.\n"
                     "Use -u or --user to print the shared user config path.\n"
                     "Use -l or --local to print the machine-local config path.\n"
                     "Use -t or --toolchain to print the toolchain config path.\n"
                     "If none of -p, -u, -t or -l is provided, bbs prints all three paths."                           },

    [CMD_BUILD] = {   .name = "build",
                     .params = "[-t target] [-p platform] [-c config]",
                     .desc = "Build and compile the project",
                     .detailed_desc = "Compile the selected target for the requested platform.\n"
                     "If no target is provided, bbs uses the only project target when possible or operates on all targets.\n"
                     "If no platform is provided, the default configured platform selection is used.\n"
                     "If no config is provided, bbs resolves the project using the 'default' config."                 },

    [CMD_RUN] = {     .name = "run",
                     .params = "[-t target] [-p platform] [-c config] | [optional args]",
                     .desc = "Run a built project, automatically building it if required",
                     .detailed_desc = "Build the selected runnable target if needed, then execute it.\n"
                     "Any remaining arguments are forwarded to the program.\n"
                     "The optional platform selects which build output to run when multiple platforms are available.\n"
                     "If no config is provided, bbs resolves the project using the 'default' config."                 },

    [CMD_INFO] = {    .name = "info",
                     .params = "<project|user|local|toolchain> [attribute] [-m] [--attr=path] [--filter=text] [--values-only]",
                     .desc = "Display parsed config or toolchain information",
                     .detailed_desc = "Use 'project', 'user', or 'local' to inspect the parsed nodes from that file.\n"
                     "Use the optional attribute argument or '--attr=path' to show only one attribute.\n"
                     "Use '-m' or '--minimal' to print only matching attribute paths.\n"
                     "Use '--filter=text' to show only matching attributes.\n"
                     "Use '--values-only' to print only matching values.\n"
                     "Use 'toolchain' to inspect the raw parsed toolchain.bbs file just like the other config files." },

    [CMD_DIST] = {    .name = "dist",
                     .params = "[-t target] [-p platform] [-c config]",
                     .desc = "Prepare the built project for distribution",
                     .detailed_desc = "Collect the selected build output and prepare it for distribution.\n"
                     "Use the optional target and platform arguments to dist a specific artifact when the project produces multiple outputs.\n"
                     "If no config is provided, bbs resolves the project using the 'default' config."                 },

    [CMD_TEST] = {    .name = "test",
                     .params = "[name] [-t target] [-p platform] [-c config]",
                     .desc = "Run unit tests for the current project",
                     .detailed_desc = "Run the project's test suite, or a specific named test when provided.\n"
                     "The optional target and platform arguments restrict execution to the matching test target or cross-compilation output.\n"
                     "If no config is provided, bbs resolves the project using the 'default' config."                 },

    [CMD_BUMPVER] = { .name = "bumpver",
                     .params = "<major/minor/patch/user/all> [-p project_id] [-t target_id]",
                     .desc = "Increment a project or target version",
                     .detailed_desc = "Increment the selected version according to the requested part.\n"
                     "Use '-p' or '--project' to choose a specific project node when the file defines more than one.\n"
                     "Use '-t' or '--target' to bump a target-local version instead of the top-level project version."},
};
