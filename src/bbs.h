#pragma once

#include "bbs_base.h"

typedef struct {
  const char* name;
  const char* params;
  const char* desc;
  const char* detailed_desc;
} cmd_info;

typedef struct {
  const char* user;
  const char* project;
  const char* local;
} base_cfgs;

typedef enum {

  /////////////////////////////////////////
  // HELPER COMMANDS
  /////////////////////////////////////////

  // Show generic help (commands)
  CMD_DEFAULT,

  // Show help about the generated build directory
  CMD_BUILDIR,

  // Show help about the config file
  CMD_CFGFILE,

  // Show help about the shared user config file
  CMD_USERCFG,

  // Show help about the local machine-specific config file
  CMD_LOCALCFG,

  // End of help commands
  CMD_HELP_END,

  /////////////////////////////////////////
  // USER COMMANDS
  /////////////////////////////////////////

  // Help for each command
  CMD_CLEAN = CMD_HELP_END,
  CMD_CFG,
  CMD_BUILD,
  CMD_RUN,
  CMD_INFO,
  CMD_PACKAGE,
  CMD_TEST,
  CMD_BUMPVER,
  CMD_UPDATE,
  CMD_MAX,
} cmd;

static cmd_info CMD_INFOS[CMD_MAX] = {
    [CMD_DEFAULT] = {    .name = "help",
                     .params = "[command/topic]",
                     .desc = "Show generic help or help for a specific command",
                     .detailed_desc = "Without arguments, list all user commands.\n"
                     "Pass a command or help topic to show its syntax, purpose, and any command-specific notes."                                },

    [CMD_BUILDIR] = {.name = "builddir",
                     .params = "",
                     .desc = "Show info about the generated files in the build directory",
                     .detailed_desc = "Explain how the generated build directory is used.\n"
                     "Describe what files are expected to appear there for configured targets and platforms."                                   },

    [CMD_CFGFILE] = { .name = "cfgfile",
                     .params = "",
                     .desc = "Show help on how to write a config file for your project",
                     .detailed_desc = "Describe the main project configuration file format.\n"
                     "Explain which attributes can be declared and how bbs reads the file from the project root."                               },

    [CMD_USERCFG] = { .name = "usercfg",
                     .params = "",
                     .desc = "Show help on the shared user config file",
                     .detailed_desc = "Explain how to use the shared user config file to define default settings.\n"
                     "These defaults can be reused across projects and overridden locally when needed."                                         },

    [CMD_LOCALCFG] = {.name = "localcfg",
                     .params = "",
                     .desc = "Show help on the machine-specific local config file",
                     .detailed_desc = "Explain how to use the local config file for machine-specific settings.\n"
                     "Store local paths and other overrides there when they should not be shared with the project."                             },

    [CMD_CLEAN] = {   .name = "clean",
                     .params = "",
                     .desc = "Cleanup the directory with the build artifacts",
                     .detailed_desc = "Remove generated build artifacts for the current project.\n"
                     "Use this when you want to discard previous outputs and force the next build to start from a clean state."                 },

    [CMD_CFG] = {     .name = "cfg",
                     .params = "[-m] [-p] [-u] [-l]",
                     .desc = "Show the resolved config file paths",
                     .detailed_desc = "Print the resolved config file paths used by bbs.\n"
                     "Use -m or --minimal to print raw paths only.\n"
                     "Use -p or --project to print the project config path.\n"
                     "Use -u or --user to print the shared user config path.\n"
                     "Use -l or --local to print the machine-local config path.\n"
                     "If none of -p, -u, or -l is provided, bbs prints all three paths."                                                        },

    [CMD_BUILD] = {   .name = "build",
                     .params = "[target] [platform]",
                     .desc = "Build and compile the project",
                     .detailed_desc = "Compile the selected target for the requested platform.\n"
                     "If no target is provided, bbs uses the only project target when possible or operates on all targets.\n"
                     "If no platform is provided, the default configured platform selection is used."                                           },

    [CMD_RUN] = {     .name = "run",
                     .params = "[target] [platform] [optional args]",
                     .desc = "Run a built project, automatically building it if required",
                     .detailed_desc = "Build the selected runnable target if needed, then execute it.\n"
                     "Any remaining arguments are forwarded to the program.\n"
                     "The optional platform selects which build output to run when multiple platforms are available."                           },

    [CMD_INFO] = {    .name = "info",
                     .params = "[attribute] [target] [platform]",
                     .desc = "Display the project attributes",
                     .detailed_desc = "Print project or target information known to bbs.\n"
                     "Use the optional attribute name to inspect a specific value.\n"
                     "Provide a target or platform when the result depends on that context."                                                    },

    [CMD_PACKAGE] = { .name = "package",
                     .params = "[target] [platform]",
                     .desc = "Prepare the built project for distribution",
                     .detailed_desc = "Collect the selected build output and prepare it for distribution.\n"
                     "Use the optional target and platform arguments to package a specific artifact when the project produces multiple outputs."},

    [CMD_TEST] = {    .name = "test",
                     .params = "[name] [target] [platform]",
                     .desc = "Run unit tests for the current project",
                     .detailed_desc = "Run the project's test suite, or a specific named test when provided.\n"
                     "The optional target and platform arguments restrict execution to the matching test target or cross-compilation output."   },

    [CMD_BUMPVER] = { .name = "bumpver",
                     .params = "<major/minor/patch/all> [target] [platform]",
                     .desc = "Increment the version of the project",
                     .detailed_desc = "Increment the project version according to the requested part.\n"
                     "Use major, minor, patch, or all depending on how versioning is defined for the project.\n"
                     "Optional target and platform arguments limit the scope when version data is target-specific."                             },

    [CMD_UPDATE] = {  .name = "update",
                     .params = "",
                     .desc = "Install or update all dependencies as necessary",
                     .detailed_desc = "Resolve, install, or refresh external dependencies required by the current project.\n"
                     "This ensures later build, test, and package commands can run with the expected toolchain and libraries."                  },
};
