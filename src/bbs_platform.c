#pragma once
#include "bbs_platform.h"

static const char* platform_host_os_name(void) {
#if defined(_WIN32)
  return "windows";
#elif defined(__APPLE__)
  return "macos";
#else
  return "linux";
#endif
}

static const char* platform_host_arch_name(void) {
#if defined(_M_ARM64) || defined(__aarch64__)
  return "arm64";
#elif defined(_M_IX86) || defined(__i386__)
  return "x86";
#else
  return "x86_64";
#endif
}

static const char* platform_path_cwd(const char* filename, platform_alloc_fn alloc_fn) {
  if (!filename || !alloc_fn)
    return NULL;

  char cwd[_MAX_PATH] = {0};
  if (!_getcwd(cwd, sizeof(cwd)))
    cwd[0] = '\0';

  size_t len = strlen(cwd) + 1 + strlen(filename) + 1;
  char* out = alloc_fn(len);
  if (!out)
    return NULL;

#if defined(_WIN32)
  snprintf(out, len, "%s\\%s", cwd, filename);
#else
  snprintf(out, len, "%s/%s", cwd, filename);
#endif
  return out;
}

static const char* platform_path_exe(const char* filename, platform_alloc_fn alloc_fn) {
  if (!filename || !alloc_fn)
    return NULL;

  char exe_path[_MAX_PATH] = {0};
  char exe_dir[_MAX_PATH] = {0};
  bool got_path = false;

#if defined(_WIN32)
  if (GetModuleFileNameA(NULL, exe_path, sizeof(exe_path)))
    got_path = true;
#elif defined(__linux__)
  ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
  if (len != -1) {
    exe_path[len] = '\0';
    got_path = true;
  }
#elif defined(__APPLE__)
  uint32_t size = sizeof(exe_path);
  if (_NSGetExecutablePath(exe_path, &size) == 0) {
    char* resolved = realpath(exe_path, NULL);
    if (resolved) {
      strncpy(exe_path, resolved, sizeof(exe_path) - 1);
      exe_path[sizeof(exe_path) - 1] = '\0';
      free(resolved);
    }
    got_path = true;
  }
#endif

  if (got_path) {
    snprintf(exe_dir, sizeof(exe_dir), "%s", exe_path);
    char* last_slash = strrchr(exe_dir, '\\');
    if (!last_slash)
      last_slash = strrchr(exe_dir, '/');
    if (last_slash)
      *last_slash = '\0';
  }

  size_t len = strlen(exe_dir) + 1 + strlen(filename) + 1;
  char* out = alloc_fn(len);
  if (!out)
    return NULL;

#if defined(_WIN32)
  snprintf(out, len, "%s\\%s", exe_dir, filename);
#else
  snprintf(out, len, "%s/%s", exe_dir, filename);
#endif
  return out;
}

static bool platform_file_exists(const char* path) {
  return path && access(path, F_OK) == 0;
}

static bool platform_file_delete(const char* path) {
  if (!path || !path[0])
    return false;

#if defined(_WIN32)
  return DeleteFileA(path) != 0;
#else
  return unlink(path) == 0;
#endif
}

static platform_timestamp platform_file_timestamp(const char* path) {
  if (!path || !path[0])
    return 0;

#if defined(_WIN32)
  WIN32_FILE_ATTRIBUTE_DATA data;
  if (!GetFileAttributesExA(path, GetFileExInfoStandard, &data))
    return 0;
  ULARGE_INTEGER ts;
  ts.LowPart = data.ftLastWriteTime.dwLowDateTime;
  ts.HighPart = data.ftLastWriteTime.dwHighDateTime;
  return (platform_timestamp)ts.QuadPart;
#else
  struct stat st;
  if (stat(path, &st) != 0)
    return 0;
#  if defined(__APPLE__)
  return (platform_timestamp)st.st_mtimespec.tv_sec * 1000000000ULL + (platform_timestamp)st.st_mtimespec.tv_nsec;
#  else
  return (platform_timestamp)st.st_mtim.tv_sec * 1000000000ULL + (platform_timestamp)st.st_mtim.tv_nsec;
#  endif
#endif
}

static platform_timestamp platform_now_ms(void) {
#if defined(_WIN32)
  return (platform_timestamp)GetTickCount64();
#else
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    return 0;
  return (platform_timestamp)ts.tv_sec * 1000ULL + (platform_timestamp)(ts.tv_nsec / 1000000ULL);
#endif
}

static void platform_sleep_ms(unsigned int ms) {
#if defined(_WIN32)
  Sleep(ms);
#else
  usleep((useconds_t)ms * 1000U);
#endif
}

static bool platform_dir_exists(const char* path) {
  if (!path || !path[0])
    return false;

#if defined(_WIN32)
  DWORD attr = GetFileAttributesA(path);
  return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
  struct stat st;
  return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

static bool platform_dir_create(const char* path) {
  if (!path || !path[0])
    return false;

#if defined(_WIN32)
  return CreateDirectoryA(path, NULL) != 0;
#else
  return mkdir(path, 0755) == 0;
#endif
}

static bool platform_dir_delete(const char* path) {
  if (!path || !path[0])
    return false;

#if defined(_WIN32)
  char search_path[_MAX_PATH] = {0};
  snprintf(search_path, sizeof(search_path), "%s\\*", path);

  WIN32_FIND_DATAA ffd;
  HANDLE hfind = FindFirstFileA(search_path, &ffd);
  if (hfind == INVALID_HANDLE_VALUE)
    return false;

  do {
    if (_stricmp(ffd.cFileName, ".") == 0 || _stricmp(ffd.cFileName, "..") == 0)
      continue;

    char full_path[_MAX_PATH] = {0};
    snprintf(full_path, sizeof(full_path), "%s\\%s", path, ffd.cFileName);

    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      platform_dir_delete(full_path);
    else
      DeleteFileA(full_path);
  } while (FindNextFileA(hfind, &ffd) != 0);

  FindClose(hfind);
  return RemoveDirectoryA(path) != 0;
#else
  DIR* dir = opendir(path);
  if (!dir)
    return false;

  struct dirent* entry = NULL;
  while ((entry = readdir(dir)) != NULL) {
    if (_stricmp(entry->d_name, ".") == 0 || _stricmp(entry->d_name, "..") == 0)
      continue;

    char full_path[_MAX_PATH] = {0};
    snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

    struct stat st;
    if (stat(full_path, &st) != 0)
      continue;

    if (S_ISDIR(st.st_mode))
      platform_dir_delete(full_path);
    else
      unlink(full_path);
  }

  closedir(dir);
  return rmdir(path) == 0;
#endif
}

static FILE* platform_popen_read(const char* cmd) {
  if (!cmd || !cmd[0])
    return NULL;

#if defined(_WIN32)
  return _popen(cmd, "r");
#else
  return popen(cmd, "r");
#endif
}

static int platform_pclose_read(FILE* pipe) {
  if (!pipe)
    return -1;

#if defined(_WIN32)
  return _pclose(pipe);
#else
  return pclose(pipe);
#endif
}

static bool platform_supports_wsl(void) {
#if defined(_WIN32)
  return true;
#else
  return false;
#endif
}

static const char* platform_wsl_distro_query_command(void) {
#if defined(_WIN32)
  return "reg query HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Lxss /s /v DistributionName 2>nul";
#else
  return NULL;
#endif
}

static bool platform_is_usable_bash_path(const char* path) {
  if (!path || !path[0])
    return false;

#if defined(_WIN32)
  size_t len = strlen(path);
  char* norm = (char*)malloc(len + 1);
  if (!norm)
    return false;

  for (size_t i = 0; i < len; ++i)
    norm[i] = path[i] == '\\' ? '/' : path[i];
  norm[len] = '\0';

  bool usable = true;
  if (_stricmp(norm, "C:/Windows/System32/bash.exe") == 0)
    usable = false;

  if (usable) {
    const char* suffix = "/microsoft/windowsapps/bash.exe";
    size_t suffix_len = strlen(suffix);
    if (len >= suffix_len && _stricmp(norm + len - suffix_len, suffix) == 0)
      usable = false;
  }

  free(norm);
  return usable;
#else
  return true;
#endif
}

static bool platform_executable_name_matches(const char* base, const char* exe_name) {
  if (!base || !base[0] || !exe_name || !exe_name[0])
    return false;

#if defined(_WIN32)
  if (_stricmp(base, exe_name) == 0)
    return strchr(exe_name, '.') != NULL;

  if (!strchr(exe_name, '.')) {
    char cand[128] = {0};
    snprintf(cand, sizeof(cand), "%s.exe", exe_name);
    return _stricmp(base, cand) == 0;
  }
  return false;
#else
  return _stricmp(base, exe_name) == 0;
#endif
}

static int platform_executable_candidates(const char* exe_name, const char** candidates, char storage[][128], int max_candidates) {
  if (!exe_name || !exe_name[0] || !candidates || !storage || max_candidates <= 0)
    return 0;

  int count = 0;
  snprintf(storage[count], 128, "%s", exe_name);
  candidates[count] = storage[count];
  ++count;

#if defined(_WIN32)
  if (!strchr(exe_name, '.') && count < max_candidates) {
    snprintf(storage[count], 128, "%s.exe", exe_name);
    candidates[count] = storage[count];
    ++count;
  }
#endif

  return count;
}

static bool platform_build_find_command(char* out, size_t out_dim, const char* name, bool all_matches) {
  if (!out || out_dim == 0 || !name || !name[0])
    return false;

#if defined(_WIN32)
  snprintf(out, out_dim, "where %s 2>nul", name);
#else
  snprintf(out, out_dim, all_matches ? "which -a %s 2>/dev/null" : "which %s 2>/dev/null", name);
#endif
  return true;
}

static bool platform_build_recursive_find_command(char* out, size_t out_dim, const char* root, const char* exe_name) {
  if (!out || out_dim == 0 || !root || !root[0] || !exe_name || !exe_name[0])
    return false;

#if defined(_WIN32)
  snprintf(out, out_dim, "where /r \"%s\" %s 2>nul", root, exe_name);
#else
  snprintf(out, out_dim, "find \"%s\" -type f \\( -name '%s' -o -name '%s.exe' \\) 2>/dev/null", root, exe_name, exe_name);
#endif
  return true;
}

static bool platform_build_pattern_match_command(char* out, size_t out_dim, const char* pattern, bool dirs_only) {
  if (!out || out_dim == 0 || !pattern || !pattern[0])
    return false;

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
    snprintf(out, out_dim, "dir /s /b \"%s\" 2>nul", win_pat);
  } else {
    snprintf(out, out_dim, dirs_only ? "for /d %%i in (\"%s\") do @echo %%~fi" : "for %%i in (\"%s\") do @echo %%~fi", pattern);
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

    snprintf(out, out_dim, "find \"%s\" %s-path '%s' 2>/dev/null", root, dirs_only ? "-type d " : "", pattern);
  } else {
    snprintf(out, out_dim, "ls -d %s 2>/dev/null", pattern);
  }
#endif
  return true;
}

static bool platform_build_file_version_command(char* out, size_t out_dim, const char* path) {
  if (!out || out_dim == 0 || !path || !path[0])
    return false;

#if defined(_WIN32)
  snprintf(out, out_dim, "powershell -NoProfile -Command \"(Get-Item '%s').VersionInfo.FileVersion\" 2>nul", path);
  return true;
#else
  (void)path;
  return false;
#endif
}

static bool platform_find_vswhere(char* out, size_t out_dim) {
  if (!out || out_dim == 0)
    return false;

#if defined(_WIN32)
  const char* pf86 = getenv("ProgramFiles(x86)");
  if (!pf86 || !pf86[0])
    return false;

  snprintf(out, out_dim, "%s\\Microsoft Visual Studio\\Installer\\vswhere.exe", pf86);
  return true;
#else
  return false;
#endif
}

static bool platform_build_vswhere_install_command(char* out, size_t out_dim, const char* vswhere_path) {
  if (!out || out_dim == 0 || !vswhere_path || !vswhere_path[0])
    return false;

#if defined(_WIN32)
  snprintf(out, out_dim, "\"%s\" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>nul", vswhere_path);
  return true;
#else
  return false;
#endif
}

static bool platform_build_dir_list_command(char* out, size_t out_dim, const char* path) {
  if (!out || out_dim == 0 || !path || !path[0])
    return false;

#if defined(_WIN32)
  snprintf(out, out_dim, "dir /b /ad \"%s\" 2>nul", path);
#else
  snprintf(out, out_dim, "ls -1 \"%s\" 2>/dev/null", path);
#endif
  return true;
}

static char* platform_quote_windows_arg(const char* arg) {
  const char* src = arg ? arg : "";
  size_t len = strlen(src);
  size_t max_len = len * 2 + 3;
  char* out = (char*)malloc(max_len);
  if (!out)
    return NULL;

  size_t wi = 0;
  out[wi++] = '"';
  size_t backslashes = 0;
  for (size_t i = 0; i < len; ++i) {
    char ch = src[i];
    if (ch == '\\') {
      ++backslashes;
      continue;
    }

    if (ch == '"') {
      while (backslashes > 0) {
        out[wi++] = '\\';
        --backslashes;
      }
      out[wi++] = '\\';
      out[wi++] = '"';
      backslashes = 0;
      continue;
    }

    while (backslashes > 0) {
      out[wi++] = '\\';
      --backslashes;
    }
    backslashes = 0;
    out[wi++] = ch;
  }

  while (backslashes > 0) {
    out[wi++] = '\\';
    out[wi++] = '\\';
    --backslashes;
  }

  out[wi++] = '"';
  out[wi] = '\0';
  return out;
}

static int platform_run_bash(const char* bash_path, const char* workdir, const char* script) {
  if (!bash_path || !bash_path[0] || !script || !script[0])
    return -1;

#if defined(_WIN32)
  char* quoted_exe = platform_quote_windows_arg(bash_path);
  char* quoted_script = platform_quote_windows_arg(script);
  if (!quoted_exe || !quoted_script) {
    free(quoted_exe);
    free(quoted_script);
    return -1;
  }

  size_t cmd_len = strlen(quoted_exe) + strlen(" -lc ") + strlen(quoted_script) + 1;
  char* cmdline = (char*)malloc(cmd_len);
  if (!cmdline) {
    free(quoted_exe);
    free(quoted_script);
    return -1;
  }

  snprintf(cmdline, cmd_len, "%s -lc %s", quoted_exe, quoted_script);

  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  memset(&si, 0, sizeof(si));
  memset(&pi, 0, sizeof(pi));
  si.cb = sizeof(si);

  BOOL ok = CreateProcessA(bash_path,
                           cmdline,
                           NULL,
                           NULL,
                           TRUE,
                           0,
                           NULL,
                           workdir && workdir[0] ? workdir : NULL,
                           &si,
                           &pi);

  free(quoted_exe);
  free(quoted_script);
  free(cmdline);

  if (!ok)
    return -(int)GetLastError();

  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD exit_code = 0;
  if (!GetExitCodeProcess(pi.hProcess, &exit_code))
    exit_code = 1;

  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  return (int)exit_code;
#else
  pid_t pid = fork();
  if (pid < 0)
    return -errno;

  if (pid == 0) {
    if (workdir && workdir[0] && chdir(workdir) != 0)
      _exit(126);
    execl(bash_path, bash_path, "-lc", script, (char*)NULL);
    _exit(errno == ENOENT ? 127 : 126);
  }

  int status = 0;
  while (waitpid(pid, &status, 0) < 0) {
    if (errno != EINTR)
      return -errno;
  }

  if (WIFEXITED(status))
    return WEXITSTATUS(status);
  if (WIFSIGNALED(status))
    return 128 + WTERMSIG(status);
  return 1;
#endif
}
