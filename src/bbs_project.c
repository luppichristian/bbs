#pragma once
#include "bbs_project.h"
#include "bbs_toolchain.c"
#include "bbs_user.c"

static bool project_parse_string_list(node* list_n, const char*** out_items, int* out_count);
static const char* project_join_scalar_list(node* list_n, int* out_count);
static bool project_apply_unity_node(node* unity_n, target* out, const char* target_label);
static bool project_load_user_config(const char* local_cfg_path, user* out);
static const char* project_target_executable_abs(const project* proj, const target* tgt, const char* platform);
static bool project_apply_dist_node(node* dist_n, target* out, const char* target_label);
static const char* project_expand_config_string(const char* text, const project* proj, const target* tgt, const char* platform, toolchain* tc, const char* workdir);
static int project_find_target_index(const project* proj, const char* name);
static const char* project_current_workdir(void);
static const char* project_path_parent(const char* path);
static bool project_prepare_packages(project* proj, toolchain* tc, const char* platform, bool refresh);
static bool project_generate_cmakelists(const project* proj, toolchain* tc, const char* platform_id, bool* changed);

typedef struct {
  const char* source;
  const char* cache_dir;
  bool is_cached;
  bool exists;
  bool has_cmakelists;
  bool has_project_config;
  package_backend backend;
  const char* status;
  const char* local_ref;
  const char* remote_ref;
} project_package_info;

static bool project_has_ver(ver v) {
  return v.major != 0 || v.minor != 0 || v.patch != 0 || v.user != 0;
}

static int project_child_count(node* n) {
  int count = 0;
  if (!n)
    return 0;

  node_foreach(n, child) {
    ++count;
  }
  return count;
}

static node** project_children_in_source_order(node* n, int* out_count) {
  if (out_count)
    *out_count = 0;
  if (!n)
    return NULL;

  int count = project_child_count(n);
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

static const char* project_target_type_name(target_type type) {
  switch (type) {
    case TARGET_TYPE_CONSOLE:
      return "console";
    case TARGET_TYPE_CONSOLELESS:
      return "consoleless";
    case TARGET_TYPE_HEADER_LIB:
      return "header_lib";
    case TARGET_TYPE_STATIC_LIB:
      return "static_lib";
    case TARGET_TYPE_DYN_LIB:
      return "dyn_lib";
    case TARGET_TYPE_OBJ_LIB:
      return "obj_lib";
    case TARGET_TYPE_TEST:
      return "test";
    case TARGET_TYPE_DRIVER:
      return "driver";
    default:
      return "target";
  }
}

static const char* project_package_source_name(package_source source) {
  switch (source) {
    case PACKAGE_SOURCE_PATH:
      return "path";
    case PACKAGE_SOURCE_REPO:
      return "repo";
    case PACKAGE_SOURCE_ARCHIVE:
      return "archive";
    default:
      return "none";
  }
}

static const char* project_package_backend_name(package_backend backend) {
  switch (backend) {
    case PACKAGE_BACKEND_CMAKE:
      return "cmake";
    case PACKAGE_BACKEND_BBS:
      return "bbs";
    default:
      return "none";
  }
}

static bool project_target_is_package(const target* tgt) {
  return tgt && (tgt->package_source != PACKAGE_SOURCE_NONE ||
                 (tgt->package_path && tgt->package_path[0]) ||
                 (tgt->package_repo_link && tgt->package_repo_link[0]) ||
                 (tgt->package_archive_link && tgt->package_archive_link[0]));
}

static bool project_is_known_target_type_name(const char* text) {
  if (!text || !text[0])
    return false;

  return _stricmp(text, "console") == 0 ||
         _stricmp(text, "consoleless") == 0 ||
         _stricmp(text, "gui") == 0 ||
         _stricmp(text, "header_lib") == 0 ||
         _stricmp(text, "header-lib") == 0 ||
         _stricmp(text, "headerlib") == 0 ||
         _stricmp(text, "static_lib") == 0 ||
         _stricmp(text, "static-lib") == 0 ||
         _stricmp(text, "staticlib") == 0 ||
         _stricmp(text, "dyn_lib") == 0 ||
         _stricmp(text, "dyn-lib") == 0 ||
         _stricmp(text, "shared_lib") == 0 ||
         _stricmp(text, "shared-lib") == 0 ||
         _stricmp(text, "obj_lib") == 0 ||
         _stricmp(text, "obj-lib") == 0 ||
         _stricmp(text, "object_lib") == 0 ||
         _stricmp(text, "object-lib") == 0 ||
         _stricmp(text, "test") == 0 ||
         _stricmp(text, "driver") == 0;
}

static target_type project_target_type_from_text(const char* text) {
  if (!text || !text[0])
    return TARGET_TYPE_CONSOLE;
  if (_stricmp(text, "console") == 0)
    return TARGET_TYPE_CONSOLE;
  if (_stricmp(text, "consoleless") == 0 || _stricmp(text, "gui") == 0)
    return TARGET_TYPE_CONSOLELESS;
  if (_stricmp(text, "header_lib") == 0 || _stricmp(text, "header-lib") == 0 || _stricmp(text, "headerlib") == 0)
    return TARGET_TYPE_HEADER_LIB;
  if (_stricmp(text, "static_lib") == 0 || _stricmp(text, "static-lib") == 0 || _stricmp(text, "staticlib") == 0)
    return TARGET_TYPE_STATIC_LIB;
  if (_stricmp(text, "dyn_lib") == 0 || _stricmp(text, "dyn-lib") == 0 || _stricmp(text, "shared_lib") == 0 || _stricmp(text, "shared-lib") == 0)
    return TARGET_TYPE_DYN_LIB;
  if (_stricmp(text, "obj_lib") == 0 || _stricmp(text, "obj-lib") == 0 || _stricmp(text, "object_lib") == 0 || _stricmp(text, "object-lib") == 0)
    return TARGET_TYPE_OBJ_LIB;
  if (_stricmp(text, "test") == 0)
    return TARGET_TYPE_TEST;
  if (_stricmp(text, "driver") == 0)
    return TARGET_TYPE_DRIVER;
  return TARGET_TYPE_CONSOLE;
}

static lang project_lang_from_text(const char* text) {
  if (!text || !text[0])
    return LANG_C;
  if (_stricmp(text, "cpp") == 0 || _stricmp(text, "c++") == 0)
    return LANG_CPP;
  return LANG_C;
}

static stdlib project_stdlib_from_text(const char* text) {
  if (!text || !text[0])
    return STDLIB_DYNAMIC;
  if (_stricmp(text, "none") == 0)
    return STDLIB_NONE;
  if (_stricmp(text, "static") == 0)
    return STDLIB_STATIC;
  return STDLIB_DYNAMIC;
}

static bool project_warning_level_from_text(const char* text, warning_level* out) {
  if (!out)
    return false;
  if (!text || !text[0] || _stricmp(text, "default") == 0) {
    *out = WARNING_LEVEL_DEFAULT;
    return true;
  }
  if (_stricmp(text, "none") == 0 || _stricmp(text, "off") == 0) {
    *out = WARNING_LEVEL_NONE;
    return true;
  }
  if (_stricmp(text, "low") == 0 || _stricmp(text, "minimal") == 0) {
    *out = WARNING_LEVEL_LOW;
    return true;
  }
  if (_stricmp(text, "medium") == 0 || _stricmp(text, "med") == 0 || _stricmp(text, "normal") == 0) {
    *out = WARNING_LEVEL_MEDIUM;
    return true;
  }
  if (_stricmp(text, "high") == 0 || _stricmp(text, "all") == 0) {
    *out = WARNING_LEVEL_HIGH;
    return true;
  }
  if (_stricmp(text, "pedantic") == 0 || _stricmp(text, "extra") == 0) {
    *out = WARNING_LEVEL_PEDANTIC;
    return true;
  }
  return false;
}

static bool project_opt_level_from_text(const char* text, opt_level* out) {
  if (!out)
    return false;
  if (!text || !text[0] || _stricmp(text, "default") == 0) {
    *out = OPT_LEVEL_DEFAULT;
    return true;
  }
  if (_stricmp(text, "none") == 0 || _stricmp(text, "off") == 0 || _stricmp(text, "o0") == 0) {
    *out = OPT_LEVEL_NONE;
    return true;
  }
  if (_stricmp(text, "debug") == 0 || _stricmp(text, "og") == 0) {
    *out = OPT_LEVEL_DEBUG;
    return true;
  }
  if (_stricmp(text, "size") == 0 || _stricmp(text, "os") == 0 || _stricmp(text, "oz") == 0) {
    *out = OPT_LEVEL_SIZE;
    return true;
  }
  if (_stricmp(text, "speed_1") == 0 || _stricmp(text, "speed-1") == 0 || _stricmp(text, "o1") == 0) {
    *out = OPT_LEVEL_SPEED_1;
    return true;
  }
  if (_stricmp(text, "speed_2") == 0 || _stricmp(text, "speed-2") == 0 || _stricmp(text, "o2") == 0) {
    *out = OPT_LEVEL_SPEED_2;
    return true;
  }
  if (_stricmp(text, "speed_3") == 0 || _stricmp(text, "speed-3") == 0 || _stricmp(text, "o3") == 0) {
    *out = OPT_LEVEL_SPEED_3;
    return true;
  }
  if (_stricmp(text, "aggressive") == 0 || _stricmp(text, "fast") == 0 || _stricmp(text, "ofast") == 0) {
    *out = OPT_LEVEL_AGGRESSIVE;
    return true;
  }
  return false;
}

static bool project_is_valid_config_name(const char* text) {
  if (!text || !text[0])
    return false;

  for (const char* p = text; *p; ++p) {
    unsigned char ch = (unsigned char)*p;
    if (!(isalnum(ch) || ch == '_' || ch == '-' || ch == '.'))
      return false;
  }

  return true;
}

static bool project_has_config_name(const project* proj, const char* name) {
  if (!proj || !name || !name[0])
    return false;

  for (int i = 0; i < proj->config_c; ++i) {
    if (proj->configs[i] && _stricmp(proj->configs[i], name) == 0)
      return true;
  }

  return false;
}

static bool project_set_config_list(project* proj, const char** configs, int config_c) {
  if (!proj)
    return false;

  int add_default = 0;
  if (!configs || config_c <= 0 || !project_has_config_name(&(project) {.configs = configs, .config_c = config_c}, "default"))
    add_default = 1;

  int total = config_c > 0 ? config_c + add_default : 1;
  const char** items = push(sizeof(*items) * (size_t)total);
  if (!items)
    return false;

  int out = 0;
  if (configs && config_c > 0) {
    for (int i = 0; i < config_c; ++i)
      items[out++] = configs[i];
  }
  if (add_default)
    items[out++] = "default";

  proj->configs = items;
  proj->config_c = out;
  return true;
}

static bool project_parse_configs(node* project_n, project* out) {
  if (!project_n || !out)
    return false;

  const char** configs = NULL;
  int config_c = 0;
  if (!project_parse_string_list(node_get_child(project_n, "configs"), &configs, &config_c))
    return false;

  for (int i = 0; i < config_c; ++i) {
    if (!project_is_valid_config_name(configs[i])) {
      error("Invalid config name '%s'.", configs[i] ? configs[i] : "");
      return false;
    }
    for (int j = i + 1; j < config_c; ++j) {
      if (_stricmp(configs[i], configs[j]) == 0) {
        error("Duplicate config name '%s'.", configs[i]);
        return false;
      }
    }
  }

  return project_set_config_list(out, configs, config_c);
}

static const char* project_resolve_active_config(const project* proj, const char* selected) {
  if (selected && selected[0])
    return selected;
  if (proj && proj->config_c > 0 && project_has_config_name(proj, "default"))
    return "default";
  return proj && proj->config_c > 0 ? proj->configs[0] : "default";
}

static const char* project_scalar_text(node* n) {
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

static bool project_read_text_child(node* scope, const char* name, const char** out) {
  if (out)
    *out = NULL;
  if (!scope || !name || !name[0])
    return false;

  node* child = node_get_child(scope, name);
  if (!child)
    return true;

  const char* text = project_scalar_text(child);
  if (!text) {
    error("Attribute '%s' must be a string or identifier.", name);
    return false;
  }

  if (out)
    *out = text;
  return true;
}

static bool project_read_bool_child(node* scope, const char* name, bool* out) {
  if (out)
    *out = false;
  if (!scope || !name || !name[0])
    return false;

  node* child = node_get_child(scope, name);
  if (!child)
    return true;
  if (child->type != NODE_TYPE_BOL) {
    error("Attribute '%s' must be a boolean.", name);
    return false;
  }

  if (out)
    *out = node_get_bool(child);
  return true;
}

static bool project_read_size_child(node* scope, const char* name, size_t* out) {
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
    *out = (size_t)value;
  return true;
}

static void project_set_default_target(target* out, target_type type) {
  if (!out)
    return;

  memset(out, 0, sizeof(*out));
  out->type = type;
  out->lang = LANG_C;
  out->runtime = STDLIB_DYNAMIC;
  out->warning_level = WARNING_LEVEL_DEFAULT;
  out->opt_level = OPT_LEVEL_DEFAULT;
  out->warnings_as_errors = false;
  out->stack_size = 0;
  out->testing = type == TARGET_TYPE_TEST;
}

static bool project_ensure_target_capacity(project* proj, int min_cap) {
  if (!proj || min_cap <= 0)
    return false;
  if (proj->target_cap >= min_cap)
    return true;

  int new_cap = proj->target_cap > 0 ? proj->target_cap : 8;
  while (new_cap < min_cap)
    new_cap *= 2;

  target* items = push((size_t)new_cap * sizeof(*items));
  if (!items)
    return false;
  memset(items, 0, (size_t)new_cap * sizeof(*items));

  if (proj->targets && proj->target_c > 0)
    memcpy(items, proj->targets, (size_t)proj->target_c * sizeof(*items));

  proj->targets = items;
  proj->target_cap = new_cap;
  return true;
}

static target* project_add_target(project* proj) {
  if (!proj || !project_ensure_target_capacity(proj, proj->target_c + 1))
    return NULL;

  target* out = &proj->targets[proj->target_c++];
  memset(out, 0, sizeof(*out));
  return out;
}

static bool project_parse_string_list(node* list_n, const char*** out_items, int* out_count) {
  if (!out_items || !out_count)
    return false;

  *out_items = NULL;
  *out_count = 0;
  if (!list_n)
    return true;

  if (list_n->type != NODE_TYPE_DEF) {
    const char* text = project_scalar_text(list_n);
    if (!text) {
      error("List attribute must contain only strings or identifiers.");
      return false;
    }

    const char** items = push(sizeof(*items));
    if (!items)
      return false;
    items[0] = text;
    *out_items = items;
    *out_count = 1;
    return true;
  }

  int count = 0;
  int child_count = 0;
  node** children = project_children_in_source_order(list_n, &child_count);
  for (int i = 0; i < child_count; ++i) {
    if (!project_scalar_text(children[i])) {
      error("List attribute '%s' contains a non-scalar item.", list_n->name ? list_n->name : "");
      return false;
    }
    if (project_scalar_text(children[i]))
      ++count;
  }

  if (count == 0)
    return true;

  const char** items = push(sizeof(*items) * (size_t)count);
  if (!items)
    return false;

  int idx = 0;
  for (int i = 0; i < child_count; ++i) {
    const char* text = project_scalar_text(children[i]);
    if (text)
      items[idx++] = text;
  }

  *out_items = items;
  *out_count = idx;
  return true;
}

static bool project_parse_named_scalar_children(node* scope, const char* child_name, const char*** out_items, int* out_count) {
  if (!out_items || !out_count)
    return false;
  if (!scope || !child_name || !child_name[0])
    return true;

  int match_count = 0;
  int child_count = 0;
  node** children = project_children_in_source_order(scope, &child_count);
  for (int i = 0; i < child_count; ++i) {
    node* child = children[i];
    if (child && child->name && _stricmp(child->name, child_name) == 0)
      ++match_count;
  }

  if (match_count <= 0)
    return true;

  const char** items = push(sizeof(*items) * (size_t)match_count);
  if (!items)
    return false;

  int wi = 0;
  for (int i = 0; i < child_count; ++i) {
    node* child = children[i];
    if (!child || !child->name || _stricmp(child->name, child_name) != 0)
      continue;

    const char* text = project_scalar_text(child);
    if (!text) {
      error("Attribute '%s' must be a string or identifier.", child_name);
      return false;
    }
    items[wi++] = text;
  }

  *out_items = items;
  *out_count = wi;
  return true;
}

static bool project_parse_unity_batches(node* unity_n, target_unity_batch** out_batches, int* out_count) {
  if (out_batches)
    *out_batches = NULL;
  if (out_count)
    *out_count = 0;
  if (!unity_n)
    return true;

  int batch_count = 0;
  int child_count = 0;
  node** children = project_children_in_source_order(unity_n, &child_count);
  for (int i = 0; i < child_count; ++i) {
    node* child = children[i];
    if (child && child->name && _stricmp(child->name, "batch") == 0)
      ++batch_count;
  }

  if (batch_count <= 0)
    return true;

  target_unity_batch* batches = push(sizeof(*batches) * (size_t)batch_count);
  if (!batches)
    return false;
  memset(batches, 0, sizeof(*batches) * (size_t)batch_count);

  int wi = 0;
  for (int i = 0; i < child_count; ++i) {
    node* child = children[i];
    if (!child || !child->name || _stricmp(child->name, "batch") != 0)
      continue;

    if (!project_parse_string_list(child, &batches[wi].selectors, &batches[wi].selector_c))
      return false;
    if (batches[wi].selector_c <= 0) {
      error("unity.batch(...) must contain at least one directory, file, or pattern.");
      return false;
    }
    ++wi;
  }

  if (out_batches)
    *out_batches = batches;
  if (out_count)
    *out_count = wi;
  return true;
}

static bool project_apply_unity_node(node* unity_n, target* out, const char* target_label) {
  if (!unity_n || !out)
    return false;
  if (unity_n->type != NODE_TYPE_DEF) {
    error("Attribute 'unity' must be a section for target '%s'.", target_label ? target_label : "");
    return false;
  }

  out->unity_configured = true;
  out->unity_enabled = true;
  out->unity_batch_size_set = false;
  out->unity_batch_size = 0;
  out->unity_batches = NULL;
  out->unity_batch_c = 0;

  int child_count = 0;
  node** children = project_children_in_source_order(unity_n, &child_count);
  for (int i = 0; i < child_count; ++i) {
    node* child = children[i];
    if (!child || !child->name)
      continue;

    if (_stricmp(child->name, "enabled") == 0) {
      if (child->type != NODE_TYPE_BOL) {
        error("Attribute 'unity.enabled' must be a boolean for target '%s'.", target_label ? target_label : "");
        return false;
      }
      out->unity_enabled = node_get_bool(child);
      continue;
    }
    if (_stricmp(child->name, "batch_size") == 0) {
      if (child->type != NODE_TYPE_INT) {
        error("Attribute 'unity.batch_size' must be an integer for target '%s'.", target_label ? target_label : "");
        return false;
      }
      int64_t value = node_get_int(child);
      if (value <= 0) {
        error("Attribute 'unity.batch_size' must be greater than zero for target '%s'.", target_label ? target_label : "");
        return false;
      }
      out->unity_batch_size = (size_t)value;
      out->unity_batch_size_set = true;
      continue;
    }
    if (_stricmp(child->name, "batch") == 0)
      continue;

    error("Unsupported unity attribute '%s' for target '%s'.", child->name, target_label ? target_label : "");
    return false;
  }

  return project_parse_unity_batches(unity_n, &out->unity_batches, &out->unity_batch_c);
}

static bool project_apply_dist_node(node* dist_n, target* out, const char* target_label) {
  if (!dist_n || !out)
    return false;
  if (dist_n->type != NODE_TYPE_DEF) {
    error("Attribute 'dist' must be a section for target '%s'.", target_label ? target_label : "");
    return false;
  }

  if (!project_parse_named_scalar_children(dist_n, "precommand", &out->pre_dist_cmds, &out->pre_dist_cmd_c))
    return false;
  if (!project_parse_named_scalar_children(dist_n, "postcommand", &out->post_dist_cmds, &out->post_dist_cmd_c))
    return false;

  const char* text = NULL;
  if (!project_read_text_child(dist_n, "name", &text))
    return false;
  if (text && text[0])
    out->dist_archive_name = text;

  node* archive_n = node_get_child(dist_n, "archive");
  if (archive_n) {
    if (archive_n->type != NODE_TYPE_BOL) {
      error("Attribute 'archive' must be a boolean.");
      return false;
    }
    out->dist_archive = node_get_bool(archive_n);
  }
  return true;
}

static const char* project_join_scalar_list(node* list_n, int* out_count) {
  if (out_count)
    *out_count = 0;
  if (!list_n)
    return NULL;

  if (list_n->type != NODE_TYPE_DEF) {
    const char* text = project_scalar_text(list_n);
    if (!text) {
      error("List attribute must contain only strings or identifiers.");
      if (out_count)
        *out_count = -1;
      return NULL;
    }
    if (text && out_count)
      *out_count = 1;
    return text;
  }

  int count = 0;
  size_t total = 0;
  int child_count = 0;
  node** children = project_children_in_source_order(list_n, &child_count);
  for (int i = 0; i < child_count; ++i) {
    const char* text = project_scalar_text(children[i]);
    if (!text) {
      error("List attribute '%s' contains a non-scalar item.", list_n->name ? list_n->name : "");
      if (out_count)
        *out_count = -1;
      return NULL;
    }
    total += strlen(text);
    if (count > 0)
      ++total;
    ++count;
  }

  if (out_count)
    *out_count = count;
  if (count == 0)
    return NULL;

  char* out = push(total + 1);
  if (!out)
    return NULL;

  size_t wi = 0;
  for (int i = 0; i < child_count; ++i) {
    const char* text = project_scalar_text(children[i]);
    if (!text)
      continue;
    if (wi > 0)
      out[wi++] = ' ';
    size_t len = strlen(text);
    memcpy(out + wi, text, len);
    wi += len;
  }

  out[wi] = '\0';
  return out;
}

static bool project_parse_meta_fields(node* scope, meta* out) {
  if (!scope || !out)
    return false;

  if (!project_read_text_child(scope, "id", &out->id))
    return false;
  if (!project_read_text_child(scope, "name", &out->name))
    return false;
  if (!project_read_text_child(scope, "authors", &out->authors))
    return false;

  node* repo_n = node_get_child(scope, "repo");
  if (repo_n && repo_n->type != NODE_TYPE_DEF) {
    const char* repo_text = project_scalar_text(repo_n);
    if (!repo_text) {
      error("Attribute 'repo' must be a string or identifier.");
      return false;
    }
    out->repo = repo_text;
  }

  node* ver_n = node_get_child(scope, "ver");
  node* lic_n = node_get_child(scope, "license");

  if (ver_n) {
    if (ver_n->type != NODE_TYPE_VER) {
      error("Attribute 'ver' must be a version value.");
      return false;
    }
    out->ver = node_get_ver(ver_n);
  }

  if (lic_n) {
    if (!project_read_text_child(lic_n, "type", &out->license.type))
      return false;
    if (!project_read_text_child(lic_n, "file", &out->license.file))
      return false;
  }

  return true;
}

static bool project_apply_target_attrs(node* scope, target* out, const char* target_label) {
  if (!scope || !out)
    return false;

  const char* text = NULL;
  if (!project_read_text_child(scope, "lang", &text))
    return false;
  if (text) {
    if (_stricmp(text, "c") != 0 && _stricmp(text, "cpp") != 0 && _stricmp(text, "c++") != 0) {
      error("Unknown lang '%s' for target '%s'.", text, target_label ? target_label : "");
      return false;
    }
    out->lang = project_lang_from_text(text);
  }

  if (!project_read_text_child(scope, "output", &text))
    return false;
  if (text)
    out->output = text;

  if (!project_read_text_child(scope, "path", &text))
    return false;
  if (text)
    out->package_path = text;

  if (!project_read_text_child(scope, "subdir", &text))
    return false;
  if (text)
    out->package_subdir = text;

  if (!project_read_text_child(scope, "cmake_target", &text))
    return false;
  if (text)
    out->package_cmake_target = text;

  node* repo_n = node_get_child(scope, "repo");
  if (repo_n) {
    if (repo_n->type != NODE_TYPE_DEF) {
      error("Attribute 'repo' must be a section for target '%s'.", target_label ? target_label : "");
      return false;
    }
    if (!project_read_text_child(repo_n, "link", &out->package_repo_link))
      return false;
    if (!project_read_text_child(repo_n, "tag", &out->package_repo_tag))
      return false;
    if (!project_read_text_child(repo_n, "commit", &out->package_repo_commit))
      return false;
  }

  node* archive_n = node_get_child(scope, "archive");
  if (archive_n) {
    if (archive_n->type != NODE_TYPE_DEF) {
      error("Attribute 'archive' must be a section for target '%s'.", target_label ? target_label : "");
      return false;
    }
    if (!project_read_text_child(archive_n, "link", &out->package_archive_link))
      return false;
    if (!project_read_text_child(archive_n, "strip_prefix", &out->package_archive_strip_prefix))
      return false;
  }

  node* value_n = node_get_child(scope, "units");
  if (value_n && !project_parse_string_list(value_n, &out->units, &out->unit_c))
    return false;
  value_n = node_get_child(scope, "include_dirs");
  if (value_n && !project_parse_string_list(value_n, &out->include_dirs, &out->include_dir_c))
    return false;
  value_n = node_get_child(scope, "link_dirs");
  if (value_n && !project_parse_string_list(value_n, &out->link_dirs, &out->link_dir_c))
    return false;
  value_n = node_get_child(scope, "dependencies");
  if (value_n && !project_parse_string_list(value_n, &out->dependencies, &out->dependency_c))
    return false;
  value_n = node_get_child(scope, "link_libs");
  if (value_n && !project_parse_string_list(value_n, &out->link_libs, &out->link_libs_count))
    return false;

  value_n = node_get_child(scope, "defines");
  if (value_n) {
    out->defines = project_join_scalar_list(value_n, &out->define_c);
    if (out->define_c < 0)
      return false;
  }

  if (!project_read_text_child(scope, "additional_compile_args", &text))
    return false;
  if (text)
    out->additional_compile_args = text;
  if (!project_read_text_child(scope, "additional_link_args", &text))
    return false;
  if (text)
    out->additional_link_args = text;

  if (!project_read_text_child(scope, "warning_level", &text))
    return false;
  if (text && !project_warning_level_from_text(text, &out->warning_level)) {
    error("Unknown warning_level '%s' for target '%s'.", text, target_label ? target_label : "");
    return false;
  }

  if (!project_read_text_child(scope, "opt_level", &text))
    return false;
  if (text && !project_opt_level_from_text(text, &out->opt_level)) {
    error("Unknown opt_level '%s' for target '%s'.", text, target_label ? target_label : "");
    return false;
  }

  if (!project_read_size_child(scope, "stack_size", &out->stack_size))
    return false;
  if (!project_read_bool_child(scope, "warnings_as_errors", &out->warnings_as_errors))
    return false;

  if (!project_read_text_child(scope, "runtime", &text))
    return false;
  if (text) {
    if (_stricmp(text, "none") != 0 && _stricmp(text, "dynamic") != 0 && _stricmp(text, "static") != 0) {
      error("Unknown runtime '%s' for target '%s'.", text, target_label ? target_label : "");
      return false;
    }
    out->runtime = project_stdlib_from_text(text);
  }
  if (!project_read_text_child(scope, "stdver", &text))
    return false;
  if (text)
    out->stdver = text;

  value_n = node_get_child(scope, "test_args");
  if (value_n && !project_parse_string_list(value_n, &out->test_args, &out->test_arg_c))
    return false;

  value_n = node_get_child(scope, "testing");
  if (value_n) {
    if (value_n->type != NODE_TYPE_BOL) {
      error("Attribute 'testing' must be a boolean.");
      return false;
    }
    out->testing = node_get_bool(value_n);
  }

  value_n = node_get_child(scope, "post_build_cmds");
  if (value_n && !project_parse_string_list(value_n, &out->post_build_cmds, &out->post_build_cmd_c))
    return false;
  value_n = node_get_child(scope, "pre_build_cmds");
  if (value_n && !project_parse_string_list(value_n, &out->pre_build_cmds, &out->pre_build_cmd_c))
    return false;
  value_n = node_get_child(scope, "pre_run_cmds");
  if (value_n && !project_parse_string_list(value_n, &out->pre_run_cmds, &out->pre_run_cmd_c))
    return false;
  value_n = node_get_child(scope, "post_run_cmds");
  if (value_n && !project_parse_string_list(value_n, &out->post_run_cmds, &out->post_run_cmd_c))
    return false;
  value_n = node_get_child(scope, "pre_dist_cmds");
  if (value_n && !project_parse_string_list(value_n, &out->pre_dist_cmds, &out->pre_dist_cmd_c))
    return false;
  value_n = node_get_child(scope, "post_dist_cmds");
  if (value_n && !project_parse_string_list(value_n, &out->post_dist_cmds, &out->post_dist_cmd_c))
    return false;
  value_n = node_get_child(scope, "dist");
  if (value_n && !project_apply_dist_node(value_n, out, target_label))
    return false;
  value_n = node_get_child(scope, "unity");
  if (value_n && !project_apply_unity_node(value_n, out, target_label))
    return false;

  return true;
}

static bool project_apply_target_attr_node(node* attr_n, target* out, const char* target_label) {
  if (!attr_n || !attr_n->name || !out)
    return true;

  const char* text = NULL;
  if (_stricmp(attr_n->name, "lang") == 0) {
    text = project_scalar_text(attr_n);
    if (!text) {
      error("Attribute 'lang' must be a string or identifier.");
      return false;
    }
    if (_stricmp(text, "c") != 0 && _stricmp(text, "cpp") != 0 && _stricmp(text, "c++") != 0) {
      error("Unknown lang '%s' for target '%s'.", text, target_label ? target_label : "");
      return false;
    }
    out->lang = project_lang_from_text(text);
    return true;
  }
  if (_stricmp(attr_n->name, "output") == 0) {
    text = project_scalar_text(attr_n);
    if (!text) {
      error("Attribute 'output' must be a string or identifier.");
      return false;
    }
    out->output = text;
    return true;
  }
  if (_stricmp(attr_n->name, "path") == 0) {
    text = project_scalar_text(attr_n);
    if (!text) {
      error("Attribute 'path' must be a string or identifier.");
      return false;
    }
    out->package_path = text;
    out->package_repo_link = NULL;
    out->package_repo_tag = NULL;
    out->package_repo_commit = NULL;
    out->package_archive_link = NULL;
    out->package_archive_strip_prefix = NULL;
    return true;
  }
  if (_stricmp(attr_n->name, "subdir") == 0) {
    text = project_scalar_text(attr_n);
    if (!text) {
      error("Attribute 'subdir' must be a string or identifier.");
      return false;
    }
    out->package_subdir = text;
    return true;
  }
  if (_stricmp(attr_n->name, "cmake_target") == 0) {
    text = project_scalar_text(attr_n);
    if (!text) {
      error("Attribute 'cmake_target' must be a string or identifier.");
      return false;
    }
    out->package_cmake_target = text;
    return true;
  }
  if (_stricmp(attr_n->name, "repo") == 0) {
    if (attr_n->type != NODE_TYPE_DEF) {
      error("Attribute 'repo' must be a section.");
      return false;
    }
    if (!project_read_text_child(attr_n, "link", &out->package_repo_link))
      return false;
    if (!project_read_text_child(attr_n, "tag", &out->package_repo_tag))
      return false;
    if (!project_read_text_child(attr_n, "commit", &out->package_repo_commit))
      return false;
    out->package_path = NULL;
    out->package_archive_link = NULL;
    out->package_archive_strip_prefix = NULL;
    return true;
  }
  if (_stricmp(attr_n->name, "archive") == 0) {
    if (attr_n->type != NODE_TYPE_DEF) {
      error("Attribute 'archive' must be a section.");
      return false;
    }
    if (!project_read_text_child(attr_n, "link", &out->package_archive_link))
      return false;
    if (!project_read_text_child(attr_n, "strip_prefix", &out->package_archive_strip_prefix))
      return false;
    out->package_path = NULL;
    out->package_repo_link = NULL;
    out->package_repo_tag = NULL;
    out->package_repo_commit = NULL;
    return true;
  }
  if (_stricmp(attr_n->name, "units") == 0)
    return project_parse_string_list(attr_n, &out->units, &out->unit_c);
  if (_stricmp(attr_n->name, "include_dirs") == 0)
    return project_parse_string_list(attr_n, &out->include_dirs, &out->include_dir_c);
  if (_stricmp(attr_n->name, "link_dirs") == 0)
    return project_parse_string_list(attr_n, &out->link_dirs, &out->link_dir_c);
  if (_stricmp(attr_n->name, "dependencies") == 0)
    return project_parse_string_list(attr_n, &out->dependencies, &out->dependency_c);
  if (_stricmp(attr_n->name, "link_libs") == 0)
    return project_parse_string_list(attr_n, &out->link_libs, &out->link_libs_count);
  if (_stricmp(attr_n->name, "defines") == 0) {
    out->defines = project_join_scalar_list(attr_n, &out->define_c);
    return out->define_c >= 0;
  }
  if (_stricmp(attr_n->name, "additional_compile_args") == 0) {
    text = project_scalar_text(attr_n);
    if (!text) {
      error("Attribute 'additional_compile_args' must be a string or identifier.");
      return false;
    }
    out->additional_compile_args = text;
    return true;
  }
  if (_stricmp(attr_n->name, "additional_link_args") == 0) {
    text = project_scalar_text(attr_n);
    if (!text) {
      error("Attribute 'additional_link_args' must be a string or identifier.");
      return false;
    }
    out->additional_link_args = text;
    return true;
  }
  if (_stricmp(attr_n->name, "warning_level") == 0) {
    text = project_scalar_text(attr_n);
    if (!text) {
      error("Attribute 'warning_level' must be a string or identifier.");
      return false;
    }
    if (!project_warning_level_from_text(text, &out->warning_level)) {
      error("Unknown warning_level '%s' for target '%s'.", text, target_label ? target_label : "");
      return false;
    }
    return true;
  }
  if (_stricmp(attr_n->name, "opt_level") == 0) {
    text = project_scalar_text(attr_n);
    if (!text) {
      error("Attribute 'opt_level' must be a string or identifier.");
      return false;
    }
    if (!project_opt_level_from_text(text, &out->opt_level)) {
      error("Unknown opt_level '%s' for target '%s'.", text, target_label ? target_label : "");
      return false;
    }
    return true;
  }
  if (_stricmp(attr_n->name, "stack_size") == 0) {
    if (attr_n->type != NODE_TYPE_INT) {
      error("Attribute 'stack_size' must be an integer.");
      return false;
    }
    int64_t value = node_get_int(attr_n);
    if (value < 0) {
      error("Attribute 'stack_size' cannot be negative.");
      return false;
    }
    out->stack_size = (size_t)value;
    return true;
  }
  if (_stricmp(attr_n->name, "warnings_as_errors") == 0) {
    if (attr_n->type != NODE_TYPE_BOL) {
      error("Attribute 'warnings_as_errors' must be a boolean.");
      return false;
    }
    out->warnings_as_errors = node_get_bool(attr_n);
    return true;
  }
  if (_stricmp(attr_n->name, "runtime") == 0) {
    text = project_scalar_text(attr_n);
    if (!text) {
      error("Attribute 'runtime' must be a string or identifier.");
      return false;
    }
    if (_stricmp(text, "none") != 0 && _stricmp(text, "dynamic") != 0 && _stricmp(text, "static") != 0) {
      error("Unknown runtime '%s' for target '%s'.", text, target_label ? target_label : "");
      return false;
    }
    out->runtime = project_stdlib_from_text(text);
    return true;
  }
  if (_stricmp(attr_n->name, "stdver") == 0) {
    text = project_scalar_text(attr_n);
    if (!text) {
      error("Attribute 'stdver' must be a string or identifier.");
      return false;
    }
    out->stdver = text;
    return true;
  }
  if (_stricmp(attr_n->name, "testing") == 0) {
    if (attr_n->type != NODE_TYPE_BOL) {
      error("Attribute 'testing' must be a boolean.");
      return false;
    }
    out->testing = node_get_bool(attr_n);
    return true;
  }
  if (_stricmp(attr_n->name, "test_args") == 0)
    return project_parse_string_list(attr_n, &out->test_args, &out->test_arg_c);
  if (_stricmp(attr_n->name, "post_build_cmds") == 0)
    return project_parse_string_list(attr_n, &out->post_build_cmds, &out->post_build_cmd_c);
  if (_stricmp(attr_n->name, "pre_build_cmds") == 0)
    return project_parse_string_list(attr_n, &out->pre_build_cmds, &out->pre_build_cmd_c);
  if (_stricmp(attr_n->name, "pre_run_cmds") == 0)
    return project_parse_string_list(attr_n, &out->pre_run_cmds, &out->pre_run_cmd_c);
  if (_stricmp(attr_n->name, "post_run_cmds") == 0)
    return project_parse_string_list(attr_n, &out->post_run_cmds, &out->post_run_cmd_c);
  if (_stricmp(attr_n->name, "pre_dist_cmds") == 0)
    return project_parse_string_list(attr_n, &out->pre_dist_cmds, &out->pre_dist_cmd_c);
  if (_stricmp(attr_n->name, "post_dist_cmds") == 0)
    return project_parse_string_list(attr_n, &out->post_dist_cmds, &out->post_dist_cmd_c);
  if (_stricmp(attr_n->name, "dist") == 0)
    return project_apply_dist_node(attr_n, out, target_label);
  if (_stricmp(attr_n->name, "unity") == 0)
    return project_apply_unity_node(attr_n, out, target_label);

  error("Unsupported filter attribute '%s'.", attr_n->name);
  return false;
}

static const char* project_filter_config_name(node* filter_n) {
  if (!filter_n)
    return NULL;

  int child_count = 0;
  node** children = project_children_in_source_order(filter_n, &child_count);
  if (!children || child_count <= 0) {
    error("filter(...) must start with a config name.");
    return NULL;
  }

  const char* config = project_scalar_text(children[0]);
  if (!config || !config[0]) {
    error("filter(...) must start with a config name.");
    return NULL;
  }

  return config;
}

static bool project_apply_filter_node(node* filter_n, target* out, const char* active_config, const char* target_label) {
  if (!filter_n || !out || !active_config || !active_config[0])
    return false;

  const char* filter_config = project_filter_config_name(filter_n);
  if (!filter_config)
    return false;
  if (_stricmp(filter_config, active_config) != 0)
    return true;

  int child_count = 0;
  node** children = project_children_in_source_order(filter_n, &child_count);
  for (int i = 1; i < child_count; ++i) {
    node* child = children[i];
    if (!child || !child->name)
      continue;
    if (!project_apply_target_attr_node(child, out, target_label))
      return false;
  }

  return true;
}

static bool project_parse_target_node(node* target_n, target* out, const char* active_config) {
  if (!target_n || !target_n->name || !out)
    return false;

  target_type type = project_target_type_from_text(target_n->name);
  if (!project_is_known_target_type_name(target_n->name)) {
    error("Unknown target type '%s'.", target_n->name);
    return false;
  }

  project_set_default_target(out, type);
  if (!project_parse_meta_fields(target_n, &out->meta))
    return false;

  if (!project_apply_target_attrs(target_n, out, target_n->name))
    return false;

  int child_count = 0;
  node** children = project_children_in_source_order(target_n, &child_count);
  for (int i = 0; i < child_count; ++i) {
    node* child = children[i];
    if (!child || !child->name || _stricmp(child->name, "filter") != 0)
      continue;
    if (!project_apply_filter_node(child, out, active_config, target_n->name))
      return false;
  }

  return true;
}

static const char* project_make_default_target_id(target_type type, int index) {
  char buf[64] = {0};
  snprintf(buf, sizeof(buf), "%s_%d", project_target_type_name(type), index + 1);
  return arena_text(buf, strlen(buf));
}

static void project_apply_meta_defaults(meta* dst, const meta* src, bool allow_id) {
  if (!dst || !src)
    return;

  if (allow_id && !dst->id)
    dst->id = src->id;
  if (!dst->name)
    dst->name = src->name;
  if (!project_has_ver(dst->ver) && project_has_ver(src->ver))
    dst->ver = src->ver;
  if (!dst->repo)
    dst->repo = src->repo;
  if (!dst->authors)
    dst->authors = src->authors;
  if (!dst->license.type)
    dst->license.type = src->license.type;
  if (!dst->license.file)
    dst->license.file = src->license.file;
}

static void project_apply_defaults(project* proj) {
  if (!proj)
    return;

  for (int i = 0; i < proj->target_c; ++i) {
    target* tgt = &proj->targets[i];

    project_apply_meta_defaults(&tgt->meta, &proj->meta, proj->target_c == 1);

    if (!tgt->meta.id)
      tgt->meta.id = tgt->output ? tgt->output : project_make_default_target_id(tgt->type, i);
    if (!tgt->output)
      tgt->output = tgt->meta.id;
    if (!tgt->meta.name)
      tgt->meta.name = tgt->meta.id;
  }
}

static bool project_apply_project_filters(node* project_n, project* out) {
  if (!project_n || !out)
    return false;

  int child_count = 0;
  node** children = project_children_in_source_order(project_n, &child_count);
  for (int i = 0; i < child_count; ++i) {
    node* child = children[i];
    if (!child || !child->name || _stricmp(child->name, "filter") != 0)
      continue;

    const char* filter_config = project_filter_config_name(child);
    if (!filter_config)
      return false;
    if (!project_has_config_name(out, filter_config)) {
      error("Filter references unknown config '%s'.", filter_config);
      return false;
    }

    for (int j = 0; j < out->target_c; ++j) {
      if (!project_apply_filter_node(child, &out->targets[j], out->active_config, out->targets[j].meta.id))
        return false;
    }
  }

  return true;
}

static bool project_validate_filter_refs(node* scope, const project* proj) {
  if (!scope || !proj)
    return false;

  int child_count = 0;
  node** children = project_children_in_source_order(scope, &child_count);
  for (int i = 0; i < child_count; ++i) {
    node* child = children[i];
    if (!child || !child->name || _stricmp(child->name, "filter") != 0)
      continue;

    const char* filter_config = project_filter_config_name(child);
    if (!filter_config)
      return false;
    if (!project_has_config_name(proj, filter_config)) {
      error("Filter references unknown config '%s'.", filter_config);
      return false;
    }
  }

  return true;
}

static bool project_validate(const project* proj) {
  if (!proj) {
    error("Parsed project is null.");
    return false;
  }
  if (proj->target_c <= 0) {
    error("Project must define at least one target under 'targets(...)'.");
    return false;
  }

  for (int i = 0; i < proj->target_c; ++i) {
    const target* tgt = &proj->targets[i];
    if (!tgt->meta.id || !tgt->meta.id[0]) {
      error("Target %d resolved to an empty id.", i + 1);
      return false;
    }
    if (!tgt->output || !tgt->output[0]) {
      error("Target '%s' resolved to an empty output.", tgt->meta.id);
      return false;
    }
    if (project_target_is_package(tgt)) {
      int package_sources = 0;
      package_sources += tgt->package_path && tgt->package_path[0] ? 1 : 0;
      package_sources += tgt->package_repo_link && tgt->package_repo_link[0] ? 1 : 0;
      package_sources += tgt->package_archive_link && tgt->package_archive_link[0] ? 1 : 0;
      if (package_sources > 1) {
        error("Target '%s' must define only one package source: path, repo, or archive.", tgt->meta.id);
        return false;
      }
      if (tgt->type == TARGET_TYPE_CONSOLE || tgt->type == TARGET_TYPE_CONSOLELESS || tgt->type == TARGET_TYPE_TEST || tgt->type == TARGET_TYPE_DRIVER) {
        error("Package target '%s' must be a linkable library target.", tgt->meta.id);
        return false;
      }
      if (tgt->unit_c > 0) {
        error("Package target '%s' cannot also define source units.", tgt->meta.id);
        return false;
      }
      if (tgt->unity_configured) {
        error("Package target '%s' cannot use unity builds.", tgt->meta.id);
        return false;
      }
      if (package_sources == 0) {
        error("Package target '%s' is missing its package source.", tgt->meta.id);
        return false;
      }
    } else if (tgt->unit_c <= 0) {
      error("Target '%s' must define at least one source unit.", tgt->meta.id);
      return false;
    }

    if (tgt->unity_configured) {
      if (tgt->type == TARGET_TYPE_HEADER_LIB) {
        error("Target '%s' cannot use unity builds because header libraries are not compiled.", tgt->meta.id);
        return false;
      }
      if (!tgt->unity_enabled && tgt->unity_batch_c > 0) {
        error("Target '%s' cannot define unity batches while unity is disabled.", tgt->meta.id);
        return false;
      }
    }

    for (int j = i + 1; j < proj->target_c; ++j) {
      if (_stricmp(tgt->meta.id, proj->targets[j].meta.id) == 0) {
        error("Duplicate target id '%s'.", tgt->meta.id);
        return false;
      }
    }
  }

  unsigned char* visit_state = push((size_t)proj->target_c * sizeof(*visit_state));
  if (!visit_state)
    return false;
  memset(visit_state, 0, (size_t)proj->target_c * sizeof(*visit_state));

  for (int i = 0; i < proj->target_c; ++i) {
    if (visit_state[i] == 2)
      continue;

    const target* tgt = &proj->targets[i];
    visit_state[i] = 1;
    for (int j = 0; j < tgt->dependency_c; ++j) {
      const char* dep_name = tgt->dependencies[j];
      int dep_idx = project_find_target_index(proj, dep_name);
      if (dep_idx < 0)
        return false;
      if (dep_idx == i) {
        error("Target '%s' cannot depend on itself.", tgt->meta.id);
        return false;
      }
    }
    visit_state[i] = 0;
  }

  for (int root = 0; root < proj->target_c; ++root) {
    if (visit_state[root] != 0)
      continue;

    int* stack = push((size_t)proj->target_c * sizeof(*stack));
    int* edge_idx = push((size_t)proj->target_c * sizeof(*edge_idx));
    if (!stack || !edge_idx)
      return false;

    int sp = 0;
    stack[0] = root;
    edge_idx[0] = 0;
    visit_state[root] = 1;

    while (sp >= 0) {
      int idx = stack[sp];
      const target* tgt = &proj->targets[idx];
      if (edge_idx[sp] >= tgt->dependency_c) {
        visit_state[idx] = 2;
        --sp;
        continue;
      }

      const char* dep_name = tgt->dependencies[edge_idx[sp]++];
      int dep_idx = project_find_target_index(proj, dep_name);
      if (dep_idx < 0)
        return false;
      if (visit_state[dep_idx] == 1) {
        error("Dependency cycle detected between targets '%s' and '%s'.", tgt->meta.id, proj->targets[dep_idx].meta.id);
        return false;
      }
      if (visit_state[dep_idx] == 2)
        continue;

      ++sp;
      stack[sp] = dep_idx;
      edge_idx[sp] = 0;
      visit_state[dep_idx] = 1;
    }
  }

  return true;
}

static bool project_validate_license_shape(node* lic_n, const char* scope_label) {
  if (!lic_n)
    return true;
  if (lic_n->type != NODE_TYPE_DEF) {
    error("Attribute 'license' must be a section in %s.", scope_label ? scope_label : "project config");
    return false;
  }

  node_foreach(lic_n, child) {
    if (!child || !child->name) {
      error("Attribute 'license' in %s must use named attributes.", scope_label ? scope_label : "project config");
      return false;
    }
    if (_stricmp(child->name, "type") != 0 && _stricmp(child->name, "file") != 0) {
      error("Unknown license attribute '%s' in %s.", child->name, scope_label ? scope_label : "project config");
      return false;
    }
  }

  return true;
}

static bool project_validate_filter_shape(node* filter_n, const char* target_label) {
  if (!filter_n)
    return false;
  if (filter_n->type != NODE_TYPE_DEF) {
    error("Attribute 'filter' must be a section for target '%s'.", target_label ? target_label : "");
    return false;
  }

  int child_count = 0;
  node** children = project_children_in_source_order(filter_n, &child_count);
  const char* filter_config = project_filter_config_name(filter_n);
  if (!filter_config)
    return false;

  target scratch = {0};
  project_set_default_target(&scratch, TARGET_TYPE_CONSOLE);
  for (int i = 1; i < child_count; ++i) {
    node* child = children[i];
    if (!child || !child->name) {
      error("filter(%s ...) entries must use named attributes.", filter_config);
      return false;
    }
    if (!project_apply_target_attr_node(child, &scratch, target_label))
      return false;
  }

  return true;
}

static bool project_validate_target_shape(node* target_n) {
  if (!target_n || !target_n->name)
    return false;
  if (!project_is_known_target_type_name(target_n->name)) {
    error("Unknown target type '%s'.", target_n->name ? target_n->name : "");
    return false;
  }

  meta meta_tmp = {0};
  if (!project_parse_meta_fields(target_n, &meta_tmp))
    return false;

  if (!project_validate_license_shape(node_get_child(target_n, "license"), target_n->name))
    return false;

  target scratch = {0};
  project_set_default_target(&scratch, project_target_type_from_text(target_n->name));
  if (!project_apply_target_attrs(target_n, &scratch, target_n->name))
    return false;

  node_foreach(target_n, child) {
    if (!child || !child->name) {
      error("Unexpected scalar item in target '%s'.", target_n->name);
      return false;
    }

    if (_stricmp(child->name, "id") == 0 ||
        _stricmp(child->name, "name") == 0 ||
        _stricmp(child->name, "authors") == 0 ||
        _stricmp(child->name, "ver") == 0 ||
        _stricmp(child->name, "license") == 0 ||
        _stricmp(child->name, "lang") == 0 ||
        _stricmp(child->name, "output") == 0 ||
        _stricmp(child->name, "path") == 0 ||
        _stricmp(child->name, "subdir") == 0 ||
        _stricmp(child->name, "cmake_target") == 0 ||
        _stricmp(child->name, "repo") == 0 ||
        _stricmp(child->name, "archive") == 0 ||
        _stricmp(child->name, "units") == 0 ||
        _stricmp(child->name, "include_dirs") == 0 ||
        _stricmp(child->name, "link_dirs") == 0 ||
        _stricmp(child->name, "dependencies") == 0 ||
        _stricmp(child->name, "link_libs") == 0 ||
        _stricmp(child->name, "defines") == 0 ||
        _stricmp(child->name, "additional_compile_args") == 0 ||
        _stricmp(child->name, "additional_link_args") == 0 ||
        _stricmp(child->name, "warning_level") == 0 ||
        _stricmp(child->name, "opt_level") == 0 ||
        _stricmp(child->name, "stack_size") == 0 ||
        _stricmp(child->name, "warnings_as_errors") == 0 ||
        _stricmp(child->name, "runtime") == 0 ||
        _stricmp(child->name, "stdver") == 0 ||
        _stricmp(child->name, "testing") == 0 ||
        _stricmp(child->name, "test_args") == 0 ||
        _stricmp(child->name, "post_build_cmds") == 0 ||
        _stricmp(child->name, "pre_build_cmds") == 0 ||
        _stricmp(child->name, "pre_run_cmds") == 0 ||
        _stricmp(child->name, "post_run_cmds") == 0 ||
        _stricmp(child->name, "pre_dist_cmds") == 0 ||
        _stricmp(child->name, "post_dist_cmds") == 0 ||
        _stricmp(child->name, "unity") == 0 ||
        _stricmp(child->name, "dist") == 0) {
      continue;
    }

    if (_stricmp(child->name, "filter") == 0) {
      if (!project_validate_filter_shape(child, target_n->name))
        return false;
      continue;
    }

    error("Unknown attribute '%s' in target '%s'.", child->name, target_n->name);
    return false;
  }

  return true;
}

static bool project_validate_project_shape(node* project_n) {
  if (!project_n)
    return false;

  project scratch = {0};
  if (!project_parse_configs(project_n, &scratch))
    return false;

  meta meta_tmp = {0};
  if (!project_parse_meta_fields(project_n, &meta_tmp))
    return false;

  node* repo_n = node_get_child(project_n, "repo");
  if (repo_n && repo_n->type == NODE_TYPE_DEF) {
    error("Attribute 'repo' must be a string or identifier.");
    return false;
  }

  if (!project_validate_license_shape(node_get_child(project_n, "license"), "project config"))
    return false;

  node_foreach(project_n, child) {
    if (!child || !child->name) {
      error("Unexpected scalar item in project config.");
      return false;
    }

    if (_stricmp(child->name, "id") == 0 ||
        _stricmp(child->name, "name") == 0 ||
        _stricmp(child->name, "authors") == 0 ||
        _stricmp(child->name, "repo") == 0 ||
        _stricmp(child->name, "ver") == 0 ||
        _stricmp(child->name, "license") == 0 ||
        _stricmp(child->name, "configs") == 0) {
      continue;
    }

    if (_stricmp(child->name, "filter") == 0) {
      if (!project_validate_filter_shape(child, "project filter"))
        return false;
      continue;
    }

    if (_stricmp(child->name, "targets") == 0) {
      if (child->type != NODE_TYPE_DEF) {
        error("Attribute 'targets' must be a section.");
        return false;
      }
      node_foreach(child, target_n) {
        if (!target_n || !target_n->name) {
          error("targets(...) must contain only named target sections.");
          return false;
        }
        if (!project_validate_target_shape(target_n))
          return false;
      }
      continue;
    }

    error("Unknown attribute '%s' in project config.", child->name);
    return false;
  }

  return true;
}

static const char* project_lang_name(lang value) {
  return value == LANG_CPP ? "cpp" : "c";
}

static const char* project_runtime_name(stdlib value) {
  switch (value) {
    case STDLIB_NONE:
      return "none";
    case STDLIB_STATIC:
      return "static";
    default:
      return "dynamic";
  }
}

static const char* project_warning_level_name(warning_level value) {
  switch (value) {
    case WARNING_LEVEL_NONE:
      return "none";
    case WARNING_LEVEL_LOW:
      return "low";
    case WARNING_LEVEL_MEDIUM:
      return "medium";
    case WARNING_LEVEL_HIGH:
      return "high";
    case WARNING_LEVEL_PEDANTIC:
      return "pedantic";
    default:
      return "default";
  }
}

static const char* project_opt_level_name(opt_level value) {
  switch (value) {
    case OPT_LEVEL_NONE:
      return "none";
    case OPT_LEVEL_DEBUG:
      return "debug";
    case OPT_LEVEL_SIZE:
      return "size";
    case OPT_LEVEL_SPEED_1:
      return "speed_1";
    case OPT_LEVEL_SPEED_2:
      return "speed_2";
    case OPT_LEVEL_SPEED_3:
      return "speed_3";
    case OPT_LEVEL_AGGRESSIVE:
      return "aggressive";
    default:
      return "default";
  }
}

static void project_print_section(const char* title) {
  printf("\n" ANSI_FG_INFO "%s" ANSI_RESET "\n", title);
}

static void project_print_field(const char* label, const char* value) {
  if (!label || !value)
    return;
  print("  %-10s %s", label, value);
}

static void project_print_fieldf(const char* label, const char* fmt, ...) {
  if (!label || !fmt)
    return;

  char value[1024] = {0};
  va_list args;
  va_start(args, fmt);
  vsnprintf(value, sizeof(value), fmt, args);
  va_end(args);
  project_print_field(label, value);
}

static void project_print_list(const char* label, const char** items, int count) {
  if (!label)
    return;

  if (!items || count <= 0) {
    print("%s: []", label);
    return;
  }

  print("%s:", label);
  for (int i = 0; i < count; ++i) {
    print("  - %s", items[i] ? items[i] : "");
  }
}

static void project_print(const project* proj) {
  if (!proj)
    return;

  project_print_section("PROJECT");
  print("Targets: %d", proj->target_c);
  if (proj->active_config)
    print("Active Config: %s", proj->active_config);
  project_print_list("Configs", proj->configs, proj->config_c);
  if (proj->meta.id)
    print("Id: %s", proj->meta.id);
  if (proj->meta.name)
    print("Name: %s", proj->meta.name);
  if (project_has_ver(proj->meta.ver)) {
    if (proj->meta.ver.user != 0)
      print("Version: %u.%u.%u.%u", proj->meta.ver.major, proj->meta.ver.minor, proj->meta.ver.patch, proj->meta.ver.user);
    else
      print("Version: %u.%u.%u", proj->meta.ver.major, proj->meta.ver.minor, proj->meta.ver.patch);
  }
  if (proj->meta.repo)
    print("Repo: %s", proj->meta.repo);
  if (proj->meta.authors)
    print("Authors: %s", proj->meta.authors);
  if (proj->meta.license.type)
    print("License Type: %s", proj->meta.license.type);
  if (proj->meta.license.file)
    print("License File: %s", proj->meta.license.file);

  for (int i = 0; i < proj->target_c; ++i) {
    const target* tgt = &proj->targets[i];

    project_print_section("TARGET");
    print("Index: %d", i + 1);
    if (tgt->meta.id)
      print("Id: %s", tgt->meta.id);
    if (tgt->meta.name)
      print("Name: %s", tgt->meta.name);
    if (project_has_ver(tgt->meta.ver)) {
      if (tgt->meta.ver.user != 0)
        print("Version: %u.%u.%u.%u", tgt->meta.ver.major, tgt->meta.ver.minor, tgt->meta.ver.patch, tgt->meta.ver.user);
      else
        print("Version: %u.%u.%u", tgt->meta.ver.major, tgt->meta.ver.minor, tgt->meta.ver.patch);
    }
    if (tgt->meta.repo)
      print("Repo: %s", tgt->meta.repo);
    if (tgt->meta.authors)
      print("Authors: %s", tgt->meta.authors);
    if (tgt->meta.license.type)
      print("License Type: %s", tgt->meta.license.type);
    if (tgt->meta.license.file)
      print("License File: %s", tgt->meta.license.file);
    print("Type: %s", project_target_type_name(tgt->type));
    print("Output: %s", tgt->output ? tgt->output : "");
    if (project_target_is_package(tgt)) {
      print("Package Source: %s", project_package_source_name(tgt->package_source));
      if (tgt->package_repo_link)
        print("Package Repo: %s", tgt->package_repo_link);
      if (tgt->package_repo_tag)
        print("Package Tag: %s", tgt->package_repo_tag);
      if (tgt->package_repo_commit)
        print("Package Commit: %s", tgt->package_repo_commit);
      if (tgt->package_path)
        print("Package Path: %s", tgt->package_path);
      if (tgt->package_subdir)
        print("Package Subdir: %s", tgt->package_subdir);
      if (tgt->package_cmake_target)
        print("Package CMake Target: %s", tgt->package_cmake_target);
      if (tgt->package_archive_link)
        print("Package Archive: %s", tgt->package_archive_link);
      if (tgt->package_archive_strip_prefix)
        print("Package Strip Prefix: %s", tgt->package_archive_strip_prefix);
      if (tgt->package_cache_dir)
        print("Package Cache Dir: %s", tgt->package_cache_dir);
      if (tgt->package_resolved_dir)
        print("Package Resolved Dir: %s", tgt->package_resolved_dir);
    }
    print("Language: %s", project_lang_name(tgt->lang));
    print("Runtime: %s", project_runtime_name(tgt->runtime));
    print("Warning Level: %s", project_warning_level_name(tgt->warning_level));
    print("Optimization: %s", project_opt_level_name(tgt->opt_level));
    print("Warnings As Errors: %s", tgt->warnings_as_errors ? "true" : "false");
    print("Stack Size: %zu", tgt->stack_size);
    print("Testing: %s", tgt->testing ? "true" : "false");
    if (tgt->stdver)
      print("Std Version: %s", tgt->stdver);
    if (tgt->defines)
      print("Defines: %s", tgt->defines);
    if (tgt->additional_compile_args)
      print("Additional Compile Args: %s", tgt->additional_compile_args);
    if (tgt->additional_link_args)
      print("Additional Link Args: %s", tgt->additional_link_args);
    if (tgt->unity_configured) {
      print("Unity Build: %s", tgt->unity_enabled ? "true" : "false");
      if (tgt->unity_batch_size_set)
        print("Unity Batch Size: %zu", tgt->unity_batch_size);
      for (int ui = 0; ui < tgt->unity_batch_c; ++ui)
        project_print_list("Unity Batch", tgt->unity_batches[ui].selectors, tgt->unity_batches[ui].selector_c);
    }
    project_print_list("Units", tgt->units, tgt->unit_c);
    project_print_list("Include Dirs", tgt->include_dirs, tgt->include_dir_c);
    project_print_list("Link Dirs", tgt->link_dirs, tgt->link_dir_c);
    project_print_list("Dependencies", tgt->dependencies, tgt->dependency_c);
    project_print_list("Link Libs", tgt->link_libs, tgt->link_libs_count);
    project_print_list("Pre Build Cmds", tgt->pre_build_cmds, tgt->pre_build_cmd_c);
    project_print_list("Post Build Cmds", tgt->post_build_cmds, tgt->post_build_cmd_c);
    project_print_list("Pre Run Cmds", tgt->pre_run_cmds, tgt->pre_run_cmd_c);
    project_print_list("Post Run Cmds", tgt->post_run_cmds, tgt->post_run_cmd_c);
    project_print_list("Test Args", tgt->test_args, tgt->test_arg_c);
    project_print_list("Pre Dist Cmds", tgt->pre_dist_cmds, tgt->pre_dist_cmd_c);
    project_print_list("Post Dist Cmds", tgt->post_dist_cmds, tgt->post_dist_cmd_c);
    print("Dist Archive: %s", tgt->dist_archive ? "true" : "false");
    if (tgt->dist_archive_name)
      print("Dist Archive Name: %s", tgt->dist_archive_name);
  }
}

static bool project_parse_project_node(node* project_n, const char* selected_config, project* out) {
  if (!project_n || !out)
    return false;

  memset(out, 0, sizeof(*out));
  if (!project_parse_configs(project_n, out))
    return false;
  out->active_config = project_resolve_active_config(out, selected_config);
  if (!project_has_config_name(out, out->active_config)) {
    error("Unknown config '%s'.", out->active_config ? out->active_config : "");
    return false;
  }
  if (!project_parse_meta_fields(project_n, &out->meta))
    return false;

  node* targets_n = node_get_child(project_n, "targets");
  if (targets_n) {
    int target_count = 0;
    node** children = project_children_in_source_order(targets_n, &target_count);
    for (int i = 0; i < target_count; ++i) {
      node* child = children[i];
      if (!child->name)
        continue;
      if (!project_validate_filter_refs(child, out))
        return false;

      target* tgt = project_add_target(out);
      if (!tgt) {
        error("Failed to allocate project target.");
        return false;
      }
      if (!project_parse_target_node(child, tgt, out->active_config))
        return false;
    }
  }

  if (!project_validate_filter_refs(project_n, out))
    return false;
  if (!project_apply_project_filters(project_n, out))
    return false;

  project_apply_defaults(out);
  return project_validate(out);
}

static bool project_load_file_config(const char* path, const char* config, project* out) {
  if (!path || !path[0] || !out)
    return false;
  if (!file_exists(path)) {
    error("Project config not found: %s", path);
    return false;
  }

  const char* text = read_entire_file(path);
  if (!text) {
    error("Failed to read project config: %s", path);
    return false;
  }

  node* tree = node_parse(text);
  if (!tree) {
    error("Failed to parse project config: %s", path);
    return false;
  }

  node* project_n = NULL;
  int project_count = 0;
  node_foreach(tree, child) {
    if (!child->name || _stricmp(child->name, "project") != 0)
      continue;
    project_n = child;
    ++project_count;
  }

  if (project_count == 0)
    project_n = tree;
  if (project_count > 1) {
    error("Multiple top-level project nodes found in %s.", path);
    return false;
  }

  if (!project_validate_project_shape(project_n))
    return false;

  if (!project_parse_project_node(project_n, config, out))
    return false;

  out->config_path = toolchain_norm_path(path);
  out->root_dir = project_path_parent(out->config_path);
  out->local_cfg_path = toolchain_join2(out->root_dir ? out->root_dir : project_current_workdir(), CFG_INFOS[CFG_LOCAL].filename);
  return project_load_user_config(out->local_cfg_path, &out->user_cfg);
}

static bool project_load_config(const char* config, project* out) {
  return project_load_file_config(get_path_cwd("project.bbs"), config, out);
}

static bool project_load_file(const char* path, project* out) {
  return project_load_file_config(path, NULL, out);
}

static bool project_load(project* out) {
  return project_load_config(NULL, out);
}

static bool project_load_user_config(const char* local_cfg_path, user* out) {
  if (!out)
    return false;

  user* cfg = user_init(NULL);
  if (!cfg)
    return false;

  if (!user_load_paths(get_path_exe(CFG_INFOS[CFG_USER].filename),
                       local_cfg_path ? local_cfg_path : get_path_cwd(CFG_INFOS[CFG_LOCAL].filename),
                       cfg))
    return false;

  *out = *cfg;
  return true;
}

static const char* project_root_dir(const project* proj) {
  if (proj && proj->root_dir && proj->root_dir[0])
    return proj->root_dir;
  return project_current_workdir();
}

static const char* project_resolve_path_from_root(const char* root, const char* path) {
  if (!path || !path[0])
    return NULL;
  if (toolchain_is_abs_path(path))
    return toolchain_norm_path(path);
  return toolchain_norm_path(toolchain_join2(root && root[0] ? root : project_current_workdir(), path));
}

static const char* project_toolchain_root_dir(toolchain* tc) {
  const char* cfg = tc && tc->project_cfg_path ? tc->project_cfg_path : get_path_cwd("project.bbs");
  const char* parent = project_path_parent(cfg);
  return parent && parent[0] ? parent : project_current_workdir();
}

static bool project_target_matches_name(const target* tgt, const char* name) {
  if (!tgt || !name || !name[0])
    return false;
  if (tgt->meta.id && _stricmp(tgt->meta.id, name) == 0)
    return true;
  if (tgt->output && _stricmp(tgt->output, name) == 0)
    return true;
  if (tgt->meta.name && _stricmp(tgt->meta.name, name) == 0)
    return true;
  return false;
}

static bool project_target_is_runnable(const target* tgt) {
  if (!tgt)
    return false;
  return tgt->type == TARGET_TYPE_CONSOLE ||
         tgt->type == TARGET_TYPE_CONSOLELESS ||
         tgt->type == TARGET_TYPE_TEST;
}

static bool project_target_is_test(const target* tgt) {
  return tgt && tgt->testing;
}

static bool project_target_supports_linking(const target* tgt) {
  if (!tgt)
    return false;
  return tgt->type == TARGET_TYPE_HEADER_LIB ||
         tgt->type == TARGET_TYPE_STATIC_LIB ||
         tgt->type == TARGET_TYPE_DYN_LIB ||
         tgt->type == TARGET_TYPE_OBJ_LIB;
}

static bool project_target_is_buildable(const target* tgt) {
  return tgt && !project_target_is_package(tgt);
}

static const char* project_target_build_target_name(const target* tgt) {
  if (!tgt)
    return NULL;
  if (project_target_is_package(tgt)) {
    if (tgt->package_cmake_target && tgt->package_cmake_target[0])
      return tgt->package_cmake_target;
    return tgt->output ? tgt->output : tgt->meta.id;
  }
  return tgt->meta.id;
}

static const char* project_host_os_name(void) {
  return platform_host_os_name();
}

static const char* project_host_arch_name(void) {
  return platform_host_arch_name();
}

static const char* project_resolved_dir(const char* root, const char* config, const char* platform) {
  char buf[_MAX_PATH * 2] = {0};
  const char* cfg = config && config[0] ? config : "default";
  const char* suffix = platform && platform[0] ? platform : NULL;

  if (suffix)
    snprintf(buf, sizeof(buf), "%s/%s-%s", root, cfg, suffix);
  else
    snprintf(buf, sizeof(buf), "%s/%s-%s-%s", root, cfg, project_host_os_name(), project_host_arch_name());

  return arena_text(buf, strlen(buf));
}

static const char* project_build_dir_name(const char* config, const char* platform) {
  char buf[_MAX_PATH] = {0};
  const char* cfg = config && config[0] ? config : "default";
  const char* suffix = platform && platform[0] ? platform : NULL;
  if (!suffix)
    snprintf(buf, sizeof(buf), "%s-%s-%s", cfg, project_host_os_name(), project_host_arch_name());
  else
    snprintf(buf, sizeof(buf), "%s-%s", cfg, suffix);
  return arena_text(buf, strlen(buf));
}

typedef struct {
  char* data;
  size_t len;
  size_t cap;
} project_textbuf;

static bool project_textbuf_reserve(project_textbuf* buf, size_t extra) {
  if (!buf)
    return false;

  size_t need = buf->len + extra + 1;
  if (need <= buf->cap)
    return true;

  size_t new_cap = buf->cap > 0 ? buf->cap : 4096;
  while (new_cap < need)
    new_cap *= 2;

  char* next = realloc(buf->data, new_cap);
  if (!next)
    return false;

  buf->data = next;
  buf->cap = new_cap;
  return true;
}

static bool project_textbuf_append(project_textbuf* buf, const char* text) {
  if (!buf || !text)
    return false;

  size_t len = strlen(text);
  if (!project_textbuf_reserve(buf, len))
    return false;

  memcpy(buf->data + buf->len, text, len);
  buf->len += len;
  buf->data[buf->len] = '\0';
  return true;
}

static bool project_textbuf_appendf(project_textbuf* buf, const char* fmt, ...) {
  if (!buf || !fmt)
    return false;

  va_list args;
  va_start(args, fmt);
  va_list args_copy;
  va_copy(args_copy, args);
  int need = vsnprintf(NULL, 0, fmt, args_copy);
  va_end(args_copy);
  if (need < 0) {
    va_end(args);
    return false;
  }

  if (!project_textbuf_reserve(buf, (size_t)need)) {
    va_end(args);
    return false;
  }

  vsnprintf(buf->data + buf->len, buf->cap - buf->len, fmt, args);
  va_end(args);
  buf->len += (size_t)need;
  return true;
}

static const char* project_dup_text(const char* text) {
  if (!text)
    text = "";

  size_t len = strlen(text);
  char* out = malloc(len + 1);
  if (!out)
    return NULL;

  memcpy(out, text, len + 1);
  return out;
}

static const char* project_normalize_slashes(const char* text) {
  if (!text)
    return project_dup_text("");

  size_t len = strlen(text);
  char* out = malloc(len + 1);
  if (!out)
    return NULL;

  for (size_t i = 0; i < len; ++i)
    out[i] = text[i] == '\\' ? '/' : text[i];
  out[len] = '\0';
  return out;
}

static const char* project_escape_cmake_string(const char* text) {
  if (!text)
    return project_dup_text("");

  size_t len = 0;
  for (const char* p = text; *p; ++p) {
    if (*p == '\\' || *p == '"')
      len += 2;
    else
      len += 1;
  }

  char* out = malloc(len + 1);
  if (!out)
    return NULL;

  size_t wi = 0;
  for (const char* p = text; *p; ++p) {
    if (*p == '\\' || *p == '"')
      out[wi++] = '\\';
    out[wi++] = *p;
  }
  out[wi] = '\0';
  return out;
}

static bool project_write_file_if_changed(const char* path, const char* text, bool* changed) {
  if (changed)
    *changed = false;
  if (!path || !text)
    return false;

  const char* current = file_exists(path) ? read_entire_file(path) : NULL;
  if (current && strcmp(current, text) == 0)
    return true;

  if (!write_entire_file(path, text))
    return false;

  if (changed)
    *changed = true;
  return true;
}

static const char* project_cmake_path_text(const char* path) {
  const char* norm = project_normalize_slashes(path);
  if (!norm)
    return NULL;
  return project_escape_cmake_string(norm);
}

static bool project_text_has_wildcards(const char* text) {
  return text && (strchr(text, '*') || strchr(text, '?'));
}

static const char* project_platform_id(os target_os, arch target_arch) {
  char buf[64] = {0};
  snprintf(buf, sizeof(buf), "%s-%s", OS_NAMES[target_os], ARCH_NAMES[target_arch]);
  return arena_text(buf, strlen(buf));
}

static const char* project_default_platform_id(void) {
  return project_platform_id((os)toolchain_detect_host_os(), (arch)toolchain_detect_host_arch());
}

static bool project_parse_platform_id(const char* platform, os* out_os, arch* out_arch) {
  if (out_os)
    *out_os = OS_MAX;
  if (out_arch)
    *out_arch = ARCH_MAX;
  if (!platform || !platform[0])
    return false;

  const char* dash = strchr(platform, '-');
  if (!dash || dash == platform || !dash[1])
    return false;

  char os_name[32] = {0};
  size_t os_len = (size_t)(dash - platform);
  if (os_len >= sizeof(os_name))
    return false;
  memcpy(os_name, platform, os_len);
  os_name[os_len] = '\0';

  const char* arch_name = dash + 1;
  int osi = -1;
  int ai = -1;
  for (int i = 0; i < OS_MAX; ++i)
    if (_stricmp(os_name, OS_NAMES[i]) == 0)
      osi = i;
  for (int i = 0; i < ARCH_MAX; ++i)
    if (_stricmp(arch_name, ARCH_NAMES[i]) == 0)
      ai = i;

  if (osi < 0 || ai < 0)
    return false;

  if (out_os)
    *out_os = (os)osi;
  if (out_arch)
    *out_arch = (arch)ai;
  return true;
}

static const char* project_resolve_platform_id(const char* platform, toolchain* tc) {
  const char* resolved = platform && platform[0] ? platform : project_default_platform_id();
  os target_os = OS_MAX;
  arch target_arch = ARCH_MAX;
  if (!project_parse_platform_id(resolved, &target_os, &target_arch)) {
    error("Invalid platform '%s'. Expected '<os>-<arch>'.", resolved);
    return NULL;
  }

  if (tc && !tc->supported[target_os][target_arch]) {
    error("Platform '%s' is not supported by the current toolchain.", resolved);
    return NULL;
  }

  return resolved;
}

static const char* project_cmake_config_name(const char* config) {
  if (!config || !config[0])
    return "Debug";
  if (_stricmp(config, "release") == 0 || _stricmp(config, "dist") == 0 || _stricmp(config, "shipping") == 0)
    return "Release";
  if (_stricmp(config, "minsizerel") == 0 || _stricmp(config, "minsize") == 0)
    return "MinSizeRel";
  if (_stricmp(config, "relwithdebinfo") == 0 || _stricmp(config, "profile") == 0)
    return "RelWithDebInfo";
  return "Debug";
}

static const char* project_cmake_arch_name(arch value) {
  switch (value) {
    case ARCH_X86:
      return "Win32";
    case ARCH_ARM64:
      return "ARM64";
    default:
      return "x64";
  }
}

static const char* project_cmake_c_standard(const char* stdver) {
  if (!stdver || !stdver[0])
    return NULL;
  if (_stricmp(stdver, "c89") == 0 || _stricmp(stdver, "c90") == 0)
    return "90";
  if (_stricmp(stdver, "c99") == 0)
    return "99";
  if (_stricmp(stdver, "c11") == 0)
    return "11";
  if (_stricmp(stdver, "c17") == 0 || _stricmp(stdver, "c18") == 0)
    return "17";
  if (_stricmp(stdver, "c23") == 0)
    return "23";
  return NULL;
}

static const char* project_cmake_cpp_standard(const char* stdver) {
  if (!stdver || !stdver[0])
    return NULL;
  if (_stricmp(stdver, "c++98") == 0 || _stricmp(stdver, "cpp98") == 0)
    return "98";
  if (_stricmp(stdver, "c++11") == 0 || _stricmp(stdver, "cpp11") == 0)
    return "11";
  if (_stricmp(stdver, "c++14") == 0 || _stricmp(stdver, "cpp14") == 0)
    return "14";
  if (_stricmp(stdver, "c++17") == 0 || _stricmp(stdver, "cpp17") == 0)
    return "17";
  if (_stricmp(stdver, "c++20") == 0 || _stricmp(stdver, "cpp20") == 0)
    return "20";
  if (_stricmp(stdver, "c++23") == 0 || _stricmp(stdver, "cpp23") == 0)
    return "23";
  return NULL;
}

static bool project_append_cmake_warning_level(project_textbuf* buf, const char* target_name, const char* scope, warning_level level) {
  if (!buf || !target_name || level == WARNING_LEVEL_DEFAULT)
    return true;

  switch (level) {
    case WARNING_LEVEL_NONE:
      return project_textbuf_appendf(buf,
                                     "target_compile_options(%s %s $<$<C_COMPILER_ID:MSVC>:/W0> $<$<CXX_COMPILER_ID:MSVC>:/W0> "
                                     "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-w> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>)\n",
                                     target_name,
                                     scope);
    case WARNING_LEVEL_LOW:
      return project_textbuf_appendf(buf,
                                     "target_compile_options(%s %s $<$<C_COMPILER_ID:MSVC>:/W1> $<$<CXX_COMPILER_ID:MSVC>:/W1> "
                                     "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>)\n",
                                     target_name,
                                     scope);
    case WARNING_LEVEL_MEDIUM:
      return project_textbuf_appendf(buf,
                                     "target_compile_options(%s %s $<$<C_COMPILER_ID:MSVC>:/W2> $<$<CXX_COMPILER_ID:MSVC>:/W2> "
                                     "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>)\n",
                                     target_name,
                                     scope);
    case WARNING_LEVEL_HIGH:
      return project_textbuf_appendf(buf,
                                     "target_compile_options(%s %s $<$<C_COMPILER_ID:MSVC>:/W3> $<$<CXX_COMPILER_ID:MSVC>:/W3> "
                                     "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>)\n",
                                     target_name,
                                     scope);
    case WARNING_LEVEL_PEDANTIC:
      return project_textbuf_appendf(buf,
                                     "target_compile_options(%s %s $<$<C_COMPILER_ID:MSVC>:/W4> $<$<CXX_COMPILER_ID:MSVC>:/W4> "
                                     "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>)\n",
                                     target_name,
                                     scope);
    default:
      return true;
  }
}

static bool project_append_cmake_opt_level(project_textbuf* buf, const char* target_name, const char* scope, opt_level level) {
  if (!buf || !target_name || level == OPT_LEVEL_DEFAULT)
    return true;

  switch (level) {
    case OPT_LEVEL_NONE:
      return project_textbuf_appendf(buf,
                                     "target_compile_options(%s %s $<$<C_COMPILER_ID:MSVC>:/Od> $<$<CXX_COMPILER_ID:MSVC>:/Od> "
                                     "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-O0> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O0>)\n",
                                     target_name,
                                     scope);
    case OPT_LEVEL_DEBUG:
      return project_textbuf_appendf(buf,
                                     "target_compile_options(%s %s $<$<C_COMPILER_ID:MSVC>:/Od> $<$<CXX_COMPILER_ID:MSVC>:/Od> "
                                     "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-Og> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Og>)\n",
                                     target_name,
                                     scope);
    case OPT_LEVEL_SIZE:
      return project_textbuf_appendf(buf,
                                     "target_compile_options(%s %s $<$<C_COMPILER_ID:MSVC>:/O1> $<$<CXX_COMPILER_ID:MSVC>:/O1> "
                                     "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-Os> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Os>)\n",
                                     target_name,
                                     scope);
    case OPT_LEVEL_SPEED_1:
      return project_textbuf_appendf(buf,
                                     "target_compile_options(%s %s $<$<C_COMPILER_ID:MSVC>:/O1> $<$<CXX_COMPILER_ID:MSVC>:/O1> "
                                     "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-O1> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O1>)\n",
                                     target_name,
                                     scope);
    case OPT_LEVEL_SPEED_2:
      return project_textbuf_appendf(buf,
                                     "target_compile_options(%s %s $<$<C_COMPILER_ID:MSVC>:/O2> $<$<CXX_COMPILER_ID:MSVC>:/O2> "
                                     "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-O2> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O2>)\n",
                                     target_name,
                                     scope);
    case OPT_LEVEL_SPEED_3:
    case OPT_LEVEL_AGGRESSIVE:
      return project_textbuf_appendf(buf,
                                     "target_compile_options(%s %s $<$<C_COMPILER_ID:MSVC>:/Ox> $<$<CXX_COMPILER_ID:MSVC>:/Ox> "
                                     "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-O3> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O3>)\n",
                                     target_name,
                                     scope);
    default:
      return true;
  }
}

static const char* project_build_root_abs(const project* proj) {
  return project_resolve_path_from_root(project_root_dir(proj), proj && proj->user_cfg.builddir ? proj->user_cfg.builddir : DEF_BUILD_DIR);
}

static const char* project_build_file_abs(const project* proj, const char* filename) {
  return toolchain_join2(project_build_root_abs(proj), filename);
}

static const char* project_build_binary_dir_abs(const project* proj, const char* config, const char* platform) {
  return project_resolve_path_from_root(project_root_dir(proj), project_resolved_dir(proj && proj->user_cfg.builddir ? proj->user_cfg.builddir : DEF_BUILD_DIR, config, platform));
}

static bool project_ensure_dir_tree(const char* path, const char* label) {
  if (!path || !path[0])
    return true;
  if (dir_exists(path))
    return true;

  size_t len = strlen(path);
  char* buf = push(len + 1);
  if (!buf)
    return false;
  memcpy(buf, path, len + 1);

  size_t start = 0;
  if (len >= 3 && buf[1] == ':' && (buf[2] == '\\' || buf[2] == '/'))
    start = 3;
  else if (len >= 1 && (buf[0] == '\\' || buf[0] == '/'))
    start = 1;

  for (size_t i = start; i < len; ++i) {
    if (buf[i] != '\\' && buf[i] != '/')
      continue;
    if (i == 0)
      continue;

    char saved = buf[i];
    buf[i] = '\0';
    if (buf[0] && !dir_exists(buf) && !dir_create(buf)) {
      error("Failed to create %s: %s", label ? label : "directory", buf);
      return false;
    }
    buf[i] = saved;
  }

  if (!dir_exists(buf) && !dir_create(buf)) {
    error("Failed to create %s: %s", label ? label : "directory", buf);
    return false;
  }

  return true;
}

static bool project_ensure_dir_exists(const char* path, const char* label) {
  if (!path || !path[0])
    return false;
  if (dir_exists(path))
    return true;
  if (!dir_create(path)) {
    error("Failed to create %s: %s", label ? label : "directory", path);
    return false;
  }
  print("Created %s at %s", label ? label : "directory", path);
  return true;
}

static const char* project_package_root_abs(void) {
  return get_path_exe("packages");
}

static bool project_path_has_ext(const char* path, const char* ext) {
  if (!path || !ext)
    return false;
  size_t pl = strlen(path);
  size_t el = strlen(ext);
  return pl >= el && _stricmp(path + pl - el, ext) == 0;
}

static const char* project_path_without_git_suffix(const char* text) {
  if (!text)
    return NULL;
  size_t len = strlen(text);
  if (!project_path_has_ext(text, ".git"))
    return arena_text(text, len);
  return arena_text(text, len - 4);
}

static uint32_t project_hash_text(const char* text) {
  uint32_t hash = 2166136261u;
  if (!text)
    return hash;
  for (const unsigned char* p = (const unsigned char*)text; *p; ++p) {
    hash ^= *p;
    hash *= 16777619u;
  }
  return hash;
}

static const char* project_sanitize_key(const char* text, size_t max_len) {
  if (!text || !text[0])
    return arena_text("pkg", 3);
  char* out = push(max_len + 1);
  if (!out)
    return NULL;
  size_t wi = 0;
  bool last_sep = false;
  for (const unsigned char* p = (const unsigned char*)text; *p && wi < max_len; ++p) {
    if (isalnum(*p)) {
      out[wi++] = (char)tolower(*p);
      last_sep = false;
    } else if (!last_sep && wi > 0) {
      out[wi++] = '_';
      last_sep = true;
    }
  }
  while (wi > 0 && out[wi - 1] == '_')
    --wi;
  if (wi == 0) {
    out[0] = 'p'; out[1] = 'k'; out[2] = 'g'; wi = 3;
  }
  out[wi] = '\0';
  return out;
}

static const char* project_path_basename_text(const char* path) {
  if (!path || !path[0])
    return NULL;
  const char* last = strrchr(path, '/');
  const char* bslash = strrchr(path, '\\');
  if (!last || (bslash && bslash > last))
    last = bslash;
  return last ? last + 1 : path;
}

static const char* project_package_cache_key_compact(const target* tgt) {
  const char* base = NULL;
  if (tgt->package_repo_link && tgt->package_repo_link[0])
    base = project_path_basename_text(project_path_without_git_suffix(tgt->package_repo_link));
  else if (tgt->package_archive_link && tgt->package_archive_link[0])
    base = project_path_basename_text(tgt->package_archive_link);
  else if (tgt->package_path && tgt->package_path[0])
    base = project_path_basename_text(tgt->package_path);
  if (!base || !base[0])
    base = tgt->meta.id ? tgt->meta.id : "pkg";

  const char* clean = project_sanitize_key(base, 24);
  char suffix_src[1024] = {0};
  snprintf(suffix_src,
           sizeof(suffix_src),
           "%s|%s|%s|%s|%s",
           tgt->package_repo_link ? tgt->package_repo_link : "",
           tgt->package_repo_tag ? tgt->package_repo_tag : "",
           tgt->package_repo_commit ? tgt->package_repo_commit : "",
           tgt->package_archive_link ? tgt->package_archive_link : "",
           tgt->package_archive_strip_prefix ? tgt->package_archive_strip_prefix : "");
  uint32_t hash = project_hash_text(suffix_src);
  char buf[64] = {0};
  snprintf(buf, sizeof(buf), "%s_%08x", clean, hash);
  return arena_text(buf, strlen(buf));
}

static const char* project_repo_cache_key(const char* link, const char* tag) {
  if (!link || !link[0])
    return NULL;

  const char* clean = project_path_without_git_suffix(link);
  size_t in_len = strlen(clean) + (tag && tag[0] ? strlen(tag) + 1 : 0);
  char* out = push(in_len + 1);
  if (!out)
    return NULL;

  size_t wi = 0;
  bool last_sep = false;
  for (const char* p = clean; *p; ++p) {
    unsigned char ch = (unsigned char)*p;
    if (isalnum(ch)) {
      out[wi++] = (char)tolower(ch);
      last_sep = false;
    } else if (!last_sep) {
      out[wi++] = '_';
      last_sep = true;
    }
  }
  if (tag && tag[0]) {
    if (wi > 0 && !last_sep)
      out[wi++] = '_';
    for (const char* p = tag; *p; ++p) {
      unsigned char ch = (unsigned char)*p;
      if (isalnum(ch))
        out[wi++] = (char)tolower(ch);
      else if (wi > 0 && out[wi - 1] != '_')
        out[wi++] = '_';
    }
  }
  while (wi > 0 && out[wi - 1] == '_')
    --wi;
  if (wi == 0) {
    out[0] = 'p';
    out[1] = 'k';
    out[2] = 'g';
    wi = 3;
  }
  out[wi] = '\0';
  return out;
}

static const char* project_package_cache_dir_for(const target* tgt) {
  const char* root = project_package_root_abs();
  const char* compact = project_package_cache_key_compact(tgt);
  const char* compact_dir = toolchain_norm_path(toolchain_join2(root, compact));
  if (dir_exists(compact_dir))
    return compact_dir;

  if (tgt->package_source == PACKAGE_SOURCE_REPO) {
    const char* legacy = project_repo_cache_key(tgt->package_repo_link, tgt->package_repo_tag);
    const char* legacy_dir = toolchain_norm_path(toolchain_join2(root, legacy));
    if (dir_exists(legacy_dir))
      return legacy_dir;
  }
  if (tgt->package_source == PACKAGE_SOURCE_ARCHIVE) {
    const char* legacy = project_repo_cache_key(tgt->package_archive_link, tgt->package_archive_strip_prefix);
    const char* legacy_dir = toolchain_norm_path(toolchain_join2(root, legacy));
    if (dir_exists(legacy_dir))
      return legacy_dir;
  }
  return compact_dir;
}

static const char* project_resolve_input_path(const char* path) {
  return project_resolve_path_from_root(project_current_workdir(), path);
}

static package_backend project_package_backend_detect(const char* dir, const char** out_project_cfg_path) {
  if (out_project_cfg_path)
    *out_project_cfg_path = NULL;
  if (!dir || !dir[0])
    return PACKAGE_BACKEND_NONE;

  const char* cmakelists = toolchain_join2(dir, "CMakeLists.txt");
  if (file_exists(cmakelists))
    return PACKAGE_BACKEND_CMAKE;

  const char* project_cfg = toolchain_join2(dir, "project.bbs");
  if (file_exists(project_cfg)) {
    if (out_project_cfg_path)
      *out_project_cfg_path = project_cfg;
    return PACKAGE_BACKEND_BBS;
  }

  return PACKAGE_BACKEND_NONE;
}

static bool project_text_looks_like_url(const char* text) {
  return text && (strstr(text, "://") != NULL || strncmp(text, "git@", 4) == 0);
}

static const char* project_resolve_repo_link(const char* link) {
  if (!link || !link[0])
    return NULL;
  return project_text_looks_like_url(link) ? link : project_resolve_input_path(link);
}

static const char* project_shell_quote(const char* text) {
  if (!text)
    return NULL;

  size_t len = strlen(text);
  char* out = push(len * 2 + 3);
  if (!out)
    return NULL;

  size_t wi = 0;
  out[wi++] = '"';
  for (size_t i = 0; i < len; ++i) {
    char ch = text[i];
    if (ch == '"' || ch == '\\' || ch == '$' || ch == '`')
      out[wi++] = '\\';
    out[wi++] = ch;
  }
  out[wi++] = '"';
  out[wi] = '\0';
  return out;
}

static const char* project_package_effective_dir(const target* tgt, const char* root_dir) {
  if (!root_dir || !root_dir[0])
    return root_dir;
  const char* dir = root_dir;
  if (tgt && tgt->package_archive_strip_prefix && tgt->package_archive_strip_prefix[0])
    dir = toolchain_join2(dir, tgt->package_archive_strip_prefix);
  if (tgt && tgt->package_subdir && tgt->package_subdir[0])
    dir = toolchain_join2(dir, tgt->package_subdir);
  return toolchain_norm_path(dir);
}

static const char* project_command_capture_first_line(const char* cmd) {
  if (!cmd || !cmd[0])
    return NULL;

  FILE* pipe = platform_popen_read(cmd);
  if (!pipe)
    return NULL;

  char line[1024] = {0};
  const char* out = NULL;
  if (fgets(line, sizeof(line), pipe) != NULL) {
    size_t len = strlen(line);
    while (len > 0 && isspace((unsigned char)line[len - 1]))
      line[--len] = '\0';
    for (size_t i = 0; i < len; ++i) {
      if (isspace((unsigned char)line[i])) {
        line[i] = '\0';
        len = i;
        break;
      }
    }
    if (len > 0)
      out = arena_text(line, len);
  }
  platform_pclose_read(pipe);
  return out;
}

static const char* project_bash_capture_first_line(toolchain* tc, const char* script) {
  if (!tc || !script || !script[0])
    return NULL;

  const char* tmp = toolchain_norm_path(toolchain_join2(project_current_workdir(), ".bbs-package-probe.txt"));
  if (!tmp)
    return NULL;

  char wrapped[4096] = {0};
  snprintf(wrapped, sizeof(wrapped), "%s > %s", script, project_shell_quote(tmp));
  if (toolchain_run_bash(tc, project_current_workdir(), wrapped) != 0)
    return NULL;

  const char* text = read_entire_file(tmp);
  file_delete(tmp);
  if (!text || !text[0])
    return NULL;

  size_t len = strcspn(text, "\r\n\t ");
  return len > 0 ? arena_text(text, len) : NULL;
}

static const char* project_git_resolve_local_head(toolchain* tc, const char* repo_dir) {
  const char* git = toolchain_get_host_tool_path(tc, "git");
  if (!git || !git[0] || !repo_dir || !repo_dir[0])
    return NULL;
  char script[4096] = {0};
  snprintf(script, sizeof(script), "%s -C %s rev-parse HEAD 2>/dev/null", project_shell_quote(toolchain_norm_path(git)), project_shell_quote(toolchain_norm_path(repo_dir)));
  return project_bash_capture_first_line(tc, script);
}

static const char* project_git_resolve_remote_ref(toolchain* tc, const char* repo_link, const char* repo_tag, const char* repo_commit) {
  const char* git = toolchain_get_host_tool_path(tc, "git");
  if (!git || !git[0] || !repo_link || !repo_link[0])
    return NULL;
  const char* resolved_link = project_resolve_repo_link(repo_link);

  if (repo_commit && repo_commit[0])
    return repo_commit;

  char script[4096] = {0};
  if (repo_tag && repo_tag[0]) {
    snprintf(script,
             sizeof(script),
             "%s ls-remote %s %s %s 2>/dev/null",
             project_shell_quote(toolchain_norm_path(git)),
             project_shell_quote(toolchain_norm_path(resolved_link)),
             project_shell_quote(toolchain_append_text("refs/tags/", repo_tag)),
             project_shell_quote(toolchain_append_text(toolchain_append_text("refs/tags/", repo_tag), "^{}")));
  } else {
    snprintf(script,
             sizeof(script),
             "%s ls-remote %s HEAD 2>/dev/null",
             project_shell_quote(toolchain_norm_path(git)),
             project_shell_quote(toolchain_norm_path(resolved_link)));
  }
  return project_bash_capture_first_line(tc, script);
}

static const char* project_package_status_label(const project_package_info* info) {
  return info && info->status ? info->status : "unknown";
}

static bool project_package_query(const target* tgt, toolchain* tc, project_package_info* out) {
  if (!out)
    return false;
  memset(out, 0, sizeof(*out));
  if (!tgt || !project_target_is_package(tgt))
    return true;

  if (tgt->package_source == PACKAGE_SOURCE_PATH) {
    out->source = project_package_effective_dir(tgt, project_resolve_path_from_root(project_toolchain_root_dir(tc), tgt->package_path));
    out->exists = dir_exists(out->source);
    out->backend = project_package_backend_detect(out->source, NULL);
    out->has_cmakelists = out->backend == PACKAGE_BACKEND_CMAKE;
    out->has_project_config = out->backend == PACKAGE_BACKEND_BBS;
    out->status = !out->exists ? "missing" : (out->backend != PACKAGE_BACKEND_NONE ? "ready" : "invalid");
    return true;
  }

  if (tgt->package_source == PACKAGE_SOURCE_REPO) {
    out->cache_dir = project_package_cache_dir_for(tgt);
    out->source = project_package_effective_dir(tgt, out->cache_dir);
    out->is_cached = true;
    out->exists = dir_exists(out->source);
    out->backend = project_package_backend_detect(out->source, NULL);
    out->has_cmakelists = out->backend == PACKAGE_BACKEND_CMAKE;
    out->has_project_config = out->backend == PACKAGE_BACKEND_BBS;
    out->local_ref = dir_exists(out->cache_dir) ? project_git_resolve_local_head(tc, out->cache_dir) : NULL;
    out->remote_ref = project_git_resolve_remote_ref(tc, tgt->package_repo_link, tgt->package_repo_tag, tgt->package_repo_commit);
    if (!dir_exists(out->cache_dir))
      out->status = "missing";
    else if (out->local_ref && out->remote_ref)
      out->status = _stricmp(out->local_ref, out->remote_ref) == 0 ? "up-to-date" : "outdated";
    else
      out->status = out->backend != PACKAGE_BACKEND_NONE ? "cached" : "invalid";
    return true;
  }

  if (tgt->package_source == PACKAGE_SOURCE_ARCHIVE) {
    out->cache_dir = project_package_cache_dir_for(tgt);
    out->source = project_package_effective_dir(tgt, out->cache_dir);
    out->is_cached = true;
    out->exists = dir_exists(out->source);
    out->backend = project_package_backend_detect(out->source, NULL);
    out->has_cmakelists = out->backend == PACKAGE_BACKEND_CMAKE;
    out->has_project_config = out->backend == PACKAGE_BACKEND_BBS;
    out->status = dir_exists(out->cache_dir) ? (out->backend != PACKAGE_BACKEND_NONE ? "cached" : "invalid") : "missing";
  }

  return true;
}

static bool project_prepare_embedded_backend(project* proj, toolchain* tc, const char* platform, bool refresh_packages) {
  if (!proj || !tc)
    return false;

  const char* build_root = project_build_root_abs(proj);
  if (!project_ensure_dir_exists(build_root, "package build directory"))
    return false;
  if (!project_prepare_packages(proj, tc, platform, refresh_packages))
    return false;
  return project_generate_cmakelists(proj, tc, platform, NULL);
}

static bool project_prepare_bbs_package(target* tgt, toolchain* tc, const char* platform, bool refresh) {
  if (!tgt || tgt->package_backend != PACKAGE_BACKEND_BBS || !tgt->package_project_cfg_path)
    return true;

  project pkg = {0};
  if (!project_load_file(tgt->package_project_cfg_path, &pkg)) {
    error("Failed to load package project for '%s': %s", tgt->meta.id ? tgt->meta.id : "", tgt->package_project_cfg_path ? tgt->package_project_cfg_path : "");
    return false;
  }

  toolchain nested_tc = *tc;
  nested_tc.project_cfg_path = pkg.config_path;
  nested_tc.local_cfg_path = pkg.local_cfg_path;
  if (!project_prepare_embedded_backend(&pkg, &nested_tc, platform, refresh)) {
    error("Failed to prepare nested package backend for '%s'.", tgt->meta.id ? tgt->meta.id : "");
    return false;
  }

  tgt->package_build_dir = project_build_root_abs(&pkg);
  return true;
}

static bool project_fetch_repo_package(target* tgt, toolchain* tc, bool refresh) {
  if (!tgt || tgt->package_source != PACKAGE_SOURCE_REPO)
    return true;

  project_package_info info = {0};
  if (!project_package_query(tgt, tc, &info))
    return false;

  const char* packages_root = toolchain_norm_path(project_package_root_abs());

  tgt->package_cache_dir = info.cache_dir;
  tgt->package_resolved_dir = project_package_effective_dir(tgt, info.cache_dir);
  if (refresh && info.exists) {
    print("Refreshing package '%s' at %s", tgt->meta.id ? tgt->meta.id : "", info.cache_dir ? info.cache_dir : "");
    char delete_script[2048] = {0};
    snprintf(delete_script, sizeof(delete_script), "rm -rf %s", project_shell_quote(info.cache_dir));
    if (toolchain_run_bash(tc, packages_root, delete_script) != 0 || dir_exists(info.cache_dir)) {
      error("Failed to delete cached package '%s': %s", tgt->meta.id ? tgt->meta.id : "", info.cache_dir ? info.cache_dir : "");
      return false;
    }
    info.exists = false;
  }
  if (info.exists)
    return true;

  if (!project_ensure_dir_tree(packages_root, "packages directory"))
    return false;

  const char* git = toolchain_get_host_tool_path(tc, "git");
  if (!git || !git[0]) {
    error("Unable to find 'git' in the current toolchain. Package '%s' cannot be fetched.", tgt->meta.id ? tgt->meta.id : "");
    print("Run 'bbs update --init-toolchain' after installing Git.");
    return false;
  }

  char script[4096] = {0};
  const char* root_q = project_shell_quote(packages_root);
  const char* git_q = project_shell_quote(toolchain_norm_path(git));
  const char* link_q = project_shell_quote(project_resolve_repo_link(tgt->package_repo_link));
  const char* dir_q = project_shell_quote(info.cache_dir);
  if (tgt->package_repo_commit && tgt->package_repo_commit[0]) {
    const char* commit_q = project_shell_quote(tgt->package_repo_commit);
    snprintf(script,
             sizeof(script),
             "mkdir -p %s && %s clone --recurse-submodules %s %s && %s -C %s -c advice.detachedHead=false checkout %s && %s -C %s submodule update --init --recursive",
             root_q,
             git_q,
             link_q,
             dir_q,
             git_q,
             dir_q,
             commit_q,
             git_q,
             dir_q);
  } else if (tgt->package_repo_tag && tgt->package_repo_tag[0]) {
    const char* tag_q = project_shell_quote(tgt->package_repo_tag);
    snprintf(script,
             sizeof(script),
             "mkdir -p %s && %s -c advice.detachedHead=false clone --depth 1 --branch %s --recurse-submodules %s %s",
             root_q,
             git_q,
             tag_q,
             link_q,
             dir_q);
  } else {
    snprintf(script,
             sizeof(script),
             "mkdir -p %s && %s clone --depth 1 --recurse-submodules %s %s",
             root_q,
             git_q,
             link_q,
             dir_q);
  }

  print("Fetching package '%s' into %s", tgt->meta.id ? tgt->meta.id : "", info.cache_dir ? info.cache_dir : "");
  if (toolchain_run_bash(tc, packages_root, script) != 0) {
    error("Failed to fetch package '%s' from %s.", tgt->meta.id ? tgt->meta.id : "", tgt->package_repo_link ? tgt->package_repo_link : "");
    return false;
  }

  return true;
}

static bool project_fetch_archive_package(target* tgt, toolchain* tc, bool refresh) {
  if (!tgt || tgt->package_source != PACKAGE_SOURCE_ARCHIVE)
    return true;

  project_package_info info = {0};
  if (!project_package_query(tgt, tc, &info))
    return false;

  const char* packages_root = toolchain_norm_path(project_package_root_abs());
  tgt->package_cache_dir = info.cache_dir;
  tgt->package_resolved_dir = project_package_effective_dir(tgt, info.cache_dir);

  if (refresh && info.cache_dir && dir_exists(info.cache_dir)) {
    print("Refreshing package '%s' at %s", tgt->meta.id ? tgt->meta.id : "", info.cache_dir);
    char delete_script[2048] = {0};
    snprintf(delete_script, sizeof(delete_script), "rm -rf %s", project_shell_quote(info.cache_dir));
    if (toolchain_run_bash(tc, packages_root, delete_script) != 0 || dir_exists(info.cache_dir)) {
      error("Failed to delete cached package '%s': %s", tgt->meta.id ? tgt->meta.id : "", info.cache_dir ? info.cache_dir : "");
      return false;
    }
    info.exists = false;
  }

  if (info.exists)
    return true;

  if (!project_ensure_dir_tree(packages_root, "packages directory"))
    return false;

  char script[4096] = {0};
  const char* cache_q = project_shell_quote(info.cache_dir);
  const char* url_q = project_shell_quote(tgt->package_archive_link);
  snprintf(script,
           sizeof(script),
           "rm -rf %s && mkdir -p %s && url=%s && case \"$url\" in *.zip|*.zip?*|*.zip#*) tmp=%s/.bbs-download.zip ;; *.tar.gz|*.tar.gz?*|*.tgz|*.tgz?*) tmp=%s/.bbs-download.tgz ;; *.tar.xz|*.tar.xz?*) tmp=%s/.bbs-download.tar.xz ;; *) tmp=%s/.bbs-download.bin ;; esac && rm -f \"$tmp\" && curl -L \"$url\" -o \"$tmp\" && case \"$url\" in *.zip|*.zip?*|*.zip#*) if command -v powershell >/dev/null 2>&1; then powershell -NoProfile -Command \"Expand-Archive -LiteralPath '$tmp' -DestinationPath '%s' -Force\"; elif command -v unzip >/dev/null 2>&1; then unzip -q \"$tmp\" -d %s; else tar -xf \"$tmp\" -C %s; fi ;; *) tar -xf \"$tmp\" -C %s ;; esac && rm -f \"$tmp\"",
           cache_q,
           cache_q,
           url_q,
           cache_q,
           cache_q,
           cache_q,
           cache_q,
           info.cache_dir,
           cache_q,
           cache_q,
           cache_q);

  print("Fetching archive package '%s' into %s", tgt->meta.id ? tgt->meta.id : "", info.cache_dir ? info.cache_dir : "");
  if (toolchain_run_bash(tc, packages_root, script) != 0) {
    error("Failed to fetch archive package '%s' from %s.", tgt->meta.id ? tgt->meta.id : "", tgt->package_archive_link ? tgt->package_archive_link : "");
    return false;
  }

  return true;
}

static bool project_resolve_package_target(target* tgt, toolchain* tc, const char* platform, bool refresh) {
  if (!tgt)
    return false;

  if (tgt->package_repo_tag && tgt->package_repo_tag[0] && tgt->package_repo_commit && tgt->package_repo_commit[0]) {
    error("Package '%s' cannot declare both repo tag and commit.", tgt->meta.id ? tgt->meta.id : "");
    return false;
  }

  tgt->package_source = tgt->package_path && tgt->package_path[0] ? PACKAGE_SOURCE_PATH :
                        (tgt->package_repo_link && tgt->package_repo_link[0] ? PACKAGE_SOURCE_REPO :
                         (tgt->package_archive_link && tgt->package_archive_link[0] ? PACKAGE_SOURCE_ARCHIVE : PACKAGE_SOURCE_NONE));
  if (tgt->package_source == PACKAGE_SOURCE_NONE)
    return true;

  tgt->package_backend = PACKAGE_BACKEND_NONE;
  tgt->package_project_cfg_path = NULL;
  tgt->package_build_dir = NULL;

  if (tgt->package_source == PACKAGE_SOURCE_PATH) {
    tgt->package_cache_dir = NULL;
    tgt->package_resolved_dir = project_package_effective_dir(tgt, project_resolve_path_from_root(project_toolchain_root_dir(tc), tgt->package_path));
  } else if (!project_fetch_repo_package(tgt, tc, refresh)) {
    return false;
  } else if (tgt->package_source == PACKAGE_SOURCE_ARCHIVE && !project_fetch_archive_package(tgt, tc, refresh)) {
    return false;
  }

  if (!tgt->package_resolved_dir || !dir_exists(tgt->package_resolved_dir)) {
    error("Package directory for '%s' does not exist: %s", tgt->meta.id ? tgt->meta.id : "", tgt->package_resolved_dir ? tgt->package_resolved_dir : "");
    return false;
  }

  tgt->package_backend = project_package_backend_detect(tgt->package_resolved_dir, &tgt->package_project_cfg_path);
  if (tgt->package_backend == PACKAGE_BACKEND_NONE) {
    error("Package '%s' must provide either CMakeLists.txt or project.bbs in %s", tgt->meta.id ? tgt->meta.id : "", tgt->package_resolved_dir ? tgt->package_resolved_dir : "");
    return false;
  }

  if (!project_prepare_bbs_package(tgt, tc, platform, refresh))
    return false;

  return true;
}

static bool project_prepare_packages(project* proj, toolchain* tc, const char* platform, bool refresh) {
  if (!proj)
    return false;
  for (int i = 0; i < proj->target_c; ++i)
    if (!project_resolve_package_target(&proj->targets[i], tc, platform, refresh))
      return false;
  return true;
}

static const char* project_dist_root_abs(const project* proj) {
  return project_resolve_path_from_root(project_root_dir(proj), proj && proj->user_cfg.distdir ? proj->user_cfg.distdir : DEF_DIST_DIR);
}

static const char* project_dist_config_dir_abs(const project* proj, const char* config, const char* platform) {
  return project_resolve_path_from_root(project_root_dir(proj), project_resolved_dir(proj && proj->user_cfg.distdir ? proj->user_cfg.distdir : DEF_DIST_DIR, config, platform));
}

static const char* project_dist_gen_dir_abs(const project* proj, const char* config, const char* platform) {
  return toolchain_join2(project_dist_config_dir_abs(proj, config, platform), "gen");
}

static const char* project_path_filename(const char* path) {
  if (!path)
    return NULL;

  const char* slash = strrchr(path, '/');
  const char* backslash = strrchr(path, '\\');
  const char* base = slash;
  if (!base || (backslash && backslash > base))
    base = backslash;
  return base ? base + 1 : path;
}

static const char* project_path_parent(const char* path) {
  if (!path || !path[0])
    return NULL;

  const char* base = project_path_filename(path);
  size_t len = base > path ? (size_t)(base - path) : 0;
  while (len > 0 && (path[len - 1] == '/' || path[len - 1] == '\\'))
    --len;

  char* out = push(len + 1);
  if (!out)
    return NULL;
  memcpy(out, path, len);
  out[len] = '\0';
  return out;
}

static const char* project_current_workdir(void) {
  char buf[_MAX_PATH] = {0};
  if (!_getcwd(buf, sizeof(buf)))
    return NULL;
  return arena_text(buf, strlen(buf));
}

static bool project_copy_file(const char* src, const char* dst) {
  if (!src || !src[0] || !dst || !dst[0])
    return false;

#if defined(_WIN32)
  if (CopyFileA(src, dst, FALSE) != 0)
    return true;
#endif

  FILE* in = fopen(src, "rb");
  if (!in)
    return false;

  FILE* out = fopen(dst, "wb");
  if (!out) {
    fclose(in);
    return false;
  }

  char buffer[65536];
  bool ok = true;
  for (;;) {
    size_t read = fread(buffer, 1, sizeof(buffer), in);
    if (read > 0 && fwrite(buffer, 1, read, out) != read) {
      ok = false;
      break;
    }
    if (read < sizeof(buffer)) {
      if (ferror(in))
        ok = false;
      break;
    }
  }

  fclose(in);
  if (fclose(out) != 0)
    ok = false;
  if (!ok)
    file_delete(dst);
  return ok;
}

static bool project_is_runtime_library_name(const char* name, os target_os) {
  if (!name || !name[0])
    return false;

  const char* ext = strrchr(name, '.');
  if (!ext)
    return false;

  if (target_os == OS_WINDOWS)
    return _stricmp(ext, ".dll") == 0;
  if (target_os == OS_MACOS)
    return _stricmp(ext, ".dylib") == 0;
  return _stricmp(ext, ".so") == 0;
}

static bool project_copy_runtime_files(const char* src_dir, const char* dst_dir, os target_os) {
  if (!src_dir || !dst_dir)
    return false;

#if defined(_WIN32)
  char pattern[_MAX_PATH] = {0};
  snprintf(pattern, sizeof(pattern), "%s\\*", src_dir);
  WIN32_FIND_DATAA data;
  HANDLE handle = FindFirstFileA(pattern, &data);
  if (handle == INVALID_HANDLE_VALUE)
    return true;

  bool ok = true;
  do {
    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      continue;
    if (!project_is_runtime_library_name(data.cFileName, target_os))
      continue;

    const char* src = toolchain_join2(src_dir, data.cFileName);
    const char* dst = toolchain_join2(dst_dir, data.cFileName);
    if (!project_copy_file(src, dst)) {
      error("Failed to copy runtime library '%s'.", src);
      ok = false;
      break;
    }
  } while (FindNextFileA(handle, &data) != 0);

  FindClose(handle);
  return ok;
#else
  DIR* dir = opendir(src_dir);
  if (!dir)
    return true;

  bool ok = true;
  struct dirent* entry = NULL;
  while ((entry = readdir(dir)) != NULL) {
    if (_stricmp(entry->d_name, ".") == 0 || _stricmp(entry->d_name, "..") == 0)
      continue;
    if (!project_is_runtime_library_name(entry->d_name, target_os))
      continue;

    const char* src = toolchain_join2(src_dir, entry->d_name);
    const char* dst = toolchain_join2(dst_dir, entry->d_name);
    if (!project_copy_file(src, dst)) {
      error("Failed to copy runtime library '%s'.", src);
      ok = false;
      break;
    }
  }

  closedir(dir);
  return ok;
#endif
}

static const char* project_version_text(ver value) {
  char buf[32] = {0};
  if (value.user != 0)
    snprintf(buf, sizeof(buf), "%u.%u.%u.%u", value.major, value.minor, value.patch, value.user);
  else
    snprintf(buf, sizeof(buf), "%u.%u.%u", value.major, value.minor, value.patch);
  return arena_text(buf, strlen(buf));
}

static const char* project_token_value(const char* name,
                                       const project* proj,
                                       const target* tgt,
                                       const char* platform,
                                       toolchain* tc,
                                       const char* workdir,
                                       os target_os,
                                       arch target_arch) {
  if (!name || !name[0])
    return NULL;
  if (strcmp(name, "CWD") == 0)
    return project_current_workdir();
  if (strcmp(name, "CFG") == 0)
    return proj && proj->active_config ? proj->active_config : "default";
  if (strcmp(name, "PLT") == 0)
    return platform && platform[0] ? platform : project_default_platform_id();
  if (strcmp(name, "OS") == 0)
    return target_os < OS_MAX ? OS_NAMES[target_os] : project_host_os_name();
  if (strcmp(name, "ARC") == 0)
    return target_arch < ARCH_MAX ? ARCH_NAMES[target_arch] : project_host_arch_name();
  if (strcmp(name, "PRJ") == 0)
    return proj && proj->meta.id ? proj->meta.id : "";
  if (strcmp(name, "PROJECT") == 0)
    return tc && tc->project_cfg_path ? tc->project_cfg_path : get_path_cwd("project.bbs");
  if (strcmp(name, "TOOLCHAIN") == 0)
    return tc && tc->toolchain_cfg_path ? tc->toolchain_cfg_path : NULL;
  if (strcmp(name, "USER") == 0)
    return tc && tc->user_cfg_path ? tc->user_cfg_path : NULL;
  if (strcmp(name, "LOCAL") == 0)
    return tc && tc->local_cfg_path ? tc->local_cfg_path : get_path_cwd("local.bbs");
  if (strcmp(name, "DBUILD") == 0)
    return project_build_binary_dir_abs(proj, proj ? proj->active_config : NULL, platform);
  if (strcmp(name, "DDIST") == 0)
    return project_dist_config_dir_abs(proj, proj ? proj->active_config : NULL, platform);
  if (strcmp(name, "DGEN") == 0)
    return project_dist_gen_dir_abs(proj, proj ? proj->active_config : NULL, platform);
  if (strcmp(name, "TARGET") == 0)
    return tgt && tgt->meta.id ? tgt->meta.id : "";
  if (strcmp(name, "OUT") == 0)
    return tgt && tgt->output ? tgt->output : (tgt && tgt->meta.id ? tgt->meta.id : "");
  if (strcmp(name, "VER") == 0) {
    ver value = {0};
    if (tgt && project_has_ver(tgt->meta.ver))
      value = tgt->meta.ver;
    else if (proj)
      value = proj->meta.ver;
    return project_has_ver(value) ? project_version_text(value) : "0.0.0";
  }
  if (strcmp(name, "EXE") == 0)
    return (proj && tgt && platform) ? project_target_executable_abs(proj, tgt, platform) : NULL;
  if (strcmp(name, "WORKDIR") == 0)
    return workdir && workdir[0] ? workdir : project_current_workdir();
  return NULL;
}

static const char* project_expand_config_string(const char* text, const project* proj, const target* tgt, const char* platform, toolchain* tc, const char* workdir) {
  if (!text)
    return NULL;

  os target_os = OS_MAX;
  arch target_arch = ARCH_MAX;
  const char* resolved_platform = platform && platform[0] ? platform : project_default_platform_id();
  project_parse_platform_id(resolved_platform, &target_os, &target_arch);

  size_t cap = strlen(text) + 32;
  char* out = (char*)malloc(cap);
  if (!out)
    return NULL;

  size_t wi = 0;
  for (size_t i = 0; text[i];) {
    if (text[i] != '$') {
      if (wi + 2 > cap) {
        size_t new_cap = cap * 2;
        char* grown = (char*)realloc(out, new_cap);
        if (!grown) {
          free(out);
          return NULL;
        }
        out = grown;
        cap = new_cap;
      }
      out[wi++] = text[i++];
      continue;
    }

    if (text[i + 1] == '$') {
      if (wi + 2 > cap) {
        size_t new_cap = cap * 2;
        char* grown = (char*)realloc(out, new_cap);
        if (!grown) {
          free(out);
          return NULL;
        }
        out = grown;
        cap = new_cap;
      }
      out[wi++] = '$';
      i += 2;
      continue;
    }

    size_t start = i + 1;
    size_t end = start;
    while (text[end] && (isupper((unsigned char)text[end]) || text[end] == '_'))
      ++end;

    if (end == start) {
      if (wi + 2 > cap) {
        size_t new_cap = cap * 2;
        char* grown = (char*)realloc(out, new_cap);
        if (!grown) {
          free(out);
          return NULL;
        }
        out = grown;
        cap = new_cap;
      }
      out[wi++] = text[i++];
      continue;
    }

    char token[64] = {0};
    size_t len = end - start;
    if (len >= sizeof(token))
      len = sizeof(token) - 1;
    memcpy(token, text + start, len);
    token[len] = '\0';

    const char* value = project_token_value(token, proj, tgt, resolved_platform, tc, workdir, target_os, target_arch);
    if (!value)
      value = arena_text(text + i, end - i);

    size_t value_len = strlen(value);
    while (wi + value_len + 1 > cap) {
      size_t new_cap = cap * 2;
      char* grown = (char*)realloc(out, new_cap);
      if (!grown) {
        free(out);
        return NULL;
      }
      out = grown;
      cap = new_cap;
    }
    memcpy(out + wi, value, value_len);
    wi += value_len;
    i = end;
  }

  out[wi] = '\0';
  const char* expanded = arena_text(out, wi);
  free(out);
  return expanded;
}

static const char* project_dist_archive_name(const project* proj, const target* tgt, const char* platform, toolchain* tc) {
  const char* fmt = proj->user_cfg.dist_archive_format ? proj->user_cfg.dist_archive_format : "zip";
  const char* pattern = tgt && tgt->dist_archive_name ? tgt->dist_archive_name : proj->user_cfg.dist_archive_name;
  if (!pattern || !pattern[0])
    pattern = "$CFG-$OS-$ARC--$VER";

  const char* expanded = project_expand_config_string(pattern, proj, tgt, platform, tc, NULL);
  if (!expanded || !expanded[0])
    expanded = project_build_dir_name(proj->active_config, platform);

  const char* ext = strrchr(expanded, '.');
  if (ext && _stricmp(ext + 1, fmt) == 0)
    return expanded;

  char buf[512] = {0};
  snprintf(buf, sizeof(buf), "%s.%s", expanded, fmt);
  return arena_text(buf, strlen(buf));
}

static bool project_create_dist_archive(const project* proj, const target* tgt, const char* platform, toolchain* tc) {
  if (!proj || !tgt || !platform || !tc)
    return false;

  const char* workdir = project_dist_config_dir_abs(proj, proj->active_config, platform);
  const char* archive_name = project_dist_archive_name(proj, tgt, platform, tc);
  const char* format = proj->user_cfg.dist_archive_format ? proj->user_cfg.dist_archive_format : "zip";
  char script[2048] = {0};

  if (_stricmp(format, "zip") == 0) {
#if defined(_WIN32)
    snprintf(script,
             sizeof(script),
             "rm -f '%s'; powershell.exe -NoProfile -Command \"Compress-Archive -Path './gen' -DestinationPath './%s' -Force\"",
             archive_name,
             archive_name);
#else
    snprintf(script, sizeof(script), "rm -f '%s' && zip -rq '%s' gen", archive_name, archive_name);
#endif
  } else if (_stricmp(format, "tar") == 0) {
    snprintf(script, sizeof(script), "rm -f '%s' && tar -cf '%s' gen", archive_name, archive_name);
  } else if (_stricmp(format, "rar") == 0) {
    snprintf(script,
             sizeof(script),
             "rm -f '%s'; if command -v rar >/dev/null 2>&1; then rar a -idq '%s' gen; elif command -v winrar >/dev/null 2>&1; then winrar a -idq '%s' gen; else exit 127; fi",
             archive_name,
             archive_name,
             archive_name);
  } else {
    error("Unknown dist_archive_format '%s'.", format);
    return false;
  }

  if (toolchain_run_bash(tc, workdir, script) != 0) {
    error("Failed to create %s archive for target '%s'.", format, tgt->meta.id ? tgt->meta.id : "");
    return false;
  }

  print("Archive: %s", toolchain_join2(workdir, archive_name));
  return true;
}

static const char* project_cmake_var_name(const char* text) {
  char buf[256] = {0};
  size_t wi = 0;
  if (!text || !text[0])
    text = "target";

  for (size_t i = 0; text[i] && wi + 1 < sizeof(buf); ++i) {
    unsigned char ch = (unsigned char)text[i];
    buf[wi++] = isalnum(ch) ? (char)toupper(ch) : '_';
  }
  buf[wi] = '\0';
  return arena_text(buf, strlen(buf));
}

static const char* project_target_usage_name(const char* text) {
  char buf[320] = {0};
  if (!text || !text[0])
    text = "target";
  snprintf(buf, sizeof(buf), "%s__usage", text);
  return arena_text(buf, strlen(buf));
}

static const char* project_cmake_define_name(const char* prefix, const char* text) {
  char buf[256] = {0};
  size_t wi = 0;
  if (prefix && prefix[0]) {
    for (size_t i = 0; prefix[i] && wi + 1 < sizeof(buf); ++i)
      buf[wi++] = prefix[i];
  }
  if (wi > 0 && wi + 1 < sizeof(buf))
    buf[wi++] = '_';
  if (!text || !text[0])
    text = "ITEM";
  for (size_t i = 0; text[i] && wi + 1 < sizeof(buf); ++i) {
    unsigned char ch = (unsigned char)text[i];
    buf[wi++] = isalnum(ch) ? (char)toupper(ch) : '_';
  }
  buf[wi] = '\0';
  return arena_text(buf, strlen(buf));
}

static bool project_append_cmake_toolchain_kv(project_textbuf* buf, const char* key, const char* value) {
  if (!buf || !key || !key[0] || !value || !value[0])
    return true;
  const char* esc = project_cmake_path_text(value);
  if (!esc)
    return false;
  return project_textbuf_appendf(buf, "set(%s \"%s\" CACHE STRING \"Generated by bbs\")\n", key, esc);
}

static const char* project_escape_regex_text(const char* text) {
  const char* norm = project_normalize_slashes(text ? text : "");
  if (!norm)
    return NULL;

  size_t len = 0;
  for (size_t i = 0; norm[i]; ++i) {
    char ch = norm[i];
    if (ch == '.' || ch == '^' || ch == '$' || ch == '+' || ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '|')
      len += 2;
    else
      len += 1;
  }

  char* out = push(len + 1);
  if (!out)
    return NULL;

  size_t wi = 0;
  for (size_t i = 0; norm[i]; ++i) {
    char ch = norm[i];
    if (ch == '.' || ch == '^' || ch == '$' || ch == '+' || ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '|')
      out[wi++] = '\\';
    out[wi++] = ch;
  }
  out[wi] = '\0';
  return out;
}

static const char* project_glob_to_regex(const char* pattern) {
  const char* norm = project_normalize_slashes(pattern ? pattern : "");
  if (!norm)
    return NULL;

  size_t len = 2;
  for (size_t i = 0; norm[i]; ++i) {
    char ch = norm[i];
    if (ch == '*') {
      if (norm[i + 1] == '*') {
        len += 2;
        ++i;
      } else {
        len += 6;
      }
    } else if (ch == '?') {
      len += 5;
    } else if (ch == '.' || ch == '^' || ch == '$' || ch == '+' || ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '|') {
      len += 2;
    } else {
      len += 1;
    }
  }
  len += 2;

  char* out = push(len + 1);
  if (!out)
    return NULL;

  size_t wi = 0;
  out[wi++] = '^';
  for (size_t i = 0; norm[i]; ++i) {
    char ch = norm[i];
    if (ch == '*') {
      if (norm[i + 1] == '*') {
        out[wi++] = '.';
        out[wi++] = '*';
        ++i;
      } else {
        memcpy(out + wi, "[^/]*", 5);
        wi += 5;
      }
      continue;
    }
    if (ch == '?') {
      memcpy(out + wi, "[^/]", 4);
      wi += 4;
      continue;
    }
    if (ch == '.' || ch == '^' || ch == '$' || ch == '+' || ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '|')
      out[wi++] = '\\';
    out[wi++] = ch;
  }
  out[wi++] = '$';
  out[wi] = '\0';
  return out;
}

static bool project_append_cmake_unity_selector_match(project_textbuf* buf, const char* selector, bool first) {
  const char* norm = project_normalize_slashes(selector ? selector : "");
  if (!norm)
    return false;

  if (project_text_has_wildcards(norm)) {
    const char* regex = project_glob_to_regex(norm);
    const char* esc = project_escape_cmake_string(regex ? regex : "");
    if (!esc)
      return false;
    return project_textbuf_appendf(buf, "%sif(bbs_rel MATCHES \"%s\")\n", first ? "" : "else", esc);
  }

  const char* prefix = norm;
  size_t prefix_len = strlen(prefix);
  while (prefix_len > 0 && prefix[prefix_len - 1] == '/')
    --prefix_len;
  const char* prefix_text = arena_text(prefix, prefix_len);
  const char* prefix_regex = project_escape_regex_text(prefix_text ? prefix_text : "");
  const char* esc = project_escape_cmake_string(prefix_text ? prefix_text : "");
  const char* regex_esc = project_escape_cmake_string(prefix_regex ? prefix_regex : "");
  if (!esc || !regex_esc)
    return false;
  return project_textbuf_appendf(buf,
                                 "%sif(bbs_rel STREQUAL \"%s\" OR bbs_rel MATCHES \"^%s/.+\")\n",
                                 first ? "" : "else",
                                 esc,
                                 regex_esc);
}

static bool project_append_cmake_manual_unity(project_textbuf* buf, const target* tgt, const char* var_name) {
  if (!buf || !tgt || !var_name || tgt->unity_batch_c <= 0)
    return false;

  const char* unity_ext = tgt->lang == LANG_CPP ? "cpp" : "c";
  const char* unity_file_prefix = project_cmake_var_name(tgt->meta.id);
  const char* target_id = project_escape_cmake_string(tgt->meta.id ? tgt->meta.id : "target");
  if (!unity_ext || !unity_file_prefix || !target_id)
    return false;

  if (!project_textbuf_appendf(buf,
                               "set(BBS_%s_ALL_SOURCES ${BBS_%s_SOURCES})\n"
                               "set(BBS_%s_UNITY_WRAPPERS)\n"
                               "set(BBS_%s_UNITY_ASSIGNED)\n"
                               "file(MAKE_DIRECTORY \"${CMAKE_CURRENT_LIST_DIR}/unity\")\n",
                               var_name,
                               var_name,
                               var_name,
                               var_name))
    return false;

  for (int bi = 0; bi < tgt->unity_batch_c; ++bi) {
    if (!project_textbuf_appendf(buf,
                                 "set(BBS_%s_BATCH_%d_SOURCES)\n"
                                 "foreach(bbs_source IN LISTS BBS_%s_ALL_SOURCES)\n"
                                 "  file(RELATIVE_PATH bbs_rel \"${BBS_PROJECT_ROOT}\" \"${bbs_source}\")\n"
                                 "  string(REPLACE \\\"\\\\\\\" \"/\" bbs_rel \"${bbs_rel}\")\n"
                                 "  set(bbs_match FALSE)\n",
                                 var_name,
                                 bi,
                                 var_name))
      return false;

    for (int si = 0; si < tgt->unity_batches[bi].selector_c; ++si)
      if (!project_append_cmake_unity_selector_match(buf, tgt->unity_batches[bi].selectors[si], si == 0))
        return false;

    if (!project_textbuf_appendf(buf,
                                 "    set(bbs_match TRUE)\n"
                                 "  endif()\n"
                                 "  if(bbs_match)\n"
                                 "    if(bbs_source IN_LIST BBS_%s_UNITY_ASSIGNED)\n"
                                 "      message(FATAL_ERROR \"Target '%s' unity batch %d overlaps another batch at ${bbs_rel}.\")\n"
                                 "    endif()\n"
                                 "    list(APPEND BBS_%s_BATCH_%d_SOURCES \"${bbs_source}\")\n"
                                 "    list(APPEND BBS_%s_UNITY_ASSIGNED \"${bbs_source}\")\n"
                                 "  endif()\n"
                                 "endforeach()\n"
                                 "list(LENGTH BBS_%s_BATCH_%d_SOURCES BBS_%s_BATCH_%d_COUNT)\n"
                                 "if(BBS_%s_BATCH_%d_COUNT EQUAL 0)\n"
                                 "  message(FATAL_ERROR \"Target '%s' unity batch %d did not match any units.\")\n"
                                 "endif()\n"
                                 "set(BBS_%s_BATCH_%d_FILE \"${CMAKE_CURRENT_LIST_DIR}/unity/%s_batch_%d.%s\")\n"
                                 "file(WRITE \"${BBS_%s_BATCH_%d_FILE}\" \"/* Generated by bbs. */\\n\")\n"
                                 "foreach(bbs_batch_source IN LISTS BBS_%s_BATCH_%d_SOURCES)\n"
                                 "  string(REPLACE \\\"\\\\\\\" \"/\" bbs_batch_source_norm \"${bbs_batch_source}\")\n"
                                 "  file(APPEND \"${BBS_%s_BATCH_%d_FILE}\" \"#include \\\"${bbs_batch_source_norm}\\\"\\n\")\n"
                                 "endforeach()\n"
                                 "list(APPEND BBS_%s_UNITY_WRAPPERS \"${BBS_%s_BATCH_%d_FILE}\")\n",
                                 var_name,
                                 target_id,
                                 bi + 1,
                                 var_name,
                                 bi,
                                 var_name,
                                 var_name,
                                 bi,
                                 var_name,
                                 bi,
                                 var_name,
                                 bi,
                                 target_id,
                                 bi + 1,
                                 var_name,
                                 bi,
                                 unity_file_prefix,
                                 bi,
                                 unity_ext,
                                 var_name,
                                 bi,
                                 var_name,
                                 bi,
                                 var_name,
                                 bi,
                                 var_name,
                                 var_name,
                                 bi))
      return false;
  }

  if (!project_textbuf_appendf(buf,
                               "set(BBS_%s_SOURCES ${BBS_%s_UNITY_WRAPPERS})\n"
                               "foreach(bbs_source IN LISTS BBS_%s_ALL_SOURCES)\n"
                               "  if(NOT bbs_source IN_LIST BBS_%s_UNITY_ASSIGNED)\n"
                               "    list(APPEND BBS_%s_SOURCES \"${bbs_source}\")\n"
                               "  endif()\n"
                               "endforeach()\n\n",
                               var_name,
                               var_name,
                               var_name,
                               var_name,
                               var_name))
    return false;

  return true;
}

static bool project_append_cmake_target(project_textbuf* buf, const project* proj, const target* tgt, const char* platform_id, toolchain* tc, const char* bash_path) {
  if (!buf || !tgt || !tgt->meta.id)
    return false;

  const char* target_name = project_escape_cmake_string(tgt->meta.id);
  const char* output_name = project_escape_cmake_string(tgt->output ? tgt->output : tgt->meta.id);
  const char* var_name = project_cmake_var_name(tgt->meta.id);
  const char* usage_target_id = project_target_usage_name(tgt->meta.id);
  const char* usage_target_name = usage_target_id ? project_escape_cmake_string(usage_target_id) : NULL;
  const char* own_usage_scope = tgt->type == TARGET_TYPE_HEADER_LIB ? "INTERFACE" : "PUBLIC";
  if (!target_name || !output_name || !var_name || !usage_target_name)
    return false;

  if (project_target_is_package(tgt)) {
    const char* package_source_dir = tgt->package_backend == PACKAGE_BACKEND_BBS && tgt->package_build_dir ? tgt->package_build_dir : tgt->package_resolved_dir;
    const char* package_dir = project_cmake_path_text(package_source_dir ? package_source_dir : "");
    const char* ext_target_name = project_escape_cmake_string(project_target_build_target_name(tgt));
    if (!package_dir || !ext_target_name)
      return false;

    if (!project_textbuf_appendf(buf,
                                 "add_subdirectory(\"%s\" \"${CMAKE_CURRENT_BINARY_DIR}/packages/%s\")\n"
                                 "if(NOT TARGET %s)\n"
                                 "  message(FATAL_ERROR \"Package '%s' did not define the expected CMake target '%s'.\")\n"
                                 "endif()\n"
                                 "add_library(%s INTERFACE)\n"
                                 "target_link_libraries(%s INTERFACE %s)\n",
                                 package_dir,
                                 var_name,
                                 ext_target_name,
                                 tgt->meta.id ? tgt->meta.id : "",
                                 project_target_build_target_name(tgt) ? project_target_build_target_name(tgt) : "",
                                 usage_target_name,
                                 usage_target_name,
                                 ext_target_name))
      return false;

    for (int i = 0; i < tgt->include_dir_c; ++i) {
      const char* dir = project_cmake_path_text(tgt->include_dirs[i] ? tgt->include_dirs[i] : "");
      if (!dir)
        return false;
      if (!project_textbuf_appendf(buf, "target_include_directories(%s INTERFACE \"${BBS_PROJECT_ROOT}/%s\")\n", usage_target_name, dir))
        return false;
    }
    for (int i = 0; i < tgt->link_dir_c; ++i) {
      const char* dir = project_cmake_path_text(tgt->link_dirs[i] ? tgt->link_dirs[i] : "");
      if (!dir)
        return false;
      if (!project_textbuf_appendf(buf, "target_link_directories(%s INTERFACE \"${BBS_PROJECT_ROOT}/%s\")\n", usage_target_name, dir))
        return false;
    }
    for (int i = 0; i < tgt->link_libs_count; ++i) {
      const char* lib = project_escape_cmake_string(tgt->link_libs[i] ? tgt->link_libs[i] : "");
      if (!lib)
        return false;
      if (!project_textbuf_appendf(buf, "target_link_libraries(%s INTERFACE \"%s\")\n", usage_target_name, lib))
        return false;
    }
    for (int i = 0; i < tgt->dependency_c; ++i) {
      int dep_idx = project_find_target_index(proj, tgt->dependencies[i]);
      if (dep_idx < 0)
        return false;
      const target* dep = &proj->targets[dep_idx];
      const char* dep_usage_target_id = project_target_usage_name(dep->meta.id);
      const char* dep_usage_target_name = dep_usage_target_id ? project_escape_cmake_string(dep_usage_target_id) : NULL;
      if (!dep_usage_target_name)
        return false;
      if (!project_textbuf_appendf(buf, "target_link_libraries(%s INTERFACE %s)\n", usage_target_name, dep_usage_target_name))
        return false;
    }

    return project_textbuf_append(buf, "\n");
  }

  if (!project_textbuf_appendf(buf, "set(BBS_%s_SOURCES)\n", var_name))
    return false;
  for (int i = 0; i < tgt->unit_c; ++i) {
    const char* unit = tgt->units[i] ? tgt->units[i] : "";
    const char* cmake_unit = project_cmake_path_text(unit);
    if (!cmake_unit)
      return false;
    if (project_text_has_wildcards(unit)) {
      if (!project_textbuf_appendf(buf,
                                   "file(GLOB_RECURSE BBS_%s_GLOB_%d CONFIGURE_DEPENDS \"${BBS_PROJECT_ROOT}/%s\")\n"
                                   "list(APPEND BBS_%s_SOURCES ${BBS_%s_GLOB_%d})\n",
                                   var_name,
                                   i,
                                   cmake_unit,
                                   var_name,
                                   var_name,
                                   i))
        return false;
    } else {
      if (!project_textbuf_appendf(buf, "list(APPEND BBS_%s_SOURCES \"${BBS_PROJECT_ROOT}/%s\")\n", var_name, cmake_unit))
        return false;
    }
  }
  if (tgt->unity_batch_c > 0) {
    if (!project_append_cmake_manual_unity(buf, tgt, var_name))
      return false;
  } else if (!project_textbuf_append(buf, "\n")) {
    return false;
  }

  switch (tgt->type) {
    case TARGET_TYPE_CONSOLE:
    case TARGET_TYPE_TEST:
      if (!project_textbuf_appendf(buf, "add_executable(%s ${BBS_%s_SOURCES})\n", target_name, var_name))
        return false;
      break;
    case TARGET_TYPE_CONSOLELESS:
      if (!project_textbuf_appendf(buf,
                                   "add_executable(%s ${BBS_%s_SOURCES})\n"
                                   "set_target_properties(%s PROPERTIES WIN32_EXECUTABLE ON)\n",
                                   target_name,
                                   var_name,
                                   target_name))
        return false;
      break;
    case TARGET_TYPE_HEADER_LIB:
      if (!project_textbuf_appendf(buf,
                                   "add_library(%s INTERFACE)\n"
                                   "target_sources(%s INTERFACE ${BBS_%s_SOURCES})\n",
                                   target_name,
                                   target_name,
                                   var_name))
        return false;
      break;
    case TARGET_TYPE_STATIC_LIB:
      if (!project_textbuf_appendf(buf, "add_library(%s STATIC ${BBS_%s_SOURCES})\n", target_name, var_name))
        return false;
      break;
    case TARGET_TYPE_DYN_LIB:
      if (!project_textbuf_appendf(buf, "add_library(%s SHARED ${BBS_%s_SOURCES})\n", target_name, var_name))
        return false;
      break;
    case TARGET_TYPE_OBJ_LIB:
      if (!project_textbuf_appendf(buf, "add_library(%s OBJECT ${BBS_%s_SOURCES})\n", target_name, var_name))
        return false;
      break;
    case TARGET_TYPE_DRIVER:
      if (!project_textbuf_appendf(buf, "add_library(%s MODULE ${BBS_%s_SOURCES})\n", target_name, var_name))
        return false;
      break;
    default:
      error("Unsupported target type '%s'.", project_target_type_name(tgt->type));
      return false;
  }

  if (!project_textbuf_appendf(buf, "add_library(%s INTERFACE)\n", usage_target_name))
    return false;
  if (!project_textbuf_appendf(buf, "target_link_libraries(%s %s %s)\n", target_name, own_usage_scope, usage_target_name))
    return false;

  if (tgt->unity_configured && tgt->unity_enabled) {
    if (!project_textbuf_appendf(buf, "set_target_properties(%s PROPERTIES UNITY_BUILD ON)\n", target_name))
      return false;
    if (tgt->unity_batch_size_set)
      if (!project_textbuf_appendf(buf, "set_target_properties(%s PROPERTIES UNITY_BUILD_BATCH_SIZE %zu)\n", target_name, tgt->unity_batch_size))
        return false;
    if (tgt->unity_batch_c > 0)
      if (!project_textbuf_appendf(buf,
                                   "set_source_files_properties(${BBS_%s_UNITY_WRAPPERS} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)\n",
                                   var_name))
        return false;
  }

  if (tgt->type != TARGET_TYPE_HEADER_LIB) {
    if (!project_textbuf_appendf(buf,
                                 "set_target_properties(%s PROPERTIES OUTPUT_NAME \"%s\")\n"
                                 "if(CMAKE_CONFIGURATION_TYPES)\n"
                                 "  set_target_properties(%s PROPERTIES "
                                 "RUNTIME_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_BINARY_DIR}/bin/Debug\" "
                                 "RUNTIME_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_BINARY_DIR}/bin/Release\" "
                                 "RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO \"${CMAKE_BINARY_DIR}/bin/RelWithDebInfo\" "
                                 "RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL \"${CMAKE_BINARY_DIR}/bin/MinSizeRel\" "
                                 "LIBRARY_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_BINARY_DIR}/lib/Debug\" "
                                 "LIBRARY_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_BINARY_DIR}/lib/Release\" "
                                 "LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO \"${CMAKE_BINARY_DIR}/lib/RelWithDebInfo\" "
                                 "LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL \"${CMAKE_BINARY_DIR}/lib/MinSizeRel\" "
                                 "ARCHIVE_OUTPUT_DIRECTORY_DEBUG \"${CMAKE_BINARY_DIR}/lib/Debug\" "
                                 "ARCHIVE_OUTPUT_DIRECTORY_RELEASE \"${CMAKE_BINARY_DIR}/lib/Release\" "
                                 "ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO \"${CMAKE_BINARY_DIR}/lib/RelWithDebInfo\" "
                                 "ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL \"${CMAKE_BINARY_DIR}/lib/MinSizeRel\")\n"
                                 "else()\n"
                                 "  set_target_properties(%s PROPERTIES "
                                 "RUNTIME_OUTPUT_DIRECTORY \"${CMAKE_BINARY_DIR}/bin\" "
                                 "LIBRARY_OUTPUT_DIRECTORY \"${CMAKE_BINARY_DIR}/lib\" "
                                 "ARCHIVE_OUTPUT_DIRECTORY \"${CMAKE_BINARY_DIR}/lib\")\n"
                                 "endif()\n",
                                 target_name,
                                 output_name,
                                 target_name,
                                 target_name))
      return false;
  } else {
    if (!project_textbuf_appendf(buf, "set_target_properties(%s PROPERTIES OUTPUT_NAME \"%s\")\n", target_name, output_name))
      return false;
  }

  if (tgt->lang == LANG_CPP) {
    if (!project_textbuf_appendf(buf, "set_target_properties(%s PROPERTIES LINKER_LANGUAGE CXX)\n", target_name))
      return false;
  }

  if (tgt->lang == LANG_C && tgt->stdver) {
    const char* std = project_cmake_c_standard(tgt->stdver);
    if (std)
      if (!project_textbuf_appendf(buf, "set_target_properties(%s PROPERTIES C_STANDARD %s C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)\n", target_name, std))
        return false;
  }
  if (tgt->lang == LANG_CPP && tgt->stdver) {
    const char* std = project_cmake_cpp_standard(tgt->stdver);
    if (std)
      if (!project_textbuf_appendf(buf, "set_target_properties(%s PROPERTIES CXX_STANDARD %s CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)\n", target_name, std))
        return false;
  }

  if (tgt->type != TARGET_TYPE_HEADER_LIB && tgt->runtime != STDLIB_NONE) {
    const char* runtime = tgt->runtime == STDLIB_STATIC ? "MultiThreaded$<$<CONFIG:Debug>:Debug>" : "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL";
    if (!project_textbuf_appendf(buf, "set_property(TARGET %s PROPERTY MSVC_RUNTIME_LIBRARY \"%s\")\n", target_name, runtime))
      return false;
  }

  for (int i = 0; i < tgt->include_dir_c; ++i) {
    const char* dir = project_cmake_path_text(tgt->include_dirs[i] ? tgt->include_dirs[i] : "");
    if (!dir)
      return false;
    if (!project_textbuf_appendf(buf, "target_include_directories(%s INTERFACE \"${BBS_PROJECT_ROOT}/%s\")\n", usage_target_name, dir))
      return false;
  }
  for (int i = 0; i < tgt->link_dir_c; ++i) {
    const char* dir = project_cmake_path_text(tgt->link_dirs[i] ? tgt->link_dirs[i] : "");
    if (!dir)
      return false;
    if (!project_textbuf_appendf(buf, "target_link_directories(%s INTERFACE \"${BBS_PROJECT_ROOT}/%s\")\n", usage_target_name, dir))
      return false;
  }
  for (int i = 0; i < tgt->link_libs_count; ++i) {
    const char* lib = project_escape_cmake_string(tgt->link_libs[i] ? tgt->link_libs[i] : "");
    if (!lib)
      return false;
    if (!project_textbuf_appendf(buf, "target_link_libraries(%s INTERFACE \"%s\")\n", usage_target_name, lib))
      return false;
  }

  for (int i = 0; i < tgt->dependency_c; ++i) {
    int dep_idx = project_find_target_index(proj, tgt->dependencies[i]);
    if (dep_idx < 0)
      return false;

    const target* dep = &proj->targets[dep_idx];
    const char* dep_target_name = project_escape_cmake_string(project_target_build_target_name(dep));
    const char* dep_usage_target_id = project_target_usage_name(dep->meta.id);
    const char* dep_usage_target_name = dep_usage_target_id ? project_escape_cmake_string(dep_usage_target_id) : NULL;
    if (!dep_target_name || !dep_usage_target_name)
      return false;

    if (!project_textbuf_appendf(buf, "target_link_libraries(%s INTERFACE %s)\n", usage_target_name, dep_usage_target_name))
      return false;
    if (project_target_supports_linking(dep)) {
      if (!project_textbuf_appendf(buf, "target_link_libraries(%s INTERFACE %s)\n", usage_target_name, dep_target_name))
        return false;
    } else {
      if (!project_textbuf_appendf(buf, "add_dependencies(%s %s)\n", target_name, dep_target_name))
        return false;
    }
  }

  if (tgt->defines && tgt->defines[0]) {
    const char* defs = project_escape_cmake_string(tgt->defines);
    if (!defs)
      return false;
    if (!project_textbuf_appendf(buf,
                                 "set(BBS_%s_DEFINES \"%s\")\n"
                                 "separate_arguments(BBS_%s_DEFINES NATIVE_COMMAND \"${BBS_%s_DEFINES}\")\n"
                                 "target_compile_definitions(%s %s ${BBS_%s_DEFINES})\n",
                                 var_name,
                                  defs,
                                  var_name,
                                  var_name,
                                  target_name,
                                  tgt->type == TARGET_TYPE_HEADER_LIB ? "INTERFACE" : "PRIVATE",
                                  var_name))
      return false;
  }

  if (tgt->additional_compile_args && tgt->additional_compile_args[0]) {
    const char* args = project_escape_cmake_string(tgt->additional_compile_args);
    if (!args)
      return false;
    if (!project_textbuf_appendf(buf,
                                 "set(BBS_%s_COMPILE_ARGS \"%s\")\n"
                                 "separate_arguments(BBS_%s_COMPILE_ARGS NATIVE_COMMAND \"${BBS_%s_COMPILE_ARGS}\")\n"
                                 "target_compile_options(%s %s ${BBS_%s_COMPILE_ARGS})\n",
                                 var_name,
                                  args,
                                  var_name,
                                  var_name,
                                  target_name,
                                  tgt->type == TARGET_TYPE_HEADER_LIB ? "INTERFACE" : "PRIVATE",
                                  var_name))
      return false;
  }

  if (tgt->additional_link_args && tgt->additional_link_args[0] && tgt->type != TARGET_TYPE_HEADER_LIB) {
    const char* args = project_escape_cmake_string(tgt->additional_link_args);
    if (!args)
      return false;
    if (!project_textbuf_appendf(buf,
                                 "set(BBS_%s_LINK_ARGS \"%s\")\n"
                                 "separate_arguments(BBS_%s_LINK_ARGS NATIVE_COMMAND \"${BBS_%s_LINK_ARGS}\")\n"
                                 "target_link_options(%s PRIVATE ${BBS_%s_LINK_ARGS})\n",
                                 var_name,
                                 args,
                                 var_name,
                                 var_name,
                                 target_name,
                                 var_name))
      return false;
  }

  if (tgt->warnings_as_errors)
    if (!project_textbuf_appendf(buf,
                                 "target_compile_options(%s %s "
                                 "$<$<C_COMPILER_ID:MSVC>:/WX> $<$<CXX_COMPILER_ID:MSVC>:/WX> "
                                 "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-Werror> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>)\n",
                                 target_name,
                                 tgt->type == TARGET_TYPE_HEADER_LIB ? "INTERFACE" : "PRIVATE"))
      return false;

  if (!project_append_cmake_warning_level(buf, target_name, tgt->type == TARGET_TYPE_HEADER_LIB ? "INTERFACE" : "PRIVATE", tgt->warning_level))
    return false;
  if (!project_append_cmake_opt_level(buf, target_name, tgt->type == TARGET_TYPE_HEADER_LIB ? "INTERFACE" : "PRIVATE", tgt->opt_level))
    return false;

  if (tgt->stack_size > 0 && tgt->type != TARGET_TYPE_HEADER_LIB)
    if (!project_textbuf_appendf(buf,
                                 "target_link_options(%s PRIVATE $<$<C_COMPILER_ID:MSVC>:/STACK:%zu> $<$<CXX_COMPILER_ID:MSVC>:/STACK:%zu>)\n",
                                 target_name,
                                 tgt->stack_size,
                                 tgt->stack_size))
      return false;

  if (proj && proj->meta.id && proj->meta.id[0]) {
    const char* proj_id = project_escape_cmake_string(proj->meta.id);
    if (!proj_id)
      return false;
    if (!project_textbuf_appendf(buf, "target_compile_definitions(%s %s BBS_PROJECT_ID=\"%s\")\n", target_name, tgt->type == TARGET_TYPE_HEADER_LIB ? "INTERFACE" : "PRIVATE", proj_id))
      return false;
  }

  if (tgt->pre_build_cmd_c > 0 && tgt->type != TARGET_TYPE_HEADER_LIB) {
    for (int i = 0; i < tgt->pre_build_cmd_c; ++i) {
      const char* expanded_cmd = project_expand_config_string(tgt->pre_build_cmds[i] ? tgt->pre_build_cmds[i] : "", proj, tgt, platform_id, tc, project_build_binary_dir_abs(proj, proj->active_config, platform_id));
      const char* cmd = project_escape_cmake_string(expanded_cmd ? expanded_cmd : "");
      const char* bash = project_cmake_path_text(bash_path);
      if (!cmd || !bash)
        return false;
      if (!project_textbuf_appendf(buf,
                                   "add_custom_command(TARGET %s PRE_BUILD COMMAND \"%s\" -lc \"%s\" VERBATIM)\n",
                                   target_name,
                                   bash,
                                   cmd))
        return false;
    }
  }
  if (tgt->post_build_cmd_c > 0 && tgt->type != TARGET_TYPE_HEADER_LIB) {
    for (int i = 0; i < tgt->post_build_cmd_c; ++i) {
      const char* expanded_cmd = project_expand_config_string(tgt->post_build_cmds[i] ? tgt->post_build_cmds[i] : "", proj, tgt, platform_id, tc, project_build_binary_dir_abs(proj, proj->active_config, platform_id));
      const char* cmd = project_escape_cmake_string(expanded_cmd ? expanded_cmd : "");
      const char* bash = project_cmake_path_text(bash_path);
      if (!cmd || !bash)
        return false;
      if (!project_textbuf_appendf(buf,
                                   "add_custom_command(TARGET %s POST_BUILD COMMAND \"%s\" -lc \"%s\" VERBATIM)\n",
                                   target_name,
                                   bash,
                                   cmd))
        return false;
    }
  }

  if (tgt->testing && project_target_is_runnable(tgt)) {
    if (!project_textbuf_appendf(buf, "add_test(NAME \"%s\" COMMAND $<TARGET_FILE:%s>", target_name, target_name))
      return false;
    for (int i = 0; i < tgt->test_arg_c; ++i) {
      const char* arg = project_escape_cmake_string(tgt->test_args[i] ? tgt->test_args[i] : "");
      if (!arg)
        return false;
      if (!project_textbuf_appendf(buf, " \"%s\"", arg))
        return false;
    }
    if (!project_textbuf_append(buf, ")\n"))
      return false;
  }

  return project_textbuf_append(buf, "\n");
}

static bool project_generate_cmakelists(const project* proj, toolchain* tc, const char* platform_id, bool* changed) {
  if (!proj || !tc)
    return false;

  const char* path = project_build_file_abs(proj, "CMakeLists.txt");
  const char* proj_id = project_escape_cmake_string(proj->meta.id ? proj->meta.id : "bbs_project");
  const char* bash_path = toolchain_get_host_tool_path(tc, "bash");
  if (!path || !proj_id)
    return false;

  project_textbuf buf = {0};
  bool has_cpp = false;
  for (int i = 0; i < proj->target_c; ++i)
    if (proj->targets[i].lang == LANG_CPP)
      has_cpp = true;

  if (!project_textbuf_append(&buf, "cmake_minimum_required(VERSION 3.20)\n\n"))
    return false;
  if (!project_textbuf_appendf(&buf, "project(%s LANGUAGES C%s)\n\n", proj_id, has_cpp ? " CXX" : ""))
    return false;
  const char* project_root = project_cmake_path_text(project_root_dir(proj));
  if (!project_root)
    return false;
  if (!project_textbuf_appendf(&buf,
                               "set(BBS_PROJECT_ROOT \"%s\")\n"
                               "include(CTest)\n"
                               "enable_testing()\n\n",
                               project_root))
    return false;

  for (int i = 0; i < proj->target_c; ++i)
    if (!project_append_cmake_target(&buf, proj, &proj->targets[i], platform_id, tc, bash_path))
      return false;

  if (!project_write_file_if_changed(path, buf.data ? buf.data : "", changed)) {
    error("Failed to write CMakeLists.txt: %s", path);
    free(buf.data);
    return false;
  }

  free(buf.data);
  return true;
}

static bool project_generate_toolchain_file(const project* proj, toolchain* tc, bool* changed) {
  if (!proj || !tc)
    return false;

  const char* path = project_build_file_abs(proj, "bbs-toolchain.cmake");
  if (!path)
    return false;

  project_textbuf buf = {0};
  if (!project_textbuf_append(&buf,
                              "if(NOT DEFINED BBS_TARGET_OS)\n"
                              "  set(BBS_TARGET_OS \"windows\")\n"
                              "endif()\n"
                              "if(NOT DEFINED BBS_TARGET_ARCH)\n"
                              "  set(BBS_TARGET_ARCH \"x86_64\")\n"
                              "endif()\n\n"
                              "if(BBS_TARGET_OS STREQUAL \"windows\")\n"
                              "  set(CMAKE_SYSTEM_NAME Windows)\n"
                              "elseif(BBS_TARGET_OS STREQUAL \"linux\")\n"
                              "  set(CMAKE_SYSTEM_NAME Linux)\n"
                              "elseif(BBS_TARGET_OS STREQUAL \"macos\")\n"
                              "  set(CMAKE_SYSTEM_NAME Darwin)\n"
                              "endif()\n\n"
                              "if(BBS_TARGET_ARCH STREQUAL \"x86_64\")\n"
                              "  set(CMAKE_SYSTEM_PROCESSOR x86_64)\n"
                              "elseif(BBS_TARGET_ARCH STREQUAL \"x86\")\n"
                              "  set(CMAKE_SYSTEM_PROCESSOR x86)\n"
                              "elseif(BBS_TARGET_ARCH STREQUAL \"arm64\")\n"
                              "  set(CMAKE_SYSTEM_PROCESSOR arm64)\n"
                              "endif()\n\n"))
    return false;

  toolchain_env* env = toolchain_host_env(tc, false);
  if (env) {
    for (int i = 0; i < env->tool_c; ++i) {
      const char* key = project_cmake_define_name("BBS_TOOL", env->tools[i].id);
      if (!project_append_cmake_toolchain_kv(&buf, key, env->tools[i].path))
        return false;
    }
    for (int i = 0; i < env->sdk_c; ++i) {
      const char* name = env->sdks[i].name;
      const char* prefix = project_cmake_define_name("BBS_SDK", name);
      if (!project_append_cmake_toolchain_kv(&buf, project_cmake_define_name(prefix, "ROOT"), env->sdks[i].base_path))
        return false;
      if (!project_append_cmake_toolchain_kv(&buf, project_cmake_define_name(prefix, "INCLUDE"), env->sdks[i].inc_path))
        return false;
      if (!project_append_cmake_toolchain_kv(&buf, project_cmake_define_name(prefix, "SOURCE"), env->sdks[i].src_path))
        return false;
      if (!project_append_cmake_toolchain_kv(&buf, project_cmake_define_name(prefix, "LIB"), env->sdks[i].lib_path))
        return false;
      if (!project_append_cmake_toolchain_kv(&buf, project_cmake_define_name(prefix, "BIN"), env->sdks[i].bin_path))
        return false;

      if (name && _stricmp(name, "vulkan_sdk") == 0)
        if (!project_append_cmake_toolchain_kv(&buf, "VULKAN_SDK", env->sdks[i].base_path))
          return false;
      if (name && _stricmp(name, "windows_sdk") == 0)
        if (!project_append_cmake_toolchain_kv(&buf, "CMAKE_WINDOWS_KITS_10_DIR", env->sdks[i].base_path))
          return false;
    }
  }

  if (!project_write_file_if_changed(path, buf.data ? buf.data : "", changed)) {
    error("Failed to write toolchain file: %s", path);
    free(buf.data);
    return false;
  }

  free(buf.data);
  return true;
}

static bool project_generate_presets(const project* proj, toolchain* tc, bool* changed) {
  if (!proj || !tc)
    return false;

  const char* path = project_build_file_abs(proj, "CMakePresets.json");
  if (!path)
    return false;

  project_textbuf buf = {0};
  if (!project_textbuf_append(&buf,
                              "{\n"
                              "  \"version\": 6,\n"
                              "  \"cmakeMinimumRequired\": {\n"
                              "    \"major\": 3,\n"
                              "    \"minor\": 20,\n"
                              "    \"patch\": 0\n"
                              "  },\n"
                              "  \"configurePresets\": [\n"))
    return false;

  bool first = true;
  for (int ci = 0; ci < proj->config_c; ++ci) {
    for (int osi = 0; osi < OS_MAX; ++osi) {
      for (int ai = 0; ai < ARCH_MAX; ++ai) {
        if (!tc->supported[osi][ai])
          continue;

        const char* platform_id = project_platform_id((os)osi, (arch)ai);
        const char* build_dir = project_escape_cmake_string(project_resolved_dir("${sourceDir}", proj->configs[ci], platform_id));
        const char* preset = project_escape_cmake_string(project_build_dir_name(proj->configs[ci], platform_id));
        const char* cmake_arch = ((os)osi) == OS_WINDOWS ? project_cmake_arch_name((arch)ai) : NULL;
        if (!platform_id || !build_dir || !preset || (((os)osi) == OS_WINDOWS && !cmake_arch))
          return false;

        if (!first && !project_textbuf_append(&buf, ",\n"))
          return false;
        first = false;

        if (((os)osi) == OS_WINDOWS) {
          if (!project_textbuf_appendf(&buf,
                                       "    {\n"
                                       "      \"name\": \"%s\",\n"
                                       "      \"binaryDir\": \"%s\",\n"
                                       "      \"architecture\": {\n"
                                       "        \"value\": \"%s\",\n"
                                       "        \"strategy\": \"set\"\n"
                                       "      },\n"
                                       "      \"cacheVariables\": {\n"
                                       "        \"CMAKE_TOOLCHAIN_FILE\": \"${sourceDir}/bbs-toolchain.cmake\",\n"
                                       "        \"BBS_TARGET_OS\": \"%s\",\n"
                                       "        \"BBS_TARGET_ARCH\": \"%s\"\n"
                                       "      }\n"
                                       "    }",
                                       preset,
                                       build_dir,
                                       cmake_arch,
                                       OS_NAMES[osi],
                                       ARCH_NAMES[ai]))
            return false;
        } else {
          if (!project_textbuf_appendf(&buf,
                                       "    {\n"
                                       "      \"name\": \"%s\",\n"
                                       "      \"binaryDir\": \"%s\",\n"
                                       "      \"cacheVariables\": {\n"
                                       "        \"CMAKE_TOOLCHAIN_FILE\": \"${sourceDir}/bbs-toolchain.cmake\",\n"
                                       "        \"BBS_TARGET_OS\": \"%s\",\n"
                                       "        \"BBS_TARGET_ARCH\": \"%s\"\n"
                                       "      }\n"
                                       "    }",
                                       preset,
                                       build_dir,
                                       OS_NAMES[osi],
                                       ARCH_NAMES[ai]))
            return false;
        }
      }
    }
  }

  if (!project_textbuf_append(&buf, "\n  ],\n  \"buildPresets\": [\n"))
    return false;

  first = true;
  for (int ci = 0; ci < proj->config_c; ++ci) {
    for (int osi = 0; osi < OS_MAX; ++osi) {
      for (int ai = 0; ai < ARCH_MAX; ++ai) {
        if (!tc->supported[osi][ai])
          continue;

        const char* platform_id = project_platform_id((os)osi, (arch)ai);
        const char* preset = project_escape_cmake_string(project_build_dir_name(proj->configs[ci], platform_id));
        const char* cmake_cfg = project_cmake_config_name(proj->configs[ci]);
        if (!preset)
          return false;
        if (!first && !project_textbuf_append(&buf, ",\n"))
          return false;
        first = false;
        if (!project_textbuf_appendf(&buf,
                                     "    {\n"
                                     "      \"name\": \"build-%s\",\n"
                                     "      \"configurePreset\": \"%s\",\n"
                                     "      \"configuration\": \"%s\"\n"
                                     "    }",
                                     preset,
                                     preset,
                                     cmake_cfg))
          return false;
      }
    }
  }

  if (!project_textbuf_append(&buf, "\n  ],\n  \"testPresets\": [\n"))
    return false;

  first = true;
  for (int ci = 0; ci < proj->config_c; ++ci) {
    for (int osi = 0; osi < OS_MAX; ++osi) {
      for (int ai = 0; ai < ARCH_MAX; ++ai) {
        if (!tc->supported[osi][ai])
          continue;

        const char* platform_id = project_platform_id((os)osi, (arch)ai);
        const char* preset = project_escape_cmake_string(project_build_dir_name(proj->configs[ci], platform_id));
        const char* cmake_cfg = project_cmake_config_name(proj->configs[ci]);
        if (!preset)
          return false;
        if (!first && !project_textbuf_append(&buf, ",\n"))
          return false;
        first = false;
        if (!project_textbuf_appendf(&buf,
                                     "    {\n"
                                     "      \"name\": \"test-%s\",\n"
                                     "      \"configurePreset\": \"%s\",\n"
                                     "      \"configuration\": \"%s\",\n"
                                     "      \"output\": {\n"
                                     "        \"outputOnFailure\": true\n"
                                     "      }\n"
                                     "    }",
                                     preset,
                                     preset,
                                     cmake_cfg))
          return false;
      }
    }
  }

  if (!project_textbuf_append(&buf, "\n  ]\n}\n"))
    return false;

  if (!project_write_file_if_changed(path, buf.data ? buf.data : "", changed)) {
    error("Failed to write CMakePresets.json: %s", path);
    free(buf.data);
    return false;
  }

  free(buf.data);
  return true;
}

static bool project_prepare_backend(project* proj, toolchain* tc, const char* platform, bool ensure_output_dir, bool refresh_packages, bool* backend_changed) {
  if (backend_changed)
    *backend_changed = false;
  if (!proj) {
    error("Project is not initialized.");
    return false;
  }
  if (!tc) {
    error("Toolchain is not initialized.");
    print("Run 'bbs update --init-toolchain' first.");
    return false;
  }

  const char* build_root = project_build_root_abs(proj);
  const char* dist_root = project_dist_root_abs(proj);
  if (!project_ensure_dir_exists(build_root, "build directory"))
    return false;
  if (!project_ensure_dir_exists(dist_root, "dist directory"))
    return false;

  if (ensure_output_dir) {
    const char* build_dir = project_build_binary_dir_abs(proj, proj->active_config, platform);
    if (!project_ensure_dir_exists(build_dir, "build output directory"))
      return false;
  }

  if (!project_prepare_packages(proj, tc, platform, refresh_packages))
    return false;

  bool toolchain_changed = false;
  bool cmakelists_changed = false;
  bool presets_changed = false;
  if (!project_generate_toolchain_file(proj, tc, &toolchain_changed) ||
      !project_generate_cmakelists(proj, tc, platform, &cmakelists_changed) ||
      !project_generate_presets(proj, tc, &presets_changed))
    return false;

  if (backend_changed)
    *backend_changed = toolchain_changed || cmakelists_changed || presets_changed;
  return true;
}

static bool project_needs_configure(const project* proj, const char* platform, bool backend_changed) {
  if (!proj)
    return true;
  if (backend_changed)
    return true;

  const char* build_dir = project_build_binary_dir_abs(proj, proj->active_config, platform);
  const char* cache_path = toolchain_join2(build_dir, "CMakeCache.txt");
  if (!file_exists(cache_path))
    return true;

  platform_timestamp cache_ts = file_timestamp(cache_path);
  if (cache_ts == 0)
    return true;

  const char* project_path = get_path_cwd("project.bbs");
  if (file_timestamp(project_path) > cache_ts)
    return true;
  if (file_timestamp(project_build_file_abs(proj, "CMakeLists.txt")) > cache_ts)
    return true;
  if (file_timestamp(project_build_file_abs(proj, "CMakePresets.json")) > cache_ts)
    return true;
  if (file_timestamp(project_build_file_abs(proj, "bbs-toolchain.cmake")) > cache_ts)
    return true;
  return false;
}

static bool project_run_cmake_preset(toolchain* tc, const project* proj, const char* preset) {
  const char* cmake = toolchain_get_host_tool_path(tc, "cmake");
  const char* build_root = project_build_root_abs(proj);
  const char* extra_args = proj && proj->user_cfg.cmake_args ? proj->user_cfg.cmake_args : NULL;
  char script[4096] = {0};
  if (!cmake || !cmake[0]) {
    error("cmake was not found in the current toolchain.");
    return false;
  }
  snprintf(script,
           sizeof(script),
           "\"%s\" --preset \"%s\"%s%s",
           cmake,
           preset,
           extra_args && extra_args[0] ? " " : "",
           extra_args && extra_args[0] ? extra_args : "");
  return toolchain_run_bash(tc, build_root, script) == 0;
}

static bool project_run_cmake_build(toolchain* tc, const project* proj, const char* preset, const char* target_name) {
  const char* cmake = toolchain_get_host_tool_path(tc, "cmake");
  const char* build_root = project_build_root_abs(proj);
  const char* extra_args = proj && proj->user_cfg.cmake_build_args ? proj->user_cfg.cmake_build_args : NULL;
  char script[4096] = {0};
  if (!cmake || !cmake[0]) {
    error("cmake was not found in the current toolchain.");
    return false;
  }
  if (target_name && target_name[0])
    snprintf(script,
             sizeof(script),
             "\"%s\" --build --preset \"build-%s\" --target \"%s\"%s%s",
             cmake,
             preset,
             target_name,
             extra_args && extra_args[0] ? " " : "",
             extra_args && extra_args[0] ? extra_args : "");
  else
    snprintf(script,
             sizeof(script),
             "\"%s\" --build --preset \"build-%s\"%s%s",
             cmake,
             preset,
             extra_args && extra_args[0] ? " " : "",
             extra_args && extra_args[0] ? extra_args : "");
  return toolchain_run_bash(tc, build_root, script) == 0;
}

static bool project_run_ctest_preset(toolchain* tc, const project* proj, const char* preset, const char* test_name) {
  const char* ctest = toolchain_get_host_tool_path(tc, "ctest");
  const char* build_root = project_build_root_abs(proj);
  const char* extra_args = proj && proj->user_cfg.ctest_args ? proj->user_cfg.ctest_args : NULL;
  char script[4096] = {0};
  if (!ctest || !ctest[0]) {
    error("ctest was not found in the current toolchain.");
    return false;
  }
  if (test_name && test_name[0])
    snprintf(script,
             sizeof(script),
             "\"%s\" --preset \"test-%s\" -R \"%s\"%s%s",
             ctest,
             preset,
             test_name,
             extra_args && extra_args[0] ? " " : "",
             extra_args && extra_args[0] ? extra_args : "");
  else
    snprintf(script,
             sizeof(script),
             "\"%s\" --preset \"test-%s\"%s%s",
             ctest,
             preset,
             extra_args && extra_args[0] ? " " : "",
             extra_args && extra_args[0] ? extra_args : "");
  return toolchain_run_bash(tc, build_root, script) == 0;
}

static bool project_run_execute(const project* proj, const target* tgt, const char* platform, toolchain* tc) {
  if (!proj || !tgt || !tc)
    return false;

  for (int i = 0; i < tgt->pre_run_cmd_c; ++i)
    if (toolchain_run_bash(tc, NULL, project_expand_config_string(tgt->pre_run_cmds[i], proj, tgt, platform, tc, NULL)) != 0)
      return false;

  const char* exe_path = project_target_executable_abs(proj, tgt, platform);
  if (!exe_path || !file_exists(exe_path)) {
    error("Built executable not found: %s", exe_path ? exe_path : "");
    return false;
  }

  char script[4096] = {0};
  snprintf(script, sizeof(script), "\"%s\"", exe_path);
  if (toolchain_run_bash(tc, NULL, script) != 0)
    return false;

  for (int i = 0; i < tgt->post_run_cmd_c; ++i)
    if (toolchain_run_bash(tc, NULL, project_expand_config_string(tgt->post_run_cmds[i], proj, tgt, platform, tc, NULL)) != 0)
      return false;

  return true;
}

static bool project_test_execute(const project* proj, const target* tgt, const char* test_name, const char* platform, toolchain* tc) {
  if (!proj || !tgt || !tc)
    return false;
  const char* preset = project_build_dir_name(proj->active_config, platform);
  return project_run_ctest_preset(tc, proj, preset, test_name);
}

static const char* project_target_executable_abs(const project* proj, const target* tgt, const char* platform) {
  if (!proj || !tgt)
    return NULL;

  const char* build_dir = project_build_binary_dir_abs(proj, proj->active_config, platform);
  const char* bin_dir = toolchain_join2(build_dir, "bin");
  const char* config_bin_dir = toolchain_join2(bin_dir, project_cmake_config_name(proj->active_config));

  char file_name[512] = {0};
  os target_os = OS_MAX;
  arch target_arch = ARCH_MAX;
  if (!project_parse_platform_id(platform, &target_os, &target_arch))
    return NULL;

  snprintf(file_name, sizeof(file_name), "%s%s", tgt->output ? tgt->output : tgt->meta.id, target_os == OS_WINDOWS ? ".exe" : "");
  const char* config_path = toolchain_join2(config_bin_dir, file_name);
  if (file_exists(config_path))
    return config_path;
  return toolchain_join2(bin_dir, file_name);
}

static int project_find_target_index(const project* proj, const char* name) {
  if (!proj || !name || !name[0])
    return -1;

  int match = -1;
  for (int i = 0; i < proj->target_c; ++i) {
    if (!project_target_matches_name(&proj->targets[i], name))
      continue;
    if (match >= 0) {
      error("Target name '%s' is ambiguous.", name);
      return -2;
    }
    match = i;
  }

  if (match < 0)
    error("Unknown target '%s'.", name);
  return match;
}

static int project_count_package_targets(const project* proj) {
  if (!proj)
    return 0;
  int count = 0;
  for (int i = 0; i < proj->target_c; ++i) {
    const target* tgt = &proj->targets[i];
    if (project_target_is_package(tgt))
      ++count;
  }
  return count;
}

static bool project_package_needs_refresh(const target* tgt) {
  if (!tgt)
    return false;
  package_source source = tgt->package_source;
  if (source == PACKAGE_SOURCE_NONE) {
    if (tgt->package_repo_link && tgt->package_repo_link[0])
      source = PACKAGE_SOURCE_REPO;
    else if (tgt->package_archive_link && tgt->package_archive_link[0])
      source = PACKAGE_SOURCE_ARCHIVE;
    else if (tgt->package_path && tgt->package_path[0])
      source = PACKAGE_SOURCE_PATH;
  }
  return source == PACKAGE_SOURCE_REPO || source == PACKAGE_SOURCE_ARCHIVE;
}

static int project_find_single_package_target(const project* proj) {
  if (!proj)
    return -1;
  int match = -1;
  for (int i = 0; i < proj->target_c; ++i) {
    if (!project_target_is_package(&proj->targets[i]))
      continue;
    if (match >= 0)
      return -2;
    match = i;
  }
  return match;
}

static bool project_refresh_packages(project* proj, toolchain* tc, const char* name) {
  if (!proj || !tc)
    return false;
  if (!name || !name[0] || strcmp(name, "*") == 0) {
    for (int i = 0; i < proj->target_c; ++i) {
      target* tgt = &proj->targets[i];
      if (!project_target_is_package(tgt) || !project_package_needs_refresh(tgt))
        continue;
      if (!project_resolve_package_target(tgt, tc, NULL, true))
        return false;
    }
    return true;
  }

  int idx = project_find_target_index(proj, name);
  if (idx < 0)
    return false;
  if (!project_target_is_package(&proj->targets[idx])) {
    error("Target '%s' is not a package target.", name);
    return false;
  }
  if (!project_package_needs_refresh(&proj->targets[idx])) {
    print("Package '%s' uses a local path and does not need refresh.", name);
    return true;
  }
  return project_resolve_package_target(&proj->targets[idx], tc, NULL, true);
}

static void project_print_package_list_header(void) {
  print("Packages:");
  print("  %-16s %-8s %-12s %-18s %s", "Name", "Source", "Status", "CMake Target", "Location");
}

static void project_print_package_list_row(const target* tgt, toolchain* tc) {
  if (!tgt)
    return;
  target copy = *tgt;
  copy.package_source = copy.package_path && copy.package_path[0] ? PACKAGE_SOURCE_PATH :
                        (copy.package_repo_link && copy.package_repo_link[0] ? PACKAGE_SOURCE_REPO :
                         (copy.package_archive_link && copy.package_archive_link[0] ? PACKAGE_SOURCE_ARCHIVE : PACKAGE_SOURCE_NONE));
  project_package_info info = {0};
  project_package_query(&copy, tc, &info);
  const char* location = copy.package_path ? copy.package_path : (copy.package_repo_link ? copy.package_repo_link : (copy.package_archive_link ? copy.package_archive_link : ""));
  print("  %-16s %-8s %-12s %-18s %s",
        copy.meta.id ? copy.meta.id : "",
        project_package_source_name(copy.package_source),
        project_package_status_label(&info),
        project_target_build_target_name(&copy) ? project_target_build_target_name(&copy) : "",
        location);
}

static void project_print_package_summary(const target* tgt, toolchain* tc) {
  if (!tgt)
    return;

  target copy = *tgt;
  copy.package_source = copy.package_path && copy.package_path[0] ? PACKAGE_SOURCE_PATH :
                        (copy.package_repo_link && copy.package_repo_link[0] ? PACKAGE_SOURCE_REPO :
                         (copy.package_archive_link && copy.package_archive_link[0] ? PACKAGE_SOURCE_ARCHIVE : PACKAGE_SOURCE_NONE));
  project_package_info info = {0};
  project_package_query(&copy, tc, &info);

  print("Package: %s", copy.meta.id ? copy.meta.id : "");
  print("  Type: %s", project_target_type_name(copy.type));
  print("  Source: %s", project_package_source_name(copy.package_source));
  print("  Backend: %s", project_package_backend_name(info.backend));
  print("  CMake Target: %s", project_target_build_target_name(&copy) ? project_target_build_target_name(&copy) : "");
  if (copy.package_repo_link)
    print("  Repo: %s", copy.package_repo_link);
  if (copy.package_repo_tag)
    print("  Tag: %s", copy.package_repo_tag);
  if (copy.package_repo_commit)
    print("  Commit: %s", copy.package_repo_commit);
  if (copy.package_path)
    print("  Path: %s", copy.package_path);
  if (copy.package_subdir)
    print("  Subdir: %s", copy.package_subdir);
  if (copy.package_cmake_target)
    print("  Declared CMake Target: %s", copy.package_cmake_target);
  if (copy.package_archive_link)
    print("  Archive: %s", copy.package_archive_link);
  if (copy.package_archive_strip_prefix)
    print("  Strip Prefix: %s", copy.package_archive_strip_prefix);
  if (info.cache_dir)
    print("  Cache Dir: %s", info.cache_dir);
  if (info.source)
    print("  Resolved Dir: %s", info.source);
  print("  Status: %s", project_package_status_label(&info));
  print("  Present: %s", info.exists ? "yes" : "no");
  print("  CMakeLists: %s", info.has_cmakelists ? "yes" : "no");
  print("  project.bbs: %s", info.has_project_config ? "yes" : "no");
  if (info.local_ref)
    print("  Local Ref: %s", info.local_ref);
  if (info.remote_ref)
    print("  Remote Ref: %s", info.remote_ref);
}

static int project_count_runnable_targets(const project* proj) {
  if (!proj)
    return 0;
  int count = 0;
  for (int i = 0; i < proj->target_c; ++i) {
    if (project_target_is_runnable(&proj->targets[i]))
      ++count;
  }
  return count;
}

static int project_count_test_targets(const project* proj) {
  if (!proj)
    return 0;
  int count = 0;
  for (int i = 0; i < proj->target_c; ++i) {
    if (project_target_is_test(&proj->targets[i]))
      ++count;
  }
  return count;
}

static int project_find_single_runnable_target(const project* proj) {
  if (!proj)
    return -1;

  int match = -1;
  for (int i = 0; i < proj->target_c; ++i) {
    if (!project_target_is_runnable(&proj->targets[i]))
      continue;
    if (match >= 0)
      return -2;
    match = i;
  }
  return match;
}

static int project_find_single_test_target(const project* proj) {
  if (!proj)
    return -1;

  int match = -1;
  for (int i = 0; i < proj->target_c; ++i) {
    if (!project_target_is_test(&proj->targets[i]))
      continue;
    if (match >= 0)
      return -2;
    match = i;
  }
  return match;
}

static void project_print_action_header(const char* action, const project* proj, const char* platform) {
  if (!action || !proj)
    return;
  print("%s", action);
  project_print_field("Config", proj->active_config ? proj->active_config : "default");
  if (platform && platform[0])
    project_print_field("Platform", platform);
}

static void project_print_target_line(const char* action, const target* tgt) {
  if (!action || !tgt)
    return;
  project_print_fieldf("Target", "%s (%s)", tgt->meta.id ? tgt->meta.id : "", project_target_type_name(tgt->type));
}

static bool project_build(const char* target_name, const char* platform, const char* config, toolchain* tc) {
  project proj = {0};
  if (!project_load_config(config, &proj))
    return false;

  const char* platform_id = project_resolve_platform_id(platform, tc);
  if (!platform_id)
    return false;
  bool backend_changed = false;
  if (!project_prepare_backend(&proj, tc, platform_id, true, false, &backend_changed))
    return false;

  const char* preset = project_build_dir_name(proj.active_config, platform_id);

  project_print_action_header("Build", &proj, platform_id);
  project_print_field("Directory", project_resolved_dir(proj.user_cfg.builddir, proj.active_config, platform_id));
  if (target_name && target_name[0]) {
    int idx = project_find_target_index(&proj, target_name);
    if (idx < 0)
      return false;
    project_print_target_line("Build", &proj.targets[idx]);
    if (!project_run_cmake_preset(tc, &proj, preset))
      return false;
    return project_run_cmake_build(tc, &proj, preset, project_target_build_target_name(&proj.targets[idx]));
  }

  if (proj.target_c == 1)
    project_print_target_line("Build", &proj.targets[0]);
  else {
    print("Build targets:");
    for (int i = 0; i < proj.target_c; ++i)
      print("  - %s (%s)", proj.targets[i].meta.id ? proj.targets[i].meta.id : "", project_target_type_name(proj.targets[i].type));
  }

  if (project_needs_configure(&proj, platform_id, backend_changed) && !project_run_cmake_preset(tc, &proj, preset))
    return false;
  return project_run_cmake_build(tc, &proj, preset, NULL);
}

static bool project_run(const char* target_name, const char* platform, const char* config, toolchain* tc) {
  project proj = {0};
  if (!project_load_config(config, &proj))
    return false;

  const char* platform_id = project_resolve_platform_id(platform, tc);
  if (!platform_id)
    return false;
  os target_os = OS_MAX;
  arch target_arch = ARCH_MAX;
  if (!project_parse_platform_id(platform_id, &target_os, &target_arch))
    return false;
  if (target_os != tc->p_os || target_arch != tc->p_arch) {
    error("Run only supports host-native outputs. Requested '%s' but host is '%s'.", platform_id, project_default_platform_id());
    return false;
  }

  bool backend_changed = false;
  if (!project_prepare_backend(&proj, tc, platform_id, true, false, &backend_changed))
    return false;

  const char* preset = project_build_dir_name(proj.active_config, platform_id);

  project_print_action_header("Run", &proj, platform_id);
  int idx = -1;
  if (target_name && target_name[0]) {
    idx = project_find_target_index(&proj, target_name);
    if (idx < 0)
      return false;
    if (!project_target_is_buildable(&proj.targets[idx])) {
      error("Target '%s' is a package target and is not runnable.", target_name);
      return false;
    }
    if (!project_target_is_runnable(&proj.targets[idx])) {
      error("Target '%s' is not runnable.", target_name);
      return false;
    }
  } else {
    idx = project_find_single_runnable_target(&proj);
    if (idx == -2) {
      error("Multiple runnable targets found. Use '-t target'.");
      return false;
    }
    if (idx < 0) {
      error("No runnable targets found.");
      return false;
    }
  }

  project_print_target_line("Run", &proj.targets[idx]);
  if (project_needs_configure(&proj, platform_id, backend_changed) && !project_run_cmake_preset(tc, &proj, preset))
    return false;
  if (!project_run_cmake_build(tc, &proj, preset, proj.targets[idx].meta.id))
    return false;
  return project_run_execute(&proj, &proj.targets[idx], platform_id, tc);
}

static bool project_test(const char* test_name, const char* target_name, const char* platform, const char* config, toolchain* tc) {
  project proj = {0};
  if (!project_load_config(config, &proj))
    return false;
  if (!tc) {
    error("Toolchain is not initialized.");
    print("Run 'bbs update --init-toolchain' first.");
    return false;
  }

  const char* platform_id = project_resolve_platform_id(platform, tc);
  if (!platform_id)
    return false;
  bool backend_changed = false;
  if (!project_prepare_backend(&proj, tc, platform_id, true, false, &backend_changed))
    return false;

  const char* preset = project_build_dir_name(proj.active_config, platform_id);

  project_print_action_header("Test", &proj, platform_id);
  const char* build_dir = project_resolved_dir(proj.user_cfg.builddir, proj.active_config, platform_id);
  project_print_field("Directory", build_dir);
  if (test_name && test_name[0])
    project_print_field("Test", test_name);

  if (target_name && target_name[0]) {
    int idx = project_find_target_index(&proj, target_name);
    if (idx < 0)
      return false;
    if (!project_target_is_buildable(&proj.targets[idx])) {
      error("Target '%s' is a package target and is not a test target.", target_name);
      return false;
    }
    if (!project_target_is_test(&proj.targets[idx])) {
      error("Target '%s' is not a test target.", target_name);
      return false;
    }
    project_print_target_line("Test", &proj.targets[idx]);
    if (project_needs_configure(&proj, platform_id, backend_changed) && !project_run_cmake_preset(tc, &proj, preset))
      return false;
    if (!project_run_cmake_build(tc, &proj, preset, proj.targets[idx].meta.id))
      return false;
    return project_test_execute(&proj, &proj.targets[idx], test_name, platform_id, tc);
  }

  int idx = project_find_single_test_target(&proj);
  if (idx == -2) {
    error("Multiple test targets found. Use '-t target'.");
    return false;
  }
  if (idx < 0) {
    error("No test targets found.");
    return false;
  }

  project_print_target_line("Test", &proj.targets[idx]);
  if (project_needs_configure(&proj, platform_id, backend_changed) && !project_run_cmake_preset(tc, &proj, preset))
    return false;
  if (!project_run_cmake_build(tc, &proj, preset, proj.targets[idx].meta.id))
    return false;
  return project_test_execute(&proj, &proj.targets[idx], test_name, platform_id, tc);
}

static bool project_dist(const char* target_name, const char* platform, const char* config, toolchain* tc) {
  project proj = {0};
  if (!project_load_config(config, &proj))
    return false;

  if (!tc) {
    error("Toolchain is not initialized.");
    print("Run 'bbs update --init-toolchain' first.");
    return false;
  }

  const char* platform_id = project_resolve_platform_id(platform, tc);
  if (!platform_id)
    return false;
  bool backend_changed = false;
  if (!project_prepare_backend(&proj, tc, platform_id, true, false, &backend_changed))
    return false;

  project_print_action_header("Dist", &proj, platform_id);
  project_print_field("Directory", project_resolved_dir(proj.user_cfg.distdir, proj.active_config, platform_id));
  if (target_name && target_name[0]) {
    int idx = project_find_target_index(&proj, target_name);
    if (idx < 0)
      return false;
    project_print_target_line("Dist", &proj.targets[idx]);
    if (!project_target_is_buildable(&proj.targets[idx])) {
      error("Target '%s' is a package target and is not distributable.", target_name);
      return false;
    }
    if (!project_target_is_runnable(&proj.targets[idx])) {
      error("Target '%s' is not distributable.", target_name);
      return false;
    }

    const char* preset = project_build_dir_name(proj.active_config, platform_id);
    if (project_needs_configure(&proj, platform_id, backend_changed) && !project_run_cmake_preset(tc, &proj, preset))
      return false;
    if (!project_run_cmake_build(tc, &proj, preset, proj.targets[idx].meta.id))
      return false;

    target* tgt = &proj.targets[idx];
    const char* dist_root = project_dist_root_abs(&proj);
    const char* dist_cfg_dir = project_dist_config_dir_abs(&proj, proj.active_config, platform_id);
    const char* gen_dir = project_dist_gen_dir_abs(&proj, proj.active_config, platform_id);
    if (!project_ensure_dir_exists(dist_root, "dist directory"))
      return false;
    if (!project_ensure_dir_exists(dist_cfg_dir, "dist output directory"))
      return false;
    if (dir_exists(gen_dir) && !dir_delete(gen_dir)) {
      error("Failed to delete dist staging directory: %s", gen_dir);
      return false;
    }
    if (!project_ensure_dir_exists(gen_dir, "dist staging directory"))
      return false;

    for (int i = 0; i < tgt->pre_dist_cmd_c; ++i)
      if (toolchain_run_bash(tc, gen_dir, project_expand_config_string(tgt->pre_dist_cmds[i], &proj, tgt, platform_id, tc, gen_dir)) != 0)
        return false;

    const char* exe_path = project_target_executable_abs(&proj, tgt, platform_id);
    if (!exe_path || !file_exists(exe_path)) {
      error("Built executable not found for target '%s': %s", tgt->meta.id ? tgt->meta.id : "", exe_path ? exe_path : "");
      return false;
    }

    const char* exe_name = project_path_filename(exe_path);
    const char* exe_dst = toolchain_join2(gen_dir, exe_name);
    if (!project_copy_file(exe_path, exe_dst)) {
      error("Failed to copy executable '%s'.", exe_path);
      return false;
    }

    os target_os = OS_MAX;
    arch target_arch = ARCH_MAX;
    if (!project_parse_platform_id(platform_id, &target_os, &target_arch))
      return false;
    const char* runtime_dir = project_path_parent(exe_path);
    if (!project_copy_runtime_files(runtime_dir, gen_dir, target_os))
      return false;

    for (int i = 0; i < tgt->post_dist_cmd_c; ++i)
      if (toolchain_run_bash(tc, gen_dir, project_expand_config_string(tgt->post_dist_cmds[i], &proj, tgt, platform_id, tc, gen_dir)) != 0)
        return false;

    if (tgt->dist_archive && !project_create_dist_archive(&proj, tgt, platform_id, tc))
      return false;

    return true;
  }

  int idx = project_find_single_runnable_target(&proj);
  if (idx == -2) {
    error("Multiple runnable targets found. Use '-t target'.");
    return false;
  }
  if (idx < 0) {
    error("No runnable targets found.");
    return false;
  }

  return project_dist(proj.targets[idx].meta.id, platform_id, proj.active_config, tc);
}

static bool project_update(toolchain* tc, bool refresh_packages) {
  bool ok = true;

  project proj = {0};
  if (!project_load(&proj))
    return false;

  ok = project_prepare_backend(&proj, tc, NULL, false, refresh_packages, NULL);
  if (ok)
    print("Project update completed for %d target(s).", proj.target_c);

  return ok;
}

static bool project_cleanup(void) {
  bool ok = true;

  user cfg = {0};
  if (!project_load_user_config(get_path_cwd(CFG_INFOS[CFG_LOCAL].filename), &cfg))
    return false;

  const char* build_dir = get_path_cwd(cfg.builddir);
  if (dir_exists(build_dir)) {
    if (!dir_delete(build_dir)) {
      error("Failed to delete build directory: %s", build_dir);
      ok = false;
    } else {
      print("Deleted build directory at %s", build_dir);
    }
  }

  const char* dist_dir = get_path_cwd(cfg.distdir);
  if (dir_exists(dist_dir)) {
    if (!dir_delete(dist_dir)) {
      error("Failed to delete dist directory: %s", dist_dir);
      ok = false;
    } else {
      print("Deleted dist directory at %s", dist_dir);
    }
  }

  const char* local_cfg = get_path_cwd("local.bbs");
  if (file_exists(local_cfg)) {
    if (!file_delete(local_cfg)) {
      error("Failed to delete local config: %s", local_cfg);
      ok = false;
    } else {
      print("Deleted local config at %s", local_cfg);
    }
  }

  if (ok) {
    print("Project cleanup completed.");
  }

  return ok;
}
