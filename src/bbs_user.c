#pragma once
#include "bbs_user.h"

static user g_user = {0};
static bool g_user_initialized = false;

static bool user_init_paths(const char* user_path, const char* local_path) {
  (void)user_path;
  (void)local_path;
  memset(&g_user, 0, sizeof(g_user));
  g_user_initialized = true;
  return true;
}

static bool user_init(void) {
  return user_init_paths(get_path_exe(USER_FILENAME), get_path_cwd(LOCAL_FILENAME));
}

static const user* user_get(void) {
  if (!g_user_initialized)
    return NULL;
  return &g_user;
}
