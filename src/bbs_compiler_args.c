#pragma once
#include "bbs_compiler_args.h"

typedef struct {
  char* data;
  size_t len;
  size_t cap;
} compiler_args_buf;

static bool compiler_args_buf_reserve(compiler_args_buf* buf, size_t extra) {
  if (!buf)
    return false;
  size_t need = buf->len + extra + 1;
  if (need <= buf->cap)
    return true;
  size_t new_cap = buf->cap > 0 ? buf->cap : 256;
  while (new_cap < need)
    new_cap *= 2;
  char* next = realloc(buf->data, new_cap);
  if (!next)
    return false;
  buf->data = next;
  buf->cap = new_cap;
  return true;
}

static bool compiler_args_buf_append(compiler_args_buf* buf, const char* text) {
  size_t len = text ? strlen(text) : 0;
  if (!compiler_args_buf_reserve(buf, len))
    return false;
  if (len > 0)
    memcpy(buf->data + buf->len, text, len);
  buf->len += len;
  buf->data[buf->len] = '\0';
  return true;
}

static bool compiler_args_buf_append_sep(compiler_args_buf* buf, const char* text) {
  if (!text || !text[0])
    return true;
  if (buf->len > 0 && !compiler_args_buf_append(buf, " "))
    return false;
  return compiler_args_buf_append(buf, text);
}

static bool compiler_args_token_eq(const char* a, const char* b) {
  return a && b && strcmp(a, b) == 0;
}

static bool compiler_args_take_token(const char** ptext, char* out, size_t out_size, bool* out_quoted) {
  const char* p = ptext ? *ptext : NULL;
  size_t wi = 0;
  bool quoted = false;
  if (out_quoted)
    *out_quoted = false;
  if (!p || !out || out_size == 0)
    return false;

  while (*p && isspace((unsigned char)*p))
    ++p;
  if (!*p) {
    *ptext = p;
    out[0] = '\0';
    return false;
  }

  while (*p) {
    if (!quoted && isspace((unsigned char)*p))
      break;
    if (*p == '"') {
      quoted = !quoted;
      if (out_quoted)
        *out_quoted = true;
      ++p;
      continue;
    }
    if (*p == '\\' && p[1] == '"') {
      if (wi + 1 < out_size)
        out[wi++] = '"';
      p += 2;
      continue;
    }
    if (wi + 1 < out_size)
      out[wi++] = *p;
    ++p;
  }

  out[wi] = '\0';
  while (*p && isspace((unsigned char)*p))
    ++p;
  *ptext = p;
  return wi > 0;
}

static bool compiler_args_append_msvc_pathflag(compiler_args_buf* out, const char* prefix, const char* value) {
  char buf[2048] = {0};
  if (!out || !prefix || !value)
    return false;
  snprintf(buf, sizeof(buf), "%s%s", prefix, value);
  return compiler_args_buf_append_sep(out, buf);
}

static bool compiler_args_append_joined(compiler_args_buf* out, const char* prefix, const char* value) {
  char buf[2048] = {0};
  if (!out || !prefix || !value)
    return false;
  snprintf(buf, sizeof(buf), "%s%s", prefix, value);
  return compiler_args_buf_append_sep(out, buf);
}

static bool compiler_args_append_nvcc_hostflag(compiler_args_buf* out, const char* value) {
  char buf[2048] = {0};
  if (!out || !value || !value[0])
    return false;
  snprintf(buf, sizeof(buf), "-Xcompiler=%s", value);
  return compiler_args_buf_append_sep(out, buf);
}

static const char* compiler_args_map_std(const char* value, bool is_cpp) {
  if (!value || !value[0])
    return NULL;
  if (is_cpp) {
    if (_stricmp(value, "c++14") == 0 || _stricmp(value, "gnu++14") == 0)
      return "/std:c++14";
    if (_stricmp(value, "c++17") == 0 || _stricmp(value, "gnu++17") == 0)
      return "/std:c++17";
    if (_stricmp(value, "c++20") == 0 || _stricmp(value, "gnu++20") == 0)
      return "/std:c++20";
    if (_stricmp(value, "c++23") == 0 || _stricmp(value, "gnu++23") == 0 || _stricmp(value, "c++2b") == 0 || _stricmp(value, "gnu++2b") == 0)
      return "/std:c++latest";
    return NULL;
  }

  if (_stricmp(value, "c11") == 0 || _stricmp(value, "gnu11") == 0)
    return "/std:c11";
  if (_stricmp(value, "c17") == 0 || _stricmp(value, "gnu17") == 0 || _stricmp(value, "c18") == 0 || _stricmp(value, "gnu18") == 0)
    return "/std:c17";
  if (_stricmp(value, "c23") == 0 || _stricmp(value, "gnu23") == 0 || _stricmp(value, "c2x") == 0 || _stricmp(value, "gnu2x") == 0)
    return "/std:clatest";
  return NULL;
}

static bool compiler_args_translate_one(compiler_args_buf* out, const char* token, const char* next, bool is_cpp, bool* out_consumed_next, bool* out_changed) {
  if (out_consumed_next)
    *out_consumed_next = false;
  if (out_changed)
    *out_changed = false;
  if (!token || !token[0])
    return true;

  if (token[0] == '/' || token[0] != '-')
    return compiler_args_buf_append_sep(out, token);

  if (strncmp(token, "-D", 2) == 0)
    return compiler_args_append_msvc_pathflag(out, "/D", token + 2);
  if (strncmp(token, "-I", 2) == 0)
    return compiler_args_append_msvc_pathflag(out, "/I", token + 2);
  if (strcmp(token, "-include") == 0 && next && next[0]) {
    if (out_consumed_next)
      *out_consumed_next = true;
    if (out_changed)
      *out_changed = true;
    return compiler_args_append_msvc_pathflag(out, "/FI", next);
  }
  if (strncmp(token, "-include", 8) == 0 && token[8] != '\0')
    return compiler_args_append_msvc_pathflag(out, "/FI", token + 8);
  if (strncmp(token, "-std=", 5) == 0) {
    const char* mapped = compiler_args_map_std(token + 5, is_cpp);
    if (out_changed)
      *out_changed = true;
    return mapped ? compiler_args_buf_append_sep(out, mapped) : true;
  }
  if (compiler_args_token_eq(token, "-Wall") || compiler_args_token_eq(token, "-Wextra") || compiler_args_token_eq(token, "-Wpedantic") || compiler_args_token_eq(token, "-pedantic")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_buf_append_sep(out, "/W4");
  }
  if (compiler_args_token_eq(token, "-Werror")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_buf_append_sep(out, "/WX");
  }
  if (compiler_args_token_eq(token, "-g") || compiler_args_token_eq(token, "-g2") || compiler_args_token_eq(token, "-g3")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_buf_append_sep(out, "/Zi");
  }
  if (compiler_args_token_eq(token, "-g0") || compiler_args_token_eq(token, "-pipe") || compiler_args_token_eq(token, "-fPIC") || compiler_args_token_eq(token, "-fpic") || compiler_args_token_eq(token, "-fno-omit-frame-pointer")) {
    if (out_changed)
      *out_changed = true;
    return true;
  }
  if (compiler_args_token_eq(token, "-O0")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_buf_append_sep(out, "/Od");
  }
  if (compiler_args_token_eq(token, "-O1") || compiler_args_token_eq(token, "-Os") || compiler_args_token_eq(token, "-Oz")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_buf_append_sep(out, "/O1");
  }
  if (compiler_args_token_eq(token, "-O2")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_buf_append_sep(out, "/O2");
  }
  if (compiler_args_token_eq(token, "-O3") || compiler_args_token_eq(token, "-Ofast")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_buf_append_sep(out, "/Ox");
  }
  if (compiler_args_token_eq(token, "-Og")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_buf_append_sep(out, "/Od");
  }
  if (compiler_args_token_eq(token, "-fexceptions")) {
    if (out_changed)
      *out_changed = true;
    return is_cpp ? compiler_args_buf_append_sep(out, "/EHsc") : true;
  }
  if (compiler_args_token_eq(token, "-fno-exceptions")) {
    if (out_changed)
      *out_changed = true;
    return is_cpp ? compiler_args_buf_append_sep(out, "/EHs-c-") : true;
  }
  if (compiler_args_token_eq(token, "-frtti")) {
    if (out_changed)
      *out_changed = true;
    return is_cpp ? compiler_args_buf_append_sep(out, "/GR") : true;
  }
  if (compiler_args_token_eq(token, "-fno-rtti")) {
    if (out_changed)
      *out_changed = true;
    return is_cpp ? compiler_args_buf_append_sep(out, "/GR-") : true;
  }
  if (compiler_args_token_eq(token, "-m32") || compiler_args_token_eq(token, "-m64") || compiler_args_token_eq(token, "-Winvalid-pch") ||
      strncmp(token, "-Wno-", 5) == 0 || strncmp(token, "-Wl,", 4) == 0 || strncmp(token, "-f", 2) == 0) {
    if (out_changed)
      *out_changed = true;
    return true;
  }

  if (out_changed)
    *out_changed = true;
  return true;
}

static const char* compiler_args_translate_msvc(const char* text, bool is_cpp, bool* out_changed) {
  compiler_args_buf out = {0};
  const char* p = text ? text : "";
  bool changed = false;
  char token[2048] = {0};
  char next[2048] = {0};

  while (compiler_args_take_token(&p, token, sizeof(token), NULL)) {
    const char* lookahead = p;
    bool consumed_next = false;
    bool changed_one = false;
    next[0] = '\0';
    compiler_args_take_token(&lookahead, next, sizeof(next), NULL);
    if (!compiler_args_translate_one(&out, token, next, is_cpp, &consumed_next, &changed_one)) {
      free(out.data);
      return NULL;
    }
    if (changed_one)
      changed = true;
    if (consumed_next)
      p = lookahead;
  }

  if (!out.data) {
    out.data = malloc(1);
    if (out.data)
      out.data[0] = '\0';
  }
  if (out_changed)
    *out_changed = changed;
  return out.data ? arena_text(out.data, strlen(out.data)) : NULL;
}

static bool compiler_args_translate_one_nvcc(compiler_args_buf* out, const char* token, const char* next, bool host_is_msvc, bool* out_consumed_next, bool* out_changed) {
  if (out_consumed_next)
    *out_consumed_next = false;
  if (out_changed)
    *out_changed = false;
  if (!token || !token[0])
    return true;

  if (token[0] == '/') {
    if (host_is_msvc) {
      if (out_changed)
        *out_changed = true;
      return compiler_args_append_nvcc_hostflag(out, token);
    }
    return compiler_args_buf_append_sep(out, token);
  }

  if (token[0] != '-')
    return compiler_args_buf_append_sep(out, token);

  if (strncmp(token, "-D", 2) == 0 || strncmp(token, "-I", 2) == 0)
    return compiler_args_buf_append_sep(out, token);

  if (strcmp(token, "-include") == 0 && next && next[0]) {
    if (out_consumed_next)
      *out_consumed_next = true;
    if (out_changed)
      *out_changed = true;
    if (!compiler_args_buf_append_sep(out, "--pre-include"))
      return false;
    return compiler_args_buf_append_sep(out, next);
  }
  if (strncmp(token, "-include", 8) == 0 && token[8] != '\0') {
    if (out_changed)
      *out_changed = true;
    return compiler_args_append_joined(out, "--pre-include=", token + 8);
  }

  if (strncmp(token, "-std=", 5) == 0) {
    const char* value = token + 5;
    const char* mapped = value;
    if (_stricmp(value, "gnu++14") == 0 || _stricmp(value, "cuda14") == 0)
      mapped = "c++14";
    else if (_stricmp(value, "gnu++17") == 0 || _stricmp(value, "cuda17") == 0)
      mapped = "c++17";
    else if (_stricmp(value, "gnu++20") == 0 || _stricmp(value, "cuda20") == 0)
      mapped = "c++20";
    else if (_stricmp(value, "gnu++23") == 0 || _stricmp(value, "c++2b") == 0 || _stricmp(value, "gnu++2b") == 0 || _stricmp(value, "cuda23") == 0)
      mapped = "c++23";
    if (out_changed)
      *out_changed = _stricmp(mapped, value) != 0 || _stricmp(value, "c++11") == 0 || _stricmp(value, "c++14") == 0 || _stricmp(value, "c++17") == 0 || _stricmp(value, "c++20") == 0 || _stricmp(value, "c++23") == 0;
    return compiler_args_append_joined(out, "--std=", mapped);
  }

  if (compiler_args_token_eq(token, "-Wall")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_append_nvcc_hostflag(out, host_is_msvc ? "/W3" : "-Wall");
  }
  if (compiler_args_token_eq(token, "-Wextra")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_append_nvcc_hostflag(out, host_is_msvc ? "/W4" : "-Wextra");
  }
  if (compiler_args_token_eq(token, "-Wpedantic") || compiler_args_token_eq(token, "-pedantic")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_append_nvcc_hostflag(out, host_is_msvc ? "/W4" : "-Wpedantic");
  }
  if (compiler_args_token_eq(token, "-Werror")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_append_nvcc_hostflag(out, host_is_msvc ? "/WX" : "-Werror");
  }

  if (compiler_args_token_eq(token, "-g") || compiler_args_token_eq(token, "-g2") || compiler_args_token_eq(token, "-g3")) {
    if (out_changed)
      *out_changed = host_is_msvc;
    if (!compiler_args_buf_append_sep(out, "-g"))
      return false;
    return host_is_msvc ? compiler_args_append_nvcc_hostflag(out, "/Zi") : true;
  }
  if (compiler_args_token_eq(token, "-g0")) {
    if (out_changed)
      *out_changed = true;
    return true;
  }
  if (compiler_args_token_eq(token, "-pipe") || compiler_args_token_eq(token, "-fPIC") || compiler_args_token_eq(token, "-fpic") || compiler_args_token_eq(token, "-fno-omit-frame-pointer")) {
    if (out_changed)
      *out_changed = host_is_msvc;
    return host_is_msvc ? true : compiler_args_append_nvcc_hostflag(out, token);
  }

  if (compiler_args_token_eq(token, "-O0")) {
    if (out_changed)
      *out_changed = host_is_msvc;
    if (!compiler_args_buf_append_sep(out, "-O0"))
      return false;
    return host_is_msvc ? compiler_args_append_nvcc_hostflag(out, "/Od") : true;
  }
  if (compiler_args_token_eq(token, "-O1")) {
    if (out_changed)
      *out_changed = host_is_msvc;
    if (!compiler_args_buf_append_sep(out, "-O1"))
      return false;
    return host_is_msvc ? compiler_args_append_nvcc_hostflag(out, "/O1") : true;
  }
  if (compiler_args_token_eq(token, "-O2")) {
    if (out_changed)
      *out_changed = host_is_msvc;
    if (!compiler_args_buf_append_sep(out, "-O2"))
      return false;
    return host_is_msvc ? compiler_args_append_nvcc_hostflag(out, "/O2") : true;
  }
  if (compiler_args_token_eq(token, "-O3") || compiler_args_token_eq(token, "-Ofast")) {
    if (out_changed)
      *out_changed = true;
    if (!compiler_args_buf_append_sep(out, "-O3"))
      return false;
    return host_is_msvc ? compiler_args_append_nvcc_hostflag(out, "/Ox") : true;
  }
  if (compiler_args_token_eq(token, "-Og")) {
    if (out_changed)
      *out_changed = true;
    if (!compiler_args_buf_append_sep(out, "-O0"))
      return false;
    return host_is_msvc ? compiler_args_append_nvcc_hostflag(out, "/Od") : true;
  }
  if (compiler_args_token_eq(token, "-Os") || compiler_args_token_eq(token, "-Oz")) {
    if (out_changed)
      *out_changed = true;
    if (!compiler_args_buf_append_sep(out, "-O2"))
      return false;
    return host_is_msvc ? compiler_args_append_nvcc_hostflag(out, "/O1") : true;
  }

  if (compiler_args_token_eq(token, "-fexceptions")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_append_nvcc_hostflag(out, host_is_msvc ? "/EHsc" : "-fexceptions");
  }
  if (compiler_args_token_eq(token, "-fno-exceptions")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_append_nvcc_hostflag(out, host_is_msvc ? "/EHs-c-" : "-fno-exceptions");
  }
  if (compiler_args_token_eq(token, "-frtti")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_append_nvcc_hostflag(out, host_is_msvc ? "/GR" : "-frtti");
  }
  if (compiler_args_token_eq(token, "-fno-rtti")) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_append_nvcc_hostflag(out, host_is_msvc ? "/GR-" : "-fno-rtti");
  }

  if (compiler_args_token_eq(token, "-m32") || compiler_args_token_eq(token, "-m64") || compiler_args_token_eq(token, "-Winvalid-pch")) {
    if (out_changed)
      *out_changed = true;
    return true;
  }

  if (strncmp(token, "-Wno-", 5) == 0) {
    if (out_changed)
      *out_changed = true;
    return host_is_msvc ? true : compiler_args_append_nvcc_hostflag(out, token);
  }

  if (strncmp(token, "-Wl,", 4) == 0) {
    if (out_changed)
      *out_changed = true;
    return compiler_args_append_joined(out, "-Xlinker=", token + 4);
  }

  if (strncmp(token, "-f", 2) == 0) {
    if (out_changed)
      *out_changed = true;
    return host_is_msvc ? true : compiler_args_append_nvcc_hostflag(out, token);
  }

  return compiler_args_buf_append_sep(out, token);
}

static const char* compiler_args_translate_nvcc(const char* text, bool host_is_msvc, bool* out_changed) {
  compiler_args_buf out = {0};
  const char* p = text ? text : "";
  bool changed = false;
  char token[2048] = {0};
  char next[2048] = {0};

  while (compiler_args_take_token(&p, token, sizeof(token), NULL)) {
    const char* lookahead = p;
    bool consumed_next = false;
    bool changed_one = false;
    next[0] = '\0';
    compiler_args_take_token(&lookahead, next, sizeof(next), NULL);
    if (!compiler_args_translate_one_nvcc(&out, token, next, host_is_msvc, &consumed_next, &changed_one)) {
      free(out.data);
      return NULL;
    }
    if (changed_one)
      changed = true;
    if (consumed_next)
      p = lookahead;
  }

  if (!out.data) {
    out.data = malloc(1);
    if (out.data)
      out.data[0] = '\0';
  }
  if (out_changed)
    *out_changed = changed;
  return out.data ? arena_text(out.data, strlen(out.data)) : NULL;
}
