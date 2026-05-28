#include "bbs.h"
#include "bbs_base.c"

static void print_section(const char* title) {
  printf("\n%s\n", title);
}

static void print_indented_text(const char* text) {
  printf("  ");
  while (*text) {
    putchar(*text);
    if (*text == '\n' && text[1] != '\0') {
      printf("  ");
    }
    ++text;
  }
  printf("\n");
}

static void format_cmd_usage(char* out, size_t out_size, cmd command) {
  const cmd_info info = CMD_INFOS[command];

  if (info.params && info.params[0] != '\0') {
    snprintf(out, out_size, "bbs %s %s", info.name, info.params);
    return;
  }

  snprintf(out, out_size, "bbs %s", info.name);
}

static void build_path(char* out, size_t out_size, const char* dir, const char* name) {
  if (!dir || dir[0] == '\0') {
    snprintf(out, out_size, "%s", name);
    return;
  }

  snprintf(out, out_size, "%s\\%s", dir, name);
}

static void get_parent_dir(char* path) {
  char* last_slash = strrchr(path, '\\');
  if (!last_slash) {
    last_slash = strrchr(path, '/');
  }

  if (last_slash) {
    *last_slash = '\0';
  }
}

static void resolve_base_cfgs(
    base_cfgs* cfgs,
    const char* argv0,
    char* project_path,
    size_t project_path_size,
    char* user_path,
    size_t user_path_size,
    char* local_path,
    size_t local_path_size) {
  char cwd[_MAX_PATH] = {0};
  char exe_dir[_MAX_PATH] = {0};

  if (!_getcwd(cwd, sizeof(cwd))) {
    cwd[0] = '\0';
  }

  if (argv0 && argv0[0] != '\0') {
    char exe_path[_MAX_PATH] = {0};

    if (_fullpath(exe_path, argv0, sizeof(exe_path))) {
      snprintf(exe_dir, sizeof(exe_dir), "%s", exe_path);
      get_parent_dir(exe_dir);
    } else {
      snprintf(exe_dir, sizeof(exe_dir), "%s", argv0);
      get_parent_dir(exe_dir);
    }
  } else {
    exe_dir[0] = '\0';
  }

  build_path(project_path, project_path_size, cwd, PROJ_FILENAME);
  build_path(local_path, local_path_size, cwd, LOCAL_FILENAME);
  build_path(user_path, user_path_size, exe_dir[0] != '\0' ? exe_dir : cwd, USER_FILENAME);

  cfgs->project = project_path;
  cfgs->user = user_path;
  cfgs->local = local_path;
}

static void print_cmd_synopsis(cmd command) {
  char usage[128] = {0};
  format_cmd_usage(usage, sizeof(usage), command);
  print("  %s", usage);
}

static void print_cmd_help(cmd command) {
  const cmd_info info = CMD_INFOS[command];
  char usage[128] = {0};

  format_cmd_usage(usage, sizeof(usage), command);
  print("  %-62s %s", usage, info.desc);
}

static void print_cmd_detailed_help(cmd command) {
  const cmd_info info = CMD_INFOS[command];

  print_section("COMMAND");
  print_cmd_synopsis(command);

  print_section("DESCRIPTION");
  printf("  %s\n", info.desc);

  if (info.detailed_desc && info.detailed_desc[0] != '\0') {
    print_section("DETAILS");
    print_indented_text(info.detailed_desc);
  }
}

static void print_usage(void) {
  print("Better Build System v%u.%u", VER_MAJOR, VER_MINOR);
  print("An integrated build system for C/C++ projects.");

  print_section("USAGE");
  print("  bbs <command> [arguments]");
  print("  bbs %s [command/topic]", CMD_INFOS[CMD_DEFAULT].name);

  print_section("GET STARTED");
  print("  Create '{PROJECT_DIR}/%s' in the project root directory.", PROJ_FILENAME);
  print("  Store shared user defaults next to the bbs executable as '{BBS_DIR}/%s'.", USER_FILENAME);
  print("  Store machine-specific project overrides in '{PROJECT_DIR}/%s'.", LOCAL_FILENAME);

  print_section("NEXT STEP");
  print("  Run 'bbs %s' to see all commands and help topics.", CMD_INFOS[CMD_DEFAULT].name);
}

static cmd parse_cmd_name(const char* name) {
  for (int i = CMD_DEFAULT + 1; i < CMD_MAX; ++i) {
    if (CMD_INFOS[i].name && _strcmpi(name, CMD_INFOS[i].name) == 0) {
      return (cmd)i;
    }
  }

  return CMD_MAX;
}

static void print_help(int argc, char** argv) {
  cmd help_page = CMD_DEFAULT;
  if (argc > 2) {
    help_page = parse_cmd_name(argv[2]);
    if (help_page == CMD_MAX) {
      warn("'bbs help %s' is not a recognized command. Showing general help instead", argv[2]);
      help_page = CMD_DEFAULT;
    }
  }

  print("Better Build System v%u.%u", VER_MAJOR, VER_MINOR);
  switch (help_page) {
    case CMD_DEFAULT: {
      print_section("COMMANDS");
      for (int i = CMD_HELP_END; i < CMD_MAX; ++i) {
        print_cmd_help((cmd)i);
      }
      break;
    }
    case CMD_BUILDIR: {
      print_cmd_detailed_help(help_page);
      print_section("TOPIC NOTES");
      print("  The build directory contains generated files and compiled artifacts.");
      print("  bbs keeps source files in the project tree and writes derived outputs under the configured build directory.");
      print("  Clean this directory when you want to remove cached outputs or force a full rebuild.");
      break;
    }
    case CMD_CFGFILE: {
      print_cmd_detailed_help(help_page);
      print_section("TOPIC NOTES");
      print("  '%s' is the main project configuration file.", PROJ_FILENAME);
      print("  Keep it in the project root and declare project metadata, targets, platforms, and build settings there.");
      print("  bbs reads this file first, then applies defaults from '%s' and overrides from '%s' when present.", USER_FILENAME, LOCAL_FILENAME);
      break;
    }
    case CMD_LOCALCFG: {
      print_cmd_detailed_help(help_page);
      print_section("TOPIC NOTES");
      print("  '%s' stores user config variables and project attributes specific to this machine.", LOCAL_FILENAME);
      print("  Keep it in the project root directory, next to '%s'.", PROJ_FILENAME);
      print("  Values in '%s' can override the defaults from '%s'.", LOCAL_FILENAME, USER_FILENAME);
      print("  In normal projects this file should usually be added to .gitignore.");
      print("  Use it for machine-specific settings that should not be shared with the rest of the project.");
      break;
    }
    case CMD_USERCFG: {
      print_cmd_detailed_help(help_page);
      print_section("TOPIC NOTES");
      print("  '%s' stores shared user config defaults.", USER_FILENAME);
      print("  Use it to define your preferred defaults across projects.");
      print("  Values from '%s' can be overridden by '%s' in a specific project.", USER_FILENAME, LOCAL_FILENAME);
      print("  Keep it next to the bbs executable so those defaults are available everywhere you run bbs.");
      break;
    }
    default: {
      print_cmd_detailed_help(help_page);
      break;
    }
  }

  print_section("NOTES");
  print("  [] indicates optional arguments. <> indicates required arguments.");
  print("  All commands operate on the project rooted at the current working directory.");
  print("  The build system is initialized automatically when a command runs.");
  print("  [target] can be inferred when the project only has one target; otherwise bbs operates on all targets.");
  print("  [platform] selects one of the platforms declared in '%s'.", PROJ_FILENAME);

  print_section("MORE HELP");
  for (int i = CMD_DEFAULT + 1; i < CMD_HELP_END; ++i)
    print("  bbs help %-29s %s", CMD_INFOS[i].name, CMD_INFOS[i].desc);
  print("  bbs %s <command>", CMD_INFOS[CMD_DEFAULT].name);
}

static int error_code(cmd c, char idx) {
  return 200 + c * 255 + idx;
}

static int run_cmd_cfg(cmdline* cl, base_cfgs* cfgs) {
  enum {
    MINIMAL = 0,
    PROJECT,
    USER,
    LOCAL
  };

  cmdopt opts[] = {
      [MINIMAL] = {"m", "minimal"},
      [PROJECT] = {"p", "project"},
      [USER] = {"u",    "user"},
      [LOCAL] = {"l",   "local"},
  };

  cmdline_options(cl, opts, _countof(opts), true);
  if ((opts[PROJECT].present == opts[USER].present) && (opts[USER].present == opts[LOCAL].present)) {
    opts[PROJECT].present = opts[USER].present = opts[LOCAL].present = true;
  }

  if (opts[0].present) {
    if (opts[PROJECT].present) print("%s", cfgs->project);
    if (opts[USER].present) print("%s", cfgs->user);
    if (opts[LOCAL].present) print("%s", cfgs->local);
  } else {
    if (opts[PROJECT].present) print("  Project config: %s", cfgs->project);
    if (opts[USER].present) print("  User config:    %s", cfgs->user);
    if (opts[LOCAL].present) print("  Local config:   %s", cfgs->local);
  }

  return 0;
}

static int run_cmd_clean(cmdline* cl, base_cfgs* cfgs) {
  return 0;
}

static int run_cmd_build(cmdline* cl, base_cfgs* cfgs) {
  return 0;
}

static int run_cmd_run(cmdline* cl, base_cfgs* cfgs) {
  return 0;
}

static int run_cmd_info(cmdline* cl, base_cfgs* cfgs) {
  return 0;
}

static int run_cmd_package(cmdline* cl, base_cfgs* cfgs) {
  return 0;
}

static int run_cmd_test(cmdline* cl, base_cfgs* cfgs) {
  return 0;
}

static int run_cmd_bumpver(cmdline* cl, base_cfgs* cfgs) {
  return 0;
}

static int run_cmd_update(cmdline* cl, base_cfgs* cfgs) {
  return 0;
}

static int run_cmd(cmd c, cmdline* cl, base_cfgs* cfgs) {
  switch (c) {
    case CMD_CFG:
      return run_cmd_cfg(cl, cfgs);
    case CMD_CLEAN:
      return run_cmd_clean(cl, cfgs);
    case CMD_BUILD:
      return run_cmd_build(cl, cfgs);
    case CMD_RUN:
      return run_cmd_run(cl, cfgs);
    case CMD_INFO:
      return run_cmd_info(cl, cfgs);
    case CMD_PACKAGE:
      return run_cmd_package(cl, cfgs);
    case CMD_TEST:
      return run_cmd_test(cl, cfgs);
    case CMD_BUMPVER:
      return run_cmd_bumpver(cl, cfgs);
    case CMD_UPDATE:
      return run_cmd_update(cl, cfgs);
    default: {
      print("Command '%s' is not implemented yet.", CMD_INFOS[c].name);
      return error_code(c, 1);
    }
  }

  return error_code(c, 0);
}

static int print_unrecognized_command(const char* name) {
  error("'%s' is not a recognized command.", name);
  print("Run 'bbs %s' to see the list of available commands.", CMD_INFOS[CMD_DEFAULT].name);
  return 2;
}

int main(int argc, char** argv) {
  atexit(release);

  if (argc == 1) {
    print_usage();
    return 0;
  }

  if (_strcmpi(argv[1], CMD_INFOS[CMD_DEFAULT].name) == 0) {
    print_help(argc, argv);
    return 0;
  }

  base_cfgs cfgs = {0};
  char project_path[_MAX_PATH] = {0};
  char user_path[_MAX_PATH] = {0};
  char local_path[_MAX_PATH] = {0};

  resolve_base_cfgs(
      &cfgs,
      argv[0],
      project_path,
      sizeof(project_path),
      user_path,
      sizeof(user_path),
      local_path,
      sizeof(local_path));

  cmdline cl = {.argv = (const char**)argv, .argc = argc};
  cmdline_pop(&cl);  // Pop argv[0]
  cmdline_pop(&cl);  // Pop argv[1]
  for (int i = CMD_HELP_END; i < CMD_MAX; ++i) {
    if (_strcmpi(argv[1], CMD_INFOS[i].name) == 0) {
      return run_cmd((cmd)i, &cl, &cfgs);
    }
  }

  return print_unrecognized_command(argv[1]);
}
