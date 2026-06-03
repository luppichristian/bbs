#pragma once
#include "bbs_toolchain.h"
#include "bbs_base.c"
#include "bbs_cmd.h"

static bool toolchain_tool_exists(toolchain* tc, const char* path);
static const char* toolchain_probe_version(const char* exe_path, const char* arg_a, const char* arg_b, const char* pat_a, const char* pat_b);
static const char* toolchain_first_path_match(const char* pattern);
static int toolchain_collect_path_matches(const char* pattern, const char** matches, int max_matches, bool dirs_only);
static const char* toolchain_path_basename(const char* path);
static const char* toolchain_join2(const char* a, const char* b);
static const char* toolchain_norm_path(const char* in);
static const char* toolchain_extract_version(const char* text);
static const char* toolchain_extract_version_by_pattern(const char* text, const char* pattern);
static const char* toolchain_path_version_fallback(const char* path);
static int toolchain_run_collect_lines(const char* cmd, const char** lines, int max_lines);
static const char* toolchain_run_extract_version(const char* cmd);
static const char* toolchain_append_text(const char* text, const char* suffix);
static const char* toolchain_label2(const char* a, const char* b);
static arch toolchain_arch_from_text(const char* text);
static void toolchain_set_tool(tool* out, const char* id, const char* path, const char* version);
static int toolchain_tool_cmp(const tool* a, const tool* b);
static int toolchain_sdk_cmp(const sdk* a, const sdk* b);
static void toolchain_sort_tools(toolchain* tc);
static void toolchain_sort_sdks(toolchain* tc);
static int toolchain_env_find_index(toolchain* tc, const char* id);
static const char* toolchain_make_env_id(const char* provider, const char* name, os target_os, arch target_arch);
static toolchain_env* toolchain_host_env(toolchain* tc, bool ensure);
static void toolchain_sort_tool_array(tool* items, int count);
static void toolchain_sort_sdk_array(sdk* items, int count);
static bool toolchain_ensure_env_capacity(toolchain* tc, int min_cap);
static bool toolchain_env_ensure_tool_capacity(toolchain_env* env, int min_cap);
static bool toolchain_env_ensure_sdk_capacity(toolchain_env* env, int min_cap);
static void toolchain_snapshot_current_host_env(toolchain* tc);
static void toolchain_discover_extra_envs(toolchain* tc);
static void toolchain_refresh_runtime_support(toolchain* tc);
static void toolchain_discover_tool(toolchain* tc, const tool_discover_strat* s);
static const tool_discover_strat* toolchain_find_discover_strat(const char* id);
static const char* toolchain_ensure_host_tool_path(toolchain* tc, const char* id);
static bool toolchain_is_usable_host_bash_path(const char* path);

static arch toolchain_detect_host_arch(void) {
  const char* name = platform_host_arch_name();
  if (_stricmp(name, "arm64") == 0)
    return ARCH_ARM64;
  if (_stricmp(name, "x86") == 0)
    return ARCH_X86;
  return ARCH_X86_64;
}

static os toolchain_detect_host_os(void) {
  const char* name = platform_host_os_name();
  if (_stricmp(name, "windows") == 0)
    return OS_WINDOWS;
  if (_stricmp(name, "macos") == 0)
    return OS_MACOS;
  return OS_LINUX;
}

typedef struct {
  bool minimal;
  bool show_tools;
  bool show_sdks;
  bool paths_only;
  bool versions_only;
  const char* tool_id_filter;
  const char* sdk_name_filter;
} toolchain_print_opts;

static bool toolchain_list_has_ci(const char** items, int count, const char* value) {
  if (!items || count <= 0 || !value || !value[0])
    return false;

  for (int i = 0; i < count; ++i) {
    if (items[i] && _stricmp(items[i], value) == 0)
      return true;
  }

  return false;
}

static const char* toolchain_get_tool_path(toolchain* tc, const char* id) {
  if (!tc || !id || !id[0])
    return NULL;

  toolchain_env* env = toolchain_host_env(tc, false);
  if (!env)
    return NULL;

  for (int i = 0; i < env->tool_c; ++i) {
    if (env->tools[i].id && _stricmp(env->tools[i].id, id) == 0) {
      if (_stricmp(id, "bash") == 0 && !toolchain_is_usable_host_bash_path(env->tools[i].path))
        continue;
      return env->tools[i].path;
    }
  }

  return NULL;
}

static const tool_discover_strat* toolchain_find_discover_strat(const char* id) {
  if (!id || !id[0])
    return NULL;

  for (size_t i = 0; i < _countof(TOOL_DISCOVER_STRATS); ++i) {
    if (TOOL_DISCOVER_STRATS[i].id && _stricmp(TOOL_DISCOVER_STRATS[i].id, id) == 0)
      return &TOOL_DISCOVER_STRATS[i];
  }

  return NULL;
}

static const char* toolchain_ensure_host_tool_path(toolchain* tc, const char* id) {
  const char* path = toolchain_get_tool_path(tc, id);
  if (path && path[0])
    return path;

  const tool_discover_strat* strat = toolchain_find_discover_strat(id);
  if (!strat)
    return NULL;

  toolchain_discover_tool(tc, strat);
  toolchain_sort_tools(tc);
  toolchain_snapshot_current_host_env(tc);
  toolchain_refresh_runtime_support(tc);
  return toolchain_get_tool_path(tc, id);
}

static const char* toolchain_get_host_tool_path(toolchain* tc, const char* id) {
  return toolchain_ensure_host_tool_path(tc, id);
}

static const char* toolchain_get_bash_path(toolchain* tc) {
  return toolchain_get_host_tool_path(tc, "bash");
}

static bool toolchain_is_usable_host_bash_path(const char* path) {
  return platform_is_usable_bash_path(path);
}

static void toolchain_set_platform_support_source(toolchain* tc, os target_os, arch target_arch, const char* source) {
  if (!tc || target_os < 0 || target_os >= OS_MAX || target_arch < 0 || target_arch >= ARCH_MAX || !source || !source[0])
    return;

  tc->support_source[target_os][target_arch] = arena_text(source, strlen(source));
}

static void toolchain_enable_platform_support_with_source(toolchain* tc, os target_os, arch target_arch, const char* source) {
  if (!tc || target_os < 0 || target_os >= OS_MAX || target_arch < 0 || target_arch >= ARCH_MAX)
    return;

  if (!tc->supported[target_os][target_arch]) {
    tc->supported[target_os][target_arch] = true;
    toolchain_set_platform_support_source(tc, target_os, target_arch, source);
    return;
  }

  if (!tc->support_source[target_os][target_arch] || !tc->support_source[target_os][target_arch][0])
    toolchain_set_platform_support_source(tc, target_os, target_arch, source);
}

static toolchain_env* toolchain_host_env(toolchain* tc, bool ensure) {
  if (!tc)
    return NULL;

  int idx = toolchain_env_find_index(tc, toolchain_make_env_id("host", "current", tc->p_os, tc->p_arch));
  if (idx >= 0)
    return &tc->envs[idx];
  if (!ensure || !toolchain_ensure_env_capacity(tc, tc->env_c + 1))
    return NULL;

  toolchain_env* env = &tc->envs[tc->env_c++];
  memset(env, 0, sizeof(*env));
  env->provider = "host";
  env->name = "current";
  env->p_os = tc->p_os;
  env->p_arch = tc->p_arch;
  env->id = toolchain_make_env_id(env->provider, env->name, env->p_os, env->p_arch);
  return env;
}

static bool toolchain_ensure_env_capacity(toolchain* tc, int min_cap) {
  if (!tc || min_cap <= 0)
    return false;
  if (tc->env_cap >= min_cap)
    return true;

  int new_cap = tc->env_cap > 0 ? tc->env_cap : 8;
  while (new_cap < min_cap)
    new_cap *= 2;

  toolchain_env* items = push((size_t)new_cap * sizeof(*items));
  if (!items)
    return false;

  if (tc->envs && tc->env_c > 0)
    memcpy(items, tc->envs, (size_t)tc->env_c * sizeof(*items));
  tc->envs = items;
  tc->env_cap = new_cap;
  return true;
}

static bool toolchain_env_ensure_tool_capacity(toolchain_env* env, int min_cap) {
  if (!env || min_cap <= 0)
    return false;
  if (env->tool_cap >= min_cap)
    return true;

  int new_cap = env->tool_cap > 0 ? env->tool_cap : 16;
  while (new_cap < min_cap)
    new_cap *= 2;

  tool* items = push((size_t)new_cap * sizeof(*items));
  if (!items)
    return false;

  if (env->tools && env->tool_c > 0)
    memcpy(items, env->tools, (size_t)env->tool_c * sizeof(*items));
  env->tools = items;
  env->tool_cap = new_cap;
  return true;
}

static bool toolchain_env_ensure_sdk_capacity(toolchain_env* env, int min_cap) {
  if (!env || min_cap <= 0)
    return false;
  if (env->sdk_cap >= min_cap)
    return true;

  int new_cap = env->sdk_cap > 0 ? env->sdk_cap : 16;
  while (new_cap < min_cap)
    new_cap *= 2;

  sdk* items = push((size_t)new_cap * sizeof(*items));
  if (!items)
    return false;

  if (env->sdks && env->sdk_c > 0)
    memcpy(items, env->sdks, (size_t)env->sdk_c * sizeof(*items));
  env->sdks = items;
  env->sdk_cap = new_cap;
  return true;
}

static int toolchain_env_find_index(toolchain* tc, const char* id) {
  if (!tc || !id || !id[0])
    return -1;

  for (int i = 0; i < tc->env_c; ++i) {
    if (tc->envs[i].id && _stricmp(tc->envs[i].id, id) == 0)
      return i;
  }

  return -1;
}

static bool toolchain_env_has_tool_id(toolchain_env* env, const char* id) {
  if (!env || !id || !id[0])
    return false;

  for (int i = 0; i < env->tool_c; ++i) {
    if (env->tools[i].id && _stricmp(env->tools[i].id, id) == 0)
      return true;
  }

  return false;
}

static bool toolchain_env_has_sdk_name(toolchain_env* env, const char* name) {
  if (!env || !name || !name[0])
    return false;

  for (int i = 0; i < env->sdk_c; ++i) {
    if (env->sdks[i].name && _stricmp(env->sdks[i].name, name) == 0)
      return true;
  }

  return false;
}

static void toolchain_env_set_support_source(toolchain_env* env, os target_os, arch target_arch, const char* source) {
  if (!env || target_os < 0 || target_os >= OS_MAX || target_arch < 0 || target_arch >= ARCH_MAX || !source || !source[0])
    return;

  env->support_source[target_os][target_arch] = arena_text(source, strlen(source));
}

static void toolchain_env_enable_support(toolchain_env* env, os target_os, arch target_arch, const char* source) {
  if (!env || target_os < 0 || target_os >= OS_MAX || target_arch < 0 || target_arch >= ARCH_MAX)
    return;

  env->supported[target_os][target_arch] = true;
  if ((!env->support_source[target_os][target_arch] || !env->support_source[target_os][target_arch][0]) && source && source[0])
    toolchain_env_set_support_source(env, target_os, target_arch, source);
}

static void toolchain_sort_tool_array(tool* items, int count) {
  if (!items || count <= 1)
    return;

  for (int i = 0; i < count - 1; ++i) {
    for (int j = i + 1; j < count; ++j) {
      if (toolchain_tool_cmp(&items[i], &items[j]) > 0) {
        tool tmp = items[i];
        items[i] = items[j];
        items[j] = tmp;
      }
    }
  }
}

static void toolchain_sort_sdk_array(sdk* items, int count) {
  if (!items || count <= 1)
    return;

  for (int i = 0; i < count - 1; ++i) {
    for (int j = i + 1; j < count; ++j) {
      if (toolchain_sdk_cmp(&items[i], &items[j]) > 0) {
        sdk tmp = items[i];
        items[i] = items[j];
        items[j] = tmp;
      }
    }
  }
}

static const char* toolchain_make_env_id(const char* provider, const char* name, os target_os, arch target_arch) {
  char buf[256] = {0};
  snprintf(buf,
           sizeof(buf),
           "%s:%s:%s:%s",
           provider ? provider : "env",
           name ? name : "default",
           target_os >= 0 && target_os < OS_MAX ? OS_NAMES[target_os] : "unknown",
           target_arch >= 0 && target_arch < ARCH_MAX ? ARCH_NAMES[target_arch] : "unknown");
  return arena_text(buf, strlen(buf));
}

static void toolchain_add_or_replace_env(toolchain* tc, const toolchain_env* src) {
  if (!tc || !src || !src->id || !src->id[0])
    return;

  int idx = toolchain_env_find_index(tc, src->id);
  if (idx >= 0) {
    tc->envs[idx] = *src;
    return;
  }

  if (!toolchain_ensure_env_capacity(tc, tc->env_c + 1))
    return;
  tc->envs[tc->env_c++] = *src;
}

static void toolchain_snapshot_current_host_env(toolchain* tc) {
  if (!tc)
    return;

  toolchain_env* env = toolchain_host_env(tc, true);
  if (!env)
    return;
  env->p_os = tc->p_os;
  env->p_arch = tc->p_arch;
  env->provider = "host";
  env->name = "current";
  env->id = toolchain_make_env_id(env->provider, env->name, env->p_os, env->p_arch);
  memcpy(env->supported, tc->supported, sizeof(env->supported));
  memcpy(env->support_source, tc->support_source, sizeof(env->support_source));
}

static void toolchain_rebuild_aggregate_support(toolchain* tc) {
  if (!tc)
    return;

  memset(tc->supported, 0, sizeof(tc->supported));
  memset(tc->support_source, 0, sizeof(tc->support_source));
  for (int ei = 0; ei < tc->env_c; ++ei) {
    toolchain_env* env = &tc->envs[ei];
    for (int osi = 0; osi < OS_MAX; ++osi) {
      for (int ai = 0; ai < ARCH_MAX; ++ai) {
        if (!env->supported[osi][ai])
          continue;
        toolchain_enable_platform_support_with_source(tc, (os)osi, (arch)ai, env->support_source[osi][ai] ? env->support_source[osi][ai] : env->id);
      }
    }
  }
}

static void toolchain_fill_env_platform_support(toolchain_env* env, bool is_current_host) {
  if (!env)
    return;

  memset(env->supported, 0, sizeof(env->supported));
  memset(env->support_source, 0, sizeof(env->support_source));

  if (!is_current_host)
    return;

  bool has_cmake = toolchain_env_has_tool_id(env, "cmake");
  bool has_vcvarsall = toolchain_env_has_tool_id(env, "vcvarsall");
  bool has_msvc = has_vcvarsall && toolchain_env_has_sdk_name(env, "msvc") && toolchain_env_has_sdk_name(env, "windows_sdk") && toolchain_env_has_sdk_name(env, "ucrt_sdk");
  bool has_xcode = toolchain_env_has_sdk_name(env, "xcode");

  if (env->p_os == OS_WINDOWS) {
    if (has_msvc) {
      toolchain_env_enable_support(env, OS_WINDOWS, ARCH_X86_64, "msvc");
      toolchain_env_enable_support(env, OS_WINDOWS, ARCH_X86, "msvc");
      toolchain_env_enable_support(env, OS_WINDOWS, ARCH_ARM64, "msvc");
    } else if (has_cmake) {
      toolchain_env_enable_support(env, OS_WINDOWS, env->p_arch, "host-toolchain");
    }
  }

  if (env->p_os == OS_LINUX && has_cmake)
    toolchain_env_enable_support(env, OS_LINUX, env->p_arch, "host-toolchain");

  if (env->p_os == OS_MACOS && has_xcode) {
    toolchain_env_enable_support(env, OS_MACOS, ARCH_X86_64, "xcode");
    toolchain_env_enable_support(env, OS_MACOS, ARCH_ARM64, "xcode");
  } else if (env->p_os == OS_MACOS && has_cmake) {
    toolchain_env_enable_support(env, OS_MACOS, env->p_arch, "host-toolchain");
  }
}

static void toolchain_fill_wsl_env_platform_support(toolchain_env* env) {
  if (!env || !env->provider || _stricmp(env->provider, "wsl") != 0)
    return;

  memset(env->supported, 0, sizeof(env->supported));
  memset(env->support_source, 0, sizeof(env->support_source));

  toolchain_env_enable_support(env, OS_LINUX, env->p_arch, env->name ? toolchain_label2("wsl:", env->name) : "wsl:default");
}

static void toolchain_apply_cached_docker_support(toolchain* tc) {
  if (!tc)
    return;

  toolchain_env* host = toolchain_host_env(tc, false);
  if (!host || !toolchain_env_has_tool_id(host, "docker") || !host->probe_docker_buildx_platforms || !host->probe_docker_buildx_platforms[0])
    return;

  const char* text = host->probe_docker_buildx_platforms;
  if (strstr(text, "linux/amd64"))
    toolchain_enable_platform_support_with_source(tc, OS_LINUX, ARCH_X86_64, "docker-buildx");
  if (strstr(text, "linux/386"))
    toolchain_enable_platform_support_with_source(tc, OS_LINUX, ARCH_X86, "docker-buildx");
  if (strstr(text, "linux/arm64"))
    toolchain_enable_platform_support_with_source(tc, OS_LINUX, ARCH_ARM64, "docker-buildx");
}

static void toolchain_refresh_runtime_support(toolchain* tc) {
  if (!tc)
    return;

  const char* current_host_id = toolchain_make_env_id("host", "current", tc->p_os, tc->p_arch);
  for (int i = 0; i < tc->env_c; ++i) {
    toolchain_env* env = &tc->envs[i];
    bool is_current_host = env->provider && _stricmp(env->provider, "host") == 0 && env->id && _stricmp(env->id, current_host_id) == 0;
    if (env->provider && _stricmp(env->provider, "wsl") == 0)
      toolchain_fill_wsl_env_platform_support(env);
    else
      toolchain_fill_env_platform_support(env, is_current_host);
  }

  toolchain_rebuild_aggregate_support(tc);
  toolchain_apply_cached_docker_support(tc);
}

static int toolchain_collect_wsl_distros(const char** distros, int max_distros) {
  if (!distros || max_distros <= 0)
    return 0;

  const char* cmd = platform_wsl_distro_query_command();
  if (!cmd)
    return 0;

  const char* lines[64] = {0};
  int line_c = toolchain_run_collect_lines(cmd, lines, _countof(lines));
  int out = 0;
  for (int i = 0; i < line_c && out < max_distros; ++i) {
    const char* line = lines[i];
    if (!line || !line[0])
      continue;

    const char* reg_sz = strstr(line, "REG_SZ");
    if (!reg_sz)
      continue;

    reg_sz += strlen("REG_SZ");
    while (*reg_sz && isspace((unsigned char)*reg_sz))
      ++reg_sz;
    if (!reg_sz[0] || toolchain_list_has_ci(distros, out, reg_sz))
      continue;

    distros[out++] = arena_text(reg_sz, strlen(reg_sz));
  }
  return out;
}

static arch toolchain_arch_from_text(const char* text) {
  if (!text || !text[0])
    return ARCH_X86_64;
  if (_stricmp(text, "aarch64") == 0 || _stricmp(text, "arm64") == 0)
    return ARCH_ARM64;
  if (_stricmp(text, "i686") == 0 || _stricmp(text, "i386") == 0 || _stricmp(text, "x86") == 0)
    return ARCH_X86;
  return ARCH_X86_64;
}

static int toolchain_wsl_collect_lines(const char* wsl_cmd, const char* distro, const char* shell_cmd, const char** lines, int max_lines) {
  if (!wsl_cmd || !wsl_cmd[0] || !shell_cmd || !shell_cmd[0] || !lines || max_lines <= 0)
    return 0;

  char cmd[4096] = {0};
  if (distro && distro[0])
    snprintf(cmd, sizeof(cmd), "%s -d \"%s\" -e sh -lc \"%s\" 2>nul", wsl_cmd, distro, shell_cmd);
  else
    snprintf(cmd, sizeof(cmd), "%s -e sh -lc \"%s\" 2>nul", wsl_cmd, shell_cmd);
  return toolchain_run_collect_lines(cmd, lines, max_lines);
}

static void toolchain_discover_wsl_env(toolchain* tc, const char* distro) {
  if (!tc)
    return;

  const char* wsl = toolchain_get_tool_path(tc, "wsl");
  const char* wsl_cmd = toolchain_path_basename(wsl ? wsl : "wsl.exe");
  const char* env_name = (distro && distro[0]) ? distro : "default";
  if (!wsl_cmd || !wsl_cmd[0])
    wsl_cmd = "wsl.exe";

  if (!distro || !distro[0]) {
    const char* distros[4] = {0};
    int distro_c = toolchain_collect_wsl_distros(distros, _countof(distros));
    if (distro_c > 0 && distros[0] && distros[0][0])
      env_name = distros[0];
  }

  toolchain_env env = {0};
  env.provider = "wsl";
  env.name = arena_text(env_name, strlen(env_name));
  env.p_os = OS_LINUX;

  const char* lines[64] = {0};
  int line_c = toolchain_wsl_collect_lines(wsl_cmd,
                                           distro,
                                           "uname -m",
                                           lines,
                                           _countof(lines));
  if (line_c <= 0)
    return;

  const char* native_arch = lines[0];
  env.p_arch = toolchain_arch_from_text(native_arch);
  env.id = toolchain_make_env_id(env.provider, env.name, env.p_os, env.p_arch);
  toolchain_env_enable_support(&env, OS_LINUX, env.p_arch, distro && distro[0] ? toolchain_join2("wsl:", distro) : "wsl:default");

  for (size_t i = 0; i < sizeof(TOOL_DISCOVER_STRATS) / sizeof(TOOL_DISCOVER_STRATS[0]); ++i) {
    const tool_discover_strat* s = &TOOL_DISCOVER_STRATS[i];
    if (!s->exe_name || !s->exe_name[0])
      continue;
    if (!(s->target_os == OS_MAX || s->target_os == OS_LINUX))
      continue;
    if (_stricmp(s->id, "wsl") == 0 || _stricmp(s->id, "vcvarsall") == 0 || _stricmp(s->id, "docker") == 0)
      continue;
    if (!toolchain_env_ensure_tool_capacity(&env, env.tool_c + 1))
      break;

    const char* path_lines[4] = {0};
    int path_c = toolchain_wsl_collect_lines(wsl_cmd, distro, toolchain_append_text("command -v ", s->exe_name), path_lines, _countof(path_lines));
    if (path_c <= 0 || !path_lines[0] || !path_lines[0][0])
      continue;

    const char* version = NULL;
    if (s->version_arg && s->version_arg[0]) {
      char ver_shell[512] = {0};
      snprintf(ver_shell, sizeof(ver_shell), "%s %s 2>&1", s->exe_name, s->version_arg);
      const char* ver_lines[32] = {0};
      int ver_c = toolchain_wsl_collect_lines(wsl_cmd, distro, ver_shell, ver_lines, _countof(ver_lines));
      for (int vi = 0; vi < ver_c && !version; ++vi)
        version = s->version_regex && s->version_regex[0] ? toolchain_extract_version_by_pattern(ver_lines[vi], s->version_regex) : toolchain_extract_version(ver_lines[vi]);
    }

    toolchain_set_tool(&env.tools[env.tool_c++], s->id, path_lines[0], version ? version : toolchain_path_version_fallback(path_lines[0]));
  }

  toolchain_sort_tool_array(env.tools, env.tool_c);
  toolchain_sort_sdk_array(env.sdks, env.sdk_c);
  toolchain_add_or_replace_env(tc, &env);
}

static void toolchain_capture_host_docker_probe(toolchain* tc) {
  if (!tc)
    return;

  toolchain_env* host = toolchain_host_env(tc, false);
  const char* docker = toolchain_get_tool_path(tc, "docker");
  if (!host || !docker || !docker[0])
    return;

  char cmd[2048] = {0};
  snprintf(cmd, sizeof(cmd), "\"%s\" buildx inspect 2>&1", docker);
  const char* lines[64] = {0};
  int line_c = toolchain_run_collect_lines(cmd, lines, _countof(lines));
  if (line_c <= 0)
    return;

  char joined[4096] = {0};
  for (int i = 0; i < line_c; ++i) {
    const char* line = lines[i];
    if (!line || !line[0])
      continue;
    if (!strstr(line, "linux/"))
      continue;
    if (joined[0])
      strncat(joined, ";", sizeof(joined) - strlen(joined) - 1);
    strncat(joined, line, sizeof(joined) - strlen(joined) - 1);
  }
  if (joined[0])
    host->probe_docker_buildx_platforms = arena_text(joined, strlen(joined));
}

static void toolchain_discover_extra_envs(toolchain* tc) {
  if (!tc)
    return;

  if (platform_supports_wsl())
    toolchain_discover_wsl_env(tc, "");
}

static void toolchain_fill_platform_support(toolchain* tc) {
  if (!tc)
    return;

  memset(tc->supported, 0, sizeof(tc->supported));
  memset(tc->support_source, 0, sizeof(tc->support_source));

  toolchain_refresh_runtime_support(tc);
}

static void toolchain_print_platform_support(toolchain* tc, bool minimal) {
  if (!tc)
    return;

  if (minimal) {
    for (int osi = 0; osi < OS_MAX; ++osi) {
      for (int ai = 0; ai < ARCH_MAX; ++ai)
        print("support|%s|%s|%s|%s", OS_NAMES[osi], ARCH_NAMES[ai], tc->supported[osi][ai] ? "true" : "false", tc->support_source[osi][ai] ? tc->support_source[osi][ai] : "");
    }
    return;
  }

  print("Supported Platforms:");
  print("  %-10s %-7s %-7s %-7s", "os", ARCH_NAMES[ARCH_X86_64], ARCH_NAMES[ARCH_X86], ARCH_NAMES[ARCH_ARM64]);
  for (int osi = 0; osi < OS_MAX; ++osi) {
    print("  %-10s %-7s %-7s %-7s",
          OS_NAMES[osi],
          tc->supported[osi][ARCH_X86_64] ? "yes" : "no",
          tc->supported[osi][ARCH_X86] ? "yes" : "no",
          tc->supported[osi][ARCH_ARM64] ? "yes" : "no");
  }

  print("Support Sources:");
  for (int osi = 0; osi < OS_MAX; ++osi) {
    for (int ai = 0; ai < ARCH_MAX; ++ai) {
      if (!tc->supported[osi][ai])
        continue;
      print("  - %s/%s: %s", OS_NAMES[osi], ARCH_NAMES[ai], tc->support_source[osi][ai] ? tc->support_source[osi][ai] : "unknown");
    }
  }
}

static bool toolchain_push_unique_path(const char** items, int* count, int max_count, const char* value) {
  if (!items || !count || max_count <= 0 || !value || !value[0] || *count >= max_count)
    return false;

  const char* norm = toolchain_norm_path(value);
  if (!norm || !norm[0] || toolchain_list_has_ci(items, *count, norm))
    return false;

  items[(*count)++] = norm;
  return true;
}

static const char* toolchain_path_basename(const char* path) {
  if (!path)
    return NULL;

  const char* slash = strrchr(path, '/');
  const char* bslash = strrchr(path, '\\');
  const char* last = slash;
  if (!last || (bslash && bslash > last))
    last = bslash;
  return last ? last + 1 : path;
}

static bool toolchain_path_name_matches(const char* path, const char* exe_name) {
  if (!path || !path[0] || !exe_name || !exe_name[0])
    return false;

  const char* base = toolchain_path_basename(path);
  if (!base || !base[0])
    return false;

  return platform_executable_name_matches(base, exe_name);
}

static int toolchain_parse_version_part(const char** text) {
  int value = 0;
  while (**text >= '0' && **text <= '9') {
    value = value * 10 + (**text - '0');
    ++(*text);
  }
  return value;
}

static int toolchain_version_cmp(const char* a, const char* b) {
  if ((!a || !a[0]) && (!b || !b[0]))
    return 0;
  if (!a || !a[0])
    return -1;
  if (!b || !b[0])
    return 1;

  const char* pa = a;
  const char* pb = b;
  while ((pa && *pa) || (pb && *pb)) {
    int va = toolchain_parse_version_part(&pa);
    int vb = toolchain_parse_version_part(&pb);
    if (va != vb)
      return (va > vb) ? 1 : -1;

    if (pa && *pa == '.')
      ++pa;
    if (pb && *pb == '.')
      ++pb;
  }

  return 0;
}

static bool toolchain_version_has_dot(const char* text) {
  return text && strchr(text, '.') != NULL;
}

static int toolchain_tool_cmp(const tool* a, const tool* b) {
  const char* ai = a->id ? a->id : "";
  const char* bi = b->id ? b->id : "";
  int id_cmp = _stricmp(ai, bi);
  if (id_cmp != 0)
    return id_cmp;

  int ver_cmp = toolchain_version_cmp(a->version, b->version);
  if (ver_cmp != 0)
    return -ver_cmp;

  const char* ap = a->path ? a->path : "";
  const char* bp = b->path ? b->path : "";
  return _stricmp(ap, bp);
}

static int toolchain_sdk_cmp(const sdk* a, const sdk* b) {
  const char* an = a->name ? a->name : "";
  const char* bn = b->name ? b->name : "";
  int name_cmp = _stricmp(an, bn);
  if (name_cmp != 0)
    return name_cmp;

  int ver_cmp = toolchain_version_cmp(a->version, b->version);
  if (ver_cmp != 0)
    return -ver_cmp;

  const char* ap = a->base_path ? a->base_path : "";
  const char* bp = b->base_path ? b->base_path : "";
  return _stricmp(ap, bp);
}

static void toolchain_sort_tools(toolchain* tc) {
  if (!tc)
    return;
  toolchain_env* env = toolchain_host_env(tc, false);
  if (!env)
    return;
  toolchain_sort_tool_array(env->tools, env->tool_c);
}

static void toolchain_sort_sdks(toolchain* tc) {
  if (!tc)
    return;
  toolchain_env* env = toolchain_host_env(tc, false);
  if (!env)
    return;
  toolchain_sort_sdk_array(env->sdks, env->sdk_c);
}

static int toolchain_find_tool_by_id(toolchain* tc, const char* id) {
  if (!tc || !id || !id[0])
    return -1;

  toolchain_env* env = toolchain_host_env(tc, false);
  if (!env)
    return -1;

  for (int i = 0; i < env->tool_c; ++i) {
    if (env->tools[i].id && _stricmp(env->tools[i].id, id) == 0)
      return i;
  }

  return -1;
}

static const char* toolchain_path_version_fallback(const char* path) {
  if (!path || !path[0])
    return NULL;

  return toolchain_extract_version(path);
}

static const char* toolchain_probe_file_version(const char* path) {
  if (!path || !path[0])
    return NULL;

  const char* base = toolchain_path_basename(path);
  if (!base || !strstr(base, ".exe"))
    return NULL;

  char cmd[2048] = {0};
  if (!platform_build_file_version_command(cmd, sizeof(cmd), path))
    return NULL;
  return toolchain_run_extract_version(cmd);
}

static void toolchain_set_tool(tool* out, const char* id, const char* path, const char* version) {
  if (!out)
    return;

  out->id = id ? arena_text(id, strlen(id)) : NULL;
  out->path = path ? toolchain_norm_path(path) : NULL;
  out->version = version;
}

static bool toolchain_should_replace_tool(const tool* curr, const char* cand_ver, const char* cand_path) {
  if (!curr)
    return true;

  int ver_cmp = toolchain_version_cmp(cand_ver, curr->version);
  if (ver_cmp != 0)
    return ver_cmp > 0;

  if ((!curr->version || !curr->version[0]) && cand_ver && cand_ver[0])
    return true;

  const char* curr_path = curr->path ? curr->path : "";
  const char* next_path = cand_path ? cand_path : "";
  return _stricmp(next_path, curr_path) < 0;
}

static void toolchain_upsert_tool(toolchain* tc, const char* id, const char* path, const char* version) {
  if (!tc || !id || !id[0] || !path || !path[0])
    return;

  toolchain_env* env = toolchain_host_env(tc, true);
  if (!env)
    return;

  int idx = toolchain_find_tool_by_id(tc, id);
  if (idx >= 0) {
    if (toolchain_should_replace_tool(&env->tools[idx], version, path))
      toolchain_set_tool(&env->tools[idx], id, path, version);
    return;
  }

  if (!toolchain_env_ensure_tool_capacity(env, env->tool_c + 1))
    return;

  toolchain_set_tool(&env->tools[env->tool_c++], id, path, version);
}

static const char* toolchain_trim_line(char* text) {
  while (*text && isspace((unsigned char)*text))
    ++text;

  size_t len = strlen(text);
  while (len > 0 && isspace((unsigned char)text[len - 1]))
    text[--len] = '\0';

  return text;
}

static const char* toolchain_norm_path(const char* in) {
  if (!in)
    return NULL;

  size_t len = strlen(in);
  char* out = push(len + 1);
  if (!out)
    return NULL;

  for (size_t i = 0; i < len; ++i) {
    char c = in[i];
    out[i] = (c == '\\') ? '/' : c;
  }
  out[len] = '\0';
  return out;
}

static bool toolchain_is_ver_char(char c) {
  return (c >= '0' && c <= '9') || c == '.';
}

static bool toolchain_is_abs_path(const char* p) {
  if (!p || !p[0])
    return false;
  if ((p[0] >= 'A' && p[0] <= 'Z' && p[1] == ':') || (p[0] >= 'a' && p[0] <= 'z' && p[1] == ':'))
    return true;
  if (p[0] == '/' || p[0] == '\\')
    return true;
  return false;
}

static const char* toolchain_extract_version(const char* text) {
  if (!text)
    return NULL;

  const char* p = text;
  while (*p) {
    if (*p >= '0' && *p <= '9') {
      const char* begin = p;
      int dot_count = 0;
      bool last_is_digit = false;
      while (toolchain_is_ver_char(*p)) {
        if (*p == '.')
          ++dot_count;
        last_is_digit = (*p >= '0' && *p <= '9');
        ++p;
      }

      if (dot_count >= 1 && last_is_digit)
        return arena_text(begin, (size_t)(p - begin));
    } else {
      ++p;
    }
  }

  return NULL;
}

static const char* toolchain_extract_integer_token(const char* text) {
  if (!text)
    return NULL;

  const char* p = text;
  while (*p) {
    if (*p >= '0' && *p <= '9') {
      const char* begin = p;
      while (*p >= '0' && *p <= '9')
        ++p;
      return arena_text(begin, (size_t)(p - begin));
    }
    ++p;
  }

  return NULL;
}

static const char* toolchain_extract_version_by_pattern(const char* text, const char* pattern) {
  if (!text || !pattern || !pattern[0])
    return toolchain_extract_version(text);

  const char* lp = strchr(pattern, '(');
  if (!lp)
    return toolchain_extract_version(text);

  size_t prefix_len = (size_t)(lp - pattern);
  const char* scan = text;
  if (prefix_len > 0) {
    const char* pos = strstr(text, pattern);
    if (!pos) {
      char prefix[256] = {0};
      if (prefix_len >= sizeof(prefix))
        prefix_len = sizeof(prefix) - 1;
      memcpy(prefix, pattern, prefix_len);
      pos = strstr(text, prefix);
      if (!pos)
        return NULL;
      scan = pos + prefix_len;
    } else {
      scan = pos + prefix_len;
    }
  }

  const char* version = toolchain_extract_version(scan);
  return version ? version : toolchain_extract_integer_token(scan);
}

static const char* toolchain_expand_token(const char* token) {
  if (!token || !token[0])
    return NULL;

  if (_stricmp(token, "{home}") == 0)
    return getenv("HOME") ? getenv("HOME") : getenv("USERPROFILE");
  if (_stricmp(token, "{user_profile}") == 0)
    return getenv("USERPROFILE");
  if (_stricmp(token, "{program_files}") == 0)
    return getenv("ProgramFiles");
  if (_stricmp(token, "{program_files_x86}") == 0)
    return getenv("ProgramFiles(x86)");
  if (_stricmp(token, "{local_app_data}") == 0)
    return getenv("LOCALAPPDATA");
  if (_stricmp(token, "{llvm_root}") == 0)
    return getenv("LLVM_ROOT");
  if (_stricmp(token, "{msys2_root}") == 0)
    return getenv("MSYS2_ROOT");
  if (_stricmp(token, "{xcode_dev_root}") == 0)
    return getenv("DEVELOPER_DIR");
  return NULL;
}

static const char* toolchain_expand_tokens(const char* text) {
  if (!text)
    return NULL;

  size_t in_len = strlen(text);
  size_t out_cap = in_len * 4 + 64;
  char* out = push(out_cap);
  if (!out)
    return NULL;

  size_t wi = 0;
  for (size_t i = 0; i < in_len && wi + 1 < out_cap; ++i) {
    if (text[i] == '{') {
      const char* end = strchr(text + i, '}');
      if (end) {
        size_t tlen = (size_t)(end - (text + i) + 1);
        char token[128] = {0};
        if (tlen < sizeof(token)) {
          memcpy(token, text + i, tlen);
          token[tlen] = '\0';
          const char* repl = toolchain_expand_token(token);
          if (repl && repl[0]) {
            size_t rlen = strlen(repl);
            if (wi + rlen + 1 < out_cap) {
              memcpy(out + wi, repl, rlen);
              wi += rlen;
              i += tlen - 1;
              continue;
            }
          }
        }
      }
    }

    out[wi++] = text[i];
  }

  out[wi] = '\0';
  return out;
}

static bool toolchain_run_first_line(const char* cmd, char* out, size_t out_dim) {
  if (!cmd || !out || out_dim == 0)
    return false;

  FILE* pipe = platform_popen_read(cmd);
  if (!pipe)
    return false;

  out[0] = '\0';
  bool ok = fgets(out, (int)out_dim, pipe) != NULL;
  platform_pclose_read(pipe);
  return ok;
}

static int toolchain_run_collect_lines(const char* cmd, const char** lines, int max_lines) {
  if (!cmd || !cmd[0] || !lines || max_lines <= 0)
    return 0;

  FILE* pipe = platform_popen_read(cmd);
  if (!pipe)
    return 0;

  int count = 0;
  char line[1024] = {0};
  while (count < max_lines && fgets(line, (int)sizeof(line), pipe) != NULL) {
    const char* trimmed = toolchain_trim_line(line);
    if (trimmed[0])
      lines[count++] = arena_text(trimmed, strlen(trimmed));
  }

  platform_pclose_read(pipe);
  return count;
}

static const char* toolchain_run_extract_version(const char* cmd) {
  if (!cmd || !cmd[0])
    return NULL;

  FILE* pipe = platform_popen_read(cmd);
  if (!pipe)
    return NULL;

  char line[1024] = {0};
  const char* found = NULL;
  int guard = 0;
  while (fgets(line, (int)sizeof(line), pipe) != NULL) {
    const char* v = toolchain_extract_version(line);
    if (v) {
      found = v;
      break;
    }
    if (++guard >= 16)
      break;
  }

  platform_pclose_read(pipe);
  return found;
}

static const char* toolchain_run_extract_version_pat(const char* cmd, const char* pattern) {
  if (!cmd || !cmd[0])
    return NULL;

  FILE* pipe = platform_popen_read(cmd);
  if (!pipe)
    return NULL;

  char line[1024] = {0};
  const char* found = NULL;
  int guard = 0;
  while (fgets(line, (int)sizeof(line), pipe) != NULL) {
    const char* v = toolchain_extract_version_by_pattern(line, pattern);
    if (v) {
      found = v;
      break;
    }
    if (++guard >= 24)
      break;
  }

  platform_pclose_read(pipe);
  return found;
}

static const char* toolchain_find_with_system(const char* name) {
  if (!name || !name[0])
    return NULL;

  char cmd[512] = {0};
  if (!platform_build_find_command(cmd, sizeof(cmd), name, false))
    return NULL;

  char line[_MAX_PATH * 2] = {0};
  if (!toolchain_run_first_line(cmd, line, sizeof(line)))
    return NULL;

  const char* trimmed = toolchain_trim_line(line);
  if (!trimmed[0])
    return NULL;
  return arena_text(trimmed, strlen(trimmed));
}

static int toolchain_collect_with_system(const char* name, const char** matches, int max_matches) {
  if (!name || !name[0] || !matches || max_matches <= 0)
    return 0;

  char cmd[512] = {0};
  if (!platform_build_find_command(cmd, sizeof(cmd), name, true))
    return 0;

  const char* lines[64] = {0};
  int count = toolchain_run_collect_lines(cmd, lines, _countof(lines));
  int out = 0;
  for (int i = 0; i < count && out < max_matches; ++i) {
    if (file_exists(lines[i]) && toolchain_path_name_matches(lines[i], name))
      toolchain_push_unique_path(matches, &out, max_matches, lines[i]);
  }

  return out;
}
static int toolchain_run_bash(toolchain* tc, const char* workdir, const char* script) {
  if (!tc) {
    error("Cannot run shell command without an initialized toolchain.");
    return -1;
  }
  if (!script || !script[0]) {
    error("Cannot run an empty bash command.");
    return -1;
  }

  const char* bash_path = toolchain_get_bash_path(tc);
  if (!bash_path || !bash_path[0]) {
    error("Unable to find 'bash' in the current toolchain. Install Git Bash, MSYS2, or another Bash-compatible shell and regenerate the toolchain if needed.");
    return -1;
  }

  return platform_run_bash(bash_path, workdir, script);
}

static const char* toolchain_join2(const char* a, const char* b) {
  if (!a || !a[0])
    return b ? arena_text(b, strlen(b)) : NULL;
  if (!b || !b[0])
    return arena_text(a, strlen(a));

  size_t al = strlen(a);
  size_t bl = strlen(b);
  bool need_sep = a[al - 1] != '/' && a[al - 1] != '\\';
  char* out = push(al + bl + (need_sep ? 2 : 1));
  if (!out)
    return NULL;
  strcpy(out, a);
  if (need_sep)
    strcat(out, "/");
  strcat(out, b);
  return out;
}

static const char* toolchain_append_text(const char* text, const char* suffix) {
  if (!text || !text[0])
    return suffix ? arena_text(suffix, strlen(suffix)) : NULL;
  if (!suffix || !suffix[0])
    return arena_text(text, strlen(text));

  size_t tl = strlen(text);
  size_t sl = strlen(suffix);
  char* out = push(tl + sl + 1);
  if (!out)
    return NULL;

  memcpy(out, text, tl);
  memcpy(out + tl, suffix, sl);
  out[tl + sl] = '\0';
  return out;
}

static const char* toolchain_label2(const char* a, const char* b) {
  return toolchain_append_text(a, b);
}

static const char* toolchain_find_in_hint_dirs(const char* exe_name, const char* hints) {
  if (!exe_name || !exe_name[0] || !hints || !hints[0])
    return NULL;

  const char* expanded = toolchain_expand_tokens(hints);
  if (!expanded)
    return NULL;

  size_t len = strlen(expanded);
  char* copy = push(len + 1);
  if (!copy)
    return NULL;
  memcpy(copy, expanded, len + 1);

  char* next = copy;
  while (next && *next) {
    char* semi = strchr(next, ';');
    if (semi)
      *semi = '\0';

    const char* dir = toolchain_trim_line(next);
    if (dir[0]) {
      const char* candidates[2] = {0};
      char storage[2][128] = {{0}};
      int candidate_c = platform_executable_candidates(exe_name, candidates, storage, _countof(storage));
      for (int i = 0; i < candidate_c; ++i) {
        const char* full = toolchain_join2(dir, candidates[i]);
        if (file_exists(full))
          return arena_text(full, strlen(full));
      }
    }

    if (!semi)
      break;
    next = semi + 1;
  }

  return NULL;
}

static int toolchain_collect_in_hint_dirs(const char* exe_name, const char* hints, const char** matches, int max_matches) {
  if (!exe_name || !exe_name[0] || !hints || !hints[0] || !matches || max_matches <= 0)
    return 0;

  const char* expanded = toolchain_expand_tokens(hints);
  if (!expanded)
    return 0;

  size_t len = strlen(expanded);
  char* copy = push(len + 1);
  if (!copy)
    return 0;
  memcpy(copy, expanded, len + 1);

  int out = 0;
  char* next = copy;
  while (next && *next) {
    char* semi = strchr(next, ';');
    if (semi)
      *semi = '\0';

    const char* dir = toolchain_trim_line(next);
    if (dir[0]) {
      const char* candidates[2] = {0};
      char storage[2][128] = {{0}};
      int candidate_c = platform_executable_candidates(exe_name, candidates, storage, _countof(storage));
      for (int i = 0; i < candidate_c; ++i) {
        const char* full = toolchain_join2(dir, candidates[i]);
        if (full && file_exists(full))
          toolchain_push_unique_path(matches, &out, max_matches, full);
      }
    }

    if (!semi || out >= max_matches)
      break;
    next = semi + 1;
  }

  return out;
}

static const char* toolchain_find_with_vswhere_vcvarsall(void) {
  char vswhere[_MAX_PATH] = {0};
  if (!platform_find_vswhere(vswhere, sizeof(vswhere)))
    return NULL;
  if (!file_exists(vswhere))
    return NULL;

  char cmd[2048] = {0};
  if (!platform_build_vswhere_install_command(cmd, sizeof(cmd), vswhere))
    return NULL;

  char line[1024] = {0};
  if (!toolchain_run_first_line(cmd, line, sizeof(line)))
    return NULL;
  const char* install = toolchain_trim_line(line);
  if (!install[0])
    return NULL;

  const char* vcvars = toolchain_join2(install, "VC/Auxiliary/Build/vcvarsall.bat");
  if (vcvars && file_exists(vcvars))
    return toolchain_norm_path(vcvars);
  return NULL;
}

static const char* toolchain_find_msvc_root_with_vswhere(void) {
  char vswhere[_MAX_PATH] = {0};
  if (!platform_find_vswhere(vswhere, sizeof(vswhere)))
    return NULL;
  if (!file_exists(vswhere))
    return NULL;

  char cmd[2048] = {0};
  if (!platform_build_vswhere_install_command(cmd, sizeof(cmd), vswhere))
    return NULL;

  char line[1024] = {0};
  if (!toolchain_run_first_line(cmd, line, sizeof(line)))
    return NULL;
  const char* install = toolchain_trim_line(line);
  if (!install[0])
    return NULL;

  const char* tools = toolchain_join2(install, "VC/Tools/MSVC");
  if (!tools || !dir_exists(tools))
    return NULL;

  char list_cmd[2048] = {0};
  if (!platform_build_dir_list_command(list_cmd, sizeof(list_cmd), tools))
    return NULL;
  if (!toolchain_run_first_line(list_cmd, line, sizeof(line)))
    return NULL;

  const char* ver_dir = toolchain_trim_line(line);
  if (!ver_dir[0])
    return NULL;

  return toolchain_norm_path(toolchain_join2(tools, ver_dir));
}

static int toolchain_collect_deep_tool_paths(const char* exe_name, const char* roots, const char** matches, int max_matches) {
  if (!exe_name || !exe_name[0] || !roots || !roots[0] || !matches || max_matches <= 0)
    return 0;

  const char* expanded = toolchain_expand_tokens(roots);
  if (!expanded)
    return 0;

  char* copy = push(strlen(expanded) + 1);
  if (!copy)
    return 0;
  strcpy(copy, expanded);

  int out = 0;
  char* next = copy;
  while (next && *next) {
    char* semi = strchr(next, ';');
    if (semi)
      *semi = '\0';

    const char* root = toolchain_trim_line(next);
    if (root[0] && dir_exists(root)) {
      char cmd[4096] = {0};
      const char* lines[64] = {0};
      if (!platform_build_recursive_find_command(cmd, sizeof(cmd), root, exe_name))
        break;

      int line_c = toolchain_run_collect_lines(cmd, lines, _countof(lines));
      for (int i = 0; i < line_c && out < max_matches; ++i) {
        if (file_exists(lines[i]) && toolchain_path_name_matches(lines[i], exe_name))
          toolchain_push_unique_path(matches, &out, max_matches, lines[i]);
      }
    }

    if (!semi || out >= max_matches)
      break;
    next = semi + 1;
  }

  return out;
}

static const char* toolchain_probe_version(const char* exe_path, const char* arg_a, const char* arg_b, const char* pat_a, const char* pat_b) {
  if (!exe_path || !exe_path[0])
    return NULL;

  char cmd[1024] = {0};
  if (arg_a && arg_a[0]) {
    snprintf(cmd, sizeof(cmd), "\"%s\" %s 2>&1", exe_path, arg_a);
    const char* v = pat_a && pat_a[0] ? toolchain_run_extract_version_pat(cmd, pat_a) : toolchain_run_extract_version(cmd);
    if (v)
      return v;
  }

  if (arg_b && arg_b[0]) {
    snprintf(cmd, sizeof(cmd), "\"%s\" %s 2>&1", exe_path, arg_b);
    const char* v = pat_b && pat_b[0] ? toolchain_run_extract_version_pat(cmd, pat_b) : toolchain_run_extract_version(cmd);
    if (v)
      return v;
  }

  {
    const char* v = toolchain_probe_file_version(exe_path);
    if (v)
      return v;
  }

  return toolchain_path_version_fallback(exe_path);
}

static bool toolchain_host_matches_os(os target_os) {
  if (target_os == OS_MAX)
    return true;

  return _stricmp(platform_host_os_name(), OS_NAMES[target_os]) == 0;
}

static bool toolchain_tool_exists(toolchain* tc, const char* path) {
  if (!tc || !path)
    return false;
  toolchain_env* env = toolchain_host_env(tc, false);
  if (!env)
    return false;
  for (int i = 0; i < env->tool_c; ++i) {
    if (env->tools[i].path && _stricmp(env->tools[i].path, path) == 0)
      return true;
  }
  return false;
}

static void toolchain_discover_tool(toolchain* tc, const tool_discover_strat* s) {
  toolchain_env* env = toolchain_host_env(tc, true);
  if (!tc || !s || !env)
    return;

  if (!toolchain_host_matches_os(s->target_os))
    return;

  if (_stricmp(s->id, "vcvarsall") == 0) {
    const char* vcvars = toolchain_find_with_vswhere_vcvarsall();
    if (vcvars) {
      const char* version = toolchain_probe_version(vcvars, s->version_arg, s->version_arg_fallback, s->version_regex, s->version_regex_fallback);
      if (!version)
        version = toolchain_path_version_fallback(vcvars);
      toolchain_upsert_tool(tc, s->id, vcvars, version);

      if (version && version[0])
        print("  [tool] %-12s found at %s (v%s)", s->id, vcvars, version);
      else
        print("  [tool] %-12s found at %s (version unknown)", s->id, vcvars);
      return;
    }
  }

  const char* matches[64] = {0};
  int match_c = 0;

  {
    const char* found[64] = {0};
    int found_c = toolchain_collect_with_system(s->exe_name, found, _countof(found));
    for (int i = 0; i < found_c; ++i)
      toolchain_push_unique_path(matches, &match_c, _countof(matches), found[i]);
  }
  if (match_c < _countof(matches)) {
    const char* found[64] = {0};
    int found_c = toolchain_collect_in_hint_dirs(s->exe_name, s->dir_hints, found, _countof(found));
    for (int i = 0; i < found_c; ++i)
      toolchain_push_unique_path(matches, &match_c, _countof(matches), found[i]);
  }
  if (match_c < _countof(matches)) {
    const char* found[64] = {0};
    int found_c = toolchain_collect_deep_tool_paths(s->exe_name, s->deep_roots, found, _countof(found));
    for (int i = 0; i < found_c; ++i)
      toolchain_push_unique_path(matches, &match_c, _countof(matches), found[i]);
  }

  for (int i = 0; i < match_c; ++i) {
    const char* path = toolchain_norm_path(matches[i]);
    if (_stricmp(s->id, "bash") == 0 && !toolchain_is_usable_host_bash_path(path))
      continue;
    if (toolchain_tool_exists(tc, path))
      continue;

    const char* version = toolchain_probe_version(path, s->version_arg, s->version_arg_fallback, s->version_regex, s->version_regex_fallback);
    if (!version)
      version = toolchain_path_version_fallback(path);
    toolchain_upsert_tool(tc, s->id, path, version);

    if (version && version[0])
      print("  [tool] %-12s found at %s (v%s)", s->id, path, version);
    else
      print("  [tool] %-12s found at %s (version unknown)", s->id, path);
  }
}

static const char* toolchain_first_existing_env_path(const char* env_vars) {
  if (!env_vars || !env_vars[0])
    return NULL;

  char* copy = push(strlen(env_vars) + 1);
  if (!copy)
    return NULL;
  strcpy(copy, env_vars);

  char* next = copy;
  while (next && *next) {
    char* semi = strchr(next, ';');
    if (semi)
      *semi = '\0';

    const char* name = toolchain_trim_line(next);
    if (name[0]) {
      const char* v = getenv(name);
      if (v && v[0] && (dir_exists(v) || file_exists(v)))
        return arena_text(v, strlen(v));
    }

    if (!semi)
      break;
    next = semi + 1;
  }

  return NULL;
}

static const char* toolchain_first_existing_hint_path(const char* hints) {
  if (!hints || !hints[0])
    return NULL;
  const char* expanded = toolchain_expand_tokens(hints);
  if (!expanded)
    return NULL;

  char* copy = push(strlen(expanded) + 1);
  if (!copy)
    return NULL;
  strcpy(copy, expanded);

  char* next = copy;
  while (next && *next) {
    char* semi = strchr(next, ';');
    if (semi)
      *semi = '\0';

    const char* p = toolchain_trim_line(next);
    if (p[0]) {
      if (strchr(p, '*')) {
        const char* m = toolchain_first_path_match(p);
        if (m)
          return m;
      } else if (dir_exists(p)) {
        return arena_text(p, strlen(p));
      }
    }

    if (!semi)
      break;
    next = semi + 1;
  }

  return NULL;
}

static int toolchain_collect_existing_env_paths(const char* env_vars, const char** matches, int max_matches) {
  if (!env_vars || !env_vars[0] || !matches || max_matches <= 0)
    return 0;

  char* copy = push(strlen(env_vars) + 1);
  if (!copy)
    return 0;
  strcpy(copy, env_vars);

  int out = 0;
  char* next = copy;
  while (next && *next) {
    char* semi = strchr(next, ';');
    if (semi)
      *semi = '\0';

    const char* name = toolchain_trim_line(next);
    if (name[0]) {
      const char* v = getenv(name);
      if (v && v[0] && (dir_exists(v) || file_exists(v)))
        toolchain_push_unique_path(matches, &out, max_matches, v);
    }

    if (!semi || out >= max_matches)
      break;
    next = semi + 1;
  }

  return out;
}

static int toolchain_collect_existing_hint_paths(const char* hints, const char** matches, int max_matches) {
  if (!hints || !hints[0] || !matches || max_matches <= 0)
    return 0;

  const char* expanded = toolchain_expand_tokens(hints);
  if (!expanded)
    return 0;

  char* copy = push(strlen(expanded) + 1);
  if (!copy)
    return 0;
  strcpy(copy, expanded);

  int out = 0;
  char* next = copy;
  while (next && *next) {
    char* semi = strchr(next, ';');
    if (semi)
      *semi = '\0';

    const char* p = toolchain_trim_line(next);
    if (p[0]) {
      if (strchr(p, '*')) {
        const char* found[64] = {0};
        int found_c = toolchain_collect_path_matches(p, found, _countof(found), true);
        for (int i = 0; i < found_c && out < max_matches; ++i)
          toolchain_push_unique_path(matches, &out, max_matches, found[i]);
      } else if (dir_exists(p) || file_exists(p)) {
        toolchain_push_unique_path(matches, &out, max_matches, p);
      }
    }

    if (!semi || out >= max_matches)
      break;
    next = semi + 1;
  }

  return out;
}

static const char* toolchain_first_path_match(const char* pattern) {
  const char* matches[1] = {0};
  int count = toolchain_collect_path_matches(pattern, matches, 1, true);
  return count > 0 ? matches[0] : NULL;
}

static int toolchain_collect_path_matches(const char* pattern, const char** matches, int max_matches, bool dirs_only) {
  if (!pattern || !pattern[0])
    return 0;

  char cmd[2048] = {0};
  if (!platform_build_pattern_match_command(cmd, sizeof(cmd), pattern, dirs_only))
    return 0;

  const char* lines[64] = {0};
  int line_c = toolchain_run_collect_lines(cmd, lines, _countof(lines));
  int out = 0;
  for (int i = 0; i < line_c && out < max_matches; ++i) {
    const char* p = lines[i];
    if (!p || !p[0])
      continue;
    if (dirs_only) {
      if (dir_exists(p))
        toolchain_push_unique_path(matches, &out, max_matches, p);
    } else if (file_exists(p) || dir_exists(p)) {
      toolchain_push_unique_path(matches, &out, max_matches, p);
    }
  }

  return out;
}

static const char* toolchain_extract_version_from_file(const char* path, const char* pattern) {
  if (!path || !path[0] || !file_exists(path))
    return NULL;

  const char* data = read_entire_file(path);
  if (!data || !data[0])
    return NULL;

  return pattern && pattern[0] ? toolchain_extract_version_by_pattern(data, pattern) : toolchain_extract_version(data);
}

static const char* toolchain_probe_sdk_version(const char* base, const char* rels, const char* pattern) {
  if (!base || !base[0] || !rels || !rels[0])
    return NULL;

  const char* base_ver = toolchain_extract_version(base);

  char* copy = push(strlen(rels) + 1);
  if (!copy)
    return NULL;
  strcpy(copy, rels);

  const char* best = NULL;
  char* next = copy;
  while (next && *next) {
    char* semi = strchr(next, ';');
    if (semi)
      *semi = '\0';

    const char* rel = toolchain_trim_line(next);
    if (rel[0]) {
      const char* candidate = toolchain_is_abs_path(rel) ? rel : toolchain_join2(base, rel);
      if (candidate) {
        if (strchr(candidate, '*')) {
          const char* files[64] = {0};
          int file_c = toolchain_collect_path_matches(candidate, files, _countof(files), false);
          for (int i = 0; i < file_c; ++i) {
            const char* ver = toolchain_extract_version_from_file(files[i], pattern);
            const char* path_ver = toolchain_extract_version(files[i]);
            if (toolchain_version_cmp(path_ver, ver) > 0)
              ver = path_ver;
            if (toolchain_version_has_dot(base_ver) && !toolchain_version_has_dot(ver))
              continue;
            if (toolchain_version_cmp(ver, best) > 0)
              best = ver;
          }
        } else {
          const char* ver = toolchain_extract_version_from_file(candidate, pattern);
          const char* path_ver = toolchain_extract_version(candidate);
          if (toolchain_version_cmp(path_ver, ver) > 0)
            ver = path_ver;
          if (toolchain_version_has_dot(base_ver) && !toolchain_version_has_dot(ver))
            ver = NULL;
          if (toolchain_version_cmp(ver, best) > 0)
            best = ver;
        }
      }
    }

    if (!semi)
      break;
    next = semi + 1;
  }

  {
    if (toolchain_version_cmp(base_ver, best) > 0)
      best = base_ver;
  }

  {
    const char* include_root = toolchain_join2(base, "Include/*");
    const char* dirs[32] = {0};
    int dir_c = toolchain_collect_path_matches(include_root, dirs, _countof(dirs), true);
    for (int i = 0; i < dir_c; ++i) {
      const char* dir_ver = toolchain_extract_version(dirs[i]);
      if (toolchain_version_cmp(dir_ver, best) > 0)
        best = dir_ver;
    }
  }

  return best;
}

static void toolchain_discover_sdk(toolchain* tc, const sdk_discover_strat* s) {
  toolchain_env* env = toolchain_host_env(tc, true);
  if (!tc || !s || !env)
    return;

  if (!toolchain_host_matches_os(s->target_os))
    return;

  const char* bases[64] = {0};
  int base_c = 0;

  {
    const char* found[64] = {0};
    int found_c = toolchain_collect_existing_env_paths(s->env_vars, found, _countof(found));
    for (int i = 0; i < found_c; ++i)
      toolchain_push_unique_path(bases, &base_c, _countof(bases), found[i]);
  }
  if (_stricmp(s->id, "msvc") == 0) {
    const char* msvc = toolchain_find_msvc_root_with_vswhere();
    if (msvc)
      toolchain_push_unique_path(bases, &base_c, _countof(bases), msvc);
  }
  if (base_c < _countof(bases)) {
    const char* found[64] = {0};
    int found_c = toolchain_collect_existing_hint_paths(s->root_hints, found, _countof(found));
    for (int i = 0; i < found_c; ++i)
      toolchain_push_unique_path(bases, &base_c, _countof(bases), found[i]);
  }

  const char* best_base = NULL;
  const char* best_ver = NULL;
  for (int i = 0; i < base_c; ++i) {
    const char* ver = toolchain_probe_sdk_version(bases[i], s->version_file_rel, s->version_regex);
    if (!best_base || toolchain_version_cmp(ver, best_ver) > 0) {
      best_base = bases[i];
      best_ver = ver;
    }
  }

  if (!best_base)
    return;

  if (!toolchain_env_ensure_sdk_capacity(env, env->sdk_c + 1))
    return;

  sdk* out = &env->sdks[env->sdk_c++];
  out->name = arena_text(s->id, strlen(s->id));
  out->base_path = toolchain_norm_path(best_base);
  out->inc_path = s->include_rel && s->include_rel[0] ? toolchain_norm_path(toolchain_is_abs_path(s->include_rel) ? s->include_rel : toolchain_join2(best_base, s->include_rel)) : NULL;
  out->src_path = s->source_rel && s->source_rel[0] ? toolchain_norm_path(toolchain_is_abs_path(s->source_rel) ? s->source_rel : toolchain_join2(best_base, s->source_rel)) : NULL;
  out->lib_path = s->lib_rel && s->lib_rel[0] ? toolchain_norm_path(toolchain_is_abs_path(s->lib_rel) ? s->lib_rel : toolchain_join2(best_base, s->lib_rel)) : NULL;
  out->bin_path = s->bin_rel && s->bin_rel[0] ? toolchain_norm_path(toolchain_is_abs_path(s->bin_rel) ? s->bin_rel : toolchain_join2(best_base, s->bin_rel)) : NULL;
  out->version = best_ver;

  if (out->version && out->version[0])
    print("  [sdk ] %-12s root %s (v%s)", s->id, out->base_path ? out->base_path : "<none>", out->version);
  else
    print("  [sdk ] %-12s root %s (version unknown)", s->id, out->base_path ? out->base_path : "<none>");
}

static void toolchain_push_child_back(node* parent, node* child) {
  if (!parent || !child)
    return;
  if (!parent->children) {
    parent->children = child;
    return;
  }
  node* last = parent->children;
  while (last->next)
    last = last->next;
  last->next = child;
}

static bool toolchain_tool_matches_filter(const tool* t, const toolchain_print_opts* opts) {
  if (!t)
    return false;
  if (!opts)
    return true;
  if (opts->tool_id_filter && opts->tool_id_filter[0]) {
    const char* id = t->id ? t->id : "";
    if (_stricmp(id, opts->tool_id_filter) != 0)
      return false;
  }
  return true;
}

static bool toolchain_sdk_matches_filter(const sdk* s, const toolchain_print_opts* opts) {
  if (!s)
    return false;
  if (!opts)
    return true;
  if (opts->sdk_name_filter && opts->sdk_name_filter[0]) {
    const char* name = s->name ? s->name : "";
    if (_stricmp(name, opts->sdk_name_filter) != 0)
      return false;
  }
  return true;
}

static void toolchain_print_env_support(toolchain_env* env, bool minimal) {
  if (!env)
    return;

  if (minimal) {
    for (int osi = 0; osi < OS_MAX; ++osi) {
      for (int ai = 0; ai < ARCH_MAX; ++ai)
        print("support|%s|%s|%s|%s|%s", env->id ? env->id : "", OS_NAMES[osi], ARCH_NAMES[ai], env->supported[osi][ai] ? "true" : "false", env->support_source[osi][ai] ? env->support_source[osi][ai] : "");
    }
    return;
  }

  print("    Supported Platforms:");
  print("      %-10s %-7s %-7s %-7s", "os", ARCH_NAMES[ARCH_X86_64], ARCH_NAMES[ARCH_X86], ARCH_NAMES[ARCH_ARM64]);
  for (int osi = 0; osi < OS_MAX; ++osi) {
    print("      %-10s %-7s %-7s %-7s",
          OS_NAMES[osi],
          env->supported[osi][ARCH_X86_64] ? "yes" : "no",
          env->supported[osi][ARCH_X86] ? "yes" : "no",
          env->supported[osi][ARCH_ARM64] ? "yes" : "no");
  }
  print("    Support Sources:");
  for (int osi = 0; osi < OS_MAX; ++osi) {
    for (int ai = 0; ai < ARCH_MAX; ++ai) {
      if (!env->supported[osi][ai])
        continue;
      print("      - %s/%s: %s", OS_NAMES[osi], ARCH_NAMES[ai], env->support_source[osi][ai] ? env->support_source[osi][ai] : "unknown");
    }
  }
}

static void toolchain_print_env_with_opts(toolchain_env* env, const toolchain_print_opts* opts) {
  if (!env || !opts)
    return;

  if (opts->minimal)
    print("env|%s|%s|%s|%s|%s", env->id ? env->id : "", env->provider ? env->provider : "", env->name ? env->name : "", OS_NAMES[env->p_os], ARCH_NAMES[env->p_arch]);
  else
    print("  - %s | %s | %s/%s", env->provider ? env->provider : "env", env->name ? env->name : "default", OS_NAMES[env->p_os], ARCH_NAMES[env->p_arch]);

  if (opts->show_tools) {
    if (!opts->minimal && !opts->paths_only && !opts->versions_only)
      print("    Tools (%d):", env->tool_c);
    for (int i = 0; i < env->tool_c; ++i) {
      tool* t = &env->tools[i];
      if (!toolchain_tool_matches_filter(t, opts))
        continue;
      const char* id = t->id ? t->id : toolchain_path_basename(t->path ? t->path : "");
      const char* pa = t->path ? t->path : "";
      const char* ve = t->version ? t->version : "unknown";
      if (opts->paths_only)
        print("%s", pa);
      else if (opts->versions_only)
        print("%s", ve);
      else if (opts->minimal)
        print("tool|%s|%s|%s|%s", env->id ? env->id : "", id ? id : "", pa, ve);
      else
        print("      - %s | %s | %s", id ? id : "", pa, ve);
    }
  }

  if (opts->show_sdks) {
    if (!opts->minimal && !opts->paths_only && !opts->versions_only)
      print("    SDKs (%d):", env->sdk_c);
    for (int i = 0; i < env->sdk_c; ++i) {
      sdk* s = &env->sdks[i];
      if (!toolchain_sdk_matches_filter(s, opts))
        continue;
      if (opts->paths_only)
        print("%s", s->base_path ? s->base_path : "");
      else if (opts->versions_only)
        print("%s", s->version ? s->version : "unknown");
      else if (opts->minimal)
        print("sdk|%s|%s|%s|%s", env->id ? env->id : "", s->name ? s->name : "", s->base_path ? s->base_path : "", s->version ? s->version : "unknown");
      else {
        print("      - %s", s->name ? s->name : "<unnamed>");
        if (s->version && s->version[0]) print("          version: %s", s->version);
        if (s->base_path) print("          base: %s", s->base_path);
        if (s->inc_path) print("          inc : %s", s->inc_path);
        if (s->src_path) print("          src : %s", s->src_path);
        if (s->lib_path) print("          lib : %s", s->lib_path);
        if (s->bin_path) print("          bin : %s", s->bin_path);
      }
    }
  }

  if (!opts->paths_only && !opts->versions_only)
    toolchain_print_env_support(env, opts->minimal);
}

static void toolchain_fill(toolchain* tc) {
  if (!tc)
    return;

  tc->p_arch = toolchain_detect_host_arch();
  tc->p_os = toolchain_detect_host_os();

  print("Discovering host tools...");

  for (size_t i = 0; i < sizeof(TOOL_DISCOVER_STRATS) / sizeof(TOOL_DISCOVER_STRATS[0]); ++i)
    toolchain_discover_tool(tc, &TOOL_DISCOVER_STRATS[i]);

  toolchain_sort_tools(tc);

  const char* cmake = toolchain_get_tool_path(tc, "cmake");
  if (!cmake || !cmake[0]) {
    error("Unable to find 'cmake' in the current toolchain. Install CMake and regenerate the toolchain.");
    print("If a tool is present on your system but was not detected, you can edit the toolchain file directly.");
    return;
  }

  const char* bash = toolchain_get_tool_path(tc, "bash");
  if (!bash || !bash[0])
    warn("Unable to find 'bash' in the current toolchain. Script execution will be unavailable until a Bash-compatible shell is installed.");
  if (!bash || !bash[0])
    print("If a tool is present on your system but was not detected, you can edit the toolchain file directly.");

  print("Discovering SDKs...");

  for (size_t i = 0; i < sizeof(SDK_DISCOVER_STRATS) / sizeof(SDK_DISCOVER_STRATS[0]); ++i)
    toolchain_discover_sdk(tc, &SDK_DISCOVER_STRATS[i]);

  toolchain_sort_sdks(tc);

  toolchain_snapshot_current_host_env(tc);
  toolchain_discover_extra_envs(tc);
  toolchain_capture_host_docker_probe(tc);
  toolchain_refresh_runtime_support(tc);

  toolchain_env* host = toolchain_host_env(tc, false);
  print("Discovery complete: %d tools, %d SDKs, %d environments.", host ? host->tool_c : 0, host ? host->sdk_c : 0, tc->env_c);
}

static void toolchain_print_with_opts(toolchain* tc, const toolchain_print_opts* opts) {
  if (!tc) {
    print("Toolchain is null.");
    return;
  }

  toolchain_print_opts defaults = {0};
  if (!opts) {
    defaults.show_tools = true;
    defaults.show_sdks = true;
    opts = &defaults;
  }

  if (!opts->minimal)
    print("Environments (%d):", tc->env_c);
  for (int i = 0; i < tc->env_c; ++i)
    toolchain_print_env_with_opts(&tc->envs[i], opts);

  if (!opts->paths_only && !opts->versions_only) {
    if (!opts->minimal)
      print("Aggregate Support:");
    toolchain_print_platform_support(tc, opts->minimal);
  }
}

static void toolchain_print(toolchain* tc) {
  toolchain_print_opts opts = {.show_tools = true, .show_sdks = true};
  toolchain_print_with_opts(tc, &opts);
}

static bool toolchain_validate_named_string(node* value_n, const char* scope_label, const char* attr_name, bool required) {
  if (!value_n)
    return !required;
  if (value_n->type != NODE_TYPE_STR) {
    error("Attribute '%s' in %s must be a string.", attr_name, scope_label);
    return false;
  }
  if (required && (!node_get_str(value_n) || !node_get_str(value_n)[0])) {
    error("Attribute '%s' in %s is required.", attr_name, scope_label);
    return false;
  }
  return true;
}

static bool toolchain_validate_host_section(node* host_n, const char* scope_label) {
  if (!host_n)
    return true;
  if (host_n->type != NODE_TYPE_DEF) {
    error("Attribute 'host' in %s must be a section.", scope_label);
    return false;
  }

  node_foreach(host_n, child) {
    if (!child || !child->name) {
      error("Attribute 'host' in %s must use named attributes.", scope_label);
      return false;
    }

    if (_stricmp(child->name, "arch") == 0) {
      const char* arch_id = node_get_idf(child);
      if (!arch_id) {
        error("Attribute 'arch' in %s must be an identifier.", scope_label);
        return false;
      }
      bool known = false;
      for (int i = 0; i < ARCH_MAX; ++i)
        if (_stricmp(ARCH_NAMES[i], arch_id) == 0)
          known = true;
      if (!known) {
        error("Unknown host arch '%s' in %s.", arch_id, scope_label);
        return false;
      }
      continue;
    }

    if (_stricmp(child->name, "os") == 0) {
      const char* os_id = node_get_idf(child);
      if (!os_id) {
        error("Attribute 'os' in %s must be an identifier.", scope_label);
        return false;
      }
      bool known = false;
      for (int i = 0; i < OS_MAX; ++i)
        if (_stricmp(OS_NAMES[i], os_id) == 0)
          known = true;
      if (!known) {
        error("Unknown host os '%s' in %s.", os_id, scope_label);
        return false;
      }
      continue;
    }

    error("Unknown host attribute '%s' in %s.", child->name, scope_label);
    return false;
  }

  return true;
}

static bool toolchain_validate_tool_item(node* tool_n, const char* scope_label) {
  if (!tool_n)
    return false;
  if (tool_n->type != NODE_TYPE_DEF) {
    error("Tool entries in %s must be sections.", scope_label);
    return false;
  }

  node_foreach(tool_n, child) {
    if (!child || !child->name) {
      error("Tool entries in %s must use named attributes.", scope_label);
      return false;
    }
    if (_stricmp(child->name, "id") != 0 && _stricmp(child->name, "path") != 0 && _stricmp(child->name, "version") != 0) {
      error("Unknown tool attribute '%s' in %s.", child->name, scope_label);
      return false;
    }
  }

  if (!toolchain_validate_named_string(node_get_child(tool_n, "id"), scope_label, "id", false))
    return false;
  if (!toolchain_validate_named_string(node_get_child(tool_n, "path"), scope_label, "path", true))
    return false;
  if (!toolchain_validate_named_string(node_get_child(tool_n, "version"), scope_label, "version", false))
    return false;
  return true;
}

static bool toolchain_validate_sdk_item(node* sdk_n, const char* scope_label) {
  if (!sdk_n)
    return false;
  if (sdk_n->type != NODE_TYPE_DEF) {
    error("SDK entries in %s must be sections.", scope_label);
    return false;
  }

  node_foreach(sdk_n, child) {
    if (!child || !child->name) {
      error("SDK entries in %s must use named attributes.", scope_label);
      return false;
    }
    if (_stricmp(child->name, "name") != 0 &&
        _stricmp(child->name, "version") != 0 &&
        _stricmp(child->name, "base_path") != 0 &&
        _stricmp(child->name, "inc_path") != 0 &&
        _stricmp(child->name, "src_path") != 0 &&
        _stricmp(child->name, "lib_path") != 0 &&
        _stricmp(child->name, "bin_path") != 0) {
      error("Unknown sdk attribute '%s' in %s.", child->name, scope_label);
      return false;
    }
  }

  if (!toolchain_validate_named_string(node_get_child(sdk_n, "name"), scope_label, "name", false))
    return false;
  if (!toolchain_validate_named_string(node_get_child(sdk_n, "version"), scope_label, "version", false))
    return false;
  if (!toolchain_validate_named_string(node_get_child(sdk_n, "base_path"), scope_label, "base_path", false))
    return false;
  if (!toolchain_validate_named_string(node_get_child(sdk_n, "inc_path"), scope_label, "inc_path", false))
    return false;
  if (!toolchain_validate_named_string(node_get_child(sdk_n, "src_path"), scope_label, "src_path", false))
    return false;
  if (!toolchain_validate_named_string(node_get_child(sdk_n, "lib_path"), scope_label, "lib_path", false))
    return false;
  if (!toolchain_validate_named_string(node_get_child(sdk_n, "bin_path"), scope_label, "bin_path", false))
    return false;
  return true;
}

static bool toolchain_validate_probes_section(node* probes_n, const char* scope_label) {
  if (!probes_n)
    return true;
  if (probes_n->type != NODE_TYPE_DEF) {
    error("Attribute 'probes' in %s must be a section.", scope_label);
    return false;
  }

  node_foreach(probes_n, child) {
    if (!child || !child->name) {
      error("Attribute 'probes' in %s must use named attributes.", scope_label);
      return false;
    }
    if (_stricmp(child->name, "docker_buildx_platforms") != 0) {
      error("Unknown probe attribute '%s' in %s.", child->name, scope_label);
      return false;
    }
  }

  return toolchain_validate_named_string(node_get_child(probes_n, "docker_buildx_platforms"), scope_label, "docker_buildx_platforms", false);
}

static bool toolchain_validate_env_item(node* env_n, const char* scope_label) {
  if (!env_n)
    return false;
  if (env_n->type != NODE_TYPE_DEF) {
    error("Environment entries in %s must be sections.", scope_label);
    return false;
  }

  node_foreach(env_n, child) {
    if (!child || !child->name) {
      error("Environment entries in %s must use named attributes.", scope_label);
      return false;
    }

    if (_stricmp(child->name, "id") == 0 ||
        _stricmp(child->name, "provider") == 0 ||
        _stricmp(child->name, "name") == 0) {
      continue;
    }
    if (_stricmp(child->name, "host") == 0 ||
        _stricmp(child->name, "probes") == 0 ||
        _stricmp(child->name, "tools") == 0 ||
        _stricmp(child->name, "sdks") == 0) {
      continue;
    }

    error("Unknown environment attribute '%s' in %s.", child->name, scope_label);
    return false;
  }

  if (!toolchain_validate_named_string(node_get_child(env_n, "id"), scope_label, "id", true))
    return false;
  if (!toolchain_validate_named_string(node_get_child(env_n, "provider"), scope_label, "provider", true))
    return false;
  if (!toolchain_validate_named_string(node_get_child(env_n, "name"), scope_label, "name", true))
    return false;
  if (!toolchain_validate_host_section(node_get_child(env_n, "host"), scope_label))
    return false;
  if (!toolchain_validate_probes_section(node_get_child(env_n, "probes"), scope_label))
    return false;

  node* tools_n = node_get_child(env_n, "tools");
  if (tools_n) {
    if (tools_n->type != NODE_TYPE_DEF) {
      error("Attribute 'tools' in %s must be a section.", scope_label);
      return false;
    }
    node_foreach(tools_n, child) {
      if (!child || !child->name) {
        error("tools(...) in %s must contain only named tool sections.", scope_label);
        return false;
      }
      if (_stricmp(child->name, "tool") != 0) {
        error("tools(...) in %s must contain only tool(...) entries.", scope_label);
        return false;
      }
      if (!toolchain_validate_tool_item(child, scope_label))
        return false;
    }
  }

  node* sdks_n = node_get_child(env_n, "sdks");
  if (sdks_n) {
    if (sdks_n->type != NODE_TYPE_DEF) {
      error("Attribute 'sdks' in %s must be a section.", scope_label);
      return false;
    }
    node_foreach(sdks_n, child) {
      if (!child || !child->name) {
        error("sdks(...) in %s must contain only named sdk sections.", scope_label);
        return false;
      }
      if (_stricmp(child->name, "sdk") != 0) {
        error("sdks(...) in %s must contain only sdk(...) entries.", scope_label);
        return false;
      }
      if (!toolchain_validate_sdk_item(child, scope_label))
        return false;
    }
  }

  return true;
}

static bool toolchain_validate_tree(node* tree) {
  if (!tree)
    return false;

  node_foreach(tree, child) {
    if (!child || !child->name) {
      error("Unexpected scalar item in toolchain config.");
      return false;
    }

    if (_stricmp(child->name, "host") == 0) {
      if (!toolchain_validate_host_section(child, "toolchain config"))
        return false;
      continue;
    }

    if (_stricmp(child->name, "tools") == 0) {
      if (child->type != NODE_TYPE_DEF) {
        error("Attribute 'tools' in toolchain config must be a section.");
        return false;
      }
      node_foreach(child, item) {
        if (!item || !item->name) {
          error("tools(...) in toolchain config must contain only named tool sections.");
          return false;
        }
        if (_stricmp(item->name, "tool") != 0) {
          error("tools(...) in toolchain config must contain only tool(...) entries.");
          return false;
        }
        if (!toolchain_validate_tool_item(item, "toolchain config"))
          return false;
      }
      continue;
    }

    if (_stricmp(child->name, "sdks") == 0) {
      if (child->type != NODE_TYPE_DEF) {
        error("Attribute 'sdks' in toolchain config must be a section.");
        return false;
      }
      node_foreach(child, item) {
        if (!item || !item->name) {
          error("sdks(...) in toolchain config must contain only named sdk sections.");
          return false;
        }
        if (_stricmp(item->name, "sdk") != 0) {
          error("sdks(...) in toolchain config must contain only sdk(...) entries.");
          return false;
        }
        if (!toolchain_validate_sdk_item(item, "toolchain config"))
          return false;
      }
      continue;
    }

    if (_stricmp(child->name, "environments") == 0) {
      if (child->type != NODE_TYPE_DEF) {
        error("Attribute 'environments' in toolchain config must be a section.");
        return false;
      }
      node_foreach(child, item) {
        if (!item || !item->name) {
          error("environments(...) in toolchain config must contain only named environment sections.");
          return false;
        }
        if (_stricmp(item->name, "environment") != 0) {
          error("environments(...) in toolchain config must contain only environment(...) entries.");
          return false;
        }
        if (!toolchain_validate_env_item(item, "toolchain config"))
          return false;
      }
      continue;
    }

    error("Unknown attribute '%s' in toolchain config.", child->name);
    return false;
  }

  return true;
}

static toolchain* toolchain_read(node* tree) {
  toolchain* tc = push(sizeof(toolchain));
  memset(tc, 0, sizeof(toolchain));
  tc->config_tree = tree;

  tc->p_arch = toolchain_detect_host_arch();
  tc->p_os = toolchain_detect_host_os();

  node* host_n = node_get_child(tree, "host");
  if (host_n) {
    node* arch_n = node_get_child(host_n, "arch");
    node* os_n = node_get_child(host_n, "os");
    const char* arch_id = arch_n ? node_get_idf(arch_n) : NULL;
    const char* os_id = os_n ? node_get_idf(os_n) : NULL;

    for (int i = 0; arch_id && i < ARCH_MAX; ++i) {
      if (_stricmp(ARCH_NAMES[i], arch_id) == 0) {
        tc->p_arch = (arch)i;
        break;
      }
    }

    for (int i = 0; os_id && i < OS_MAX; ++i) {
      if (_stricmp(OS_NAMES[i], os_id) == 0) {
        tc->p_os = (os)i;
        break;
      }
    }
  }

  node* tools_n = node_get_child(tree, "tools");
  if (tools_n) {
    toolchain_env* host = toolchain_host_env(tc, true);
    node_foreach(tools_n, it) {
      if (!host)
        break;

      if (!toolchain_env_ensure_tool_capacity(host, host->tool_c + 1))
        break;

      node* id_n = node_get_child(it, "id");
      node* path_n = node_get_child(it, "path");
      node* ver_n = node_get_child(it, "version");
      if (!path_n)
        continue;

      const char* path = toolchain_norm_path(node_get_str(path_n));
      const char* id = id_n ? node_get_str(id_n) : toolchain_path_basename(path);
      toolchain_set_tool(&host->tools[host->tool_c++], id, path, ver_n ? node_get_str(ver_n) : NULL);
    }
  }

  node* sdks_n = node_get_child(tree, "sdks");
  if (sdks_n) {
    toolchain_env* host = toolchain_host_env(tc, true);
    node_foreach(sdks_n, it) {
      if (!host)
        break;

      if (!toolchain_env_ensure_sdk_capacity(host, host->sdk_c + 1))
        break;

      sdk* s = &host->sdks[host->sdk_c++];
      node* name_n = node_get_child(it, "name");
      node* ver_n = node_get_child(it, "version");
      node* base_n = node_get_child(it, "base_path");
      node* inc_n = node_get_child(it, "inc_path");
      node* src_n = node_get_child(it, "src_path");
      node* lib_n = node_get_child(it, "lib_path");
      node* bin_n = node_get_child(it, "bin_path");

      s->name = name_n ? node_get_str(name_n) : NULL;
      s->version = ver_n ? node_get_str(ver_n) : NULL;
      s->base_path = base_n ? toolchain_norm_path(node_get_str(base_n)) : NULL;
      s->inc_path = inc_n ? toolchain_norm_path(node_get_str(inc_n)) : NULL;
      s->src_path = src_n ? toolchain_norm_path(node_get_str(src_n)) : NULL;
      s->lib_path = lib_n ? toolchain_norm_path(node_get_str(lib_n)) : NULL;
      s->bin_path = bin_n ? toolchain_norm_path(node_get_str(bin_n)) : NULL;
    }
  }

  node* envs_n = node_get_child(tree, "environments");
  if (envs_n) {
    node_foreach(envs_n, it) {
      if (!toolchain_ensure_env_capacity(tc, tc->env_c + 1))
        break;

      toolchain_env* env = &tc->envs[tc->env_c++];
      memset(env, 0, sizeof(*env));

      node* id_n = node_get_child(it, "id");
      node* provider_n = node_get_child(it, "provider");
      node* name_n = node_get_child(it, "name");
      node* host_n = node_get_child(it, "host");
      env->id = id_n ? node_get_str(id_n) : NULL;
      env->provider = provider_n ? node_get_str(provider_n) : NULL;
      env->name = name_n ? node_get_str(name_n) : NULL;
      env->p_os = OS_WINDOWS;
      env->p_arch = ARCH_X86_64;

      if (host_n) {
        node* arch_n = node_get_child(host_n, "arch");
        node* os_n = node_get_child(host_n, "os");
        const char* arch_id = arch_n ? node_get_idf(arch_n) : NULL;
        const char* os_id = os_n ? node_get_idf(os_n) : NULL;
        for (int i = 0; arch_id && i < ARCH_MAX; ++i)
          if (_stricmp(ARCH_NAMES[i], arch_id) == 0)
            env->p_arch = (arch)i;
        for (int i = 0; os_id && i < OS_MAX; ++i)
          if (_stricmp(OS_NAMES[i], os_id) == 0)
            env->p_os = (os)i;
      }

      node* probes_n = node_get_child(it, "probes");
      if (probes_n) {
        node* n_docker = node_get_child(probes_n, "docker_buildx_platforms");
        env->probe_docker_buildx_platforms = n_docker ? node_get_str(n_docker) : NULL;
      }

      node* env_tools_n = node_get_child(it, "tools");
      if (env_tools_n) {
        node_foreach(env_tools_n, jt) {
          if (!toolchain_env_ensure_tool_capacity(env, env->tool_c + 1))
            break;
          node* env_id_n = node_get_child(jt, "id");
          node* path_n = node_get_child(jt, "path");
          node* ver_n = node_get_child(jt, "version");
          if (!path_n)
            continue;
          const char* path = toolchain_norm_path(node_get_str(path_n));
          const char* id = env_id_n ? node_get_str(env_id_n) : toolchain_path_basename(path);
          toolchain_set_tool(&env->tools[env->tool_c++], id, path, ver_n ? node_get_str(ver_n) : NULL);
        }
      }

      node* env_sdks_n = node_get_child(it, "sdks");
      if (env_sdks_n) {
        node_foreach(env_sdks_n, jt) {
          if (!toolchain_env_ensure_sdk_capacity(env, env->sdk_c + 1))
            break;
          sdk* s = &env->sdks[env->sdk_c++];
          node* n_name = node_get_child(jt, "name");
          node* n_ver = node_get_child(jt, "version");
          node* n_base = node_get_child(jt, "base_path");
          node* n_inc = node_get_child(jt, "inc_path");
          node* n_src = node_get_child(jt, "src_path");
          node* n_lib = node_get_child(jt, "lib_path");
          node* n_bin = node_get_child(jt, "bin_path");
          s->name = n_name ? node_get_str(n_name) : NULL;
          s->version = n_ver ? node_get_str(n_ver) : NULL;
          s->base_path = n_base ? toolchain_norm_path(node_get_str(n_base)) : NULL;
          s->inc_path = n_inc ? toolchain_norm_path(node_get_str(n_inc)) : NULL;
          s->src_path = n_src ? toolchain_norm_path(node_get_str(n_src)) : NULL;
          s->lib_path = n_lib ? toolchain_norm_path(node_get_str(n_lib)) : NULL;
          s->bin_path = n_bin ? toolchain_norm_path(node_get_str(n_bin)) : NULL;
        }
      }

      toolchain_sort_tool_array(env->tools, env->tool_c);
      toolchain_sort_sdk_array(env->sdks, env->sdk_c);
    }
  }

  if (!envs_n)
    toolchain_snapshot_current_host_env(tc);

  toolchain_refresh_runtime_support(tc);

  toolchain_sort_tools(tc);
  toolchain_sort_sdks(tc);
  return tc;
}

static node* toolchain_write(toolchain* tc) {
  node* tree = node_alloc();
  if (!tree)
    return NULL;

  node* envs_n = node_create_named("environments");
  if (envs_n)
    toolchain_push_child_back(tree, envs_n);

  for (int ei = 0; envs_n && ei < tc->env_c; ++ei) {
    toolchain_env* env = &tc->envs[ei];
    node* env_n = node_create_named("environment");
    if (!env_n)
      continue;
    if (env->id)
      toolchain_push_child_back(env_n, node_create_str("id", env->id));
    if (env->provider)
      toolchain_push_child_back(env_n, node_create_str("provider", env->provider));
    if (env->name)
      toolchain_push_child_back(env_n, node_create_str("name", env->name));

    node* probes_n = node_create_named("probes");
    bool has_probes = false;
    if (probes_n) {
      if (env->probe_docker_buildx_platforms) {
        toolchain_push_child_back(probes_n, node_create_str("docker_buildx_platforms", env->probe_docker_buildx_platforms));
        has_probes = true;
      }
      if (has_probes)
        toolchain_push_child_back(env_n, probes_n);
    }

    node* env_host_n = node_create_named("host");
    if (env_host_n) {
      toolchain_push_child_back(env_host_n, node_create_idf("arch", ARCH_NAMES[env->p_arch]));
      toolchain_push_child_back(env_host_n, node_create_idf("os", OS_NAMES[env->p_os]));
      toolchain_push_child_back(env_n, env_host_n);
    }

    node* env_tools_n = env->tool_c > 0 ? node_create_named("tools") : NULL;
    if (env_tools_n)
      toolchain_push_child_back(env_n, env_tools_n);
    for (int i = 0; env_tools_n && i < env->tool_c; ++i) {
      node* t = node_create_named("tool");
      if (!t)
        continue;
      if (env->tools[i].id)
        toolchain_push_child_back(t, node_create_str("id", env->tools[i].id));
      toolchain_push_child_back(t, node_create_str("path", toolchain_norm_path(env->tools[i].path ? env->tools[i].path : "")));
      if (env->tools[i].version && env->tools[i].version[0])
        toolchain_push_child_back(t, node_create_str("version", env->tools[i].version));
      toolchain_push_child_back(env_tools_n, t);
    }

    node* env_sdks_n = env->sdk_c > 0 ? node_create_named("sdks") : NULL;
    if (env_sdks_n)
      toolchain_push_child_back(env_n, env_sdks_n);
    for (int i = 0; env_sdks_n && i < env->sdk_c; ++i) {
      node* s = node_create_named("sdk");
      if (!s)
        continue;
      if (env->sdks[i].name)
        toolchain_push_child_back(s, node_create_str("name", env->sdks[i].name));
      if (env->sdks[i].version)
        toolchain_push_child_back(s, node_create_str("version", env->sdks[i].version));
      if (env->sdks[i].base_path)
        toolchain_push_child_back(s, node_create_str("base_path", toolchain_norm_path(env->sdks[i].base_path)));
      if (env->sdks[i].inc_path)
        toolchain_push_child_back(s, node_create_str("inc_path", toolchain_norm_path(env->sdks[i].inc_path)));
      if (env->sdks[i].src_path)
        toolchain_push_child_back(s, node_create_str("src_path", toolchain_norm_path(env->sdks[i].src_path)));
      if (env->sdks[i].lib_path)
        toolchain_push_child_back(s, node_create_str("lib_path", toolchain_norm_path(env->sdks[i].lib_path)));
      if (env->sdks[i].bin_path)
        toolchain_push_child_back(s, node_create_str("bin_path", toolchain_norm_path(env->sdks[i].bin_path)));
      toolchain_push_child_back(env_sdks_n, s);
    }

    toolchain_push_child_back(envs_n, env_n);
  }

  return tree;
}

static toolchain* toolchain_init(const char* path, bool reinit, cmd_ctx* cmdctx) {
  bool exists = file_exists(path);
  if (exists && (!reinit)) {
    const char* data = read_entire_file(path);
    if (!data) {
      error("Failed to open file %s: no read access.", path);
      return NULL;
    }

    node* tree = node_parse(data);
    if (!tree) {
      error("Failed to parse file %s.", path);
      return NULL;
    }
    if (!toolchain_validate_tree(tree)) {
      error("Failed to validate file %s.", path);
      return NULL;
    }

    toolchain* tc = toolchain_read(tree);
    if (tc && cmdctx) {
      tc->project_cfg_path = cmdctx->cfg_paths[CFG_PROJECT];
      tc->user_cfg_path = cmdctx->cfg_paths[CFG_USER];
      tc->local_cfg_path = cmdctx->cfg_paths[CFG_LOCAL];
      tc->toolchain_cfg_path = cmdctx->cfg_paths[CFG_TOOLCHAIN];
    }
    return tc;
  }

  print("Generating toolchain cache...");
  toolchain* previous = NULL;
  if (exists) {
    const char* data = read_entire_file(path);
    if (data) {
      node* old_tree = node_parse(data);
      if (old_tree && toolchain_validate_tree(old_tree))
        previous = toolchain_read(old_tree);
      else if (old_tree)
        warn("Ignoring invalid existing toolchain cache while regenerating %s.", path);
    }
  }
  toolchain* tc = push(sizeof(toolchain));
  memset(tc, 0, sizeof(toolchain));
  if (cmdctx) {
    tc->project_cfg_path = cmdctx->cfg_paths[CFG_PROJECT];
    tc->user_cfg_path = cmdctx->cfg_paths[CFG_USER];
    tc->local_cfg_path = cmdctx->cfg_paths[CFG_LOCAL];
    tc->toolchain_cfg_path = cmdctx->cfg_paths[CFG_TOOLCHAIN];
  }
  toolchain_fill(tc);
  if (!toolchain_get_tool_path(tc, "cmake"))
    return NULL;
  if (previous) {
    for (int i = 0; i < previous->env_c; ++i)
      if (toolchain_env_find_index(tc, previous->envs[i].id) < 0)
        toolchain_add_or_replace_env(tc, &previous->envs[i]);
    toolchain_rebuild_aggregate_support(tc);
  }
  node* tree = toolchain_write(tc);
  tc->config_tree = tree;
  const char* str = node_write(tree);
  write_entire_file(path, str);
  return tc;
}
