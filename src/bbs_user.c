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

static void user_apply_defaults(user* u) {
  if (!u)
    return;

  u->builddir = DEF_BUILD_DIR;
  u->distdir = DEF_DIST_DIR;
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

  return true;
}

static bool user_load_paths(const char* user_path, const char* local_path, user* out) {
  if (!out)
    return false;

  user_apply_defaults(out);

  node* user_scope = user_parse_scope_file(user_path);
  if (user_path && user_path[0] && file_exists(user_path) && !user_scope)
    return false;

  node* local_scope = user_parse_scope_file(local_path);
  if (local_path && local_path[0] && file_exists(local_path) && !local_scope)
    return false;

  node* merged = user_merge_scopes(user_scope, local_scope);
  if (!merged)
    return false;

  return user_apply_scope(merged, out);
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
