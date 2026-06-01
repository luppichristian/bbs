#pragma once
#include "bbs_project.h"
#include "bbs_toolchain.c"
#include "bbs_user.c"

static bool project_parse_string_list(node* list_n, const char*** out_items, int* out_count);
static const char* project_join_scalar_list(node* list_n, int* out_count);

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
  if (!project_read_text_child(scope, "repo", &out->repo))
    return false;
  if (!project_read_text_child(scope, "authors", &out->authors))
    return false;

  node* ver_n = node_get_child(scope, "version");
  node* lic_n = node_get_child(scope, "license");

  if (ver_n) {
    if (ver_n->type != NODE_TYPE_VER) {
      error("Attribute 'version' must be a version value.");
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

static bool project_parse_meta_scope(node* scope, meta* out) {
  if (!scope || !out)
    return false;

  if (!project_parse_meta_fields(scope, out))
    return false;

  node* meta_n = node_get_child(scope, "meta");
  if (meta_n)
    return project_parse_meta_fields(meta_n, out);

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

  node* value_n = node_get_child(scope, "units");
  if (value_n && !project_parse_string_list(value_n, &out->units, &out->unit_c))
    return false;
  value_n = node_get_child(scope, "include_dirs");
  if (value_n && !project_parse_string_list(value_n, &out->include_dirs, &out->include_dir_c))
    return false;
  value_n = node_get_child(scope, "link_dirs");
  if (value_n && !project_parse_string_list(value_n, &out->link_dirs, &out->link_dir_c))
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
  if (_stricmp(attr_n->name, "units") == 0)
    return project_parse_string_list(attr_n, &out->units, &out->unit_c);
  if (_stricmp(attr_n->name, "include_dirs") == 0)
    return project_parse_string_list(attr_n, &out->include_dirs, &out->include_dir_c);
  if (_stricmp(attr_n->name, "link_dirs") == 0)
    return project_parse_string_list(attr_n, &out->link_dirs, &out->link_dir_c);
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
  if (!project_parse_meta_scope(target_n, &out->meta))
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
    if (tgt->unit_c <= 0) {
      error("Target '%s' must define at least one source unit.", tgt->meta.id);
      return false;
    }

    for (int j = i + 1; j < proj->target_c; ++j) {
      if (_stricmp(tgt->meta.id, proj->targets[j].meta.id) == 0) {
        error("Duplicate target id '%s'.", tgt->meta.id);
        return false;
      }
    }
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
  printf(ANSI_FG_INFO "\n%s\n" ANSI_RESET, title);
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
    project_print_list("Units", tgt->units, tgt->unit_c);
    project_print_list("Include Dirs", tgt->include_dirs, tgt->include_dir_c);
    project_print_list("Link Dirs", tgt->link_dirs, tgt->link_dir_c);
    project_print_list("Link Libs", tgt->link_libs, tgt->link_libs_count);
    project_print_list("Pre Build Cmds", tgt->pre_build_cmds, tgt->pre_build_cmd_c);
    project_print_list("Post Build Cmds", tgt->post_build_cmds, tgt->post_build_cmd_c);
    project_print_list("Pre Run Cmds", tgt->pre_run_cmds, tgt->pre_run_cmd_c);
    project_print_list("Post Run Cmds", tgt->post_run_cmds, tgt->post_run_cmd_c);
    project_print_list("Test Args", tgt->test_args, tgt->test_arg_c);
    project_print_list("Pre Dist Cmds", tgt->pre_dist_cmds, tgt->pre_dist_cmd_c);
    project_print_list("Post Dist Cmds", tgt->post_dist_cmds, tgt->post_dist_cmd_c);
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
  if (!project_parse_meta_scope(project_n, &out->meta))
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

  if (project_count == 0) {
    error("No top-level project node found in %s.", path);
    return false;
  }
  if (project_count > 1) {
    error("Multiple top-level project nodes found in %s.", path);
    return false;
  }

  return project_parse_project_node(project_n, config, out);
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

static const char* project_join_args(const char** items, int count) {
  if (!items || count <= 0)
    return NULL;

  size_t total = 0;
  for (int i = 0; i < count; ++i) {
    if (!items[i])
      continue;
    total += strlen(items[i]) + 1;
  }
  if (total == 0)
    return NULL;

  char* out = push(total + 1);
  if (!out)
    return NULL;

  size_t wi = 0;
  for (int i = 0; i < count; ++i) {
    if (!items[i])
      continue;
    if (wi > 0)
      out[wi++] = ' ';
    size_t len = strlen(items[i]);
    memcpy(out + wi, items[i], len);
    wi += len;
  }
  out[wi] = '\0';
  return out;
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
  print("%s config: %s", action, proj->active_config ? proj->active_config : "default");
  if (platform && platform[0])
    print("%s platform: %s", action, platform);
}

static void project_print_target_line(const char* action, const target* tgt) {
  if (!action || !tgt)
    return;
  print("%s target: %s (%s)", action, tgt->meta.id ? tgt->meta.id : "", project_target_type_name(tgt->type));
}

static bool project_build(const char* target_name, const char* platform, const char* config, toolchain* tc) {
  (void)tc;
  project proj = {0};
  if (!project_load_config(config, &proj))
    return false;

  project_print_action_header("Build", &proj, platform);
  print("Build dir: %s", project_resolved_dir(BUILD_DIR, proj.active_config, platform));
  if (target_name && target_name[0]) {
    int idx = project_find_target_index(&proj, target_name);
    if (idx < 0)
      return false;
    project_print_target_line("Build", &proj.targets[idx]);
    return true;
  }

  if (proj.target_c == 1) {
    project_print_target_line("Build", &proj.targets[0]);
    return true;
  }

  print("Build targets:");
  for (int i = 0; i < proj.target_c; ++i)
    print("  - %s (%s)", proj.targets[i].meta.id ? proj.targets[i].meta.id : "", project_target_type_name(proj.targets[i].type));
  return true;
}

static bool project_run(const char* target_name, const char* platform, const char* config, toolchain* tc) {
  (void)tc;
  project proj = {0};
  if (!project_load_config(config, &proj))
    return false;

  project_print_action_header("Run", &proj, platform);
  if (target_name && target_name[0]) {
    int idx = project_find_target_index(&proj, target_name);
    if (idx < 0)
      return false;
    if (!project_target_is_runnable(&proj.targets[idx])) {
      error("Target '%s' is not runnable.", target_name);
      return false;
    }
    project_print_target_line("Run", &proj.targets[idx]);
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

  project_print_target_line("Run", &proj.targets[idx]);
  return true;
}

static bool project_test(const char* test_name, const char* target_name, const char* platform, const char* config, toolchain* tc) {
  project proj = {0};
  if (!project_load_config(config, &proj))
    return false;
  if (!tc) {
    error("Toolchain is not initialized.");
    print("Run 'bbs toolchain init' first.");
    return false;
  }

  const char* ctest = toolchain_get_host_tool_path(tc, "ctest");
  if (!ctest || !ctest[0]) {
    error("ctest was not found in the current toolchain.");
    return false;
  }

  project_print_action_header("Test", &proj, platform);
  const char* build_dir = project_resolved_dir(BUILD_DIR, proj.active_config, platform);
  print("Test dir: %s", build_dir);
  print("Test runner: %s", ctest);
  if (test_name && test_name[0])
    print("Test name: %s", test_name);

  if (target_name && target_name[0]) {
    int idx = project_find_target_index(&proj, target_name);
    if (idx < 0)
      return false;
    if (!project_target_is_test(&proj.targets[idx])) {
      error("Target '%s' is not a test target.", target_name);
      return false;
    }
    project_print_target_line("Test", &proj.targets[idx]);
    const char* extra_args = project_join_args(proj.targets[idx].test_args, proj.targets[idx].test_arg_c);
    char script[4096] = {0};
    if (test_name && test_name[0] && extra_args && extra_args[0])
      snprintf(script, sizeof(script), "\"%s\" --test-dir \"%s\" -R \"%s\" %s", ctest, build_dir, test_name, extra_args);
    else if (test_name && test_name[0])
      snprintf(script, sizeof(script), "\"%s\" --test-dir \"%s\" -R \"%s\"", ctest, build_dir, test_name);
    else if (extra_args && extra_args[0])
      snprintf(script, sizeof(script), "\"%s\" --test-dir \"%s\" %s", ctest, build_dir, extra_args);
    else
      snprintf(script, sizeof(script), "\"%s\" --test-dir \"%s\"", ctest, build_dir);
    return toolchain_run_bash(tc, NULL, script) == 0;
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
  const char* extra_args = project_join_args(proj.targets[idx].test_args, proj.targets[idx].test_arg_c);
  char script[4096] = {0};
  if (test_name && test_name[0] && extra_args && extra_args[0])
    snprintf(script, sizeof(script), "\"%s\" --test-dir \"%s\" -R \"%s\" %s", ctest, build_dir, test_name, extra_args);
  else if (test_name && test_name[0])
    snprintf(script, sizeof(script), "\"%s\" --test-dir \"%s\" -R \"%s\"", ctest, build_dir, test_name);
  else if (extra_args && extra_args[0])
    snprintf(script, sizeof(script), "\"%s\" --test-dir \"%s\" %s", ctest, build_dir, extra_args);
  else
    snprintf(script, sizeof(script), "\"%s\" --test-dir \"%s\"", ctest, build_dir);
  return toolchain_run_bash(tc, NULL, script) == 0;
}

static bool project_dist(const char* target_name, const char* platform, const char* config, toolchain* tc) {
  (void)tc;
  project proj = {0};
  if (!project_load_config(config, &proj))
    return false;

  project_print_action_header("Dist", &proj, platform);
  print("Dist dir: %s", project_resolved_dir(DIST_DIR, proj.active_config, platform));
  if (target_name && target_name[0]) {
    int idx = project_find_target_index(&proj, target_name);
    if (idx < 0)
      return false;
    project_print_target_line("Dist", &proj.targets[idx]);
    return true;
  }

  if (proj.target_c == 1) {
    project_print_target_line("Dist", &proj.targets[0]);
    return true;
  }

  print("Dist targets:");
  for (int i = 0; i < proj.target_c; ++i)
    print("  - %s (%s)", proj.targets[i].meta.id ? proj.targets[i].meta.id : "", project_target_type_name(proj.targets[i].type));
  return true;
}

static bool project_update(void) {
  bool ok = true;

  project proj = {0};
  if (!project_load(&proj))
    return false;

  const char* build_dir = get_path_cwd(BUILD_DIR);
  if (!dir_exists(build_dir)) {
    if (!dir_create(build_dir)) {
      error("Failed to create build directory: %s", build_dir);
      ok = false;
    } else {
      print("Created build directory: %s", build_dir);
    }
  }

  const char* dist_dir = get_path_cwd(DIST_DIR);
  if (!dir_exists(dist_dir)) {
    if (!dir_create(dist_dir)) {
      error("Failed to create dist directory: %s", dist_dir);
      ok = false;
    } else {
      print("Created dist directory: %s", dist_dir);
    }
  }

  if (ok) {
    print("Project update completed for %d target(s).", proj.target_c);
  }

  return ok;
}

static bool project_cleanup(void) {
  bool ok = true;

  const char* build_dir = get_path_cwd(BUILD_DIR);
  if (dir_exists(build_dir)) {
    if (!dir_delete(build_dir)) {
      error("Failed to delete build directory: %s", build_dir);
      ok = false;
    } else {
      print("Deleted build directory: %s", build_dir);
    }
  }

  const char* dist_dir = get_path_cwd(DIST_DIR);
  if (dir_exists(dist_dir)) {
    if (!dir_delete(dist_dir)) {
      error("Failed to delete dist directory: %s", dist_dir);
      ok = false;
    } else {
      print("Deleted dist directory: %s", dist_dir);
    }
  }

  const char* local_cfg = get_path_cwd("local.bbs");
  if (file_exists(local_cfg)) {
    if (!file_delete(local_cfg)) {
      error("Failed to delete local config: %s", local_cfg);
      ok = false;
    } else {
      print("Deleted local config: %s", local_cfg);
    }
  }

  if (ok) {
    print("Project cleanup completed.");
  }

  return ok;
}
