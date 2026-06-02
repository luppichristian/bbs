#pragma once
#include "bbs_cmd.h"
#include "bbs_project.c"

static void print_section(const char* title) {
  printf("\n" ANSI_FG_INFO "%s" ANSI_RESET "\n", title);
}

static const char* cfg_short_name(const char* filename) {
  static char buf[64];
  const char* dot = filename ? strchr(filename, '.') : NULL;
  size_t len = dot ? (size_t)(dot - filename) : (filename ? strlen(filename) : 0);
  if (len >= sizeof(buf))
    len = sizeof(buf) - 1;
  if (len > 0)
    memcpy(buf, filename, len);
  buf[len] = '\0';
  return buf;
}

static void print_summary_entry(const char* lhs, const char* rhs, size_t width) {
  size_t lhs_len = lhs ? strlen(lhs) : 0;
  if (lhs_len <= width) {
    print("  %-*s  %s", (int)width, lhs ? lhs : "", rhs ? rhs : "");
    return;
  }

  print("  %s", lhs ? lhs : "");
  print("  %*s%s", (int)width + 2, "", rhs ? rhs : "");
}

static void print_cfg_path(const char* label, const char* path) {
  print("  %-9s  %s", label, path);
}

static void print_indented_text(const char* text) {
  printf("  ");
  while (*text) {
    putchar(*text);
    if (*text == '\n' && text[1] != '\0')
      printf("  ");
    ++text;
  }
  printf("\n");
}

static const char* cfg_loc_label(cfg_loc loc) {
  switch (loc) {
    case CFG_LOC_CWD:
      return "project root";
    case CFG_LOC_EXE:
      return "bbs executable directory";
    default:
      return "unknown location";
  }
}

static const char* cmdctx_cfg_path(cmd_ctx* cmdctx, cfg c) {
  return cmdctx->cfg_paths[c];
}

static cfg parse_cfg_name(const char* name) {
  if (!name)
    return CFG_MAX;

  for (int i = 0; i < CFG_MAX; ++i) {
    const cfg_info info = CFG_INFOS[i];
    if (info.filename && _stricmp(name, info.filename) == 0)
      return (cfg)i;

    const char* dot = info.filename ? strchr(info.filename, '.') : NULL;
    size_t name_len = dot ? (size_t)(dot - info.filename) : (info.filename ? strlen(info.filename) : 0);
    if (info.filename && strlen(name) == name_len && _strnicmp(name, info.filename, name_len) == 0)
      return (cfg)i;
  }

  return CFG_MAX;
}

static void print_cfg_help(cfg c) {
  const cfg_info info = CFG_INFOS[c];

  print_section("Config");
  print("  %s", info.filename);

  print_section("Description");
  print("  %s", info.desc);

  if (info.detailed_desc && info.detailed_desc[0] != '\0') {
    print_section("Details");
    print_indented_text(info.detailed_desc);
  }

  print_section("Location");
  print("  Expected in the %s.", cfg_loc_label(info.loc));
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
  print_summary_entry(usage, info.desc, 44);
}

static void print_cmd_detailed_help(cmd command) {
  const cmd_info info = CMD_INFOS[command];

  print_section("Command");
  print_cmd_synopsis(command);

  print_section("Description");
  printf("  %s\n", info.desc);

  if (info.detailed_desc && info.detailed_desc[0] != '\0') {
    print_section("Details");
    print_indented_text(info.detailed_desc);
  }
}

static void print_usage(void) {
  print(ANSI_BOLD "Better Build System v%u.%u" ANSI_RESET, VER_MAJOR, VER_MINOR);
  print("Minimal build tooling for C/C++ projects.");

  print_section("Usage");
  print("  bbs <command> [options]");
  print("  bbs %s [command|config]", CMD_INFOS[CMD_HELP].name);

  print_section("Getting Started");
  print("  Create '{PROJECT_DIR}/%s' in the %s.", CFG_INFOS[CFG_PROJECT].filename, cfg_loc_label(CFG_INFOS[CFG_PROJECT].loc));
  print("  Add shared defaults in '{BBS_DIR}/%s' in the %s.", CFG_INFOS[CFG_USER].filename, cfg_loc_label(CFG_INFOS[CFG_USER].loc));
  print("  Add machine-local overrides in '{PROJECT_DIR}/%s' in the %s.", CFG_INFOS[CFG_LOCAL].filename, cfg_loc_label(CFG_INFOS[CFG_LOCAL].loc));

  print_section("Next Step");
  print("  Run 'bbs %s' to see the command and config reference.", CMD_INFOS[CMD_HELP].name);
}

static cmd parse_cmd_name(const char* name) {
  for (int i = CMD_HELP; i < CMD_MAX; ++i) {
    if (CMD_INFOS[i].name && _strcmpi(name, CMD_INFOS[i].name) == 0)
      return (cmd)i;
  }

  return CMD_MAX;
}

static void print_help(int argc, char** argv) {
  cmd help_page = CMD_HELP;
  cfg cfg_page = CFG_MAX;
  if (argc > 2) {
    help_page = parse_cmd_name(argv[2]);
    if (help_page == CMD_MAX)
      cfg_page = parse_cfg_name(argv[2]);
    if (help_page == CMD_MAX && cfg_page == CFG_MAX)
      warn("'bbs help %s' is not a recognized command or config. Showing general help instead", argv[2]);
  }

  print(ANSI_BOLD "Better Build System v%u.%u" ANSI_RESET, VER_MAJOR, VER_MINOR);
  if (help_page != CMD_MAX)
    switch (help_page) {
      case CMD_HELP:
        print_section("Commands");
        for (int i = CMD_HELP; i < CMD_MAX; ++i)
          print_cmd_help((cmd)i);
        print_section("Configs");
        for (int i = 0; i < CFG_MAX; ++i)
          print_summary_entry(cfg_short_name(CFG_INFOS[i].filename), CFG_INFOS[i].desc, 11);
        break;
      default:
        print_cmd_detailed_help(help_page);
        break;
    }
  else if (cfg_page != CFG_MAX) {
    print_cfg_help(cfg_page);
  } else {
    print_section("Commands");
    for (int i = CMD_HELP; i < CMD_MAX; ++i)
      print_cmd_help((cmd)i);
    print_section("Configs");
    for (int i = 0; i < CFG_MAX; ++i)
      print_summary_entry(cfg_short_name(CFG_INFOS[i].filename), CFG_INFOS[i].desc, 11);
  }

  print_section("Notes");
  print("  [] indicates optional arguments. <> indicates required arguments.");
  print("  Commands operate on the project rooted at the current working directory.");
  print("  The build system is initialized automatically when a command runs.");
  print("  [-t target] can be inferred when the project defines only one target; otherwise bbs operates on all targets.");
  print("  [-p platform] selects one supported target platform, such as 'windows-x86_64'.");
  print("  Use '*' with -t, -p, or -c to execute supported commands for all matching targets, platforms, or configs.");
  print("  For 'run', '-p *' expands only to the host-native platform because bbs cannot execute cross-platform outputs locally.");

  print_section("More Help");
  print("  bbs %s <command>", CMD_INFOS[CMD_HELP].name);
  print("  bbs %s <config>", CMD_INFOS[CMD_HELP].name);
}

static int error_code(cmd c, char idx) {
  return 200 + c * 255 + idx;
}

typedef struct {
  const char* target;
  const char* platform;
  const char* config;
} build_cmd_args;

typedef struct {
  platform_timestamp newest_ts;
  unsigned long long file_count;
  unsigned long long hash;
} watch_snapshot;

enum {
  AUTO_POLL_MS = 250,
  AUTO_DEBOUNCE_MS = 500,
};

static bool cmd_run_build_matrix(const char* target, const char* platform, const char* config, toolchain* tc);
static const char* cmdline_extract_option_value(cmdline* cl, const char* short_name, const char* long_name);

static bool cmdopt_is_star(const char* value) {
  return value && strcmp(value, "*") == 0;
}

static bool watch_snapshot_equals(const watch_snapshot* a, const watch_snapshot* b) {
  if (!a || !b)
    return false;
  return a->hash == b->hash && a->file_count == b->file_count && a->newest_ts == b->newest_ts;
}

static unsigned long long watch_hash_bytes(unsigned long long hash, const char* text) {
  const unsigned char* p = (const unsigned char*)(text ? text : "");
  while (*p) {
    hash ^= (unsigned long long)(*p++);
    hash *= 1099511628211ULL;
  }
  return hash;
}

static unsigned long long watch_hash_u64(unsigned long long hash, unsigned long long value) {
  for (int i = 0; i < 8; ++i) {
    hash ^= (unsigned long long)((value >> (i * 8)) & 0xffU);
    hash *= 1099511628211ULL;
  }
  return hash;
}

static const char* watch_basename(const char* path) {
  if (!path)
    return NULL;

  const char* slash = strrchr(path, '/');
  const char* bslash = strrchr(path, '\\');
  const char* last = slash;
  if (!last || (bslash && bslash > last))
    last = bslash;
  return last ? last + 1 : path;
}

static bool watch_path_equals(const char* a, const char* b) {
  if (!a || !b)
    return false;
#if defined(_WIN32)
  return _stricmp(a, b) == 0;
#else
  return strcmp(a, b) == 0;
#endif
}

static bool watch_should_skip_dir(const char* dir_path, const char** ignored_dirs, int ignored_dir_c) {
  const char* name = watch_basename(dir_path);
  if (name && (_stricmp(name, ".git") == 0 || _stricmp(name, ".bbs-bootstrap") == 0))
    return true;

  for (int i = 0; i < ignored_dir_c; ++i) {
    if (ignored_dirs[i] && watch_path_equals(dir_path, ignored_dirs[i]))
      return true;
  }

  return false;
}

static void watch_snapshot_add_file(watch_snapshot* snap, const char* rel_path, platform_timestamp ts) {
  if (!snap || !rel_path)
    return;
  if (ts > snap->newest_ts)
    snap->newest_ts = ts;
  ++snap->file_count;
  snap->hash = watch_hash_bytes(snap->hash, rel_path);
  snap->hash = watch_hash_u64(snap->hash, ts);
}

static void watch_snapshot_add_dir(watch_snapshot* snap, const char* rel_path, platform_timestamp ts) {
  if (!snap || !rel_path || !rel_path[0])
    return;
  if (ts > snap->newest_ts)
    snap->newest_ts = ts;
  ++snap->file_count;
  snap->hash = watch_hash_bytes(snap->hash, "dir:");
  snap->hash = watch_hash_bytes(snap->hash, rel_path);
  snap->hash = watch_hash_u64(snap->hash, ts);
}

static bool watch_collect_snapshot_dir(const char* root, const char* rel_path, const char** ignored_dirs, int ignored_dir_c, watch_snapshot* snap) {
  const char* dir_path = rel_path && rel_path[0] ? toolchain_join2(root, rel_path) : root;
  if (!dir_path)
    return false;
  if (watch_should_skip_dir(dir_path, ignored_dirs, ignored_dir_c))
    return true;

#if defined(_WIN32)
  char pattern[_MAX_PATH * 2] = {0};
  snprintf(pattern, sizeof(pattern), "%s\\*", dir_path);

  WIN32_FIND_DATAA data;
  HANDLE handle = FindFirstFileA(pattern, &data);
  if (handle == INVALID_HANDLE_VALUE)
    return true;

  bool ok = true;
  do {
    if (_stricmp(data.cFileName, ".") == 0 || _stricmp(data.cFileName, "..") == 0)
      continue;

    const char* child_rel = rel_path && rel_path[0] ? toolchain_join2(rel_path, data.cFileName) : arena_text(data.cFileName, strlen(data.cFileName));
    const char* child_path = toolchain_join2(root, child_rel);
    if (!child_rel || !child_path) {
      ok = false;
      break;
    }

    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      watch_snapshot_add_dir(snap, child_rel, file_timestamp(child_path));
      if (!watch_collect_snapshot_dir(root, child_rel, ignored_dirs, ignored_dir_c, snap)) {
        ok = false;
        break;
      }
      continue;
    }

    watch_snapshot_add_file(snap, child_rel, file_timestamp(child_path));
  } while (FindNextFileA(handle, &data) != 0);

  FindClose(handle);
  return ok;
#else
  DIR* dir = opendir(dir_path);
  if (!dir)
    return true;

  bool ok = true;
  struct dirent* entry = NULL;
  while ((entry = readdir(dir)) != NULL) {
    if (_stricmp(entry->d_name, ".") == 0 || _stricmp(entry->d_name, "..") == 0)
      continue;

    const char* child_rel = rel_path && rel_path[0] ? toolchain_join2(rel_path, entry->d_name) : arena_text(entry->d_name, strlen(entry->d_name));
    const char* child_path = toolchain_join2(root, child_rel);
    if (!child_rel || !child_path) {
      ok = false;
      break;
    }

    struct stat st;
    if (stat(child_path, &st) != 0)
      continue;

    if (S_ISDIR(st.st_mode)) {
      watch_snapshot_add_dir(snap, child_rel, file_timestamp(child_path));
      if (!watch_collect_snapshot_dir(root, child_rel, ignored_dirs, ignored_dir_c, snap)) {
        ok = false;
        break;
      }
      continue;
    }

    watch_snapshot_add_file(snap, child_rel, file_timestamp(child_path));
  }

  closedir(dir);
  return ok;
#endif
}

static bool watch_collect_snapshot(const char* root, const char** ignored_dirs, int ignored_dir_c, watch_snapshot* out) {
  if (!root || !root[0] || !out)
    return false;

  out->newest_ts = 0;
  out->file_count = 0;
  out->hash = 1469598103934665603ULL;
  return watch_collect_snapshot_dir(root, NULL, ignored_dirs, ignored_dir_c, out);
}

static void cmd_parse_build_args(cmd_ctx* cmdctx, build_cmd_args* out, cmdopt* extra_opts, size_t extra_opt_c) {
  if (!cmdctx || !out)
    return;

  out->target = cmdline_extract_option_value(cmdctx->cl, "t", "target");
  out->platform = cmdline_extract_option_value(cmdctx->cl, "p", "platform");
  out->config = cmdline_extract_option_value(cmdctx->cl, "c", "config");

  enum {
    TARGET = 0,
    PLATFORM,
  };

  cmdopt opts[] = {
      [TARGET] = {"t",   "target"},
      [PLATFORM] = {"p", "platform"},
  };

  cmdline_consume_all_options(cmdctx->cl, opts, _countof(opts));
  if (extra_opts && extra_opt_c > 0)
    cmdline_consume_all_options(cmdctx->cl, extra_opts, extra_opt_c);
  cmdline_validate(cmdctx->cl);
}

static bool cmd_parse_uint_option(const char* value, const char* option_name, unsigned int* out_value) {
  if (!option_name || !out_value)
    return false;
  if (!value || !value[0]) {
    error("option '--%s' expects a value.", option_name);
    return false;
  }

  char* end = NULL;
  unsigned long parsed = strtoul(value, &end, 10);
  if (!end || *end != '\0') {
    error("option '--%s' expects an integer value. Got '%s'.", option_name, value);
    return false;
  }

  *out_value = (unsigned int)parsed;
  return true;
}

static bool cmd_exec_build_args(const build_cmd_args* args, toolchain* tc) {
  const char* target = args ? args->target : NULL;
  const char* platform = args ? args->platform : NULL;
  const char* config = args ? args->config : NULL;
  if (cmdopt_is_star(target) || cmdopt_is_star(platform) || cmdopt_is_star(config))
    return cmd_run_build_matrix(target, platform, config, tc);
  return project_build(target, platform, config, tc);
}

static bool cmd_exec_build_args_retry(const build_cmd_args* args, toolchain* tc, unsigned int retry_count, unsigned int retry_delay_ms) {
  for (unsigned int attempt = 0; attempt <= retry_count; ++attempt) {
    if (cmd_exec_build_args(args, tc))
      return true;
    if (attempt == retry_count)
      break;
    warn("Rebuild attempt %u failed. Retrying in %ums.", attempt + 2, retry_delay_ms);
    sleep_ms(retry_delay_ms);
  }

  return false;
}

static bool watch_wait_for_quiet(const char* root,
                                 const char** ignored_dirs,
                                 int ignored_dir_c,
                                 unsigned int poll_ms,
                                 unsigned int debounce_ms,
                                 watch_snapshot* snapshot) {
  if (!snapshot)
    return false;
  if (debounce_ms == 0)
    return true;

  watch_snapshot stable = *snapshot;
  platform_timestamp quiet_started = now_ms();
  for (;;) {
    sleep_ms(poll_ms);

    watch_snapshot next = {0};
    if (!watch_collect_snapshot(root, ignored_dirs, ignored_dir_c, &next))
      continue;
    if (!watch_snapshot_equals(&next, &stable)) {
      stable = next;
      quiet_started = now_ms();
      continue;
    }

    platform_timestamp elapsed = now_ms() - quiet_started;
    if (elapsed >= debounce_ms) {
      *snapshot = stable;
      return true;
    }
  }
}

static int run_cmd_auto(cmd_ctx* cmdctx) {
  build_cmd_args args = {0};
  const char* debounce_value = cmdline_extract_option_value(cmdctx->cl, NULL, "debounce");
  cmd_parse_build_args(cmdctx, &args, NULL, 0);

  toolchain* tc = toolchain_init(cmdctx_cfg_path(cmdctx, CFG_TOOLCHAIN), false, cmdctx);
  if (!cmd_exec_build_args(&args, tc))
    return error_code(CMD_AUTO, 0);

  project proj = {0};
  const char* watch_root = project_current_workdir();
  const char* ignored_dirs[2] = {0};
  int ignored_dir_c = 0;
  if (project_load_config(args.config, &proj)) {
    ignored_dirs[ignored_dir_c++] = get_path_cwd(proj.user_cfg.builddir ? proj.user_cfg.builddir : DEF_BUILD_DIR);
    ignored_dirs[ignored_dir_c++] = get_path_cwd(proj.user_cfg.distdir ? proj.user_cfg.distdir : DEF_DIST_DIR);
  } else {
    ignored_dirs[ignored_dir_c++] = get_path_cwd(DEF_BUILD_DIR);
    ignored_dirs[ignored_dir_c++] = get_path_cwd(DEF_DIST_DIR);
  }

  unsigned int debounce_ms = proj.user_cfg.auto_debounce_ms ? proj.user_cfg.auto_debounce_ms : AUTO_DEBOUNCE_MS;
  unsigned int retry_count = proj.user_cfg.auto_retry_count;
  unsigned int retry_delay_ms = proj.user_cfg.auto_retry_delay_ms;
  if (debounce_value && !cmd_parse_uint_option(debounce_value, "debounce", &debounce_ms))
    return error_code(CMD_AUTO, 3);

  watch_snapshot prev = {0};
  if (!watch_collect_snapshot(watch_root, ignored_dirs, ignored_dir_c, &prev)) {
    error("Failed to initialize file watcher snapshot.");
    return error_code(CMD_AUTO, 1);
  }

  print("Auto");
  project_print_field("Directory", watch_root);
  project_print_fieldf("Debounce", "%ums", debounce_ms);
  project_print_fieldf("Retry Count", "%u", retry_count);
  project_print_fieldf("Retry Delay", "%ums", retry_delay_ms);
  print("Watching for changes. Press Ctrl+C to stop.");

  for (;;) {
    sleep_ms(AUTO_POLL_MS);

    watch_snapshot curr = {0};
    if (!watch_collect_snapshot(watch_root, ignored_dirs, ignored_dir_c, &curr))
      continue;
    if (watch_snapshot_equals(&curr, &prev))
      continue;

    print("\nChange detected. Waiting for files to settle...");
    if (!watch_wait_for_quiet(watch_root, ignored_dirs, ignored_dir_c, AUTO_POLL_MS, debounce_ms, &curr))
      continue;

    prev = curr;
    print("Rebuilding...");
    if (cmd_exec_build_args_retry(&args, tc, retry_count, retry_delay_ms)) {
      if (!watch_collect_snapshot(watch_root, ignored_dirs, ignored_dir_c, &prev)) {
        error("Failed to refresh file watcher snapshot after rebuild.");
        return error_code(CMD_AUTO, 2);
      }
    } else {
      error("Rebuild failed after retries. Watching for more changes.");
    }
  }
}

static bool cmd_collect_configs(const char* selected, const char*** out_configs, int* out_count) {
  if (!out_configs || !out_count)
    return false;
  *out_configs = NULL;
  *out_count = 0;

  if (!cmdopt_is_star(selected))
    return true;

  project proj = {0};
  if (!project_load(&proj))
    return false;
  *out_configs = proj.configs;
  *out_count = proj.config_c;
  return true;
}

static bool cmd_collect_platforms(toolchain* tc, bool host_only, const char*** out_platforms, int* out_count) {
  if (!out_platforms || !out_count || !tc)
    return false;
  *out_platforms = NULL;
  *out_count = 0;

  if (host_only) {
    const char** items = push(sizeof(*items));
    if (!items)
      return false;
    items[0] = project_platform_id(tc->p_os, tc->p_arch);
    *out_platforms = items;
    *out_count = 1;
    return true;
  }

  const char** items = push(sizeof(*items) * (size_t)(OS_MAX * ARCH_MAX));
  if (!items)
    return false;

  int count = 0;
  for (int osi = 0; osi < OS_MAX; ++osi)
    for (int ai = 0; ai < ARCH_MAX; ++ai)
      if (tc->supported[osi][ai])
        items[count++] = project_platform_id((os)osi, (arch)ai);

  *out_platforms = items;
  *out_count = count;
  return true;
}

static bool cmd_run_build_matrix(const char* target, const char* platform, const char* config, toolchain* tc) {
  const char** configs = NULL;
  int config_c = 0;
  if (!cmd_collect_configs(config, &configs, &config_c))
    return false;

  int failures = 0;
  int runs = 0;
  int config_iters = cmdopt_is_star(config) ? config_c : 1;
  for (int ci = 0; ci < config_iters; ++ci) {
    const char* cfg = cmdopt_is_star(config) ? configs[ci] : config;
    project proj = {0};
    if (!project_load_config(cfg, &proj)) {
      ++failures;
      continue;
    }

    const char** platforms = NULL;
    int platform_c = 0;
    if (cmdopt_is_star(platform) && !cmd_collect_platforms(tc, false, &platforms, &platform_c)) {
      ++failures;
      continue;
    }

    int platform_iters = cmdopt_is_star(platform) ? platform_c : 1;
    for (int pi = 0; pi < platform_iters; ++pi) {
      const char* p = cmdopt_is_star(platform) ? platforms[pi] : (platform ? platform : project_platform_id(tc->p_os, tc->p_arch));
      if (cmdopt_is_star(target)) {
        ++runs;
        if (!project_build(NULL, p, cfg, tc))
          ++failures;
      } else {
        ++runs;
        if (!project_build(target, p, cfg, tc))
          ++failures;
      }
    }
  }

  return runs > 0 && failures == 0;
}

static bool cmd_run_run_matrix(const char* target, const char* platform, const char* config, toolchain* tc) {
  const char** configs = NULL;
  int config_c = 0;
  if (!cmd_collect_configs(config, &configs, &config_c))
    return false;

  int failures = 0;
  int runs = 0;
  int config_iters = cmdopt_is_star(config) ? config_c : 1;
  for (int ci = 0; ci < config_iters; ++ci) {
    const char* cfg = cmdopt_is_star(config) ? configs[ci] : config;
    project proj = {0};
    if (!project_load_config(cfg, &proj)) {
      ++failures;
      continue;
    }

    const char** platforms = NULL;
    int platform_c = 0;
    if (cmdopt_is_star(platform) && !cmd_collect_platforms(tc, true, &platforms, &platform_c)) {
      ++failures;
      continue;
    }

    int platform_iters = cmdopt_is_star(platform) ? platform_c : 1;
    for (int pi = 0; pi < platform_iters; ++pi) {
      const char* p = cmdopt_is_star(platform) ? platforms[pi] : (platform ? platform : project_platform_id(tc->p_os, tc->p_arch));
      if (cmdopt_is_star(target)) {
        int matched = 0;
        bool built = false;
        for (int ti = 0; ti < proj.target_c; ++ti) {
          if (!project_target_is_runnable(&proj.targets[ti]))
            continue;
          if (!built) {
            ++runs;
            if (!project_build(NULL, p, cfg, tc)) {
              ++failures;
              built = true;
              break;
            }
            built = true;
          }
          ++matched;
          ++runs;
          project_print_action_header("Run", &proj, p);
          project_print_target_line("Run", &proj.targets[ti]);
          if (!project_run_execute(&proj, &proj.targets[ti], p, tc))
            ++failures;
        }
        if (matched == 0) {
          error("No runnable targets found.");
          ++failures;
        }
      } else {
        ++runs;
        if (!project_run(target, p, cfg, tc))
          ++failures;
      }
    }
  }

  return runs > 0 && failures == 0;
}

static bool cmd_run_dist_matrix(const char* target, const char* platform, const char* config, toolchain* tc) {
  const char** configs = NULL;
  int config_c = 0;
  if (!cmd_collect_configs(config, &configs, &config_c))
    return false;

  int failures = 0;
  int runs = 0;
  int config_iters = cmdopt_is_star(config) ? config_c : 1;
  for (int ci = 0; ci < config_iters; ++ci) {
    const char* cfg = cmdopt_is_star(config) ? configs[ci] : config;
    project proj = {0};
    if (!project_load_config(cfg, &proj)) {
      ++failures;
      continue;
    }

    const char** platforms = NULL;
    int platform_c = 0;
    if (cmdopt_is_star(platform) && !cmd_collect_platforms(tc, false, &platforms, &platform_c)) {
      ++failures;
      continue;
    }

    int platform_iters = cmdopt_is_star(platform) ? platform_c : 1;
    for (int pi = 0; pi < platform_iters; ++pi) {
      const char* p = cmdopt_is_star(platform) ? platforms[pi] : (platform ? platform : project_platform_id(tc->p_os, tc->p_arch));
      if (cmdopt_is_star(target)) {
        for (int ti = 0; ti < proj.target_c; ++ti) {
          ++runs;
          if (!project_dist(proj.targets[ti].meta.id, p, cfg, tc))
            ++failures;
        }
      } else {
        ++runs;
        if (!project_dist(target, p, cfg, tc))
          ++failures;
      }
    }
  }

  return runs > 0 && failures == 0;
}

static bool cmd_run_test_matrix(const char* test_name, const char* target, const char* platform, const char* config, toolchain* tc) {
  const char** configs = NULL;
  int config_c = 0;
  if (!cmd_collect_configs(config, &configs, &config_c))
    return false;

  int failures = 0;
  int runs = 0;
  int config_iters = cmdopt_is_star(config) ? config_c : 1;
  for (int ci = 0; ci < config_iters; ++ci) {
    const char* cfg = cmdopt_is_star(config) ? configs[ci] : config;
    project proj = {0};
    if (!project_load_config(cfg, &proj)) {
      ++failures;
      continue;
    }

    const char** platforms = NULL;
    int platform_c = 0;
    if (cmdopt_is_star(platform) && !cmd_collect_platforms(tc, false, &platforms, &platform_c)) {
      ++failures;
      continue;
    }

    int platform_iters = cmdopt_is_star(platform) ? platform_c : 1;
    for (int pi = 0; pi < platform_iters; ++pi) {
      const char* p = cmdopt_is_star(platform) ? platforms[pi] : (platform ? platform : project_platform_id(tc->p_os, tc->p_arch));
      if (cmdopt_is_star(target)) {
        int matched = 0;
        bool built = false;
        for (int ti = 0; ti < proj.target_c; ++ti) {
          if (!project_target_is_test(&proj.targets[ti]))
            continue;
          if (!built) {
            ++runs;
            if (!project_build(NULL, p, cfg, tc)) {
              ++failures;
              built = true;
              break;
            }
            built = true;
          }
          ++matched;
          ++runs;
          project_print_action_header("Test", &proj, p);
          project_print_field("Directory", project_resolved_dir(proj.user_cfg.builddir, proj.active_config, p));
          if (test_name && test_name[0])
            project_print_field("Test", test_name);
          project_print_target_line("Test", &proj.targets[ti]);
          if (!project_test_execute(&proj, &proj.targets[ti], test_name, p, tc))
            ++failures;
        }
        if (matched == 0) {
          error("No test targets found.");
          ++failures;
        }
      } else {
        ++runs;
        if (!project_test(test_name, target, p, cfg, tc))
          ++failures;
      }
    }
  }

  return runs > 0 && failures == 0;
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
  if ((opts[PROJECT].present == opts[USER].present) && (opts[USER].present == opts[LOCAL].present) && (opts[LOCAL].present == opts[TOOLCHAIN].present))
    opts[PROJECT].present = opts[USER].present = opts[LOCAL].present = opts[TOOLCHAIN].present = true;

  if (opts[0].present) {
    if (opts[PROJECT].present) print("%s", cmdctx_cfg_path(cmdctx, CFG_PROJECT));
    if (opts[USER].present) print("%s", cmdctx_cfg_path(cmdctx, CFG_USER));
    if (opts[LOCAL].present) print("%s", cmdctx_cfg_path(cmdctx, CFG_LOCAL));
    if (opts[TOOLCHAIN].present) print("%s", cmdctx_cfg_path(cmdctx, CFG_TOOLCHAIN));
  } else {
    if (opts[PROJECT].present) print_cfg_path("Project", cmdctx_cfg_path(cmdctx, CFG_PROJECT));
    if (opts[USER].present) print_cfg_path("User", cmdctx_cfg_path(cmdctx, CFG_USER));
    if (opts[LOCAL].present) print_cfg_path("Local", cmdctx_cfg_path(cmdctx, CFG_LOCAL));
    if (opts[TOOLCHAIN].present) print_cfg_path("Toolchain", cmdctx_cfg_path(cmdctx, CFG_TOOLCHAIN));
  }

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
    if (!project_cleanup())
      return error_code(CMD_CLEAN, 5);
    return 0;
  }

  cfg c = parse_cfg_name(scope);
  switch (c) {
    case CFG_PROJECT:
      return clean_one_file("project config", cmdctx_cfg_path(cmdctx, c), CMD_CLEAN, 0);
    case CFG_LOCAL:
      return clean_one_file("local config", cmdctx_cfg_path(cmdctx, c), CMD_CLEAN, 1);
    case CFG_USER:
      return clean_one_file("user config", cmdctx_cfg_path(cmdctx, c), CMD_CLEAN, 2);
    case CFG_TOOLCHAIN:
      return clean_one_file("toolchain config", cmdctx_cfg_path(cmdctx, c), CMD_CLEAN, 3);
    default:
      break;
  }

  error("Unknown clean target '%s'.", scope);
  print("Use 'project', 'user', 'local', or 'toolchain'.");
  return error_code(CMD_CLEAN, 4);
}

static int run_cmd_update(cmd_ctx* cmdctx) {
  const char* config = cmdline_extract_option_value(cmdctx->cl, "c", "config");

  enum {
    INFO = 0,
    INIT_TOOLCHAIN,
    REFRESH_PACKAGES,
  };

  cmdopt opts[] = {
      [INFO] = { "i",           "info"},
      [INIT_TOOLCHAIN] = {NULL, "init-toolchain"},
      [REFRESH_PACKAGES] = {NULL, "refresh-packages"},
  };

  cmdline_consume_all_options(cmdctx->cl, opts, _countof(opts));
  cmdline_validate(cmdctx->cl);

  if (opts[INFO].present) {
    project proj = {0};
    if (!project_load_config(config, &proj))
      return error_code(CMD_UPDATE, 1);
    project_print(&proj);
  }

  toolchain* tc = toolchain_init(cmdctx_cfg_path(cmdctx, CFG_TOOLCHAIN), opts[INIT_TOOLCHAIN].present, cmdctx);
  if (!tc)
    return error_code(CMD_UPDATE, 2);
  if (!project_update(tc, opts[REFRESH_PACKAGES].present))
    return error_code(CMD_UPDATE, 0);

  return 0;
}

static const char* gen_gitignore_text(void) {
  return "build/\n"
         "dist/\n"
         "gen/\n"
         "\n"
         "# bbs local overrides\n"
         "local.bbs\n"
         "toolchain.bbs\n"
         "\n"
         "# CMake-generated files\n"
         "cmake-build-*/\n"
         "CMakeFiles/\n"
         "CMakeCache.txt\n"
         "CTestTestfile.cmake\n"
         "cmake_install.cmake\n"
         "compile_commands.json\n"
         "install_manifest.txt\n"
         "\n"
         "# Visual Studio and IDE state\n"
         ".vs/\n"
         ".vscode/\n"
         ".idea/\n"
         "*.sln\n"
         "*.vcxproj\n"
         "*.vcxproj.filters\n"
         "*.vcxproj.user\n"
         "\n"
         "# Native build outputs\n"
         "*.o\n"
         "*.obj\n"
         "*.a\n"
         "*.lib\n"
         "*.dll\n"
         "*.dylib\n"
         "*.so\n"
         "*.exe\n"
         "*.exp\n"
         "*.ilk\n"
         "*.out\n"
         "*.pdb\n"
         "\n"
         "# Logs and OS metadata\n"
         "*.log\n"
         "Thumbs.db\n"
         ".DS_Store\n";
}

static bool gen_ensure_dir_tree(const char* path, const char* label) {
  if (!path || !path[0])
    return true;
  if (dir_exists(path))
    return true;

  size_t len = strlen(path);
  char* buf = push(len + 1);
  if (!buf)
    return false;
  memcpy(buf, path, len + 1);

  size_t start = 0;
  if (len >= 3 && buf[1] == ':' && (buf[2] == '\\' || buf[2] == '/'))
    start = 3;
  else if (len >= 1 && (buf[0] == '\\' || buf[0] == '/'))
    start = 1;

  for (size_t i = start; i < len; ++i) {
    if (buf[i] != '\\' && buf[i] != '/')
      continue;
    if (i == 0)
      continue;

    char saved = buf[i];
    buf[i] = '\0';
    if (buf[0] && !dir_exists(buf) && !dir_create(buf)) {
      error("Failed to create %s: %s", label ? label : "directory", buf);
      return false;
    }
    buf[i] = saved;
  }

  if (!dir_exists(buf) && !dir_create(buf)) {
    error("Failed to create %s: %s", label ? label : "directory", buf);
    return false;
  }

  return true;
}

static bool gen_ensure_parent_dir(const char* path) {
  const char* parent = project_path_parent(path);
  if (!parent || !parent[0])
    return true;
  return gen_ensure_dir_tree(parent, "generator output directory");
}

static int gen_write_text_file(const char* path, const char* text, bool override_existing, cmd c, char err_idx_base) {
  if (!path || !path[0] || !text)
    return error_code(c, err_idx_base);

  if (file_exists(path) && !override_existing) {
    error("Refusing to overwrite existing file: %s", path);
    print("Use '-o' or '--override' to overwrite it.");
    return error_code(c, err_idx_base + 1);
  }
  if (!gen_ensure_parent_dir(path))
    return error_code(c, err_idx_base + 2);
  if (!write_entire_file(path, text)) {
    error("Failed to write '%s'.", path);
    return error_code(c, err_idx_base + 3);
  }

  print("Generated %s", path);
  return 0;
}

static int gen_copy_output_file(const char* src, const char* dst, bool override_existing, cmd c, char err_idx_base) {
  if (!src || !src[0] || !dst || !dst[0])
    return error_code(c, err_idx_base);

  if (!file_exists(src)) {
    error("Generator source file does not exist: %s", src);
    return error_code(c, err_idx_base + 1);
  }
  if (file_exists(dst) && !override_existing) {
    error("Refusing to overwrite existing file: %s", dst);
    print("Use '-o' or '--override' to overwrite it.");
    return error_code(c, err_idx_base + 2);
  }
  if (!gen_ensure_parent_dir(dst))
    return error_code(c, err_idx_base + 3);
  if (!project_copy_file(src, dst)) {
    error("Failed to copy '%s' to '%s'.", src, dst);
    return error_code(c, err_idx_base + 4);
  }

  print("Generated %s", dst);
  return 0;
}

static bool gen_append_github_command_steps(project_textbuf* buf, const char* title, const char* windows_cmd, const char* unix_cmd) {
  return project_textbuf_appendf(buf,
                                 "      - name: %s (Windows)\n"
                                 "        if: runner.os == 'Windows'\n"
                                 "        shell: pwsh\n"
                                 "        run: |\n"
                                 "          %s\n"
                                 "\n"
                                 "      - name: %s (Unix)\n"
                                 "        if: runner.os != 'Windows'\n"
                                 "        shell: bash\n"
                                 "        run: |\n"
                                 "          %s\n"
                                 "\n",
                                 title,
                                 windows_cmd,
                                 title,
                                 unix_cmd);
}

static bool gen_append_github_checkout_bbs_step(project_textbuf* buf) {
  return project_textbuf_append(buf,
                                "      - name: Checkout bbs\n"
                                "        uses: actions/checkout@v4\n"
                                "        with:\n"
                                "          repository: luppichristian/bbs\n"
                                "          path: .bbs-bootstrap\n"
                                "\n");
}

static const char* gen_trim_text(const char* text) {
  if (!text)
    return NULL;

  while (*text && isspace((unsigned char)*text))
    ++text;

  size_t len = strlen(text);
  while (len > 0 && isspace((unsigned char)text[len - 1]))
    --len;

  return arena_text(text, len);
}

static bool gen_parse_github_platforms(const char* csv, const char*** out_platforms, int* out_count) {
  if (!out_platforms || !out_count)
    return false;
  *out_platforms = NULL;
  *out_count = 0;

  if (!csv || !csv[0])
    return true;

  int count = 1;
  for (const char* p = csv; *p; ++p)
    if (*p == ',')
      ++count;

  const char** items = push(sizeof(*items) * (size_t)count);
  if (!items)
    return false;

  int out = 0;
  const char* begin = csv;
  for (const char* p = csv;; ++p) {
    if (*p != ',' && *p != '\0')
      continue;

    const char* raw = arena_text(begin, (size_t)(p - begin));
    const char* platform = gen_trim_text(raw);
    if (!platform || !platform[0]) {
      error("Empty platform entry in '--platform'.");
      return false;
    }

    os target_os = OS_MAX;
    arch target_arch = ARCH_MAX;
    if (!project_parse_platform_id(platform, &target_os, &target_arch)) {
      error("Invalid platform '%s'. Expected '<os>-<arch>'.", platform);
      return false;
    }

    for (int i = 0; i < out; ++i) {
      if (_stricmp(items[i], platform) == 0) {
        error("Duplicate platform '%s' in '--platform'.", platform);
        return false;
      }
    }

    items[out++] = platform;
    if (*p == '\0')
      break;
    begin = p + 1;
  }

  *out_platforms = items;
  *out_count = out;
  return true;
}

static const char* gen_github_runner_for_platform(const char* platform) {
  os target_os = OS_MAX;
  arch target_arch = ARCH_MAX;
  if (!project_parse_platform_id(platform, &target_os, &target_arch))
    return NULL;

  switch (target_os) {
    case OS_WINDOWS:
      if (target_arch == ARCH_X86_64 || target_arch == ARCH_X86)
        return "windows-latest";
      break;
    case OS_LINUX:
      if (target_arch == ARCH_X86_64 || target_arch == ARCH_X86)
        return "ubuntu-latest";
      break;
    case OS_MACOS:
      if (target_arch == ARCH_X86_64)
        return "macos-latest";
      if (target_arch == ARCH_ARM64)
        return "macos-latest";
      break;
    default:
      break;
  }

  return NULL;
}

static const char* gen_default_config_name(const project* proj) {
  return (proj && proj->config_c > 0 && proj->configs && proj->configs[0] && proj->configs[0][0]) ? proj->configs[0] : "default";
}

static const char* gen_github_runner_for_os(os target_os) {
  switch (target_os) {
    case OS_WINDOWS:
      return "windows-latest";
    case OS_LINUX:
      return "ubuntu-latest";
    case OS_MACOS:
      return "macos-latest";
    default:
      return NULL;
  }
}

static const char* gen_github_job_name_for_os(os target_os) {
  switch (target_os) {
    case OS_WINDOWS:
      return "windows";
    case OS_LINUX:
      return "linux";
    case OS_MACOS:
      return "macos";
    default:
      return NULL;
  }
}

static const char* gen_github_host_platform_for_os(os target_os) {
  switch (target_os) {
    case OS_WINDOWS:
      return "windows-x86_64";
    case OS_LINUX:
      return "linux-x86_64";
    case OS_MACOS:
      return "macos-x86_64";
    default:
      return NULL;
  }
}

static bool gen_platform_list_contains(const char** platforms, int platform_c, const char* platform) {
  if (!platforms || platform_c <= 0 || !platform || !platform[0])
    return false;

  for (int i = 0; i < platform_c; ++i) {
    if (platforms[i] && _stricmp(platforms[i], platform) == 0)
      return true;
  }

  return false;
}

static bool gen_collect_platforms_by_os(const char** platforms, int platform_c, const char*** out_groups, int* out_counts) {
  if (!out_groups || !out_counts)
    return false;

  for (int i = 0; i < OS_MAX; ++i) {
    out_groups[i] = NULL;
    out_counts[i] = 0;
  }

  for (int i = 0; i < platform_c; ++i) {
    os target_os = OS_MAX;
    arch target_arch = ARCH_MAX;
    if (!project_parse_platform_id(platforms[i], &target_os, &target_arch))
      return false;
    if (target_os < 0 || target_os >= OS_MAX)
      return false;
    ++out_counts[target_os];
  }

  for (int osi = 0; osi < OS_MAX; ++osi) {
    if (out_counts[osi] <= 0)
      continue;
    out_groups[osi] = push(sizeof(*out_groups[osi]) * (size_t)out_counts[osi]);
    if (!out_groups[osi])
      return false;
    out_counts[osi] = 0;
  }

  for (int i = 0; i < platform_c; ++i) {
    os target_os = OS_MAX;
    arch target_arch = ARCH_MAX;
    if (!project_parse_platform_id(platforms[i], &target_os, &target_arch))
      return false;
    ((const char**)out_groups[target_os])[out_counts[target_os]++] = platforms[i];
  }

  return true;
}

static const char* gen_build_platform_command_block(const char* exe_path, const char* action, const char** platforms, int platform_c) {
  if (!exe_path || !exe_path[0] || !action || !action[0] || !platforms || platform_c <= 0)
    return NULL;

  project_textbuf buf = {0};
  for (int i = 0; i < platform_c; ++i) {
    if (!project_textbuf_appendf(&buf,
                                 i == 0 ? "%s %s -t * -p %s -c ${{ matrix.config }}" : "\n          %s %s -t * -p %s -c ${{ matrix.config }}",
                                 exe_path,
                                 action,
                                 platforms[i])) {
      if (buf.data)
        free(buf.data);
      return NULL;
    }
  }

  const char* out = arena_text(buf.data ? buf.data : "", buf.len);
  if (buf.data)
    free(buf.data);
  return out;
}

static const char* gen_build_host_command_block(const char* exe_path, const char* action, const char* host_platform) {
  if (!exe_path || !exe_path[0] || !action || !action[0] || !host_platform || !host_platform[0])
    return NULL;

  char text[512] = {0};
  snprintf(text, sizeof(text), "%s %s -t * -p %s -c ${{ matrix.config }}", exe_path, action, host_platform);
  return arena_text(text, strlen(text));
}

static bool gen_append_github_upload_steps(project_textbuf* buf, const char* dist_root, const char* job_name, const char** platforms, int platform_c) {
  if (!buf || !dist_root || !job_name || !platforms || platform_c <= 0)
    return false;

  for (int i = 0; i < platform_c; ++i) {
    if (!project_textbuf_appendf(buf,
                                 "      - name: Upload %s dist (%s)\n"
                                 "        uses: actions/upload-artifact@v4\n"
                                 "        with:\n"
                                 "          name: dist-%s-${{ matrix.config }}-%s\n"
                                 "          path: %s/${{ matrix.config }}-%s\n",
                                 job_name,
                                 platforms[i],
                                 job_name,
                                 platforms[i],
                                 dist_root,
                                 platforms[i]))
      return false;
  }

  return true;
}

static bool gen_append_github_os_job(project_textbuf* buf,
                                     const project* proj,
                                     os target_os,
                                     const char** platforms,
                                     int platform_c,
                                     bool has_tests,
                                     bool has_runnables,
                                     const char* dist_root) {
  if (!buf || !proj || !platforms || platform_c <= 0 || !dist_root)
    return false;

  const char* job_name = gen_github_job_name_for_os(target_os);
  const char* runner = gen_github_runner_for_os(target_os);
  const char* host_platform = gen_github_host_platform_for_os(target_os);
  if (!job_name || !runner || !host_platform)
    return false;

  const char* build_win = gen_build_platform_command_block("./.bbs-bootstrap/build/bbs.exe", "build", platforms, platform_c);
  const char* build_unix = gen_build_platform_command_block("./.bbs-bootstrap/build/bbs", "build", platforms, platform_c);
  const char* dist_win = gen_build_platform_command_block("./.bbs-bootstrap/build/bbs.exe", "dist", platforms, platform_c);
  const char* dist_unix = gen_build_platform_command_block("./.bbs-bootstrap/build/bbs", "dist", platforms, platform_c);
  if (!build_win || !build_unix || !dist_win || !dist_unix)
    return false;

  if (!project_textbuf_appendf(buf,
                               "  %s:\n"
                               "    name: %s / ${{ matrix.config }}\n"
                               "    runs-on: %s\n"
                               "    strategy:\n"
                               "      fail-fast: false\n"
                               "      matrix:\n"
                               "        config:\n",
                               job_name,
                               job_name,
                               runner))
    return false;

  const char* default_config = gen_default_config_name(proj);
  if (proj->config_c > 0 && proj->configs) {
    for (int i = 0; i < proj->config_c; ++i) {
      const char* cfg = proj->configs[i] && proj->configs[i][0] ? proj->configs[i] : default_config;
      if (!project_textbuf_appendf(buf, "          - %s\n", cfg))
        return false;
    }
  } else if (!project_textbuf_appendf(buf, "          - %s\n", default_config)) {
    return false;
  }

  if (!project_textbuf_append(buf,
                              "\n"
                              "    steps:\n"
                              "      - uses: actions/checkout@v4\n\n"))
    return false;

  if (!gen_append_github_checkout_bbs_step(buf))
    return false;

  if (!gen_append_github_command_steps(buf,
                                       "Build bbs",
                                       "cmake -S .bbs-bootstrap -B .bbs-bootstrap/build\n          cmake --build .bbs-bootstrap/build --config Release",
                                       "cmake -S .bbs-bootstrap -B .bbs-bootstrap/build\n          cmake --build .bbs-bootstrap/build --config Release"))
    return false;

  if (!gen_append_github_command_steps(buf,
                                       "Init toolchain",
                                       "./.bbs-bootstrap/build/bbs.exe update --init-toolchain",
                                       "./.bbs-bootstrap/build/bbs update --init-toolchain"))
    return false;

  if (!gen_append_github_command_steps(buf, "Build project", build_win, build_unix))
    return false;

  if (has_tests && gen_platform_list_contains(platforms, platform_c, host_platform)) {
    const char* test_win = gen_build_host_command_block("./.bbs-bootstrap/build/bbs.exe", "test", host_platform);
    const char* test_unix = gen_build_host_command_block("./.bbs-bootstrap/build/bbs", "test", host_platform);
    if (!test_win || !test_unix)
      return false;
    if (!gen_append_github_command_steps(buf, "Run tests", test_win, test_unix))
      return false;
  }

  if (has_runnables && gen_platform_list_contains(platforms, platform_c, host_platform)) {
    const char* run_win = gen_build_host_command_block("./.bbs-bootstrap/build/bbs.exe", "run", host_platform);
    const char* run_unix = gen_build_host_command_block("./.bbs-bootstrap/build/bbs", "run", host_platform);
    if (!run_win || !run_unix)
      return false;
    if (!gen_append_github_command_steps(buf, "Run targets", run_win, run_unix))
      return false;
  }

  if (!gen_append_github_command_steps(buf, "Create distributions", dist_win, dist_unix))
    return false;

  if (!gen_append_github_upload_steps(buf, dist_root, job_name, platforms, platform_c))
    return false;

  return project_textbuf_append(buf, "\n");
}

static bool gen_build_github_workflow_text(const project* proj, project_textbuf* buf, const char** platforms, int platform_c) {
  if (!proj || !buf)
    return false;

  bool has_tests = project_count_test_targets(proj) > 0;
  bool has_runnables = project_count_runnable_targets(proj) > 0;
  const char* dist_root = (proj->user_cfg.distdir && proj->user_cfg.distdir[0]) ? proj->user_cfg.distdir : DEF_DIST_DIR;

  const char* default_platforms[] = {
      "windows-x86_64",
      "linux-x86_64",
      "macos-x86_64",
  };
  const char** selected_platforms = platforms;
  int selected_platform_c = platform_c;
  if (!selected_platforms || selected_platform_c <= 0) {
    selected_platforms = default_platforms;
    selected_platform_c = (int)_countof(default_platforms);
  }

  const char** platform_groups[OS_MAX] = {0};
  int platform_counts[OS_MAX] = {0};
  if (!gen_collect_platforms_by_os(selected_platforms, selected_platform_c, platform_groups, platform_counts))
    return false;

  if (!project_textbuf_append(buf,
                              "name: bbs\n\n"
                              "on:\n"
                              "  push:\n"
                              "  pull_request:\n"
                              "  workflow_dispatch:\n\n"
                              "jobs:\n"))
    return false;

  for (int osi = 0; osi < OS_MAX; ++osi) {
    if (platform_counts[osi] <= 0)
      continue;
    if (!gen_append_github_os_job(buf, proj, (os)osi, platform_groups[osi], platform_counts[osi], has_tests, has_runnables, dist_root))
      return false;
  }

  return true;
}

static int run_cmd_gen(cmd_ctx* cmdctx) {
  cmdline* cl = cmdctx->cl;
  const char* github_platforms_csv = cmdline_extract_option_value(cl, "p", "platform");
  enum {
    OVERRIDE = 0,
  };

  cmdopt opts[] = {
      [OVERRIDE] = {"o", "override"},
  };

  cmdline_consume_all_options(cl, opts, _countof(opts));
  const char* format = cmdline_consume_param(cl);
  cmdline_validate(cl);

  if (!format || !format[0]) {
    error("Missing generator format.");
    print("Use 'bbs gen gitignore', 'bbs gen github', or a configured custom generator name.");
    return error_code(CMD_GEN, 0);
  }

  const char* path = NULL;
  const char* text = NULL;
  if (_stricmp(format, "gitignore") == 0) {
    path = get_path_cwd(".gitignore");
    text = gen_gitignore_text();
    return gen_write_text_file(path, text, opts[OVERRIDE].present, CMD_GEN, 10);
  }

  if (_stricmp(format, "github") == 0) {
    project proj = {0};
    if (!project_load(&proj))
      return error_code(CMD_GEN, 20);

    const char** github_platforms = NULL;
    int github_platform_c = 0;
    if (!gen_parse_github_platforms(github_platforms_csv, &github_platforms, &github_platform_c))
      return error_code(CMD_GEN, 21);

    project_textbuf buf = {0};
    if (!gen_build_github_workflow_text(&proj, &buf, github_platforms, github_platform_c)) {
      if (buf.data)
        free(buf.data);
      error("Failed to build GitHub workflow content.");
      return error_code(CMD_GEN, 22);
    }

    int rc = gen_write_text_file(get_path_cwd(".github/workflows/bbs.yml"), buf.data ? buf.data : "", opts[OVERRIDE].present, CMD_GEN, 23);
    if (buf.data)
      free(buf.data);
    return rc;
  }

  user* u = user_init(cmdctx);
  if (!u)
    return error_code(CMD_GEN, 30);

  const user_gen* custom = user_find_gen(u, format);
  if (custom) {
    return gen_copy_output_file(custom->copyfile, get_path_cwd(custom->name), opts[OVERRIDE].present, CMD_GEN, 31);
  } else {
    error("Unknown generator format '%s'.", format);
    print("Supported built-in formats: gitignore, github");
    return error_code(CMD_GEN, 1);
  }
}

static int run_cmd_build(cmd_ctx* cmdctx) {
  build_cmd_args args = {0};
  cmd_parse_build_args(cmdctx, &args, NULL, 0);

  toolchain* tc = toolchain_init(cmdctx_cfg_path(cmdctx, CFG_TOOLCHAIN), false, cmdctx);
  if (!cmd_exec_build_args(&args, tc))
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
      [TARGET] = {"t",   "target"},
      [PLATFORM] = {"p", "platform"},
  };

  cmdline_consume_all_options(cmdctx->cl, opts, _countof(opts));
  cmdline_validate(cmdctx->cl);

  toolchain* tc = toolchain_init(cmdctx_cfg_path(cmdctx, CFG_TOOLCHAIN), false, cmdctx);
  if (cmdopt_is_star(target) || cmdopt_is_star(platform) || cmdopt_is_star(config)) {
    if (!cmd_run_run_matrix(target, platform, config, tc))
      return error_code(CMD_RUN, 0);
    return 0;
  }

  if (!project_run(target, platform, config, tc))
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
  if (!out || out_size == 0)
    return;

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
  for (int i = 0; i < child_count; ++i)
    total += info_count_flat_matches(children[i], path, attr_filter, text_filter);
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
  for (int i = 0; i < child_count; ++i)
    info_print_flat_node(children[i], path, attr_filter, text_filter, mode);
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
    for (int i = 0; i < child_count; ++i)
      info_print_node(children[i], path, depth + 1, attr_filter, text_filter);
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
      print("%s is not available.", title);
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
    print("Nodes: %d", total_nodes);
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
    print("Matches: %d", visible_nodes);
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

  cfg c = parse_cfg_name(scope);
  switch (c) {
    case CFG_PROJECT:
      return run_cmd_info_file("Project File Attributes", cmdctx_cfg_path(cmdctx, c), attr_filter, text_filter, mode);
    case CFG_USER:
      return run_cmd_info_file("User File Attributes", cmdctx_cfg_path(cmdctx, c), attr_filter, text_filter, mode);
    case CFG_LOCAL:
      return run_cmd_info_file("Local File Attributes", cmdctx_cfg_path(cmdctx, c), attr_filter, text_filter, mode);
    case CFG_TOOLCHAIN:
      return run_cmd_info_file("Toolchain File Attributes", cmdctx_cfg_path(cmdctx, c), attr_filter, text_filter, mode);
    default:
      break;
  }

  error("Unknown info topic '%s'.", scope);
  print("Use 'project', 'user', 'local', or 'toolchain'.");
  return error_code(CMD_INFO, 1);
}

static int run_cmd_package(cmd_ctx* cmdctx) {
  const char* query = cmdline_consume_param(cmdctx->cl);
  const char* subquery = cmdline_consume_param(cmdctx->cl);
  enum {
    REFRESH = 0,
  };

  cmdopt opts[] = {
      [REFRESH] = {NULL, "refresh"},
  };

  cmdline_consume_all_options(cmdctx->cl, opts, _countof(opts));
  cmdline_validate(cmdctx->cl);

  if (!query || !query[0])
    query = "list";

  project proj = {0};
  if (!project_load(&proj))
    return error_code(CMD_PACKAGE, 1);

  toolchain* tc = toolchain_init(cmdctx_cfg_path(cmdctx, CFG_TOOLCHAIN), false, cmdctx);

  if (_stricmp(query, "refresh") == 0) {
    const char* refresh_target = subquery && subquery[0] ? subquery : "*";
    if (!tc)
      return error_code(CMD_PACKAGE, 4);
    if (!project_refresh_packages(&proj, tc, refresh_target))
      return error_code(CMD_PACKAGE, 5);
    print("Package refresh completed.");
    return 0;
  }

  if (_stricmp(query, "list") == 0) {
    if (opts[REFRESH].present) {
      if (!tc)
        return error_code(CMD_PACKAGE, 4);
      if (!project_prepare_packages(&proj, tc, true))
        return error_code(CMD_PACKAGE, 5);
    }
    project_print_package_list_header();
    for (int i = 0; i < proj.target_c; ++i) {
      const target* tgt = &proj.targets[i];
      if (!project_target_is_package(tgt))
        continue;
      project_print_package_list_row(tgt, tc);
    }
    return 0;
  }

  int idx = project_find_target_index(&proj, query);
  if (idx < 0)
    return error_code(CMD_PACKAGE, 2);

  const target* tgt = &proj.targets[idx];
  if (!project_target_is_package(tgt)) {
    error("Target '%s' is not a package target.", query);
    return error_code(CMD_PACKAGE, 3);
  }

  if (opts[REFRESH].present) {
    if (!tc)
      return error_code(CMD_PACKAGE, 4);
    if (!project_resolve_package_target(&proj.targets[idx], tc, true))
      return error_code(CMD_PACKAGE, 5);
  }

  project_print_package_summary(&proj.targets[idx], tc);
  return 0;
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
      [TARGET] = {"t",   "target"},
      [PLATFORM] = {"p", "platform"},
  };

  cmdline_consume_all_options(cmdctx->cl, opts, _countof(opts));
  cmdline_validate(cmdctx->cl);

  toolchain* tc = toolchain_init(cmdctx_cfg_path(cmdctx, CFG_TOOLCHAIN), false, cmdctx);
  if (cmdopt_is_star(target) || cmdopt_is_star(platform) || cmdopt_is_star(config)) {
    if (!cmd_run_dist_matrix(target, platform, config, tc))
      return error_code(CMD_DIST, 0);
    return 0;
  }

  if (!project_dist(target, platform, config, tc))
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
      [TARGET] = {"t",   "target"},
      [PLATFORM] = {"p", "platform"},
  };

  cmdline_consume_all_options(cmdctx->cl, opts, _countof(opts));
  const char* test_name = cmdline_consume_param(cmdctx->cl);
  cmdline_validate(cmdctx->cl);

  toolchain* tc = toolchain_init(cmdctx_cfg_path(cmdctx, CFG_TOOLCHAIN), false, cmdctx);
  if (cmdopt_is_star(target) || cmdopt_is_star(platform) || cmdopt_is_star(config)) {
    if (!cmd_run_test_matrix(test_name, target, platform, config, tc))
      return error_code(CMD_TEST, 0);
    return 0;
  }

  if (!project_test(test_name, target, platform, config, tc))
    return error_code(CMD_TEST, 0);
  return 0;
}

static bool bumpver_match_meta_field(node* scope, const char* field_name, const char* value);

static bool bumpver_match_project_id(node* project, const char* project_id) {
  if (!project_id || !project_id[0])
    return true;

  return bumpver_match_meta_field(project, "id", project_id);
}

static bool bumpver_match_meta_field(node* scope, const char* field_name, const char* value) {
  if (!scope || !field_name || !field_name[0] || !value || !value[0])
    return false;

  node* field = node_get_child(scope, field_name);
  if (!field)
    return false;

  if (field->type == NODE_TYPE_STR)
    return strlen(value) == field->txt_dim && _strnicmp(value, field->_str, field->txt_dim) == 0;
  if (field->type == NODE_TYPE_IDF)
    return strlen(value) == field->txt_dim && _strnicmp(value, field->_idf, field->txt_dim) == 0;
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

  if (matches == 0 && bumpver_match_project_id(tree, project_id)) {
    match = tree;
    matches = 1;
  }

  if (out_matches)
    *out_matches = matches;
  return matches == 1 ? match : NULL;
}

static bool bumpver_match_target_name(node* target, const char* target_name) {
  if (!target_name || !target_name[0])
    return true;

  return bumpver_match_meta_field(target, "id", target_name) || bumpver_match_meta_field(target, "output", target_name) ||
         bumpver_match_meta_field(target, "name", target_name);
}

static node* bumpver_find_target(node* project, const char* target_name, int* out_matches) {
  node* targets = node_get_child(project, "targets");
  node* match = NULL;
  int matches = 0;
  if (!targets) {
    if (out_matches)
      *out_matches = 0;
    return NULL;
  }

  node_foreach(targets, child) {
    if (!child->name)
      continue;
    if (!bumpver_match_target_name(child, target_name))
      continue;
    match = child;
    ++matches;
  }

  if (out_matches)
    *out_matches = matches;
  return matches == 1 ? match : NULL;
}

static node* bumpver_find_version(node* scope) {
  if (!scope)
    return NULL;

  return node_get_child(scope, "ver");
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
  const char* project_id = cmdline_extract_option_value(cl, "p", "project");
  const char* target_id = cmdline_extract_option_value(cl, "t", "target");
  const char* part = cmdline_consume_param(cl);
  const char* positional_project_id = cmdline_consume_param(cl);
  cmdline_validate(cl);

  if (!project_id)
    project_id = positional_project_id;
  else if (positional_project_id && positional_project_id[0] && _stricmp(project_id, positional_project_id) != 0) {
    error("Project id specified twice ('%s' and '%s').", project_id, positional_project_id);
    return error_code(CMD_BUMPVER, 16);
  }

  if (!part || !part[0]) {
    error("Missing version part.");
    print("Use 'bbs bumpver <major|minor|patch|user|all> [-p project_id] [-t target_id]'.");
    return error_code(CMD_BUMPVER, 0);
  }

  const char* project_path = cmdctx_cfg_path(cmdctx, CFG_PROJECT);
  if (!file_exists(project_path)) {
    error("Project file not found: %s", project_path);
    return error_code(CMD_BUMPVER, 1);
  }

  const char* text = read_entire_file(project_path);
  if (!text) {
    error("Failed to read '%s'.", project_path);
    return error_code(CMD_BUMPVER, 2);
  }

  node* tree = node_parse(text);
  if (!tree) {
    error("Failed to parse '%s'.", project_path);
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
      error("No project scope found in '%s'.", project_path);
    } else {
      error("Multiple top-level project nodes found. Specify [project_id].");
    }
    return error_code(CMD_BUMPVER, 4);
  }

  node* scope = project;
  const char* scope_label = "project";
  int target_matches = 0;
  if (target_id && target_id[0]) {
    node* target = bumpver_find_target(project, target_id, &target_matches);
    if (!target) {
      if (target_matches == 0)
        error("No target node found matching '%s'.", target_id);
      else
        error("Multiple target nodes match '%s'.", target_id);
      return error_code(CMD_BUMPVER, 17);
    }
    scope = target;
    scope_label = "target";
  }

  node* version = bumpver_find_version(scope);
  if (!version) {
    if (target_id && target_id[0])
      error("Target '%s' does not define a version.", target_id);
    else
      error("Project does not define a version.");
    return error_code(CMD_BUMPVER, 5);
  }
  if (version->type != NODE_TYPE_VER) {
    error("%s version must be a version value.", scope_label);
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
    if (!bumpver_inc_byte(&version->_ver.major, "major") || !bumpver_inc_byte(&version->_ver.minor, "minor") ||
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
  if (!write_entire_file(project_path, updated)) {
    error("Failed to write '%s'.", project_path);
    return error_code(CMD_BUMPVER, 15);
  }

  char before_text[32] = {0};
  char after_text[32] = {0};
  if (before_parts >= 4)
    snprintf(before_text, sizeof(before_text), "%u.%u.%u.%u", before.major, before.minor, before.patch, before.user);
  else
    snprintf(before_text, sizeof(before_text), "%u.%u.%u", before.major, before.minor, before.patch);
  info_format_value(version, after_text, sizeof(after_text));

  if (target_id && target_id[0])
    print("Updated target version for '%s': %s -> %s", target_id, before_text, after_text);
  else if (project_id && project_id[0])
    print("Updated project version for '%s': %s -> %s", project_id, before_text, after_text);
  else
    print("Updated version: %s -> %s", before_text, after_text);
  print("File: %s", project_path);
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
    case CMD_GEN:
      return run_cmd_gen(cmdctx);
    case CMD_BUILD:
      return run_cmd_build(cmdctx);
    case CMD_AUTO:
      return run_cmd_auto(cmdctx);
    case CMD_RUN:
      return run_cmd_run(cmdctx);
    case CMD_INFO:
      return run_cmd_info(cmdctx);
    case CMD_PACKAGE:
      return run_cmd_package(cmdctx);
    case CMD_DIST:
      return run_cmd_dist(cmdctx);
    case CMD_TEST:
      return run_cmd_test(cmdctx);
    case CMD_BUMPVER:
      return run_cmd_bumpver(cmdctx);
    default:
      print("Command '%s' is not implemented yet.", CMD_INFOS[c].name);
      return error_code(c, 1);
  }
}

static int print_unrecognized_command(const char* name) {
  error("'%s' is not a recognized command.", name);
  print("Run 'bbs %s' to see the list of available commands.", CMD_INFOS[CMD_HELP].name);
  return 2;
}

static cmd_ctx* init_cmd_ctx(int argc, char** argv) {
  cmd_ctx* cmdctx = push(sizeof(cmd_ctx));
  memset(cmdctx, 0, sizeof(cmd_ctx));
  cmdctx->cl = push(sizeof(cmdline));
  memset(cmdctx->cl, 0, sizeof(cmdline));

  cmdctx->cl->argv = argv;
  cmdctx->cl->argc = argc;
  cmdline_pop(cmdctx->cl);
  cmdline_pop(cmdctx->cl);

  for (int i = 0; i < CFG_MAX; ++i) {
    const cfg_info info = CFG_INFOS[i];
    cmdctx->cfg_paths[i] = info.loc == CFG_LOC_CWD ? get_path_cwd(info.filename) : get_path_exe(info.filename);
  }
  return cmdctx;
}
