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
                     .desc = "Project definition",
                     .detailed_desc = "Defines the project, its targets, supported platforms, and build settings.\n"
                     "Keep it in the project root.\n"
                     "bbs reads this file first, then applies defaults from 'user.bbs' and overrides from 'local.bbs' when present."},

     [CFG_USER] = {.loc = CFG_LOC_EXE,
                      .filename = "user.bbs",
                      .desc = "Shared user defaults",
                      .detailed_desc = "Stores your default settings across projects.\n"
                      "Keep it next to the bbs executable.\n"
                      "Values from 'user.bbs' can be overridden by 'local.bbs' in a specific project."},

     [CFG_LOCAL] = {.loc = CFG_LOC_CWD,
                       .filename = "local.bbs",
                       .desc = "Machine-local overrides",
                       .detailed_desc = "Stores machine-specific overrides for a single project.\n"
                       "Keep it in the project root next to 'project.bbs'.\n"
                       "Use it for values that should not be shared.\n"
                       "In most projects this file should be added to .gitignore."},

     [CFG_TOOLCHAIN] = {.loc = CFG_LOC_EXE,
                      .filename = "toolchain.bbs",
                      .desc = "Generated toolchain cache",
                      .detailed_desc = "Caches detected toolchain state.\n"
                      "bbs creates and refreshes this file while preparing the build environment.\n"
                      "Keep it next to the bbs executable.\n"
                      "Regenerate it with 'bbs update --init-toolchain' after local toolchain changes."},
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
  CMD_GEN,
  CMD_CFG,
  CMD_BUILD,
  CMD_RUN,
  CMD_INFO,
  CMD_PACKAGE,
  CMD_DIST,
  CMD_TEST,
  CMD_BUMPVER,
  CMD_MAX,
} cmd;

static cmd_info CMD_INFOS[CMD_MAX] = {
     [CMD_HELP] = {   .name = "help",
                   .params = "[command/topic]",
                   .desc = "Show help for commands and configs",
                   .detailed_desc = "Without arguments, lists all available commands and config topics.\n"
                   "Pass a command or config name to show its usage and details."},

     [CMD_CLEAN] = {  .name = "clean",
                   .params = "[project|user|local|toolchain]",
                   .desc = "Remove selected bbs config files",
                   .detailed_desc = "Removes one selected bbs config file.\n"
                   "Use 'project', 'user', 'local', or 'toolchain' to clean only that file.\n"
                   "If no argument is provided, bbs cleans both 'project' and 'local'."},

     [CMD_UPDATE] = { .name = "update",
                     .params = "[-i|--info] [-c config] [--init-toolchain] [--refresh-packages]",
                     .desc = "Generate derived project files",
                     .detailed_desc = "Parses and validates the project configuration, then generates the derived files and directories used by bbs.\n"
                     "Use '-i' or '--info' to print the resolved project state before updating.\n"
                     "Use '-c' or '--config' to resolve the project with a specific config for the info output.\n"
                     "If the toolchain file is missing, bbs generates it automatically.\n"
                     "Use '--init-toolchain' to force regeneration instead of reusing the cached toolchain file.\n"
                     "Use '--refresh-packages' to re-fetch repo-backed packages into the shared cache before regenerating backend files."},

     [CMD_GEN] = {    .name = "gen",
                   .params = "<format> [-o|--override] [-p platform[,platform...]]",
                    .desc = "Generate utility files in the project root",
                    .detailed_desc = "Generates one supported utility file in the current project root.\n"
                     "Use 'gitignore' to generate a complete '.gitignore' template for bbs-based C/C++ projects.\n"
                    "Use 'github' to generate a multi-platform GitHub Actions workflow for build, test, run, and dist commands.\n"
                    "Use '-p' or '--platform' with a comma-separated list such as 'windows-x86_64,linux-x86_64' to override the workflow matrix.\n"
                    "Custom generators can also be defined in 'user.bbs' or 'local.bbs' with 'gen(name(...), copyfile(...))'.\n"
                    "If the target file already exists, bbs refuses to overwrite it unless '-o' or '--override' is provided."},

     [CMD_CFG] = {    .name = "cfg",
                   .params = "[-m] [-p] [-u] [-l] [-t]",
                   .desc = "Show resolved config paths",
                   .detailed_desc = "Prints the resolved config file paths used by bbs.\n"
                   "Use -m or --minimal to print raw paths only.\n"
                   "Use -p or --project to print the project config path.\n"
                   "Use -u or --user to print the shared user config path.\n"
                   "Use -l or --local to print the machine-local config path.\n"
                   "Use -t or --toolchain to print the toolchain config path.\n"
                   "If none of -p, -u, -t or -l is provided, bbs prints all four paths."},

     [CMD_BUILD] = {  .name = "build",
                   .params = "[-t target|*] [-p platform|*] [-c config|*]",
                   .desc = "Build selected targets",
                   .detailed_desc = "Builds the selected target for the requested platform.\n"
                   "If no target is provided, bbs uses the only project target when possible or operates on all targets.\n"
                   "If no platform is provided, the default platform selection is used.\n"
                   "Use '*' with -t, -p, or -c to execute the command for all matching targets, platforms, or configs.\n"
                   "If no config is provided, bbs resolves the project using the 'default' config."},

     [CMD_RUN] = {    .name = "run",
                   .params = "[-t target|*] [-p platform|*] [-c config|*] | [optional args]",
                   .desc = "Run a target, building it if needed",
                   .detailed_desc = "Builds the selected runnable target if needed, then executes it.\n"
                   "Any remaining arguments are forwarded to the program.\n"
                   "The optional platform selects which build output to run when multiple platforms are available.\n"
                   "Use '*' with -t or -c to execute the command for all matching runnable targets or configs.\n"
                   "Use '-p *' to run all host-native runnable outputs for the current machine.\n"
                   "If no config is provided, bbs resolves the project using the 'default' config."},

      [CMD_INFO] = {   .name = "info",
                    .params = "<project|user|local|toolchain> [attribute] [-m] [--attr=path] [--filter=text] [--values-only]",
                    .desc = "Inspect parsed config data",
                    .detailed_desc = "Use 'project', 'user', or 'local' to inspect the parsed nodes from that file.\n"
                    "Use the optional attribute argument or '--attr=path' to show only one attribute.\n"
                    "Use '-m' or '--minimal' to print only matching attribute paths.\n"
                    "Use '--filter=text' to show only matching attributes.\n"
                    "Use '--values-only' to print only matching values.\n"
                    "Use 'toolchain' to inspect the parsed toolchain.bbs file just like the other config files."},

     [CMD_PACKAGE] = { .name = "package",
                    .params = "<list|refresh [name|*]|package_name> [--refresh]",
                    .desc = "Inspect configured packages",
                    .detailed_desc = "Use 'bbs package list' to print all package-backed targets in the current project.\n"
                    "Use 'bbs package refresh' to refresh all repo/archive-backed packages, or 'bbs package refresh <name>' to refresh one package.\n"
                    "Use 'bbs package <package_name>' to inspect one configured package, including its source, cache location, and resolution status.\n"
                    "Use '--refresh' to re-fetch repo-backed packages before printing package info."},

      [CMD_DIST] = {   .name = "dist",
                    .params = "[-t target|*] [-p platform|*] [-c config|*]",
                    .desc = "Package build output for distribution",
                   .detailed_desc = "Collects the selected build output and prepares it for distribution.\n"
                   "Use the optional target and platform arguments to package a specific artifact when the project produces multiple outputs.\n"
                   "Use '*' with -t, -p, or -c to execute the command for all matching targets, platforms, or configs.\n"
                   "If no config is provided, bbs resolves the project using the 'default' config."},

     [CMD_TEST] = {   .name = "test",
                   .params = "[name] [-t target|*] [-p platform|*] [-c config|*]",
                   .desc = "Run project tests",
                   .detailed_desc = "Runs the project's test suite, or a specific named test when provided.\n"
                   "The optional target and platform arguments restrict execution to the matching test target or cross-compilation output.\n"
                   "Use '*' with -t, -p, or -c to execute the command for all matching test targets, platforms, or configs.\n"
                   "If no config is provided, bbs resolves the project using the 'default' config."},

     [CMD_BUMPVER] = {.name = "bumpver",
                   .params = "<major/minor/patch/user/all> [-p project_id] [-t target_id]",
                   .desc = "Increment a project or target version",
                   .detailed_desc = "Increments the selected version according to the requested part.\n"
                   "Use '-p' or '--project' to choose a specific project node when the file defines more than one.\n"
                   "Use '-t' or '--target' to bump a target-local version instead of the top-level project version."},
};
