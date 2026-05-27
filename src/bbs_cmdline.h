#pragma once

#include "bbs_base.h"

typedef struct {
  const char** argv;
  int argc;
} cmdline;

typedef struct {
  const char* short_name;  // "a"
  const char* long_name;   // "all"
  bool present;
  const char* value;
} cmdopt;

static inline bool cmdline_empty(cmdline* cl) {
  return cl->argc <= 0;
}

static inline const char* cmdline_peek(cmdline* cl) {
  return (cl->argc > 0) ? cl->argv[0] : NULL;
}

static inline const char* cmdline_pop(cmdline* cl) {
  if (cl->argc <= 0)
    return NULL;

  const char* s = cl->argv[0];
  cl->argc--;
  cl->argv++;
  return s;
}

static inline bool cmdline_is_shortopt(const char* s) {
  return s && s[0] == '-' && s[1] && s[1] != '-';
}

static inline bool cmdline_is_longopt(const char* s) {
  return s && s[0] == '-' && s[1] == '-' && s[2];
}

static inline void cmdline_parse_options(
    cmdline* cl,
    cmdopt* opts,
    size_t opt_count) {
  while (!cmdline_empty(cl)) {
    const char* arg = cmdline_peek(cl);

    if (strcmp(arg, "--") == 0) {
      cmdline_pop(cl);
      break;
    }

    if (cmdline_is_longopt(arg)) {
      const char* name = arg + 2;
      const char* value = NULL;

      char buffer[256];
      const char* eq = strchr(name, '=');

      if (eq) {
        size_t len = (size_t)(eq - name);

        if (len >= sizeof(buffer))
          len = sizeof(buffer) - 1;

        memcpy(buffer, name, len);
        buffer[len] = 0;

        name = buffer;
        value = eq + 1;
      }

      for (size_t i = 0; i < opt_count; i++) {
        if (opts[i].long_name &&
            strcmp(opts[i].long_name, name) == 0) {
          opts[i].present = true;
          opts[i].value = value;
          break;
        }
      }

      cmdline_pop(cl);
      continue;
    }

    if (cmdline_is_shortopt(arg)) {
      const char* p = arg + 1;

      while (*p) {
        char name[2] = {*p, 0};

        for (size_t i = 0; i < opt_count; i++) {
          if (opts[i].short_name &&
              strcmp(opts[i].short_name, name) == 0) {
            opts[i].present = true;
            break;
          }
        }

        p++;
      }

      cmdline_pop(cl);
      continue;
    }

    break;
  }
}
