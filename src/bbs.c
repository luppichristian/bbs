#include "bbs.h"
#include "bbs_base.c"
#include "bbs_project.c"
#include "bbs_toolchain.c"

static void print_section(const char* title) {
  printf(ANSI_FG_INFO "\n%s\n" ANSI_RESET, title);
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
  print(ANSI_BOLD "Better Build System v%u.%u" ANSI_RESET, VER_MAJOR, VER_MINOR);
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

  print(ANSI_BOLD "Better Build System v%u.%u" ANSI_RESET, VER_MAJOR, VER_MINOR);
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
  print("  [-t target] can be inferred when the project only has one target; otherwise bbs operates on all targets.");
  print("  [-p platform] selects one of the platforms declared in '%s'.", PROJ_FILENAME);

  print_section("MORE HELP");
  for (int i = CMD_DEFAULT + 1; i < CMD_HELP_END; ++i)
    print("  bbs help %-29s %s", CMD_INFOS[i].name, CMD_INFOS[i].desc);
  print("  bbs %s <command>", CMD_INFOS[CMD_DEFAULT].name);
}

static int error_code(cmd c, char idx) {
  return 200 + c * 255 + idx;
}

static const char* cmdline_extract_option_value(cmdline* cl, const char* short_name, const char* long_name) {
  if (!cl || cl->argc <= 0)
    return NULL;

  int wi = 0;
  const char* value = NULL;

  for (int ri = 0; ri < cl->argc; ++ri) {
    const char* arg = cl->argv[ri];
    if (!arg) {
      cl->argv[wi++] = cl->argv[ri];
      continue;
    }

    if (long_name && cmdline_is_longopt(arg)) {
      const char* name = arg + 2;
      size_t long_len = strlen(long_name);
      if (_strnicmp(name, long_name, long_len) == 0) {
        if (name[long_len] == '=') {
          value = name + long_len + 1;
          continue;
        }
        if (name[long_len] == '\0') {
          if (ri + 1 < cl->argc && !cmdline_is_longopt(cl->argv[ri + 1]) && !cmdline_is_shortopt(cl->argv[ri + 1])) {
            value = cl->argv[++ri];
          } else {
            warn("option '--%s' expects a value.", long_name);
          }
          continue;
        }
      }
    }

    if (short_name && cmdline_is_shortopt(arg) && strlen(arg) == 2 && arg[1] == short_name[0]) {
      if (ri + 1 < cl->argc && !cmdline_is_longopt(cl->argv[ri + 1]) && !cmdline_is_shortopt(cl->argv[ri + 1])) {
        value = cl->argv[++ri];
      } else {
        warn("option '-%s' expects a value.", short_name);
      }
      continue;
    }

    cl->argv[wi++] = cl->argv[ri];
  }

  cl->argc = wi;
  return value;
}

static int run_cmd_cfg(cmd_ctx* cmdctx) {
  cmdline* cl = cmdctx->cl;
  enum {
    MINIMAL = 0,
    PROJECT,
    USER,
    LOCAL,
    TOOLCHAIN
  };

  cmdopt opts[] = {
      [MINIMAL] = {"m",   "minimal"},
      [PROJECT] = {"p",   "project"},
      [USER] = {"u",      "user"},
      [LOCAL] = {"l",     "local"},
      [TOOLCHAIN] = {"t", "toolchain"},
  };

  cmdline_consume_all_options(cl, opts, _countof(opts));
  cmdline_validate(cl);
  if ((opts[PROJECT].present == opts[USER].present) && (opts[USER].present == opts[LOCAL].present) && (opts[LOCAL].present == opts[TOOLCHAIN].present)) {
    opts[PROJECT].present = opts[USER].present = opts[LOCAL].present = opts[TOOLCHAIN].present = true;
  }

  if (opts[0].present) {
    if (opts[PROJECT].present) print("%s", cmdctx->project);
    if (opts[USER].present) print("%s", cmdctx->user);
    if (opts[LOCAL].present) print("%s", cmdctx->local);
    if (opts[TOOLCHAIN].present) print("%s", cmdctx->toolchain);
  } else {
    if (opts[PROJECT].present) print("  Project config: %s", cmdctx->project);
    if (opts[USER].present) print("  User config:    %s", cmdctx->user);
    if (opts[LOCAL].present) print("  Local config:   %s", cmdctx->local);
    if (opts[TOOLCHAIN].present) print("  Toolchain config:   %s", cmdctx->toolchain);
  }

  return 0;
}

static void toolchain_cmd_build_print_opts(toolchain_print_opts* out, cmdopt* opts) {
  if (!out || !opts)
    return;

  enum {
    MINIMAL = 0,
    TOOLS,
    SDKS,
    TYPE,
    TOOL,
    SDK,
    PATHS_ONLY,
    VERSIONS_ONLY,
  };

  memset(out, 0, sizeof(*out));
  out->minimal = opts[MINIMAL].present;
  out->show_tools = opts[TOOLS].present;
  out->show_sdks = opts[SDKS].present;
  out->paths_only = opts[PATHS_ONLY].present;
  out->versions_only = opts[VERSIONS_ONLY].present;
  if (!out->show_tools && !out->show_sdks)
    out->show_tools = out->show_sdks = true;

  if (out->paths_only && out->versions_only) {
    warn("'--paths-only' and '--versions-only' cannot be used together. Using '--paths-only'.");
    out->versions_only = false;
  }

  if (opts[TYPE].value && opts[TYPE].value[0]) {
    const char* type = opts[TYPE].value;
    if (_stricmp(type, "build_system") == 0 || _stricmp(type, "c_compiler") == 0 || _stricmp(type, "cpp_compiler") == 0 || _stricmp(type, "archiver") == 0 || _stricmp(type, "linker") == 0 || _stricmp(type, "misc") == 0) {
      out->type_filter = tool_type_from_idf(type);
      out->has_type_filter = true;
    } else {
      warn("ignoring unknown tool type '%s'.", type);
    }
  }
  out->tool_id_filter = opts[TOOL].value;
  out->sdk_name_filter = opts[SDK].value;
}

static int run_cmd_toolchain(cmd_ctx* cmdctx) {
  cmdline* cl = cmdctx->cl;
  const char* sub = cmdline_consume_param(cl);
  enum {
    MINIMAL = 0,
    TOOLS,
    SDKS,
    TYPE,
    TOOL,
    SDK,
    PATHS_ONLY,
    VERSIONS_ONLY,
  };

  cmdopt opts[] = {
      [MINIMAL] = { "m",       "minimal"},
      [TOOLS] = {NULL,         "tools"},
      [SDKS] = {NULL,          "sdks"},
      [TYPE] = {NULL,          "type"},
      [TOOL] = {NULL,          "tool"},
      [SDK] = {NULL,           "sdk"},
      [PATHS_ONLY] = {NULL,    "paths-only"},
      [VERSIONS_ONLY] = {NULL, "versions-only"},
  };

  cmdline_consume_all_options(cl, opts, _countof(opts));
  cmdline_validate(cl);

  toolchain_print_opts print_opts = {0};
  toolchain_cmd_build_print_opts(&print_opts, opts);

  if (sub && _strcmpi(sub, "init") == 0) {
    print("Initializing toolchain file: %s", cmdctx->toolchain);
    toolchain* tc = toolchain_init(cmdctx->toolchain, true, cmdctx);
    if (tc) {
      print("Toolchain initialized successfully.");
      toolchain_print_with_opts(tc, &print_opts);
    }
    return tc ? 0 : error_code(CMD_TOOLCHAIN, 0);
  }

  if (sub && _strcmpi(sub, "clean") == 0) {
    if (!file_exists(cmdctx->toolchain)) {
      print("Toolchain is not initialized. Nothing to clean.");
      return 0;
    }
    if (!file_delete(cmdctx->toolchain)) {
      return error_code(CMD_TOOLCHAIN, 0);
    }
    print("Toolchain config deleted.");
    return 0;
  }

  if (!file_exists(cmdctx->toolchain)) {
    print("Toolchain is not currently initialized.");
    print("Run 'bbs toolchain init' to manually init.");
    return 0;
  }

  toolchain* tc = toolchain_init(cmdctx->toolchain, false, cmdctx);
  print("Current toolchain setup:");
  toolchain_print_with_opts(tc, &print_opts);
  return 0;
}

static int clean_one_file(const char* label, const char* path, cmd c, char err_idx) {
  if (!file_exists(path)) {
    print("%s is not present. Nothing to clean.", label);
    return 0;
  }

  if (!file_delete(path)) {
    error("Failed to delete %s: %s", label, path);
    return error_code(c, err_idx);
  }

  print("Deleted %s: %s", label, path);
  return 0;
}

static int run_cmd_clean(cmd_ctx* cmdctx) {
  cmdline* cl = cmdctx->cl;
  const char* scope = cmdline_consume_param(cl);
  cmdline_validate(cl);

  if (!scope || !scope[0]) {
    if (!project_cleanup()) {
      return error_code(CMD_CLEAN, 5);
    }
    return 0;
  }

  if (_strcmpi(scope, "project") == 0)
    return clean_one_file("project config", cmdctx->project, CMD_CLEAN, 0);
  if (_strcmpi(scope, "user") == 0)
    return clean_one_file("user config", cmdctx->user, CMD_CLEAN, 2);
  if (_strcmpi(scope, "local") == 0)
    return clean_one_file("local config", cmdctx->local, CMD_CLEAN, 1);
  if (_strcmpi(scope, "toolchain") == 0)
    return clean_one_file("toolchain config", cmdctx->toolchain, CMD_CLEAN, 3);

  error("Unknown clean target '%s'.", scope);
  print("Use 'project', 'user', 'local', or 'toolchain'.");
  return error_code(CMD_CLEAN, 4);
}

static int run_cmd_update(cmd_ctx* cmdctx) {
  const char* config = cmdline_extract_option_value(cmdctx->cl, "c", "config");

  enum {
    INFO = 0,
  };

  cmdopt opts[] = {
      [INFO] = {"i", "info"},
  };

  cmdline_consume_all_options(cmdctx->cl, opts, _countof(opts));
  cmdline_validate(cmdctx->cl);

  if (opts[INFO].present) {
    project proj = {0};
    if (!project_load_config(config, &proj))
      return error_code(CMD_UPDATE, 1);
    project_print(&proj);
  }

   if (!project_update()) {
    return error_code(CMD_UPDATE, 0);
  }

  return 0;
}

static int run_cmd_build(cmd_ctx* cmdctx) {
  const char* target = cmdline_extract_option_value(cmdctx->cl, "t", "target");
  const char* platform = cmdline_extract_option_value(cmdctx->cl, "p", "platform");
  const char* config = cmdline_extract_option_value(cmdctx->cl, "c", "config");

  enum {
    TARGET = 0,
    PLATFORM,
  };

  cmdopt opts[] = {
      [TARGET] = {"t", "target"},
      [PLATFORM] = {"p", "platform"},
  };

  cmdline_consume_all_options(cmdctx->cl, opts, _countof(opts));
  cmdline_validate(cmdctx->cl);

  if (!project_build(target, platform, config))
    return error_code(CMD_BUILD, 0);

  return 0;
}

static int run_cmd_run(cmd_ctx* cmdctx) {
  const char* target = cmdline_extract_option_value(cmdctx->cl, "t", "target");
  const char* platform = cmdline_extract_option_value(cmdctx->cl, "p", "platform");
  const char* config = cmdline_extract_option_value(cmdctx->cl, "c", "config");

  enum {
    TARGET = 0,
    PLATFORM,
  };

  cmdopt opts[] = {
      [TARGET] = {"t", "target"},
      [PLATFORM] = {"p", "platform"},
  };

  cmdline_consume_all_options(cmdctx->cl, opts, _countof(opts));

  if (!project_run(target, platform, config))
    return error_code(CMD_RUN, 0);

  return 0;
}

static bool text_contains_ci(const char* text, const char* needle) {
  if (!needle || !needle[0])
    return true;
  if (!text)
    return false;

  size_t needle_len = strlen(needle);
  if (needle_len == 0)
    return true;

  for (const char* p = text; *p; ++p) {
    if (_strnicmp(p, needle, needle_len) == 0)
      return true;
  }

  return false;
}

static void info_indent(size_t depth) {
  for (size_t i = 0; i < depth; ++i)
    printf("  ");
}

static void info_pretty_name(const char* src, char* out, size_t out_size) {
  if (!out || out_size == 0)
    return;

  if (!src || !src[0]) {
    snprintf(out, out_size, "Value");
    return;
  }

  size_t wi = 0;
  bool new_word = true;
  for (size_t i = 0; src[i] && wi + 1 < out_size; ++i) {
    char ch = src[i];
    if (ch == '_' || ch == '-' || ch == '.') {
      if (wi > 0 && out[wi - 1] != ' ' && wi + 1 < out_size)
        out[wi++] = ' ';
      new_word = true;
      continue;
    }

    out[wi++] = new_word ? (char)toupper((unsigned char)ch) : ch;
    new_word = false;
  }

  out[wi] = '\0';
}

static void info_format_value(const node* n, char* out, size_t out_size) {
  if (!out || out_size == 0) {
    return;
  }

  out[0] = '\0';
  if (!n)
    return;

  switch (n->type) {
    case NODE_TYPE_STR:
      snprintf(out, out_size, "%.*s", (int)n->txt_dim, n->_str ? n->_str : "");
      break;
    case NODE_TYPE_INT:
      snprintf(out, out_size, "%lld", (long long)n->_int);
      break;
    case NODE_TYPE_FLT:
      snprintf(out, out_size, "%g", n->_flt);
      break;
    case NODE_TYPE_VER:
      if (n->ver_parts >= 4)
        snprintf(out, out_size, "%u.%u.%u.%u", n->_ver.major, n->_ver.minor, n->_ver.patch, n->_ver.user);
      else
        snprintf(out, out_size, "%u.%u.%u", n->_ver.major, n->_ver.minor, n->_ver.patch);
      break;
    case NODE_TYPE_IDF:
      snprintf(out, out_size, "%.*s", (int)n->txt_dim, n->_idf ? n->_idf : "");
      break;
    case NODE_TYPE_BOL:
      snprintf(out, out_size, "%s", n->_bol ? "Yes" : "No");
      break;
    default:
      break;
  }
}

static int info_child_count(node* n) {
  int count = 0;
  if (!n)
    return 0;

  node_foreach(n, child) {
    ++count;
  }
  return count;
}

static node** info_children_in_source_order(node* n, int* out_count) {
  if (out_count)
    *out_count = 0;
  if (!n)
    return NULL;

  int count = info_child_count(n);
  if (count <= 0)
    return NULL;

  node** items = push(sizeof(node*) * (size_t)count);
  if (!items)
    return NULL;

  int idx = count - 1;
  node_foreach(n, child) {
    items[idx--] = child;
  }

  if (out_count)
    *out_count = count;
  return items;
}

static void info_build_path(char* out, size_t out_size, const char* parent_path, node* n) {
  if (!out || out_size == 0)
    return;

  if (!n || !n->name || !n->name[0]) {
    snprintf(out, out_size, "%s", parent_path ? parent_path : "");
    return;
  }

  if (parent_path && parent_path[0])
    snprintf(out, out_size, "%s.%s", parent_path, n->name);
  else
    snprintf(out, out_size, "%s", n->name);
}

static int info_count_nodes(node* n) {
  if (!n)
    return 0;

  int total = 1;
  node_foreach(n, child) {
    total += info_count_nodes(child);
  }
  return total;
}

static bool info_node_matches_self(node* n, const char* path, const char* attr_filter, const char* text_filter) {
  if (!n)
    return false;

  if (attr_filter && attr_filter[0]) {
    bool path_match = path && _stricmp(path, attr_filter) == 0;
    bool name_match = n->name && _stricmp(n->name, attr_filter) == 0;
    if (!path_match && !name_match)
      return false;
  }

  if (text_filter && text_filter[0]) {
    char value[256] = {0};
    char label[256] = {0};
    info_format_value(n, value, sizeof(value));
    info_pretty_name(n->name, label, sizeof(label));
    if (!text_contains_ci(path, text_filter) && !text_contains_ci(n->name, text_filter) && !text_contains_ci(label, text_filter) && !text_contains_ci(value, text_filter))
      return false;
  }

  return true;
}

static bool info_node_matches_tree(node* n, const char* parent_path, const char* attr_filter, const char* text_filter) {
  if (!n)
    return false;

  char path[1024] = {0};
  info_build_path(path, sizeof(path), parent_path, n);
  if (info_node_matches_self(n, path, attr_filter, text_filter))
    return true;

  int child_count = 0;
  node** children = info_children_in_source_order(n, &child_count);
  for (int i = 0; i < child_count; ++i) {
    if (info_node_matches_tree(children[i], path, attr_filter, text_filter))
      return true;
  }

  return false;
}

static int info_count_visible_nodes(node* n, const char* parent_path, const char* attr_filter, const char* text_filter) {
  if (!n)
    return 0;

  int total = 0;
  int child_count = 0;
  node** children = info_children_in_source_order(n, &child_count);
  char path[1024] = {0};
  info_build_path(path, sizeof(path), parent_path, n);
  bool self_match = info_node_matches_self(n, path, attr_filter, text_filter);
  bool child_match = false;

  for (int i = 0; i < child_count; ++i) {
    int child_total = info_count_visible_nodes(children[i], path, attr_filter, text_filter);
    total += child_total;
    if (child_total > 0)
      child_match = true;
  }

  if (self_match || child_match)
    ++total;
  return total;
}

typedef enum {
  INFO_PRINT_NORMAL = 0,
  INFO_PRINT_MINIMAL,
  INFO_PRINT_VALUES_ONLY,
} info_print_mode;

static int info_count_flat_matches(node* n, const char* parent_path, const char* attr_filter, const char* text_filter) {
  if (!n)
    return 0;

  char path[1024] = {0};
  info_build_path(path, sizeof(path), parent_path, n);
  int total = 0;

  if (!n->children) {
    if (info_node_matches_self(n, path, attr_filter, text_filter))
      return 1;
    return 0;
  }

  int child_count = 0;
  node** children = info_children_in_source_order(n, &child_count);
  for (int i = 0; i < child_count; ++i) {
    total += info_count_flat_matches(children[i], path, attr_filter, text_filter);
  }
  return total;
}

static void info_print_flat_node(node* n, const char* parent_path, const char* attr_filter, const char* text_filter, info_print_mode mode) {
  if (!n)
    return;

  char path[1024] = {0};
  char value[512] = {0};
  info_build_path(path, sizeof(path), parent_path, n);

  if (!n->children) {
    if (!info_node_matches_self(n, path, attr_filter, text_filter))
      return;

    if (mode == INFO_PRINT_VALUES_ONLY) {
      info_format_value(n, value, sizeof(value));
      printf("%s\n", value[0] ? value : "(empty)");
    } else {
      printf("%s\n", path);
    }
    return;
  }

  int child_count = 0;
  node** children = info_children_in_source_order(n, &child_count);
  for (int i = 0; i < child_count; ++i) {
    info_print_flat_node(children[i], path, attr_filter, text_filter, mode);
  }
}

static void info_print_node(node* n, const char* parent_path, size_t depth, const char* attr_filter, const char* text_filter) {
  if (!n)
    return;

  char path[1024] = {0};
  char label[256] = {0};
  char value[512] = {0};
  info_build_path(path, sizeof(path), parent_path, n);
  if (!info_node_matches_tree(n, path[0] ? parent_path : "", attr_filter, text_filter))
    return;

  info_pretty_name(n->name, label, sizeof(label));
  info_format_value(n, value, sizeof(value));

  if (n->children) {
    info_indent(depth);
    printf(ANSI_BOLD "%s" ANSI_RESET, label);
    if (path[0])
      printf(ANSI_FG_TEXT "  [%s]" ANSI_RESET, path);
    printf("\n");

    int child_count = 0;
    node** children = info_children_in_source_order(n, &child_count);
    for (int i = 0; i < child_count; ++i) {
      info_print_node(children[i], path, depth + 1, attr_filter, text_filter);
    }
    return;
  }

  info_indent(depth);
  printf("%s: %s", label, value[0] ? value : "(empty)");
  if (path[0])
    printf(ANSI_FG_TEXT "  [%s]" ANSI_RESET, path);
  printf("\n");
}

static int run_cmd_info_file(const char* title, const char* path, const char* attr_filter, const char* text_filter, info_print_mode mode) {
  if (!file_exists(path)) {
    if (mode == INFO_PRINT_NORMAL) {
      print("%s is not present.", title);
      print("Expected path: %s", path);
    }
    return 0;
  }

  const char* data = read_entire_file(path);
  if (!data) {
    error("Failed to read '%s'.", path);
    return 1;
  }

  node* tree = node_parse(data);
  if (!tree) {
    error("Failed to parse '%s'.", path);
    return 1;
  }

  int total_nodes = 0;
  int visible_nodes = 0;
  int flat_matches = 0;
  int child_count = 0;
  node** children = info_children_in_source_order(tree, &child_count);
  for (int i = 0; i < child_count; ++i) {
    total_nodes += info_count_nodes(children[i]);
    visible_nodes += info_count_visible_nodes(children[i], "", attr_filter, text_filter);
    flat_matches += info_count_flat_matches(children[i], "", attr_filter, text_filter);
  }

  if (mode == INFO_PRINT_NORMAL) {
    print_section(title);
    print("File: %s", path);
    print("Nodes parsed: %d", total_nodes);
    if (attr_filter && attr_filter[0])
      print("Attribute: %s", attr_filter);
    if (text_filter && text_filter[0])
      print("Filter: %s", text_filter);
  }

  if ((mode == INFO_PRINT_NORMAL && visible_nodes == 0) || (mode != INFO_PRINT_NORMAL && flat_matches == 0)) {
    if (mode == INFO_PRINT_NORMAL)
      print("No matching attributes found.");
    return 0;
  }

  if (mode == INFO_PRINT_NORMAL) {
    print("Attributes shown: %d", visible_nodes);
    printf("\n");
  }

  for (int i = 0; i < child_count; ++i) {
    if (mode == INFO_PRINT_NORMAL)
      info_print_node(children[i], "", 0, attr_filter, text_filter);
    else
      info_print_flat_node(children[i], "", attr_filter, text_filter, mode);
  }
  return 0;
}

static int run_cmd_info(cmd_ctx* cmdctx) {
  cmdline* cl = cmdctx->cl;
  const char* scope = cmdline_consume_param(cl);

  if (!scope || !scope[0]) {
    error("Missing info topic.");
    print("Use 'bbs info project', 'bbs info user', 'bbs info local', or 'bbs info toolchain'.");
    return error_code(CMD_INFO, 0);
  }

  const char* positional_attr = cmdline_consume_param(cl);
  enum {
    ATTR = 0,
    FILTER,
    MINIMAL,
    VALUES_ONLY,
  };

  cmdopt opts[] = {
      [ATTR] = {NULL,        "attr"},
      [FILTER] = {NULL,      "filter"},
      [MINIMAL] = { "m",     "minimal"},
      [VALUES_ONLY] = {NULL, "values-only"},
  };

  cmdline_consume_all_options(cl, opts, _countof(opts));
  cmdline_validate(cl);

  const char* attr_filter = opts[ATTR].value && opts[ATTR].value[0] ? opts[ATTR].value : positional_attr;
  const char* text_filter = opts[FILTER].value;
  info_print_mode mode = INFO_PRINT_NORMAL;
  if (opts[VALUES_ONLY].present)
    mode = INFO_PRINT_VALUES_ONLY;
  else if (opts[MINIMAL].present)
    mode = INFO_PRINT_MINIMAL;

  if (_strcmpi(scope, "project") == 0)
    return run_cmd_info_file("Project File Attributes", cmdctx->project, attr_filter, text_filter, mode);
  if (_strcmpi(scope, "user") == 0)
    return run_cmd_info_file("User File Attributes", cmdctx->user, attr_filter, text_filter, mode);
  if (_strcmpi(scope, "local") == 0)
    return run_cmd_info_file("Local File Attributes", cmdctx->local, attr_filter, text_filter, mode);
  if (_strcmpi(scope, "toolchain") == 0)
    return run_cmd_info_file("Toolchain File Attributes", cmdctx->toolchain, attr_filter, text_filter, mode);

  error("Unknown info topic '%s'.", scope);
  print("Use 'project', 'user', 'local', or 'toolchain'.");
  return error_code(CMD_INFO, 1);
}

static int run_cmd_dist(cmd_ctx* cmdctx) {
  const char* target = cmdline_extract_option_value(cmdctx->cl, "t", "target");
  const char* platform = cmdline_extract_option_value(cmdctx->cl, "p", "platform");
  const char* config = cmdline_extract_option_value(cmdctx->cl, "c", "config");

  enum {
    TARGET = 0,
    PLATFORM,
  };

  cmdopt opts[] = {
      [TARGET] = {"t", "target"},
      [PLATFORM] = {"p", "platform"},
  };

  cmdline_consume_all_options(cmdctx->cl, opts, _countof(opts));
  cmdline_validate(cmdctx->cl);

  if (!project_dist(target, platform, config))
    return error_code(CMD_DIST, 0);

  return 0;
}

static int run_cmd_test(cmd_ctx* cmdctx) {
  const char* target = cmdline_extract_option_value(cmdctx->cl, "t", "target");
  const char* platform = cmdline_extract_option_value(cmdctx->cl, "p", "platform");
  const char* config = cmdline_extract_option_value(cmdctx->cl, "c", "config");

  enum {
    TARGET = 0,
    PLATFORM,
  };

  cmdopt opts[] = {
      [TARGET] = {"t", "target"},
      [PLATFORM] = {"p", "platform"},
  };

  cmdline_consume_all_options(cmdctx->cl, opts, _countof(opts));
  const char* test_name = cmdline_consume_param(cmdctx->cl);
  cmdline_validate(cmdctx->cl);

  if (!project_test(test_name, target, platform, config))
    return error_code(CMD_TEST, 0);

  return 0;
}

static bool bumpver_match_project_id(node* project, const char* project_id) {
  if (!project_id || !project_id[0])
    return true;

  node* id = node_get_child(project, "id");
  if (!id)
    return false;

  if (id->type == NODE_TYPE_STR)
    return strlen(project_id) == id->txt_dim && _strnicmp(project_id, id->_str, id->txt_dim) == 0;
  if (id->type == NODE_TYPE_IDF)
    return strlen(project_id) == id->txt_dim && _strnicmp(project_id, id->_idf, id->txt_dim) == 0;
  return false;
}

static node* bumpver_find_project(node* tree, const char* project_id, int* out_matches) {
  node* match = NULL;
  int matches = 0;
  if (!tree) {
    if (out_matches)
      *out_matches = 0;
    return NULL;
  }

  node_foreach(tree, child) {
    if (!child->name || _stricmp(child->name, "project") != 0)
      continue;
    if (!bumpver_match_project_id(child, project_id))
      continue;
    match = child;
    ++matches;
  }

  if (out_matches)
    *out_matches = matches;
  return matches == 1 ? match : NULL;
}

static bool bumpver_inc_byte(uint8_t* value, const char* label) {
  if (!value)
    return false;
  if (*value == UINT8_MAX) {
    error("Cannot bump %s beyond %u.", label, (unsigned)UINT8_MAX);
    return false;
  }
  ++(*value);
  return true;
}

static int run_cmd_bumpver(cmd_ctx* cmdctx) {
  cmdline* cl = cmdctx->cl;
  const char* part = cmdline_consume_param(cl);
  const char* project_id = cmdline_consume_param(cl);
  cmdline_validate(cl);

  if (!part || !part[0]) {
    error("Missing version part.");
    print("Use 'bbs bumpver <major|minor|patch|user|all> [project_id]'.");
    return error_code(CMD_BUMPVER, 0);
  }

  if (!file_exists(cmdctx->project)) {
    error("Project file not found: %s", cmdctx->project);
    return error_code(CMD_BUMPVER, 1);
  }

  const char* text = read_entire_file(cmdctx->project);
  if (!text) {
    error("Failed to read '%s'.", cmdctx->project);
    return error_code(CMD_BUMPVER, 2);
  }

  node* tree = node_parse(text);
  if (!tree) {
    error("Failed to parse '%s'.", cmdctx->project);
    return error_code(CMD_BUMPVER, 3);
  }

  int matches = 0;
  node* project = bumpver_find_project(tree, project_id, &matches);
  if (!project) {
    if (project_id && project_id[0]) {
      if (matches == 0)
        error("No project node found with id '%s'.", project_id);
      else
        error("Multiple project nodes found with id '%s'.", project_id);
    } else if (matches == 0) {
      error("No top-level project node found in '%s'.", cmdctx->project);
    } else {
      error("Multiple top-level project nodes found. Specify [project_id].");
    }
    return error_code(CMD_BUMPVER, 4);
  }

  node* version = node_get_child(project, "version");
  if (!version) {
    error("Project node does not define a version.");
    return error_code(CMD_BUMPVER, 5);
  }
  if (version->type != NODE_TYPE_VER) {
    error("Project version must be a version value.");
    return error_code(CMD_BUMPVER, 6);
  }

  ver before = version->_ver;
  uint8_t before_parts = version->ver_parts >= 4 ? 4 : 3;
  if (_stricmp(part, "major") == 0) {
    if (!bumpver_inc_byte(&version->_ver.major, "major"))
      return error_code(CMD_BUMPVER, 7);
    version->_ver.minor = 0;
    version->_ver.patch = 0;
    if (before_parts >= 4)
      version->_ver.user = 0;
  } else if (_stricmp(part, "minor") == 0) {
    if (!bumpver_inc_byte(&version->_ver.minor, "minor"))
      return error_code(CMD_BUMPVER, 8);
    version->_ver.patch = 0;
    if (before_parts >= 4)
      version->_ver.user = 0;
  } else if (_stricmp(part, "patch") == 0) {
    if (!bumpver_inc_byte(&version->_ver.patch, "patch"))
      return error_code(CMD_BUMPVER, 9);
    if (before_parts >= 4)
      version->_ver.user = 0;
  } else if (_stricmp(part, "user") == 0) {
    if (version->ver_parts < 4)
      version->ver_parts = 4;
    if (!bumpver_inc_byte(&version->_ver.user, "user"))
      return error_code(CMD_BUMPVER, 10);
  } else if (_stricmp(part, "all") == 0) {
    if (!bumpver_inc_byte(&version->_ver.major, "major") ||
        !bumpver_inc_byte(&version->_ver.minor, "minor") ||
        !bumpver_inc_byte(&version->_ver.patch, "patch"))
      return error_code(CMD_BUMPVER, 11);
    if (version->ver_parts >= 4 && !bumpver_inc_byte(&version->_ver.user, "user"))
      return error_code(CMD_BUMPVER, 12);
  } else {
    error("Unknown version part '%s'.", part);
    print("Use one of: major, minor, patch, user, all.");
    return error_code(CMD_BUMPVER, 13);
  }

  const char* updated = node_edit(version, text);
  if (!updated) {
    error("Failed to update the version text region.");
    return error_code(CMD_BUMPVER, 14);
  }
  if (!write_entire_file(cmdctx->project, updated)) {
    error("Failed to write '%s'.", cmdctx->project);
    return error_code(CMD_BUMPVER, 15);
  }

  char before_text[32] = {0};
  char after_text[32] = {0};
  if (before_parts >= 4)
    snprintf(before_text, sizeof(before_text), "%u.%u.%u.%u", before.major, before.minor, before.patch, before.user);
  else
    snprintf(before_text, sizeof(before_text), "%u.%u.%u", before.major, before.minor, before.patch);
  info_format_value(version, after_text, sizeof(after_text));

  print("Updated version: %s -> %s", before_text, after_text);
  print("Edited: %s", cmdctx->project);
  return 0;
}

static int run_cmd(cmd c, cmd_ctx* cmdctx) {
  switch (c) {
    case CMD_CFG:
      return run_cmd_cfg(cmdctx);
    case CMD_CLEAN:
      return run_cmd_clean(cmdctx);
    case CMD_UPDATE:
      return run_cmd_update(cmdctx);
    case CMD_BUILD:
      return run_cmd_build(cmdctx);
    case CMD_RUN:
      return run_cmd_run(cmdctx);
    case CMD_INFO:
      return run_cmd_info(cmdctx);
    case CMD_DIST:
      return run_cmd_dist(cmdctx);
    case CMD_TEST:
      return run_cmd_test(cmdctx);
    case CMD_BUMPVER:
      return run_cmd_bumpver(cmdctx);
    case CMD_TOOLCHAIN:
      return run_cmd_toolchain(cmdctx);
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

static cmd_ctx* init_cmd_ctx(int argc, char** argv) {
  cmd_ctx* cmdctx = push(sizeof(cmd_ctx));
  memset(cmdctx, 0, sizeof(cmd_ctx));
  cmdctx->cl = push(sizeof(cmdline));
  memset(cmdctx->cl, 0, sizeof(cmdline));

  cmdctx->cl->argv = argv;
  cmdctx->cl->argc = argc;
  cmdline_pop(cmdctx->cl);  // Pop argv[0]
  cmdline_pop(cmdctx->cl);  // Pop argv[1]

  enum {
    DEBUG = 0,
  };

  cmdopt base_opts[] = {
      [DEBUG] = {"d", "debug"}
  };
  cmdline_consume_all_options(cmdctx->cl, base_opts, 1);
  cmdctx->debug = base_opts[DEBUG].present;

  cmdctx->project = get_path_cwd(PROJ_FILENAME);
  cmdctx->user = get_path_exe(USER_FILENAME);
  cmdctx->local = get_path_cwd(LOCAL_FILENAME);
  cmdctx->toolchain = get_path_exe(TOOLCHAIN_FILENAME);
  return cmdctx;
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

  cmd_ctx* cmdctx = init_cmd_ctx(argc, argv);
  for (int i = CMD_HELP_END; i < CMD_MAX; ++i) {
    if (_strcmpi(argv[1], CMD_INFOS[i].name) == 0) {
      return run_cmd((cmd)i, cmdctx);
    }
  }

  return print_unrecognized_command(argv[1]);
}
