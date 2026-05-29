#pragma once
#include "bbs_toolchain.h"
#include "bbs.h"
#include "bbs_base.c"

static bool toolchain_tool_exists(toolchain* tc, tool_type type, const char* path);
static const char* toolchain_probe_version(const char* exe_path, const char* arg_a, const char* arg_b, const char* pat_a, const char* pat_b);
static const char* toolchain_first_path_match(const char* pattern);
static int toolchain_collect_path_matches(const char* pattern, const char** matches, int max_matches, bool dirs_only);
static const char* toolchain_norm_path(const char* in);
static const char* toolchain_extract_version(const char* text);
static const char* toolchain_run_extract_version(const char* cmd);

typedef struct {
  bool minimal;
  bool show_tools;
  bool show_sdks;
  bool has_type_filter;
  bool paths_only;
  bool versions_only;
  tool_type type_filter;
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

#if defined(_WIN32)
  if (_stricmp(base, exe_name) == 0)
    return strchr(exe_name, '.') != NULL;

  if (!strchr(exe_name, '.')) {
    char cand[128] = {0};
    const char* exts[] = {".exe", ".bat", ".cmd", ".com"};
    for (size_t i = 0; i < _countof(exts); ++i) {
      snprintf(cand, sizeof(cand), "%s%s", exe_name, exts[i]);
      if (_stricmp(base, cand) == 0)
        return true;
    }
  }
#else
  if (_stricmp(base, exe_name) == 0)
    return true;
#endif

  return false;
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
  if (a->type != b->type)
    return (a->type < b->type) ? -1 : 1;

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

  for (int i = 0; i < tc->tool_c - 1; ++i) {
    for (int j = i + 1; j < tc->tool_c; ++j) {
      if (toolchain_tool_cmp(&tc->tools[i], &tc->tools[j]) > 0) {
        tool tmp = tc->tools[i];
        tc->tools[i] = tc->tools[j];
        tc->tools[j] = tmp;
      }
    }
  }
}

static void toolchain_sort_sdks(toolchain* tc) {
  if (!tc)
    return;

  for (int i = 0; i < tc->sdk_c - 1; ++i) {
    for (int j = i + 1; j < tc->sdk_c; ++j) {
      if (toolchain_sdk_cmp(&tc->sdks[i], &tc->sdks[j]) > 0) {
        sdk tmp = tc->sdks[i];
        tc->sdks[i] = tc->sdks[j];
        tc->sdks[j] = tmp;
      }
    }
  }
}

static int toolchain_find_tool_by_id(toolchain* tc, tool_type type, const char* id) {
  if (!tc || !id || !id[0])
    return -1;

  for (int i = 0; i < tc->tool_c; ++i) {
    if (tc->tools[i].type == type && tc->tools[i].id && _stricmp(tc->tools[i].id, id) == 0)
      return i;
  }

  return -1;
}

static const char* toolchain_path_version_fallback(const char* path) {
  if (!path || !path[0])
    return NULL;

  return toolchain_extract_version(path);
}

static tool_type toolchain_type_from_path_and_id(const char* path, const char* id) {
  const char* base = toolchain_path_basename(path ? path : "");
  const char* name = id && id[0] ? id : base;
  if (!name)
    return TOOL_TYPE_MISC;

  if (_stricmp(name, "cmake") == 0 || _stricmp(name, "make") == 0 || _stricmp(name, "ninja") == 0 || _stricmp(name, "premake5") == 0 || _stricmp(name, "nmake") == 0)
    return TOOL_TYPE_BUILD_SYSTEM;
  if (_stricmp(name, "gcc") == 0 || _stricmp(name, "clang") == 0 || _stricmp(name, "cl") == 0)
    return TOOL_TYPE_C_COMPILER;
  if (_stricmp(name, "g++") == 0 || _stricmp(name, "clang++") == 0)
    return TOOL_TYPE_CPP_COMPILER;
  if (_stricmp(name, "ar") == 0 || _stricmp(name, "llvm-ar") == 0 || _stricmp(name, "lib") == 0)
    return TOOL_TYPE_ARCHIVERS;
  if (_stricmp(name, "ld") == 0 || _stricmp(name, "lld") == 0 || _stricmp(name, "link") == 0)
    return TOOL_TYPE_LINKERS;
  return TOOL_TYPE_MISC;
}

static const char* toolchain_probe_file_version(const char* path) {
#if defined(_WIN32)
  if (!path || !path[0])
    return NULL;

  const char* base = toolchain_path_basename(path);
  if (!base || !strstr(base, ".exe"))
    return NULL;

  char cmd[2048] = {0};
  snprintf(cmd, sizeof(cmd), "powershell -NoProfile -Command \"(Get-Item '%s').VersionInfo.FileVersion\" 2>nul", path);
  return toolchain_run_extract_version(cmd);
#else
  (void)path;
  return NULL;
#endif
}

static void toolchain_set_tool(tool* out, const char* id, tool_type type, const char* path, const char* version) {
  if (!out)
    return;

  out->id = id ? arena_text(id, strlen(id)) : NULL;
  out->type = type;
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

static void toolchain_upsert_tool(toolchain* tc, const char* id, tool_type type, const char* path, const char* version) {
  if (!tc || !id || !id[0] || !path || !path[0])
    return;

  int idx = toolchain_find_tool_by_id(tc, type, id);
  if (idx >= 0) {
    if (toolchain_should_replace_tool(&tc->tools[idx], version, path))
      toolchain_set_tool(&tc->tools[idx], id, type, path, version);
    return;
  }

  if (tc->tool_c >= TOOL_ARRAY_DIM)
    return;

  toolchain_set_tool(&tc->tools[tc->tool_c++], id, type, path, version);
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

#if defined(_WIN32)
  FILE* pipe = _popen(cmd, "r");
#else
  FILE* pipe = popen(cmd, "r");
#endif
  if (!pipe)
    return false;

  out[0] = '\0';
  bool ok = fgets(out, (int)out_dim, pipe) != NULL;
#if defined(_WIN32)
  _pclose(pipe);
#else
  pclose(pipe);
#endif
  return ok;
}

static int toolchain_run_collect_lines(const char* cmd, const char** lines, int max_lines) {
  if (!cmd || !cmd[0] || !lines || max_lines <= 0)
    return 0;

#if defined(_WIN32)
  FILE* pipe = _popen(cmd, "r");
#else
  FILE* pipe = popen(cmd, "r");
#endif
  if (!pipe)
    return 0;

  int count = 0;
  char line[1024] = {0};
  while (count < max_lines && fgets(line, (int)sizeof(line), pipe) != NULL) {
    const char* trimmed = toolchain_trim_line(line);
    if (trimmed[0])
      lines[count++] = arena_text(trimmed, strlen(trimmed));
  }

#if defined(_WIN32)
  _pclose(pipe);
#else
  pclose(pipe);
#endif
  return count;
}

static const char* toolchain_run_extract_version(const char* cmd) {
  if (!cmd || !cmd[0])
    return NULL;

#if defined(_WIN32)
  FILE* pipe = _popen(cmd, "r");
#else
  FILE* pipe = popen(cmd, "r");
#endif
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

#if defined(_WIN32)
  _pclose(pipe);
#else
  pclose(pipe);
#endif
  return found;
}

static const char* toolchain_run_extract_version_pat(const char* cmd, const char* pattern) {
  if (!cmd || !cmd[0])
    return NULL;

#if defined(_WIN32)
  FILE* pipe = _popen(cmd, "r");
#else
  FILE* pipe = popen(cmd, "r");
#endif
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

#if defined(_WIN32)
  _pclose(pipe);
#else
  pclose(pipe);
#endif
  return found;
}

static const char* toolchain_find_with_system(const char* name) {
  if (!name || !name[0])
    return NULL;

  char cmd[512] = {0};
#if defined(_WIN32)
  snprintf(cmd, sizeof(cmd), "where %s 2>nul", name);
#else
  snprintf(cmd, sizeof(cmd), "which %s 2>/dev/null", name);
#endif

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
#if defined(_WIN32)
  snprintf(cmd, sizeof(cmd), "where %s 2>nul", name);
#else
  snprintf(cmd, sizeof(cmd), "which -a %s 2>/dev/null", name);
#endif

  const char* lines[64] = {0};
  int count = toolchain_run_collect_lines(cmd, lines, _countof(lines));
  int out = 0;
  for (int i = 0; i < count && out < max_matches; ++i) {
    if (file_exists(lines[i]) && toolchain_path_name_matches(lines[i], name))
      toolchain_push_unique_path(matches, &out, max_matches, lines[i]);
  }

  return out;
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
      const char* full = toolchain_join2(dir, exe_name);
#if defined(_WIN32)
      if (file_exists(full))
        return arena_text(full, strlen(full));
      {
        const char* full_exe = toolchain_join2(dir, toolchain_join2(exe_name, ".exe"));
        if (file_exists(full_exe))
          return arena_text(full_exe, strlen(full_exe));
      }
#else
      if (file_exists(full))
        return arena_text(full, strlen(full));
#endif
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
      const char* full = toolchain_join2(dir, exe_name);
      if (full && file_exists(full))
        toolchain_push_unique_path(matches, &out, max_matches, full);

#if defined(_WIN32)
      const char* full_exe = toolchain_join2(dir, toolchain_join2(exe_name, ".exe"));
      if (full_exe && file_exists(full_exe))
        toolchain_push_unique_path(matches, &out, max_matches, full_exe);
#endif
    }

    if (!semi || out >= max_matches)
      break;
    next = semi + 1;
  }

  return out;
}

static const char* toolchain_find_with_vswhere_vcvarsall(void) {
#if defined(_WIN32)
  const char* pf86 = getenv("ProgramFiles(x86)");
  if (!pf86 || !pf86[0])
    return NULL;

  char vswhere[_MAX_PATH] = {0};
  snprintf(vswhere, sizeof(vswhere), "%s\\Microsoft Visual Studio\\Installer\\vswhere.exe", pf86);
  if (!file_exists(vswhere))
    return NULL;

  char cmd[2048] = {0};
  snprintf(cmd, sizeof(cmd), "\"%s\" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>nul", vswhere);

  char line[1024] = {0};
  if (!toolchain_run_first_line(cmd, line, sizeof(line)))
    return NULL;
  const char* install = toolchain_trim_line(line);
  if (!install[0])
    return NULL;

  const char* vcvars = toolchain_join2(install, "VC/Auxiliary/Build/vcvarsall.bat");
  if (vcvars && file_exists(vcvars))
    return toolchain_norm_path(vcvars);
#endif
  return NULL;
}

static const char* toolchain_find_msvc_root_with_vswhere(void) {
#if defined(_WIN32)
  const char* pf86 = getenv("ProgramFiles(x86)");
  if (!pf86 || !pf86[0])
    return NULL;

  char vswhere[_MAX_PATH] = {0};
  snprintf(vswhere, sizeof(vswhere), "%s\\Microsoft Visual Studio\\Installer\\vswhere.exe", pf86);
  if (!file_exists(vswhere))
    return NULL;

  char cmd[2048] = {0};
  snprintf(cmd, sizeof(cmd), "\"%s\" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>nul", vswhere);

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
  snprintf(list_cmd, sizeof(list_cmd), "dir /b /ad \"%s\" 2>nul", tools);
  if (!toolchain_run_first_line(list_cmd, line, sizeof(line)))
    return NULL;

  const char* ver_dir = toolchain_trim_line(line);
  if (!ver_dir[0])
    return NULL;

  return toolchain_norm_path(toolchain_join2(tools, ver_dir));
#endif
  return NULL;
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
#if defined(_WIN32)
      snprintf(cmd, sizeof(cmd), "where /r \"%s\" %s 2>nul", root, exe_name);
      int line_c = toolchain_run_collect_lines(cmd, lines, _countof(lines));
      for (int i = 0; i < line_c && out < max_matches; ++i) {
        if (file_exists(lines[i]) && toolchain_path_name_matches(lines[i], exe_name))
          toolchain_push_unique_path(matches, &out, max_matches, lines[i]);
      }

      if (out < max_matches && !strstr(exe_name, ".exe")) {
        snprintf(cmd, sizeof(cmd), "where /r \"%s\" %s.exe 2>nul", root, exe_name);
        line_c = toolchain_run_collect_lines(cmd, lines, _countof(lines));
        for (int i = 0; i < line_c && out < max_matches; ++i) {
          if (file_exists(lines[i]) && toolchain_path_name_matches(lines[i], exe_name))
            toolchain_push_unique_path(matches, &out, max_matches, lines[i]);
        }
      }
#else
      snprintf(cmd, sizeof(cmd), "find \"%s\" -type f \\( -name '%s' -o -name '%s.exe' \\) 2>/dev/null", root, exe_name, exe_name);
      int line_c = toolchain_run_collect_lines(cmd, lines, _countof(lines));
      for (int i = 0; i < line_c && out < max_matches; ++i) {
        if (file_exists(lines[i]) && toolchain_path_name_matches(lines[i], exe_name))
          toolchain_push_unique_path(matches, &out, max_matches, lines[i]);
      }
#endif
    }

    if (!semi || out >= max_matches)
      break;
    next = semi + 1;
  }

  return out;
}

static void toolchain_try_add_tool_from_dir(toolchain* tc, tool_type type, const char* id, const char* dir, const char* exe_base, const char* arg_a, const char* arg_b, const char* pat_a, const char* pat_b) {
  if (!tc || !dir || !dir[0] || !exe_base || !exe_base[0] || tc->tool_c >= TOOL_ARRAY_DIM)
    return;

  const char* p1 = toolchain_join2(dir, exe_base);
  const char* p2 = toolchain_join2(dir, toolchain_join2(exe_base, ".exe"));
  const char* path = NULL;

  if (p1 && file_exists(p1))
    path = p1;
  else if (p2 && file_exists(p2))
    path = p2;

  if (!path)
    return;

  path = toolchain_norm_path(path);
  if (toolchain_tool_exists(tc, type, path))
    return;

  const char* version = toolchain_probe_version(path, arg_a, arg_b, pat_a, pat_b);
  if (!version)
    version = toolchain_path_version_fallback(path);
  toolchain_upsert_tool(tc, id, type, path, version);

  if (version && version[0])
    print("  [tool] %-12s found at %s (v%s)", id, path, version);
  else
    print("  [tool] %-12s found at %s (version unknown)", id, path);
}

static void toolchain_discover_vs_tool_bins(toolchain* tc) {
#if defined(_WIN32)
  const char* pf86 = getenv("ProgramFiles(x86)");
  if (!pf86 || !pf86[0])
    return;

  char vswhere[_MAX_PATH] = {0};
  snprintf(vswhere, sizeof(vswhere), "%s\\Microsoft Visual Studio\\Installer\\vswhere.exe", pf86);
  if (!file_exists(vswhere))
    return;

  char cmd[2048] = {0};
  snprintf(cmd, sizeof(cmd), "\"%s\" -all -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>nul", vswhere);

  const char* installs[32] = {0};
  int install_c = toolchain_run_collect_lines(cmd, installs, 32);
  for (int i = 0; i < install_c; ++i) {
    const char* install = installs[i];
    const char* msvc_root = toolchain_join2(install, "VC/Tools/MSVC");
    if (!dir_exists(msvc_root))
      continue;

    char ver_cmd[2048] = {0};
    snprintf(ver_cmd, sizeof(ver_cmd), "dir /b /ad \"%s\" 2>nul", msvc_root);
    const char* versions[32] = {0};
    int ver_c = toolchain_run_collect_lines(ver_cmd, versions, 32);
    for (int v = 0; v < ver_c; ++v) {
      const char* ver_root = toolchain_join2(msvc_root, versions[v]);
      const char* bin_x64 = toolchain_join2(ver_root, "bin/Hostx64/x64");
      const char* bin_x86 = toolchain_join2(ver_root, "bin/Hostx64/x86");
      const char* bin_arm64 = toolchain_join2(ver_root, "bin/Hostx64/arm64");
      const char* bin_hx86_x86 = toolchain_join2(ver_root, "bin/Hostx86/x86");
      const char* bin_hx86_x64 = toolchain_join2(ver_root, "bin/Hostx86/x64");
      const char* bin_hx86_arm64 = toolchain_join2(ver_root, "bin/Hostx86/arm64");
      const char* bin_harm64_x64 = toolchain_join2(ver_root, "bin/Hostarm64/x64");
      const char* bin_harm64_x86 = toolchain_join2(ver_root, "bin/Hostarm64/x86");
      const char* bin_harm64_arm64 = toolchain_join2(ver_root, "bin/Hostarm64/arm64");

      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_C_COMPILER, "cl", bin_x64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_CPP_COMPILER, "cl", bin_x64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_LINKERS, "link", bin_x64, "link", "", "/?", "Version", "LINK");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_ARCHIVERS, "lib", bin_x64, "lib", "", "/?", "Version", "Library Manager Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_MISC, "dumpbin", bin_x64, "dumpbin", "/?", "", "Version", "Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_BUILD_SYSTEM, "nmake", bin_x64, "nmake", "/?", "", "Version", "Version");

      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_C_COMPILER, "cl", bin_x86, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_CPP_COMPILER, "cl", bin_x86, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_LINKERS, "link", bin_x86, "link", "", "/?", "Version", "LINK");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_ARCHIVERS, "lib", bin_x86, "lib", "", "/?", "Version", "Library Manager Version");

      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_C_COMPILER, "cl", bin_arm64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_CPP_COMPILER, "cl", bin_arm64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_LINKERS, "link", bin_arm64, "link", "", "/?", "Version", "LINK");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_ARCHIVERS, "lib", bin_arm64, "lib", "", "/?", "Version", "Library Manager Version");

      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_C_COMPILER, "cl", bin_hx86_x86, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_CPP_COMPILER, "cl", bin_hx86_x86, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_LINKERS, "link", bin_hx86_x86, "link", "", "/?", "Version", "LINK");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_ARCHIVERS, "lib", bin_hx86_x86, "lib", "", "/?", "Version", "Library Manager Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_MISC, "dumpbin", bin_hx86_x86, "dumpbin", "/?", "", "Version", "Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_BUILD_SYSTEM, "nmake", bin_hx86_x86, "nmake", "/?", "", "Version", "Version");

      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_C_COMPILER, "cl", bin_hx86_x64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_CPP_COMPILER, "cl", bin_hx86_x64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_LINKERS, "link", bin_hx86_x64, "link", "", "/?", "Version", "LINK");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_ARCHIVERS, "lib", bin_hx86_x64, "lib", "", "/?", "Version", "Library Manager Version");

      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_C_COMPILER, "cl", bin_hx86_arm64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_CPP_COMPILER, "cl", bin_hx86_arm64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_LINKERS, "link", bin_hx86_arm64, "link", "", "/?", "Version", "LINK");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_ARCHIVERS, "lib", bin_hx86_arm64, "lib", "", "/?", "Version", "Library Manager Version");

      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_C_COMPILER, "cl", bin_harm64_x64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_CPP_COMPILER, "cl", bin_harm64_x64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_LINKERS, "link", bin_harm64_x64, "link", "", "/?", "Version", "LINK");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_ARCHIVERS, "lib", bin_harm64_x64, "lib", "", "/?", "Version", "Library Manager Version");

      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_C_COMPILER, "cl", bin_harm64_x86, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_CPP_COMPILER, "cl", bin_harm64_x86, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_LINKERS, "link", bin_harm64_x86, "link", "", "/?", "Version", "LINK");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_ARCHIVERS, "lib", bin_harm64_x86, "lib", "", "/?", "Version", "Library Manager Version");

      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_C_COMPILER, "cl", bin_harm64_arm64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_CPP_COMPILER, "cl", bin_harm64_arm64, "cl", "", "/?", "Version", "Compiler Version");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_LINKERS, "link", bin_harm64_arm64, "link", "", "/?", "Version", "LINK");
      toolchain_try_add_tool_from_dir(tc, TOOL_TYPE_ARCHIVERS, "lib", bin_harm64_arm64, "lib", "", "/?", "Version", "Library Manager Version");
    }
  }
#else
  (void)tc;
#endif
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

static bool toolchain_tool_exists(toolchain* tc, tool_type type, const char* path) {
  if (!tc || !path)
    return false;
  for (int i = 0; i < tc->tool_c; ++i) {
    if (tc->tools[i].type == type && tc->tools[i].path && _stricmp(tc->tools[i].path, path) == 0)
      return true;
  }
  return false;
}

static void toolchain_discover_tool(toolchain* tc, const tool_discover_strat* s) {
  if (!tc || !s || tc->tool_c >= TOOL_ARRAY_DIM)
    return;

  const char* matches[64] = {0};
  int match_c = 0;

  {
    const char* found[64] = {0};
    int found_c = toolchain_collect_with_system(s->exe_name, found, _countof(found));
    for (int i = 0; i < found_c; ++i)
      toolchain_push_unique_path(matches, &match_c, _countof(matches), found[i]);
  }
  if (_stricmp(s->id, "vcvarsall") == 0) {
    const char* vcvars = toolchain_find_with_vswhere_vcvarsall();
    if (vcvars)
      toolchain_push_unique_path(matches, &match_c, _countof(matches), vcvars);
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

  for (int i = 0; i < match_c && tc->tool_c < TOOL_ARRAY_DIM; ++i) {
    const char* path = toolchain_norm_path(matches[i]);
    if (toolchain_tool_exists(tc, s->type, path))
      continue;

    const char* version = toolchain_probe_version(path, s->version_arg, s->version_arg_fallback, s->version_regex, s->version_regex_fallback);
    if (!version)
      version = toolchain_path_version_fallback(path);
    toolchain_upsert_tool(tc, s->id, s->type, path, version);

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
  bool recursive = strstr(pattern, "**") != NULL;
#if defined(_WIN32)
  if (recursive) {
    char win_pat[2048] = {0};
    strncpy(win_pat, pattern, sizeof(win_pat) - 1);
    while (strstr(win_pat, "**")) {
      char* star = strstr(win_pat, "**");
      star[0] = '*';
      memmove(star + 1, star + 2, strlen(star + 2) + 1);
    }
    snprintf(cmd, sizeof(cmd), "dir /s /b \"%s\" 2>nul", win_pat);
  } else {
    snprintf(cmd, sizeof(cmd), dirs_only ? "for /d %%i in (\"%s\") do @echo %%~fi" : "for %%i in (\"%s\") do @echo %%~fi", pattern);
  }
#else
  if (recursive) {
    const char* wildcard = strchr(pattern, '*');
    size_t prefix_len = wildcard ? (size_t)(wildcard - pattern) : strlen(pattern);
    while (prefix_len > 0 && pattern[prefix_len - 1] != '/' && pattern[prefix_len - 1] != '\\')
      --prefix_len;

    char root[1024] = {0};
    if (prefix_len == 0) {
      strcpy(root, ".");
    } else {
      if (prefix_len >= sizeof(root))
        prefix_len = sizeof(root) - 1;
      memcpy(root, pattern, prefix_len);
      root[prefix_len] = '\0';
    }
    snprintf(cmd, sizeof(cmd), "find \"%s\" %s-path '%s' 2>/dev/null", root, dirs_only ? "-type d " : "", pattern);
  } else {
    snprintf(cmd, sizeof(cmd), "ls -d %s 2>/dev/null", pattern);
  }
#endif
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
  if (!tc || !s || tc->sdk_c >= SDK_ARRAY_DIM)
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

  sdk* out = &tc->sdks[tc->sdk_c++];
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

static const char* tool_type_to_idf(tool_type t) {
  switch (t) {
    case TOOL_TYPE_BUILD_SYSTEM: return "build_system";
    case TOOL_TYPE_C_COMPILER:   return "c_compiler";
    case TOOL_TYPE_CPP_COMPILER: return "cpp_compiler";
    case TOOL_TYPE_ARCHIVERS:    return "archiver";
    case TOOL_TYPE_LINKERS:      return "linker";
    case TOOL_TYPE_MISC:         return "misc";
    default:                     return "misc";
  }
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

static tool_type tool_type_from_idf(const char* s) {
  if (!s) return TOOL_TYPE_MISC;
  if (_stricmp(s, "build_system") == 0) return TOOL_TYPE_BUILD_SYSTEM;
  if (_stricmp(s, "c_compiler") == 0) return TOOL_TYPE_C_COMPILER;
  if (_stricmp(s, "cpp_compiler") == 0) return TOOL_TYPE_CPP_COMPILER;
  if (_stricmp(s, "archiver") == 0) return TOOL_TYPE_ARCHIVERS;
  if (_stricmp(s, "linker") == 0) return TOOL_TYPE_LINKERS;
  return TOOL_TYPE_MISC;
}

static bool toolchain_tool_matches_filter(const tool* t, const toolchain_print_opts* opts) {
  if (!t)
    return false;
  if (!opts)
    return true;
  if (opts->has_type_filter && t->type != opts->type_filter)
    return false;
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

static void toolchain_fill(toolchain* tc) {
  if (!tc)
    return;

  print("Discovering host tools...");

  for (size_t i = 0; i < sizeof(TOOL_DISCOVER_STRATS) / sizeof(TOOL_DISCOVER_STRATS[0]); ++i)
    toolchain_discover_tool(tc, &TOOL_DISCOVER_STRATS[i]);

  print("Discovering Visual Studio tool bins...");
  toolchain_discover_vs_tool_bins(tc);

  print("Discovering SDKs...");

  for (size_t i = 0; i < sizeof(SDK_DISCOVER_STRATS) / sizeof(SDK_DISCOVER_STRATS[0]); ++i)
    toolchain_discover_sdk(tc, &SDK_DISCOVER_STRATS[i]);

  toolchain_sort_tools(tc);
  toolchain_sort_sdks(tc);

  print("Discovery completed: %d tools, %d sdks.", tc->tool_c, tc->sdk_c);
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

  int tool_count = 0;
  int sdk_count = 0;
  for (int i = 0; i < tc->tool_c; ++i)
    tool_count += toolchain_tool_matches_filter(&tc->tools[i], opts) ? 1 : 0;
  for (int i = 0; i < tc->sdk_c; ++i)
    sdk_count += toolchain_sdk_matches_filter(&tc->sdks[i], opts) ? 1 : 0;

  if (opts->show_tools) {
    if (!opts->minimal)
      print("Tools (%d):", tool_count);
    for (int i = 0; i < tc->tool_c; ++i) {
      tool* t = &tc->tools[i];
      if (!toolchain_tool_matches_filter(t, opts))
        continue;

      const char* ty = tool_type_to_idf(t->type);
      const char* id = t->id ? t->id : toolchain_path_basename(t->path ? t->path : "");
      const char* pa = t->path ? t->path : "";
      const char* ve = t->version ? t->version : "unknown";
      if (opts->paths_only)
        print("%s", pa);
      else if (opts->versions_only)
        print("%s", ve);
      else if (opts->minimal)
        print("%s|%s|%s|%s", ty, id ? id : "", pa, ve);
      else
        print("  - %s | %s | %s | %s", ty, id ? id : "", pa, ve);
    }
  }

  if (opts->show_sdks) {
    if (!opts->minimal)
      print("SDKs (%d):", sdk_count);
    for (int i = 0; i < tc->sdk_c; ++i) {
      sdk* s = &tc->sdks[i];
      if (!toolchain_sdk_matches_filter(s, opts))
        continue;

      if (opts->paths_only) {
        print("%s", s->base_path ? s->base_path : "");
        continue;
      }

      if (opts->versions_only) {
        print("%s", s->version ? s->version : "unknown");
        continue;
      }

      if (opts->minimal) {
        print("sdk|%s|%s|%s", s->name ? s->name : "<unnamed>", s->base_path ? s->base_path : "", s->version ? s->version : "unknown");
        continue;
      }

      print("  - %s", s->name ? s->name : "<unnamed>");
      if (s->version && s->version[0]) print("      version: %s", s->version);
      if (s->base_path) print("      base: %s", s->base_path);
      if (s->inc_path) print("      inc : %s", s->inc_path);
      if (s->src_path) print("      src : %s", s->src_path);
      if (s->lib_path) print("      lib : %s", s->lib_path);
      if (s->bin_path) print("      bin : %s", s->bin_path);
    }
  }
}

static void toolchain_print(toolchain* tc) {
  toolchain_print_opts opts = {.show_tools = true, .show_sdks = true};
  toolchain_print_with_opts(tc, &opts);
}

static toolchain* toolchain_read(node* tree) {
  toolchain* tc = push(sizeof(toolchain));
  memset(tc, 0, sizeof(toolchain));

  node* tools_n = node_get_child(tree, "tools");
  if (tools_n) {
    node_foreach(tools_n, it) {
      if (tc->tool_c >= TOOL_ARRAY_DIM)
        break;

      node* id_n = node_get_child(it, "id");
      node* type_n = node_get_child(it, "type");
      node* path_n = node_get_child(it, "path");
      node* ver_n = node_get_child(it, "version");
      if (!path_n)
        continue;

      const char* path = toolchain_norm_path(node_get_str(path_n));
      const char* id = id_n ? node_get_str(id_n) : toolchain_path_basename(path);
      toolchain_upsert_tool(tc, id, tool_type_from_idf(type_n ? node_get_idf(type_n) : NULL), path, ver_n ? node_get_str(ver_n) : NULL);
    }
  }

  node* sdks_n = node_get_child(tree, "sdks");
  if (sdks_n) {
    node_foreach(sdks_n, it) {
      if (tc->sdk_c >= SDK_ARRAY_DIM)
        break;

      sdk* s = &tc->sdks[tc->sdk_c++];
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

  toolchain_sort_tools(tc);
  toolchain_sort_sdks(tc);
  return tc;
}

static node* toolchain_write(toolchain* tc) {
  node* tree = node_alloc();
  if (!tree)
    return NULL;

  node* tools_n = node_create_named("tools");
  if (tools_n)
    toolchain_push_child_back(tree, tools_n);

  for (int i = 0; i < tc->tool_c; ++i) {
    node* t = node_create_named("tool");
    if (!t)
      continue;

    if (tc->tools[i].id)
      toolchain_push_child_back(t, node_create_str("id", tc->tools[i].id));
    toolchain_push_child_back(t, node_create_idf("type", tool_type_to_idf(tc->tools[i].type)));
    toolchain_push_child_back(t, node_create_str("path", toolchain_norm_path(tc->tools[i].path ? tc->tools[i].path : "")));
    if (tc->tools[i].version && tc->tools[i].version[0])
      toolchain_push_child_back(t, node_create_str("version", tc->tools[i].version));
    toolchain_push_child_back(tools_n, t);
  }

  node* sdks_n = node_create_named("sdks");
  if (sdks_n)
    toolchain_push_child_back(tree, sdks_n);

  for (int i = 0; i < tc->sdk_c; ++i) {
    node* s = node_create_named("sdk");
    if (!s)
      continue;

    if (tc->sdks[i].name)
      toolchain_push_child_back(s, node_create_str("name", tc->sdks[i].name));
    if (tc->sdks[i].version)
      toolchain_push_child_back(s, node_create_str("version", tc->sdks[i].version));
    if (tc->sdks[i].base_path)
      toolchain_push_child_back(s, node_create_str("base_path", toolchain_norm_path(tc->sdks[i].base_path)));
    if (tc->sdks[i].inc_path)
      toolchain_push_child_back(s, node_create_str("inc_path", toolchain_norm_path(tc->sdks[i].inc_path)));
    if (tc->sdks[i].src_path)
      toolchain_push_child_back(s, node_create_str("src_path", toolchain_norm_path(tc->sdks[i].src_path)));
    if (tc->sdks[i].lib_path)
      toolchain_push_child_back(s, node_create_str("lib_path", toolchain_norm_path(tc->sdks[i].lib_path)));
    if (tc->sdks[i].bin_path)
      toolchain_push_child_back(s, node_create_str("bin_path", toolchain_norm_path(tc->sdks[i].bin_path)));
    toolchain_push_child_back(sdks_n, s);
  }

  return tree;
}

static toolchain* toolchain_init(const char* path, bool reinit, cmd_ctx* cmdctx) {
  (void)cmdctx;
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

    return toolchain_read(tree);
  }

  print("Generating toolchain...");
  file_delete(path);
  toolchain* tc = push(sizeof(toolchain));
  memset(tc, 0, sizeof(toolchain));
  toolchain_fill(tc);
  node* tree = toolchain_write(tc);
  const char* str = node_write(tree);
  write_entire_file(path, str);
  return tc;
}
