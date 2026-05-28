#include "bbs_base.h"

static arena garena = {0};
static void* push(size_t sz) {
  if (sz == 0)
    return NULL;

  if (!garena.blk || garena.curr + sz > garena.blk->size) {
    size_t blk_size = (sz > ARENA_DIM) ? sz : ARENA_DIM;
    arena_blk* blk = (arena_blk*)malloc(sizeof(*blk));
    if (!blk)
      return NULL;

    blk->buff = malloc(blk_size);
    if (!blk->buff) {
      free(blk);
      return NULL;
    }

    blk->size = blk_size;
    blk->used = 0;
    blk->prev = garena.blk;
    blk->next = NULL;
    if (garena.blk)
      garena.blk->next = blk;

    garena.blk = blk;
    garena.curr = 0;
  }

  void* out = (uint8_t*)garena.blk->buff + garena.blk->used;
  garena.blk->used += sz;
  garena.curr = garena.blk->used;
  return out;
}
static void pop(size_t sz) {
  while (garena.blk && sz > 0) {
    if (sz < garena.blk->used) {
      garena.blk->used -= sz;
      garena.curr = garena.blk->used;
      return;
    }

    sz -= garena.blk->used;
    arena_blk* prev = garena.blk->prev;
    free(garena.blk->buff);
    free(garena.blk);
    garena.blk = prev;
    garena.curr = garena.blk ? garena.blk->used : 0;
  }

  if (!garena.blk)
    garena.curr = 0;
}
static void release() {
  while (garena.blk) {
    arena_blk* prev = garena.blk->prev;
    free(garena.blk->buff);
    free(garena.blk);
    garena.blk = prev;
  }

  garena.curr = 0;
}

static void print(const char* fmt, ...) {
  va_list list;
  va_start(list, fmt);
  vfprintf(stdout, fmt, list);
  fputc('\n', stdout);
  va_end(list);
}
static void error(const char* fmt, ...) {
  va_list list;
  va_start(list, fmt);
  fputs("Error: ", stderr);
  vfprintf(stderr, fmt, list);
  fputc('\n', stderr);
  va_end(list);
}
static void warn(const char* fmt, ...) {
  va_list list;
  va_start(list, fmt);
  fputs("Warn: ", stderr);
  vfprintf(stderr, fmt, list);
  fputc('\n', stderr);
  va_end(list);
}

static bool cmdline_empty(cmdline* cl) {
  return cl->argc <= 0;
}

static const char* cmdline_peek(cmdline* cl) {
  return (cl->argc > 0) ? cl->argv[0] : NULL;
}

static const char* cmdline_pop(cmdline* cl) {
  if (cl->argc <= 0)
    return NULL;

  const char* s = cl->argv[0];
  cl->argc--;
  cl->argv++;
  return s;
}

static bool cmdline_is_shortopt(const char* s) {
  return s && s[0] == '-' && s[1] && s[1] != '-';
}

static bool cmdline_is_longopt(const char* s) {
  return s && s[0] == '-' && s[1] == '-' && s[2];
}

static void cmdline_options(cmdline* cl, cmdopt* opts, size_t opt_count, bool warnings) {
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
      bool recognized = false;
      const char* eq = strchr(name, '=');
      if (eq) {
        size_t len = (size_t)(eq - name);
        if (len >= sizeof(buffer)) {
          len = sizeof(buffer) - 1;
        }

        memcpy(buffer, name, len);
        buffer[len] = '\0';
        name = buffer;
        value = eq + 1;
      }

      for (size_t i = 0; i < opt_count; ++i) {
        if (opts[i].long_name && strcmp(opts[i].long_name, name) == 0) {
          opts[i].present = true;
          opts[i].value = value;
          recognized = true;
          break;
        }
      }

      if (!recognized) {
        warn("ignoring unrecognized option '%s'.", arg);
      }

      cmdline_pop(cl);
      continue;
    }

    if (cmdline_is_shortopt(arg)) {
      const char* p = arg + 1;

      while (*p) {
        char name[2] = {*p, 0};
        bool recognized = false;
        for (size_t i = 0; i < opt_count; ++i) {
          if (opts[i].short_name && strcmp(opts[i].short_name, name) == 0) {
            opts[i].present = true;
            recognized = true;
            break;
          }
        }

        if ((!recognized) && warnings) {
          warn("ignoring unrecognized option '-%c'.", *p);
        }

        ++p;
      }

      cmdline_pop(cl);
      continue;
    }

    break;
  }
}

static const char* attr_get_str(attr* a) {
  if (a->type != ATTR_TYPE_STR)
    return 0;
  return a->_str;
}
static const char* attr_get_idf(attr* a) {
  if (a->type != ATTR_TYPE_IDF)
    return 0;
  return a->_idf;
}
static int64_t attr_get_int(attr* a) {
  if (a->type != ATTR_TYPE_INT)
    return 0;
  return a->_int;
}
static double attr_get_flt(attr* a) {
  if (a->type != ATTR_TYPE_FLT)
    return 0.0;
  return a->_flt;
}
static ver attr_get_ver(attr* a) {
  ver v = {0};
  if (a->type != ATTR_TYPE_VER)
    return v;
  return a->_ver;
}
static bool attr_check_type(attr* a, attr_type type) {
  return a->type == type;
}

#define attr_foreach(attrib, child) for (attr* child = attrib->children; child; child = child->next)
static attr* attr_get_child(attr* a, const char* name) {
  attr_foreach(a, child) {
    if (_strcmpi(child->name, name) == 0) {
      return child;
    }
  }

  return NULL;
}

static void attr_push_child(attr* a, attr* child) {
  child->next = a->children;
  a->children = child;
}

static attr* attr_parse(const char* str) {
  // TODO: Implement
}

static const char* attr_write(attr* attr) {
  // TODO: Implement
}
