#include "bbs.h"

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
  printf("  %s\n", usage);
}

static void print_cmd_help(cmd command) {
  const cmd_info info = CMD_INFOS[command];
  char usage[128] = {0};

  format_cmd_usage(usage, sizeof(usage), command);
  printf("  %-62s %s\n", usage, info.desc);
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
  printf("Better Build System v%u.%u\n", VER_MAJOR, VER_MINOR);
  printf("An integrated build system for C/C++ projects.\n");

  print_section("USAGE");
  printf("  bbs <command> [arguments]\n");
  printf("  bbs %s [command/topic]\n", CMD_INFOS[CMD_DEFAULT].name);

  print_section("GET STARTED");
  printf("  Create '{PROJECT_DIR}/%s' in the project root directory.\n", PROJ_FILENAME);
  printf("  Store shared user defaults next to the bbs executable as '%s'.\n", USER_FILENAME);
  printf("  Store machine-specific project overrides in '{PROJECT_DIR}/%s'.\n", LOCAL_FILENAME);

  print_section("NEXT STEP");
  printf("  Run 'bbs %s' to see all commands and help topics.\n", CMD_INFOS[CMD_DEFAULT].name);
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
      printf("Warning: 'bbs help %s' is not a recognized command. Showing general help instead.\n",
             argv[2]);
      help_page = CMD_DEFAULT;
    }
  }

  printf("Better Build System v%u.%u\n", VER_MAJOR, VER_MINOR);
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
      printf("  The build directory contains generated files and compiled artifacts.\n");
      printf("  bbs keeps source files in the project tree and writes derived outputs under the configured build directory.\n");
      printf("  Clean this directory when you want to remove cached outputs or force a full rebuild.\n");
      break;
    }
    case CMD_CFGFILE: {
      print_cmd_detailed_help(help_page);
      print_section("TOPIC NOTES");
      printf("  '%s' is the main project configuration file.\n", PROJ_FILENAME);
      printf("  Keep it in the project root and declare project metadata, targets, platforms, and build settings there.\n");
      printf("  bbs reads this file first, then applies defaults from '%s' and overrides from '%s' when present.\n",
             USER_FILENAME,
             LOCAL_FILENAME);
      break;
    }
    case CMD_LOCALCFG: {
      print_cmd_detailed_help(help_page);
      print_section("TOPIC NOTES");
      printf("  '%s' stores user config variables and project attributes specific to this machine.\n", LOCAL_FILENAME);
      printf("  Keep it in the project root directory, next to '%s'.\n", PROJ_FILENAME);
      printf("  Values in '%s' can override the defaults from '%s'.\n", LOCAL_FILENAME, USER_FILENAME);
      printf("  In normal projects this file should usually be added to .gitignore.\n");
      printf("  Use it for machine-specific settings that should not be shared with the rest of the project.\n");
      break;
    }
    case CMD_USERCFG: {
      print_cmd_detailed_help(help_page);
      print_section("TOPIC NOTES");
      printf("  '%s' stores shared user config defaults.\n", USER_FILENAME);
      printf("  Use it to define your preferred defaults across projects.\n");
      printf("  Values from '%s' can be overridden by '%s' in a specific project.\n", USER_FILENAME, LOCAL_FILENAME);
      printf("  Keep it next to the bbs executable so those defaults are available everywhere you run bbs.\n");
      break;
    }
    default: {
      print_cmd_detailed_help(help_page);
      break;
    }
  }

  print_section("NOTES");
  printf("  [] indicates optional arguments. <> indicates required arguments.\n");
  printf("  All commands operate on the project rooted at the current working directory.\n");
  printf("  The build system is initialized automatically when a command runs.\n");
  printf("  [target] can be inferred when the project only has one target; otherwise bbs operates on all targets.\n");
  printf("  [platform] selects one of the platforms declared in '%s'.\n", PROJ_FILENAME);

  print_section("MORE HELP");
  for (int i = CMD_DEFAULT + 1; i < CMD_HELP_END; ++i)
    printf("  bbs help %-29s %s\n", CMD_INFOS[i].name, CMD_INFOS[i].desc);
  printf("  bbs %s <command>\n", CMD_INFOS[CMD_DEFAULT].name);
}

static int run_cmd(cmd c, int argc, char** argv, base_cfgs* cfgs) {
  if (c == CMD_CFG) {
    u32 minimal_mode = false;
    if ((argc >= 3) && (_strcmpi("-m", argv[2]) == 0)) {  // TODO: Properly document this
      minimal_mode = true;
    }

    if (minimal_mode) {
      printf("%s\n", cfgs->project);
      printf("%s\n", cfgs->user);
      printf("%s\n", cfgs->local);
    } else {
      printf("  Project config: %s\n", cfgs->project);
      printf("  User config:    %s\n", cfgs->user);
      printf("  Local config:   %s\n", cfgs->local);
    }
    return 0;
  }

  printf("Command '%s' is not implemented yet.\n", CMD_INFOS[c].name);
  return 1;
}

static int print_unrecognized_command(const char* name) {
  printf("Error: '%s' is not a recognized command.\n", name);
  printf("Run 'bbs %s' to see the list of available commands.\n", CMD_INFOS[CMD_DEFAULT].name);
  return 1;
}

int main(int argc, char** argv) {
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

  for (int i = CMD_HELP_END; i < CMD_MAX; ++i) {
    if (_strcmpi(argv[1], CMD_INFOS[i].name) == 0) {
      return run_cmd((cmd)i, argc, argv, &cfgs);
    }
  }

  return print_unrecognized_command(argv[1]);
}
