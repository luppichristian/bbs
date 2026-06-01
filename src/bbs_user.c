#pragma once
#include "bbs_user.h"

static user* user_init(cmd_ctx* ctx) {
  user* u = push(sizeof(user));
  return u;
}
