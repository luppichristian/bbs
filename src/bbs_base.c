#pragma once
#include "bbs_base.h"

#if defined(_WIN32)
#  include <io.h>
#  include <windows.h>
#  define access _access
#  define F_OK   0
#elif defined(__APPLE__)
#  include <mach-o/dyld.h>
#  include <unistd.h>
#else
#  include <unistd.h>
#endif

#if !defined(_WIN32) && !defined(_MAX_PATH)
#  define _MAX_PATH 4096
#endif

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
static void release(void) {
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

static const char* cmdline_consume_param(cmdline* cl) {
  while (!cmdline_empty(cl)) {
    const char* arg = cmdline_peek(cl);
    if (strcmp(arg, "--") == 0) {
      cmdline_pop(cl);
      continue;
    }
    if (cmdline_is_longopt(arg) || cmdline_is_shortopt(arg))
      return NULL;
    return cmdline_pop(cl);
  }
  return NULL;
}

static void cmdline_consume_options(cmdline* cl, cmdopt* opts, size_t count) {
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
        if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
        memcpy(buffer, name, len);
        buffer[len] = '\0';
        name = buffer;
        value = eq + 1;
      }

      for (size_t i = 0; i < count; ++i) {
        if (opts[i].long_name && strcmp(opts[i].long_name, name) == 0) {
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
        for (size_t i = 0; i < count; ++i) {
          if (opts[i].short_name && strcmp(opts[i].short_name, name) == 0) {
            opts[i].present = true;
            break;
          }
        }
        ++p;
      }

      cmdline_pop(cl);
      continue;
    }

    break;
  }
}

static void cmdline_consume_all_options(cmdline* cl, cmdopt* opts, size_t count) {
  int wi = 0;
  char** argv = cl->argv;

  for (int ri = 0; ri < cl->argc; ri++) {
    const char* arg = argv[ri];

    if (strcmp(arg, "--") == 0) {
      argv[wi++] = argv[ri];
      ri++;
      while (ri < cl->argc) argv[wi++] = argv[ri++];
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
        if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
        memcpy(buffer, name, len);
        buffer[len] = '\0';
        name = buffer;
        value = eq + 1;
      }

      for (size_t i = 0; i < count; ++i) {
        if (opts[i].long_name && strcmp(opts[i].long_name, name) == 0) {
          opts[i].present = true;
          opts[i].value = value;
          recognized = true;
          break;
        }
      }

      if (!recognized) argv[wi++] = argv[ri];
      continue;
    }

    if (cmdline_is_shortopt(arg)) {
      const char* p = arg + 1;
      bool any_matched = false;
      while (*p) {
        char name[2] = {*p, 0};
        for (size_t i = 0; i < count; ++i) {
          if (opts[i].short_name && strcmp(opts[i].short_name, name) == 0) {
            opts[i].present = true;
            any_matched = true;
            break;
          }
        }
        ++p;
      }

      if (!any_matched) argv[wi++] = argv[ri];
      continue;
    }

    argv[wi++] = argv[ri];
  }

  cl->argc = wi;
}

static void cmdline_validate(cmdline* cl) {
  for (int i = 0; i < cl->argc; i++) {
    const char* a = cl->argv[i];
    if (!a) continue;
    if (a[0] == '-') {
      warn("ignoring unrecognized option '%s'.", a);
    } else {
      warn("ignoring unexpected parameter '%s'.", a);
    }
  }
}

static const char* node_get_str(node* a) {
  if (a->type != NODE_TYPE_STR)
    return 0;
  return a->_str;
}
static const char* node_get_idf(node* a) {
  if (a->type != NODE_TYPE_IDF)
    return 0;
  return a->_idf;
}
static int64_t node_get_int(node* a) {
  if (a->type != NODE_TYPE_INT)
    return 0;
  return a->_int;
}
static double node_get_flt(node* a) {
  if (a->type != NODE_TYPE_FLT)
    return 0.0;
  return a->_flt;
}
static ver node_get_ver(node* a) {
  ver v = {0};
  if (a->type != NODE_TYPE_VER)
    return v;
  return a->_ver;
}
static bool node_get_bool(node* a) {
  if (a->type != NODE_TYPE_BOL)
    return false;
  return a->_bol;
}
static bool node_check_type(node* a, node_type type) {
  return a->type == type;
}

#define node_foreach(parent, child) for (node* child = parent->children; child; child = child->next)
static node* node_get_child(node* a, const char* name) {
  node_foreach(a, child) {
    if (child->name && _strnicmp(child->name, name, child->name_dim) == 0 && name[child->name_dim] == '\0') {
      return child;
    }
  }

  return NULL;
}

static void node_push_child(node* a, node* child) {
  child->next = a->children;
  a->children = child;
}

typedef enum {
  TOK_EOF = 0,
  TOK_NAME,
  TOK_STRING,
  TOK_LPAREN,
  TOK_RPAREN,
} tok_type;

typedef struct {
  tok_type type;
  const char* begin;
  const char* end;
} token;

typedef struct {
  const char* src;
  const char* cur;
  bool has_look;
  token look;
} parser;

static size_t token_len(token t) {
  return (size_t)(t.end - t.begin);
}

static const char* arena_text(const char* begin, size_t len) {
  char* out = push(len + 1);
  if (!out)
    return NULL;

  memcpy(out, begin, len);
  out[len] = '\0';
  return out;
}

static bool token_all_digits(const char* begin, const char* end) {
  if (begin >= end)
    return false;

  for (const char* p = begin; p < end; ++p) {
    if (!isdigit((unsigned char)*p))
      return false;
  }

  return true;
}

static bool token_is_int(token t) {
  if (t.type != TOK_NAME)
    return false;

  const char* begin = t.begin;
  const char* end = t.end;
  if (begin < end && (*begin == '+' || *begin == '-'))
    ++begin;

  return token_all_digits(begin, end);
}

static bool token_is_float(token t) {
  if (t.type != TOK_NAME)
    return false;

  const char* dot = NULL;
  for (const char* p = t.begin; p < t.end; ++p) {
    if (*p == '.') {
      if (dot)
        return false;
      dot = p;
    }
  }

  if (!dot)
    return false;

  const char* begin = t.begin;
  const char* end = t.end;
  if (begin < end && (*begin == '+' || *begin == '-'))
    ++begin;

  if (dot == begin || dot + 1 == end)
    return false;

  return token_all_digits(begin, dot) && token_all_digits(dot + 1, end);
}

static bool token_is_version(token t) {
  if (t.type != TOK_NAME)
    return false;

  size_t dots = 0;
  const char* segment = t.begin;
  for (const char* p = t.begin; p <= t.end; ++p) {
    if (p == t.end || *p == '.') {
      if (!token_all_digits(segment, p))
        return false;
      if (p != t.end) {
        ++dots;
        segment = p + 1;
      }
    }
  }

  return dots >= 2;
}

static bool token_is_bool(token t) {
  if (t.type != TOK_NAME)
    return false;
  size_t len = token_len(t);
  if (len == 4 && _strnicmp(t.begin, "true", 4) == 0)
    return true;
  if (len == 5 && _strnicmp(t.begin, "false", 5) == 0)
    return true;
  if (len == 2 && _strnicmp(t.begin, "on", 2) == 0)
    return true;
  if (len == 3 && _strnicmp(t.begin, "off", 3) == 0)
    return true;
  return false;
}

static node_type token_to_type(token t) {
  switch (t.type) {
    case TOK_STRING:
      return NODE_TYPE_STR;
    case TOK_NAME:
      if (token_is_version(t))
        return NODE_TYPE_VER;
      if (token_is_int(t))
        return NODE_TYPE_INT;
      if (token_is_float(t))
        return NODE_TYPE_FLT;
      if (token_is_bool(t))
        return NODE_TYPE_BOL;
      return NODE_TYPE_IDF;
    default:
      return NODE_TYPE_DEF;
  }
}

static void parser_skip(parser* p) {
  for (;;) {
    while (*p->cur && isspace((unsigned char)*p->cur))
      ++p->cur;

    if (p->cur[0] == '/' && p->cur[1] == '/') {
      while (*p->cur && *p->cur != '\n' && *p->cur != '\r')
        ++p->cur;
      continue;
    }

    if (*p->cur == '#') {
      while (*p->cur && *p->cur != '\n' && *p->cur != '\r')
        ++p->cur;
      continue;
    }

    break;
  }
}

static token lexer_next(parser* p) {
  parser_skip(p);

  token t = {.type = TOK_EOF, .begin = p->cur, .end = p->cur};
  if (!*p->cur)
    return t;

  switch (*p->cur) {
    case '(':
      t.type = TOK_LPAREN;
      t.end = ++p->cur;
      return t;
    case ')':
      t.type = TOK_RPAREN;
      t.end = ++p->cur;
      return t;
    case '"': {
      ++p->cur;
      t.type = TOK_STRING;
      t.begin = p->cur;
      while (*p->cur) {
        if (*p->cur == '\\' && p->cur[1] != '\0') {
          p->cur += 2;
          continue;
        }
        if (*p->cur == '"')
          break;
        ++p->cur;
      }
      t.end = p->cur;
      if (*p->cur == '"')
        ++p->cur;
      return t;
    }
    default:
      break;
  }

  t.type = TOK_NAME;
  t.begin = p->cur;
  while (*p->cur && !isspace((unsigned char)*p->cur) && *p->cur != '(' && *p->cur != ')' && *p->cur != '"' && *p->cur != ',')
    ++p->cur;
  t.end = p->cur;
  return t;
}

static token parser_peek(parser* p) {
  if (!p->has_look) {
    p->look = lexer_next(p);
    p->has_look = true;
  }

  return p->look;
}

static token parser_next(parser* p) {
  if (p->has_look) {
    p->has_look = false;
    return p->look;
  }

  return lexer_next(p);
}

static node* node_alloc(void) {
  node* n = push(sizeof(*n));
  if (!n)
    return NULL;

  memset(n, 0, sizeof(*n));
  n->type = NODE_TYPE_DEF;
  return n;
}

static void node_assign_scalar(node* n, token t) {
  n->type = token_to_type(t);
  switch (n->type) {
    case NODE_TYPE_STR:
      n->_str = t.begin;
      break;
    case NODE_TYPE_INT: {
      char buf[64];
      size_t len = token_len(t);
      if (len >= sizeof(buf))
        len = sizeof(buf) - 1;
      memcpy(buf, t.begin, len);
      buf[len] = '\0';
      n->_int = (int64_t)strtoll(buf, NULL, 10);
      break;
    }
    case NODE_TYPE_FLT: {
      char buf[64];
      size_t len = token_len(t);
      if (len >= sizeof(buf))
        len = sizeof(buf) - 1;
      memcpy(buf, t.begin, len);
      buf[len] = '\0';
      n->_flt = strtod(buf, NULL);
      break;
    }
    case NODE_TYPE_VER: {
      ver v = {0};
      const char* part = t.begin;
      uint8_t* fields[] = {&v.major, &v.minor, &v.patch, &v.user};
      size_t idx = 0;

      for (const char* p = t.begin; p <= t.end && idx < _countof(fields); ++p) {
        if (p == t.end || *p == '.') {
          if (part < p) {
            char buf[16];
            size_t len = (size_t)(p - part);
            if (len >= sizeof(buf))
              len = sizeof(buf) - 1;
            memcpy(buf, part, len);
            buf[len] = '\0';
            *fields[idx++] = (uint8_t)strtoul(buf, NULL, 10);
          }
          part = p + 1;
        }
      }

      n->_ver = v;
      break;
    }
    case NODE_TYPE_IDF:
      n->_idf = t.begin;
      break;
    case NODE_TYPE_BOL: {
      size_t len = token_len(t);
      n->_bol = (len == 4 && _strnicmp(t.begin, "true", 4) == 0) ||
                (len == 2 && _strnicmp(t.begin, "on", 2) == 0);
      break;
    }
    default:
      break;
  }
}

static node* node_parse_item(parser* p);

static node* node_parse_named(parser* p, token name_tok) {
  node* n = node_alloc();
  if (!n)
    return NULL;

  n->name = name_tok.begin;
  n->name_dim = token_len(name_tok);

  token lp = parser_next(p);
  (void)lp;

  parser saved = *p;
  token first = parser_next(p);
  if (first.type == TOK_RPAREN) {
    return n;
  }

  token second = parser_peek(p);
  if (second.type == TOK_RPAREN && first.type != TOK_LPAREN && first.type != TOK_RPAREN) {
    node_assign_scalar(n, first);
    n->txt_offset = (size_t)(first.begin - p->src);
    n->txt_dim = token_len(first);
    parser_next(p);  // consume ')'
    return n;
  }

  *p = saved;
  n->type = NODE_TYPE_DEF;
  while (true) {
    token t = parser_peek(p);
    if (t.type == TOK_EOF) {
      break;
    }
    if (t.type == TOK_RPAREN) {
      parser_next(p);
      break;
    }

    node* child = node_parse_item(p);
    if (!child)
      break;

    child->parent = n;
    node_push_child(n, child);
  }

  return n;
}

static node* node_parse_item(parser* p) {
  token t = parser_next(p);
  if (t.type == TOK_EOF || t.type == TOK_RPAREN)
    return NULL;

  if (t.type == TOK_NAME) {
    token next = parser_peek(p);
    if (next.type == TOK_LPAREN)
      return node_parse_named(p, t);
  }

  node* n = node_alloc();
  if (!n)
    return NULL;

  node_assign_scalar(n, t);
  n->txt_offset = (size_t)(t.begin - p->src);
  n->txt_dim = token_len(t);
  return n;
}

// Tree is allocated with push()
static node* node_parse(const char* str) {
  parser p = {.src = str, .cur = str, .has_look = false};
  node* root = node_alloc();
  if (!root)
    return NULL;

  while (true) {
    token t = parser_peek(&p);
    if (t.type == TOK_EOF)
      break;

    node* child = node_parse_item(&p);
    if (!child)
      break;

    child->parent = root;
    node_push_child(root, child);
  }

  return root;
}

// Pop a parsed node tree
// Children list is prepend-ordered (reverse of allocation order),
// so iterating head-to-tail pops most-recently-allocated first.
static void node_pop(node* n) {
  if (!n)
    return;

  node* child = n->children;
  while (child) {
    node* next = child->next;
    node_pop(child);
    child = next;
  }
  pop(sizeof(*n));
}

static size_t node_plain_len(const node* n) {
  if (!n)
    return 0;

  switch (n->type) {
    case NODE_TYPE_STR:
      return n->txt_dim;
    case NODE_TYPE_INT: {
      char buf[32];
      return (size_t)snprintf(buf, sizeof(buf), "%lld", (long long)n->_int);
    }
    case NODE_TYPE_FLT: {
      char buf[64];
      return (size_t)snprintf(buf, sizeof(buf), "%g", n->_flt);
    }
    case NODE_TYPE_VER: {
      char buf[32];
      if (n->_ver.user != 0)
        return (size_t)snprintf(buf, sizeof(buf), "%u.%u.%u.%u", n->_ver.major, n->_ver.minor, n->_ver.patch, n->_ver.user);
      return (size_t)snprintf(buf, sizeof(buf), "%u.%u.%u", n->_ver.major, n->_ver.minor, n->_ver.patch);
    }
    case NODE_TYPE_IDF:
      return n->txt_dim;
    case NODE_TYPE_BOL:
      return n->_bol ? 4 : 5;
    default:
      if (n->name && !n->children)
        return n->name_dim;
      return 0;
  }
}

static void node_plain_write(const node* n, char* out) {
  switch (n->type) {
    case NODE_TYPE_STR:
      memcpy(out, n->_str, n->txt_dim);
      break;
    case NODE_TYPE_INT:
      sprintf(out, "%lld", (long long)n->_int);
      break;
    case NODE_TYPE_FLT:
      sprintf(out, "%g", n->_flt);
      break;
    case NODE_TYPE_VER:
      if (n->_ver.user != 0)
        sprintf(out, "%u.%u.%u.%u", n->_ver.major, n->_ver.minor, n->_ver.patch, n->_ver.user);
      else
        sprintf(out, "%u.%u.%u", n->_ver.major, n->_ver.minor, n->_ver.patch);
      break;
    case NODE_TYPE_IDF:
      memcpy(out, n->_idf, n->txt_dim);
      break;
    case NODE_TYPE_BOL:
      memcpy(out, n->_bol ? "true" : "false", n->_bol ? 4 : 5);
      break;
    default:
      if (n->name && !n->children)
        memcpy(out, n->name, n->name_dim);
      break;
  }
}

static size_t node_full_value_len(const node* n) {
  if (!n)
    return 0;

  if (n->type == NODE_TYPE_STR) {
    size_t len = 2;
    for (size_t i = 0; i < n->txt_dim; ++i) {
      switch (n->_str[i]) {
        case '\\':
        case '"':
        case '\n':
        case '\r':
        case '\t':
          len += 2;
          break;
        default:
          ++len;
          break;
      }
    }
    return len;
  }

  return node_plain_len(n);
}

static void node_full_value_write(const node* n, char* out) {
  if (n->type == NODE_TYPE_STR) {
    *out++ = '"';
    for (size_t i = 0; i < n->txt_dim; ++i) {
      switch (n->_str[i]) {
        case '\\':
        case '"':
          *out++ = '\\';
          *out++ = n->_str[i];
          break;
        case '\n':
          *out++ = '\\';
          *out++ = 'n';
          break;
        case '\r':
          *out++ = '\\';
          *out++ = 'r';
          break;
        case '\t':
          *out++ = '\\';
          *out++ = 't';
          break;
        default:
          *out++ = n->_str[i];
          break;
      }
    }
    *out++ = '"';
    return;
  }

  node_plain_write(n, out);
}

static size_t node_write_len(const node* n, size_t depth, bool top_level) {
  if (!n)
    return 0;

  size_t len = 0;
  if (!n->name) {
    if (n->children) {
      node_foreach(n, child) {
        len += node_write_len(child, depth, top_level);
        if (top_level && child->next)
          ++len;
      }
      return len;
    }

    return depth * 2 + node_full_value_len(n) + 1;
  }

  if (n->children) {
    len += depth * 2 + n->name_dim + 2;
    node_foreach(n, child) {
      len += node_write_len(child, depth + 1, false);
    }
    len += depth * 2 + 2;
    return len;
  }

  if (n->type == NODE_TYPE_DEF)
    return depth * 2 + n->name_dim + 1;

  return depth * 2 + n->name_dim + 2 + node_full_value_len(n) + 1;
}

static void node_write_into(const node* n, size_t depth, bool top_level, char** out) {
  if (!n)
    return;

  if (!n->name) {
    if (n->children) {
      node_foreach(n, child) {
        node_write_into(child, depth, top_level, out);
        if (top_level && child->next)
          *(*out)++ = '\n';
      }
      return;
    }

    for (size_t i = 0; i < depth * 2; ++i)
      *(*out)++ = ' ';
    node_full_value_write(n, *out);
    *out += node_full_value_len(n);
    *(*out)++ = '\n';
    return;
  }

  for (size_t i = 0; i < depth * 2; ++i)
    *(*out)++ = ' ';
  memcpy(*out, n->name, n->name_dim);
  *out += n->name_dim;

  if (n->children) {
    *(*out)++ = '(';
    *(*out)++ = '\n';
    node_foreach(n, child) {
      node_write_into(child, depth + 1, false, out);
    }
    for (size_t i = 0; i < depth * 2; ++i)
      *(*out)++ = ' ';
    *(*out)++ = ')';
    *(*out)++ = '\n';
    return;
  }

  if (n->type == NODE_TYPE_DEF) {
    *(*out)++ = '\n';
    return;
  }

  *(*out)++ = '(';
  node_full_value_write(n, *out);
  *out += node_full_value_len(n);
  *(*out)++ = ')';
  *(*out)++ = '\n';
}

// String is allocated with push()
static const char* node_write(node* n) {
  size_t len = node_write_len(n, 0, true);
  char* out = push(len + 1);
  if (!out)
    return NULL;

  char* p = out;
  node_write_into(n, 0, true, &p);
  *p = '\0';
  return out;
}

// Edit a text section using txt_offset, txt_dim from node
// Used for example for bumping version node
static const char* node_edit(node* n, const char* str) {
  if (!n || !str)
    return NULL;

  const char* replacement = NULL;
  size_t replacement_len = 0;

  if (n->children) {
    replacement = node_write(n);
    replacement_len = strlen(replacement);
  } else {
    switch (n->type) {
      case NODE_TYPE_STR:
        replacement = n->_str;
        replacement_len = n->txt_dim;
        break;
      case NODE_TYPE_INT: {
        char buf[32];
        replacement_len = (size_t)snprintf(buf, sizeof(buf), "%lld", (long long)n->_int);
        replacement = arena_text(buf, replacement_len);
        break;
      }
      case NODE_TYPE_FLT: {
        char buf[64];
        replacement_len = (size_t)snprintf(buf, sizeof(buf), "%g", n->_flt);
        replacement = arena_text(buf, replacement_len);
        break;
      }
      case NODE_TYPE_VER: {
        char buf[32];
        if (n->_ver.user != 0)
          replacement_len = (size_t)snprintf(buf, sizeof(buf), "%u.%u.%u.%u", n->_ver.major, n->_ver.minor, n->_ver.patch, n->_ver.user);
        else
          replacement_len = (size_t)snprintf(buf, sizeof(buf), "%u.%u.%u", n->_ver.major, n->_ver.minor, n->_ver.patch);
        replacement = arena_text(buf, replacement_len);
        break;
      }
      case NODE_TYPE_IDF:
        replacement = n->_idf;
        replacement_len = n->txt_dim;
        break;
      case NODE_TYPE_BOL:
        replacement = n->_bol ? "true" : "false";
        replacement_len = n->_bol ? 4 : 5;
        break;
      default:
        replacement = n->name ? n->name : "";
        replacement_len = n->name ? n->name_dim : 0;
        break;
    }
  }

  size_t src_len = strlen(str);
  size_t prefix_len = n->txt_offset;
  size_t suffix_len = (n->txt_offset + n->txt_dim <= src_len) ? (src_len - n->txt_offset - n->txt_dim) : 0;

  char* out = push(prefix_len + replacement_len + suffix_len + 1);
  if (!out)
    return NULL;

  memcpy(out, str, prefix_len);
  memcpy(out + prefix_len, replacement, replacement_len);
  memcpy(out + prefix_len + replacement_len, str + n->txt_offset + n->txt_dim, suffix_len);
  out[prefix_len + replacement_len + suffix_len] = '\0';
  return out;
}

static void node_debug_print_impl(node* n, size_t depth) {
  if (!n)
    return;

  for (size_t i = 0; i < depth; ++i)
    printf("  ");

  if (n->name)
    printf("%.*s", (int)n->name_dim, n->name);

  const char* name = n->type < NODE_TYPE_MAX ? NODE_TYPE_NAMES[n->type] : "?";
  printf(" [%s]", name);
  switch (n->type) {
    case NODE_TYPE_STR:
      printf(" = \"%.*s\"", (int)n->txt_dim, n->_str ? n->_str : "");
      break;
    case NODE_TYPE_INT:
      printf(" = %lld", (long long)n->_int);
      break;
    case NODE_TYPE_FLT:
      printf(" = %g", n->_flt);
      break;
    case NODE_TYPE_VER:
      printf(" = %u.%u.%u.%u", n->_ver.major, n->_ver.minor, n->_ver.patch, n->_ver.user);
      break;
    case NODE_TYPE_IDF:
      printf(" = %.*s", (int)n->txt_dim, n->_idf ? n->_idf : "");
      break;
    case NODE_TYPE_BOL:
      printf(" = %s", n->_bol ? "true" : "false");
      break;
    default:
      break;
  }

  printf("\n");

  node_foreach(n, child) {
    node_debug_print_impl(child, depth + 1);
  }
}

static void node_debug_print(node* n) {
  node_debug_print_impl(n, 0);
}

static const char* read_entire_file(const char* p) {
  FILE* f = fopen(p, "rb");
  if (!f)
    return NULL;

  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return NULL;
  }

  long size = ftell(f);
  if (size < 0) {
    fclose(f);
    return NULL;
  }

  rewind(f);

  char* data = push((size_t)size + 1);
  if (!data) {
    fclose(f);
    return NULL;
  }

  size_t read = fread(data, 1, (size_t)size, f);
  fclose(f);

  if (read != (size_t)size) {
    free(data);
    return NULL;
  }

  data[size] = '\0';
  return data;
}

static bool write_entire_file(const char* p, const char* text) {
  FILE* f = fopen(p, "wb");
  if (!f)
    return false;

  while (*text) {
    if (fputc(*text++, f) == EOF) {
      fclose(f);
      return false;
    }
  }

  fclose(f);
  return true;
}

static const char* get_path_cwd(const char* filename) {
  char cwd[_MAX_PATH] = {0};
  if (!_getcwd(cwd, sizeof(cwd))) {
    cwd[0] = '\0';
  }

  size_t len = strlen(cwd) + 1 + strlen(filename) + 1;
  char* out = push(len);
  snprintf(out, len, "%s\\%s", cwd, filename);
  return out;
}

static const char* get_path_exe(const char* filename) {
  char exe_path[_MAX_PATH] = {0};
  char exe_dir[_MAX_PATH] = {0};
  bool got_path = false;

#if defined(_WIN32)
  if (GetModuleFileNameA(NULL, exe_path, sizeof(exe_path))) {
    got_path = true;
  }
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
    if (!last_slash) last_slash = strrchr(exe_dir, '/');
    if (last_slash) *last_slash = '\0';
  }

  size_t len = strlen(exe_dir) + 1 + strlen(filename) + 1;
  char* out = push(len);
  snprintf(out, len, "%s\\%s", exe_dir, filename);
  return out;
}

static bool file_exists(const char* path) {
  return access(path, F_OK) == 0;
}
