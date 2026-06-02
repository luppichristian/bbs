#pragma once
#include "bbs_user.h"

static const char* user_scalar_text(node* n) {
  if (!n)
    return NULL;

  switch (n->type) {
    case NODE_TYPE_STR:
      return node_get_str(n);
    case NODE_TYPE_IDF:
      return node_get_idf(n);
    default:
      return NULL;
  }
}

static int user_child_count(node* n) {
  int count = 0;
  if (!n)
    return 0;

  node_foreach(n, child) {
    ++count;
  }
  return count;
}

static node** user_children_in_source_order(node* n, int* out_count) {
  if (out_count)
    *out_count = 0;
  if (!n)
    return NULL;

  int count = user_child_count(n);
  if (count <= 0)
    return NULL;

  node** items = push(sizeof(*items) * (size_t)count);
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

static bool user_is_archive_format(const char* text) {
  if (!text || !text[0])
    return false;

  return _stricmp(text, "zip") == 0 || _stricmp(text, "tar") == 0 || _stricmp(text, "rar") == 0;
}

static void user_append_child(node* parent, node* child) {
  if (!parent || !child)
    return;

  child->next = NULL;
  if (!parent->children) {
    parent->children = child;
    return;
  }

  node* last = parent->children;
  while (last->next)
    last = last->next;
  last->next = child;
}

static node* user_clone_node(node* src) {
  if (!src)
    return NULL;

  node* dst = push(sizeof(node));
  if (!dst)
    return NULL;

  *dst = *src;
  dst->next = NULL;
  dst->parent = NULL;
  dst->children = NULL;

  node_foreach(src, child) {
    node* child_copy = user_clone_node(child);
    if (!child_copy)
      return NULL;
    child_copy->parent = dst;
    user_append_child(dst, child_copy);
  }

  return dst;
}

static node* user_scope_from_tree(node* tree, const char* path) {
  if (!tree)
    return NULL;

  node* user_n = NULL;
  int user_count = 0;
  node_foreach(tree, child) {
    if (!child->name || _stricmp(child->name, "user") != 0)
      continue;
    user_n = child;
    ++user_count;
  }

  if (user_count > 1) {
    error("Multiple top-level user nodes found in %s.", path);
    return NULL;
  }

  return user_count == 1 ? user_n : tree;
}

static node* user_parse_scope_file(const char* path) {
  if (!path || !path[0])
    return NULL;
  if (!file_exists(path))
    return NULL;

  const char* text = read_entire_file(path);
  if (!text) {
    error("Failed to read user config: %s", path);
    return NULL;
  }

  node* tree = node_parse(text);
  if (!tree) {
    error("Failed to parse user config: %s", path);
    return NULL;
  }

  return user_scope_from_tree(tree, path);
}

static node* user_merge_scopes(node* base, node* override) {
  node* merged = push(sizeof(node));
  if (!merged)
    return NULL;
  memset(merged, 0, sizeof(*merged));

  if (override) {
    node_foreach(override, child) {
      node* copy = user_clone_node(child);
      if (!copy)
        return NULL;
      copy->parent = merged;
      user_append_child(merged, copy);
    }
  }

  if (base) {
    node_foreach(base, child) {
      if (child->name && node_get_child(merged, child->name))
        continue;

      node* copy = user_clone_node(child);
      if (!copy)
        return NULL;
      copy->parent = merged;
      user_append_child(merged, copy);
    }
  }

  return merged;
}

static bool user_read_text_child(node* scope, const char* name, const char** out) {
  if (out)
    *out = NULL;
  if (!scope || !name || !name[0])
    return false;

  node* child = node_get_child(scope, name);
  if (!child)
    return true;

  const char* text = user_scalar_text(child);
  if (!text) {
    error("Attribute '%s' must be a string or identifier.", name);
    return false;
  }

  if (out)
    *out = text;
  return true;
}

static bool user_read_uint_child(node* scope, const char* name, unsigned int* out) {
  if (out)
    *out = 0;
  if (!scope || !name || !name[0])
    return false;

  node* child = node_get_child(scope, name);
  if (!child)
    return true;
  if (child->type != NODE_TYPE_INT) {
    error("Attribute '%s' must be an integer.", name);
    return false;
  }

  int64_t value = node_get_int(child);
  if (value < 0) {
    error("Attribute '%s' cannot be negative.", name);
    return false;
  }

  if (out)
    *out = (unsigned int)value;
  return true;
}

static int user_find_gen_index(const user* u, const char* name) {
  if (!u || !name || !name[0])
    return -1;

  for (int i = 0; i < u->gen_c; ++i) {
    if (u->gens[i].name && _stricmp(u->gens[i].name, name) == 0)
      return i;
  }

  return -1;
}

static const user_gen* user_find_gen(const user* u, const char* name) {
  int idx = user_find_gen_index(u, name);
  return idx >= 0 ? &u->gens[idx] : NULL;
}

static bool user_parse_gen_node(node* gen_n, user_gen* out) {
  if (!gen_n || !out)
    return false;
  if (gen_n->type != NODE_TYPE_DEF) {
    error("Attribute 'gen' must be a section.");
    return false;
  }

  memset(out, 0, sizeof(*out));
  if (!user_read_text_child(gen_n, "name", &out->name))
    return false;
  if (!out->name || !out->name[0]) {
    error("Generator 'gen' is missing required attribute 'name'.");
    return false;
  }

  if (!user_read_text_child(gen_n, "copyfile", &out->copyfile))
    return false;
  if (!out->copyfile || !out->copyfile[0]) {
    error("Generator '%s' is missing required attribute 'copyfile'.", out->name);
    return false;
  }

  return true;
}

static bool user_add_gen(user* out, user_gen gen, bool replace_existing) {
  if (!out || !gen.name || !gen.name[0])
    return false;

  int idx = user_find_gen_index(out, gen.name);
  if (idx >= 0) {
    if (!replace_existing) {
      error("Duplicate generator '%s'.", gen.name);
      return false;
    }

    out->gens[idx] = gen;
    return true;
  }

  user_gen* items = push(sizeof(*items) * (size_t)(out->gen_c + 1));
  if (!items)
    return false;

  if (out->gens && out->gen_c > 0)
    memcpy(items, out->gens, sizeof(*items) * (size_t)out->gen_c);

  items[out->gen_c++] = gen;
  out->gens = items;
  return true;
}

static bool user_collect_scope_gens(node* scope, user* out, bool replace_existing) {
  if (!scope || !out)
    return true;

  int child_count = 0;
  node** children = user_children_in_source_order(scope, &child_count);
  for (int i = 0; i < child_count; ++i) {
    node* child = children[i];
    if (!child || !child->name || _stricmp(child->name, "gen") != 0)
      continue;

    user_gen gen = {0};
    if (!user_parse_gen_node(child, &gen))
      return false;
    if (!user_add_gen(out, gen, replace_existing))
      return false;
  }

  return true;
}

static void user_apply_defaults(user* u) {
  if (!u)
    return;

  u->builddir = DEF_BUILD_DIR;
  u->distdir = DEF_DIST_DIR;
  u->auto_debounce_ms = 500;
  u->auto_retry_count = 3;
  u->auto_retry_delay_ms = 250;
  u->dist_archive_format = "zip";
  u->dist_archive_name = "$CFG-$OS-$ARC--$VER";
  u->cmake_args = NULL;
  u->cmake_build_args = NULL;
  u->ctest_args = NULL;
  u->gens = NULL;
  u->gen_c = 0;
}

static bool user_apply_scope(node* scope, user* out) {
  if (!scope || !out)
    return false;

  const char* text = NULL;
  if (!user_read_text_child(scope, "builddir", &text))
    return false;
  if (text && text[0])
    out->builddir = text;

  text = NULL;
  if (!user_read_text_child(scope, "distdir", &text))
    return false;
  if (text && text[0])
    out->distdir = text;

  unsigned int uint_value = 0;
  if (!user_read_uint_child(scope, "auto_debounce_ms", &uint_value))
    return false;
  if (node_get_child(scope, "auto_debounce_ms"))
    out->auto_debounce_ms = uint_value;

  uint_value = 0;
  if (!user_read_uint_child(scope, "auto_retry_count", &uint_value))
    return false;
  if (node_get_child(scope, "auto_retry_count"))
    out->auto_retry_count = uint_value;

  uint_value = 0;
  if (!user_read_uint_child(scope, "auto_retry_delay_ms", &uint_value))
    return false;
  if (node_get_child(scope, "auto_retry_delay_ms"))
    out->auto_retry_delay_ms = uint_value;

  text = NULL;
  if (!user_read_text_child(scope, "dist_archive_format", &text))
    return false;
  if (text && text[0]) {
    if (!user_is_archive_format(text)) {
      error("Unknown dist_archive_format '%s'.", text);
      return false;
    }
    out->dist_archive_format = text;
  }

  text = NULL;
  if (!user_read_text_child(scope, "dist_archive_name", &text))
    return false;
  if (text && text[0])
    out->dist_archive_name = text;

  text = NULL;
  if (!user_read_text_child(scope, "cmake_args", &text))
    return false;
  if (text && text[0])
    out->cmake_args = text;

  text = NULL;
  if (!user_read_text_child(scope, "cmake_build_args", &text))
    return false;
  if (text && text[0])
    out->cmake_build_args = text;

  text = NULL;
  if (!user_read_text_child(scope, "ctest_args", &text))
    return false;
  if (text && text[0])
    out->ctest_args = text;

  return true;
}

static bool user_validate_gen_shape(node* gen_n, const char* scope_label) {
  if (!gen_n)
    return false;
  if (gen_n->type != NODE_TYPE_DEF) {
    error("Attribute 'gen' must be a section in %s config.", scope_label);
    return false;
  }

  node_foreach(gen_n, child) {
    if (!child || !child->name) {
      error("Generator entries in %s config must use named attributes.", scope_label);
      return false;
    }
    if (_stricmp(child->name, "name") != 0 && _stricmp(child->name, "copyfile") != 0) {
      error("Unknown generator attribute '%s' in %s config.", child->name, scope_label);
      return false;
    }
  }

  user_gen gen = {0};
  return user_parse_gen_node(gen_n, &gen);
}

static bool user_validate_scope(node* scope, const char* scope_label) {
  if (!scope)
    return true;

  user tmp = {0};
  user_apply_defaults(&tmp);
  if (!user_apply_scope(scope, &tmp))
    return false;

  node_foreach(scope, child) {
    if (!child || !child->name) {
      error("Unexpected scalar item in %s config.", scope_label);
      return false;
    }

    if (_stricmp(child->name, "builddir") == 0 ||
        _stricmp(child->name, "distdir") == 0 ||
        _stricmp(child->name, "auto_debounce_ms") == 0 ||
        _stricmp(child->name, "auto_retry_count") == 0 ||
        _stricmp(child->name, "auto_retry_delay_ms") == 0 ||
        _stricmp(child->name, "dist_archive_format") == 0 ||
        _stricmp(child->name, "dist_archive_name") == 0 ||
        _stricmp(child->name, "cmake_args") == 0 ||
        _stricmp(child->name, "cmake_build_args") == 0 ||
        _stricmp(child->name, "ctest_args") == 0) {
      continue;
    }

    if (_stricmp(child->name, "gen") == 0) {
      if (!user_validate_gen_shape(child, scope_label))
        return false;
      continue;
    }

    error("Unknown attribute '%s' in %s config.", child->name, scope_label);
    return false;
  }

  return true;
}

static bool user_load_paths(const char* user_path, const char* local_path, user* out) {
  if (!out)
    return false;

  user_apply_defaults(out);

  node* user_scope = user_parse_scope_file(user_path);
  if (user_path && user_path[0] && file_exists(user_path) && !user_scope)
    return false;
  if (user_scope && !user_validate_scope(user_scope, "user"))
    return false;

  node* local_scope = user_parse_scope_file(local_path);
  if (local_path && local_path[0] && file_exists(local_path) && !local_scope)
    return false;
  if (local_scope && !user_validate_scope(local_scope, "local"))
    return false;

  node* merged = user_merge_scopes(user_scope, local_scope);
  if (!merged)
    return false;

  if (!user_apply_scope(merged, out))
    return false;
  if (!user_collect_scope_gens(user_scope, out, false))
    return false;
  if (!user_collect_scope_gens(local_scope, out, true))
    return false;

  return true;
}

static user* user_init(cmd_ctx* ctx) {
  user* u = push(sizeof(user));
  if (!u)
    return NULL;

  memset(u, 0, sizeof(*u));
  user_apply_defaults(u);
  if (!ctx)
    return u;

  if (!user_load_paths(ctx->cfg_paths[CFG_USER], ctx->cfg_paths[CFG_LOCAL], u))
    return NULL;

  return u;
}
