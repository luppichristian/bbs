#pragma once
#include "bbs_builders.h"

#if !defined(_WIN32)
#  include <dlfcn.h>
#endif

typedef bool (*builder_callback_fn)(bbs_sig signal, bbs_ctx* ctx, bbs_proj* prj, bbs_tgt* tgt);

typedef struct {
  const builder* def;
  const char* dll_path;
  builder_callback_fn callback;
#if defined(_WIN32)
  HMODULE handle;
#else
  void* handle;
#endif
} builder_runtime;

typedef struct {
  bool active;
  bool loaded;
  bool hotbuild_mode;
  cmd command;
  cmd_ctx* cmdctx;
  toolchain* tc;
  bbs_ctx ctx;
  builder_runtime* builders;
  int builder_c;
} builder_session;

static builder_session g_builder_session = {0};

static toolchain_env* builders_host_env(toolchain* tc) {
  if (!tc || !tc->envs || tc->env_c <= 0)
    return NULL;

  for (int i = 0; i < tc->env_c; ++i) {
    toolchain_env* env = &tc->envs[i];
    if (env->provider && env->name && _stricmp(env->provider, "host") == 0 && _stricmp(env->name, "current") == 0)
      return env;
  }

  return NULL;
}

static const bbs_tool* builders_find_tool_result(toolchain* tc, const char* id) {
  toolchain_env* env = builders_host_env(tc);
  if (!env || !id || !id[0])
    return NULL;

  for (int i = 0; i < env->tool_c; ++i) {
    if (env->tools[i].id && _stricmp(env->tools[i].id, id) == 0)
      return &env->tools[i];
  }

  return NULL;
}

static const bbs_sdk* builders_find_sdk_result(toolchain* tc, const char* name) {
  toolchain_env* env = builders_host_env(tc);
  if (!env || !name || !name[0])
    return NULL;

  for (int i = 0; i < env->sdk_c; ++i) {
    if (env->sdks[i].name && _stricmp(env->sdks[i].name, name) == 0)
      return &env->sdks[i];
  }

  return NULL;
}

static const char* builders_copy_text(const char* text) {
  return text ? arena_text(text, strlen(text)) : NULL;
}

static bool builders_ensure_dir(const char* path) {
  if (!path || !path[0])
    return false;
  if (dir_exists(path))
    return true;

  char* copy = push(strlen(path) + 1);
  if (!copy)
    return false;
  strcpy(copy, path);
  for (char* p = copy; *p; ++p) {
    if (*p != '/' && *p != '\\')
      continue;
    if (p == copy)
      continue;
    if (p == copy + 2 && copy[1] == ':')
      continue;
    char saved = *p;
    *p = '\0';
    if (copy[0] && !dir_exists(copy) && !dir_create(copy))
      return false;
    *p = saved;
  }

  return dir_exists(copy) || dir_create(copy);
}

static const char* builders_join_text(const char* a, const char* b, const char* sep) {
  if (!a || !a[0])
    return builders_copy_text(b);
  if (!b || !b[0])
    return builders_copy_text(a);
  if (!sep)
    sep = "";

  size_t al = strlen(a);
  size_t bl = strlen(b);
  size_t sl = strlen(sep);
  char* out = push(al + sl + bl + 1);
  if (!out)
    return NULL;
  memcpy(out, a, al);
  memcpy(out + al, sep, sl);
  memcpy(out + al + sl, b, bl + 1);
  return out;
}

static const char* builders_specific_signal_name(cmd c, bool post) {
  switch (c) {
    case CMD_BUILD:
      return post ? "post_build" : "pre_build";
    case CMD_RUN:
      return post ? "post_run" : "pre_run";
    case CMD_DIST:
      return post ? "post_dist" : "pre_dist";
    case CMD_TEST:
      return post ? "post_test" : "pre_test";
    case CMD_UPDATE:
      return post ? "post_update" : "pre_update";
    case CMD_PACKAGE:
      return post ? "post_package" : "pre_package";
    case CMD_GEN:
      return post ? "post_gen" : "pre_gen";
    case CMD_CFG:
      return post ? "post_cfg" : "pre_cfg";
    case CMD_CLEAN:
      return post ? "post_clean" : "pre_clean";
    case CMD_INFO:
      return post ? "post_info" : "pre_info";
    case CMD_BUMPVER:
      return post ? "post_bumpver" : "pre_bumpver";
    case CMD_AUTO:
      return post ? "post_auto" : "pre_auto";
    default:
      return post ? "post_cmd" : "pre_cmd";
  }
}

static bbs_sig builders_specific_signal(cmd c, bool post) {
  switch (c) {
    case CMD_BUILD:
      return post ? BBS_SIG_POST_BUILD : BBS_SIG_PRE_BUILD;
    case CMD_RUN:
      return post ? BBS_SIG_POST_RUN : BBS_SIG_PRE_RUN;
    case CMD_DIST:
      return post ? BBS_SIG_POST_DIST : BBS_SIG_PRE_DIST;
    case CMD_TEST:
      return post ? BBS_SIG_POST_TEST : BBS_SIG_PRE_TEST;
    case CMD_UPDATE:
      return post ? BBS_SIG_POST_UPDATE : BBS_SIG_PRE_UPDATE;
    case CMD_PACKAGE:
      return post ? BBS_SIG_POST_PACKAGE : BBS_SIG_PRE_PACKAGE;
    case CMD_GEN:
      return post ? BBS_SIG_POST_GEN : BBS_SIG_PRE_GEN;
    case CMD_CFG:
      return post ? BBS_SIG_POST_CFG : BBS_SIG_PRE_CFG;
    case CMD_CLEAN:
      return post ? BBS_SIG_POST_CLEAN : BBS_SIG_PRE_CLEAN;
    case CMD_INFO:
      return post ? BBS_SIG_POST_INFO : BBS_SIG_PRE_INFO;
    case CMD_BUMPVER:
      return post ? BBS_SIG_POST_BUMPVER : BBS_SIG_PRE_BUMPVER;
    case CMD_AUTO:
      return post ? BBS_SIG_POST_AUTO : BBS_SIG_PRE_AUTO;
    default:
      return post ? BBS_SIG_POST_CMD : BBS_SIG_PRE_CMD;
  }
}

static bool builders_dispatch_signal(bbs_sig signal, project* proj, target* tgt) {
  if (!g_builder_session.active || !g_builder_session.loaded)
    return true;

  g_builder_session.ctx.signal = signal;
  for (int i = 0; i < g_builder_session.builder_c; ++i) {
    builder_runtime* rt = &g_builder_session.builders[i];
    if (!rt->callback)
      continue;
    if (!rt->callback(signal, &g_builder_session.ctx, (bbs_proj*)proj, (bbs_tgt*)tgt)) {
      error("Builder '%s' rejected signal '%s'.", rt->def && rt->def->meta.id ? rt->def->meta.id : "", bbs_sig_name(signal));
      return false;
    }
  }

  return true;
}

static bool builders_dispatch_project_signal(bbs_sig signal, project* proj) {
  if (!proj || !proj->targets || proj->target_c <= 0)
    return builders_dispatch_signal(signal, proj, NULL);

  for (int i = 0; i < proj->target_c; ++i)
    if (!builders_dispatch_signal(signal, proj, &proj->targets[i]))
      return false;

  return true;
}

static const char* builders_module_ext(void) {
#if defined(_WIN32)
  return ".dll";
#elif defined(__APPLE__)
  return ".dylib";
#else
  return ".so";
#endif
}

static bool builders_write_cmakelists(const project* proj, const builder* bld, const char* root_dir, const char* out_dir, const char* import_lib) {
  char target_name[128] = {0};
  size_t wi = 0;
  const char* src = bld->meta.id ? bld->meta.id : "builder";
  for (size_t i = 0; src[i] && wi + 1 < sizeof(target_name); ++i) {
    char ch = src[i];
    target_name[wi++] = (isalnum((unsigned char)ch) || ch == '_') ? ch : '_';
  }
  target_name[wi] = '\0';
  if (!target_name[0])
    strcpy(target_name, "builder");

  project_textbuf buf = {0};
  bool has_cpp = bld->lang == LANG_CPP;
  if (!project_textbuf_append(&buf, "cmake_minimum_required(VERSION 3.20)\n\n"))
    return false;
  if (!project_textbuf_appendf(&buf, "project(bbs_builder_%s LANGUAGES C%s)\n\n", target_name, has_cpp ? " CXX" : ""))
    return false;
  if (!project_textbuf_appendf(&buf, "add_library(%s SHARED\n", target_name))
    return false;
  for (int i = 0; i < bld->unit_c; ++i) {
    const char* unit = project_resolve_path_from_root(proj->root_dir, bld->units[i]);
    if (!project_textbuf_appendf(&buf, "  \"%s\"\n", project_escape_cmake_string(unit ? unit : bld->units[i])))
      return false;
  }
  if (!project_textbuf_append(&buf, ")\n\n"))
    return false;

  if (!project_textbuf_appendf(&buf, "target_include_directories(%s PRIVATE \"%s\" \"%s\"", target_name,
                                project_escape_cmake_string(toolchain_norm_path(toolchain_join2(project_path_parent(get_path_exe("bbs.exe")), "..\\pub"))),
                                project_escape_cmake_string(proj->root_dir ? proj->root_dir : project_current_workdir())))
    return false;
  for (int i = 0; i < bld->include_dir_c; ++i) {
    const char* dir = project_resolve_path_from_root(proj->root_dir, bld->include_dirs[i]);
    if (!project_textbuf_appendf(&buf, " \"%s\"", project_escape_cmake_string(dir ? dir : bld->include_dirs[i])))
      return false;
  }
  if (!project_textbuf_append(&buf, ")\n"))
    return false;

  if (bld->defines && bld->defines[0]) {
    const char* esc = project_escape_cmake_string(bld->defines);
    if (!project_textbuf_appendf(&buf, "separate_arguments(BBS_BUILDER_DEFINES NATIVE_COMMAND \"%s\")\n", esc))
      return false;
    if (!project_textbuf_appendf(&buf, "target_compile_definitions(%s PRIVATE ${BBS_BUILDER_DEFINES})\n", target_name))
      return false;
  }
  if (bld->additional_compile_args && bld->additional_compile_args[0]) {
    const char* esc = project_escape_cmake_string(bld->additional_compile_args);
    if (!project_textbuf_appendf(&buf, "separate_arguments(BBS_BUILDER_COMPILE_ARGS NATIVE_COMMAND \"%s\")\n", esc))
      return false;
    if (!project_textbuf_appendf(&buf, "target_compile_options(%s PRIVATE ${BBS_BUILDER_COMPILE_ARGS})\n", target_name))
      return false;
  }
  if (bld->additional_link_args && bld->additional_link_args[0]) {
    const char* esc = project_escape_cmake_string(bld->additional_link_args);
    if (!project_textbuf_appendf(&buf, "separate_arguments(BBS_BUILDER_LINK_ARGS NATIVE_COMMAND \"%s\")\n", esc))
      return false;
    if (!project_textbuf_appendf(&buf, "target_link_options(%s PRIVATE ${BBS_BUILDER_LINK_ARGS})\n", target_name))
      return false;
  }
  if (!project_textbuf_appendf(&buf, "set_target_properties(%s PROPERTIES PREFIX \"\" OUTPUT_NAME \"%s\" RUNTIME_OUTPUT_DIRECTORY \"%s\" LIBRARY_OUTPUT_DIRECTORY \"%s\")\n",
                                target_name,
                                project_escape_cmake_string(bld->output ? bld->output : bld->meta.id),
                                project_escape_cmake_string(out_dir),
                                project_escape_cmake_string(out_dir)))
    return false;
  if (bld->lang == LANG_CPP && !project_textbuf_appendf(&buf, "set_target_properties(%s PROPERTIES LINKER_LANGUAGE CXX)\n", target_name))
    return false;
  if (!project_textbuf_append(&buf, "if(WIN32)\n"))
    return false;
  if (!project_textbuf_appendf(&buf, "  target_link_libraries(%s PRIVATE \"%s\")\n", target_name, project_escape_cmake_string(import_lib ? import_lib : "")))
    return false;
  if (!project_textbuf_append(&buf, "endif()\n"))
    return false;

  return write_entire_file(toolchain_join2(root_dir, "CMakeLists.txt"), buf.data ? buf.data : "");
}

static bool builders_compile_one(const project* proj, const builder* bld, toolchain* tc, const char** out_dll_path) {
  const char* cmake = toolchain_get_host_tool_path(tc, "cmake");
  const char* build_root = toolchain_join2(project_build_root_abs(proj), ".bbs-builders");
  const char* builder_root = toolchain_join2(build_root, bld->meta.id ? bld->meta.id : "builder");
  const char* build_dir = toolchain_join2(builder_root, "build");
  const char* out_dir = toolchain_join2(builder_root, "out");
  const char* import_lib = get_path_exe("bbs.lib");
  char script[4096] = {0};
  if (!cmake || !cmake[0]) {
    error("cmake was not found in the current toolchain.");
    return false;
  }
  if (!builders_ensure_dir(build_root) || !builders_ensure_dir(builder_root) || !builders_ensure_dir(out_dir)) {
    error("Failed to create builder build directories under %s.", builder_root ? builder_root : "");
    return false;
  }
  if (!builders_write_cmakelists(proj, bld, builder_root, out_dir, import_lib))
    return false;

  snprintf(script,
           sizeof(script),
           "\"%s\" -S %s -B %s > /dev/null && \"%s\" --build %s --config Release > /dev/null",
           cmake,
           project_shell_quote(toolchain_norm_path(builder_root)),
           project_shell_quote(toolchain_norm_path(build_dir)),
           cmake,
           project_shell_quote(toolchain_norm_path(build_dir)));
  if (toolchain_run_bash(tc, builder_root, script) != 0) {
    error("Failed to build builder '%s'.", bld->meta.id ? bld->meta.id : "");
    return false;
  }

  if (out_dll_path) {
    const char* file_name = toolchain_append_text(bld->output ? bld->output : bld->meta.id, builders_module_ext());
    const char* direct = toolchain_join2(out_dir, file_name);
    if (file_exists(direct))
      *out_dll_path = direct;
    else {
      const char* release_dir = toolchain_join2(out_dir, "Release");
      *out_dll_path = toolchain_join2(release_dir, file_name);
    }
  }
  return true;
}

static bool builders_ensure_loaded(const project* proj) {
  if (!g_builder_session.active || g_builder_session.loaded)
    return true;
  g_builder_session.loaded = true;
  g_builder_session.builder_c = proj ? proj->builder_c : 0;
  if (!proj || proj->builder_c <= 0)
    return true;

  g_builder_session.builders = push(sizeof(*g_builder_session.builders) * (size_t)proj->builder_c);
  if (!g_builder_session.builders)
    return false;
  memset(g_builder_session.builders, 0, sizeof(*g_builder_session.builders) * (size_t)proj->builder_c);

  for (int i = 0; i < proj->builder_c; ++i) {
    builder_runtime* rt = &g_builder_session.builders[i];
    rt->def = &proj->builders[i];
    if (!builders_compile_one(proj, &proj->builders[i], g_builder_session.tc, &rt->dll_path))
      return false;
#if defined(_WIN32)
    rt->handle = LoadLibraryA(rt->dll_path);
    if (!rt->handle) {
      error("Failed to load builder DLL '%s'.", rt->dll_path ? rt->dll_path : "");
      return false;
    }
    rt->callback = (builder_callback_fn)GetProcAddress(rt->handle, "bbs_callback");
#else
    rt->handle = dlopen(rt->dll_path, RTLD_NOW);
    if (!rt->handle) {
      error("Failed to load builder module '%s'.", rt->dll_path ? rt->dll_path : "");
      return false;
    }
    rt->callback = (builder_callback_fn)dlsym(rt->handle, "bbs_callback");
#endif
    if (!rt->callback) {
      error("Builder '%s' does not export bbs_callback.", proj->builders[i].meta.id ? proj->builders[i].meta.id : "");
      return false;
    }
  }

  return builders_dispatch_signal(BBS_SIG_INIT, (project*)proj, NULL);
}

static bool builders_is_supported_command(cmd c) {
  return c == CMD_BUILD || c == CMD_RUN || c == CMD_DIST || c == CMD_TEST || c == CMD_UPDATE || c == CMD_AUTO;
}

static bool builders_command_has_build_phase(cmd c) {
  return c == CMD_BUILD || c == CMD_RUN || c == CMD_DIST || c == CMD_TEST || c == CMD_AUTO;
}

void bbs_print(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  fputc('\n', stdout);
  va_end(args);
}

void bbs_warn(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fputs(LABEL_WARN, stderr);
  vfprintf(stderr, fmt, args);
  fputs(ANSI_RESET, stderr);
  fputc('\n', stderr);
  va_end(args);
}

void bbs_error(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fputs(LABEL_ERROR, stderr);
  vfprintf(stderr, fmt, args);
  fputs(ANSI_RESET, stderr);
  fputc('\n', stderr);
  va_end(args);
}

static bool builders_set_text_ptr(const char** dst, const char* value) {
  if (!dst)
    return false;
  *dst = builders_copy_text(value);
  return value == NULL || *dst != NULL;
}

bool bbs_ctx_set_text(bbs_ctx* ctx, bbs_ctx_text_field field, const char* value) {
  if (!ctx)
    return false;
  switch (field) {
    case BBS_CTX_TEXT_WORKDIR:
      return builders_set_text_ptr(&ctx->workdir, value);
    case BBS_CTX_TEXT_SELECTED_TARGET:
      return builders_set_text_ptr(&ctx->selected_target, value);
    case BBS_CTX_TEXT_SELECTED_PLATFORM:
      return builders_set_text_ptr(&ctx->selected_platform, value);
    case BBS_CTX_TEXT_SELECTED_CONFIG:
      return builders_set_text_ptr(&ctx->selected_config, value);
    default:
      return false;
  }
}

bool bbs_project_set_text(bbs_proj* proj, bbs_project_text_field field, const char* value) {
  if (!proj)
    return false;
  switch (field) {
    case BBS_PROJECT_TEXT_ID:
      return builders_set_text_ptr(&proj->meta.id, value);
    case BBS_PROJECT_TEXT_NAME:
      return builders_set_text_ptr(&proj->meta.name, value);
    case BBS_PROJECT_TEXT_REPO:
      return builders_set_text_ptr(&proj->meta.repo, value);
    case BBS_PROJECT_TEXT_AUTHORS:
      return builders_set_text_ptr(&proj->meta.authors, value);
    case BBS_PROJECT_TEXT_LICENSE_TYPE:
      return builders_set_text_ptr(&proj->meta.license.type, value);
    case BBS_PROJECT_TEXT_LICENSE_FILE:
      return builders_set_text_ptr(&proj->meta.license.file, value);
    case BBS_PROJECT_TEXT_ACTIVE_CONFIG:
      return builders_set_text_ptr(&proj->active_config, value);
    case BBS_PROJECT_TEXT_BUILD_DIR:
      return builders_set_text_ptr(&proj->build_dir, value);
    case BBS_PROJECT_TEXT_ASSETS_DIR:
      return builders_set_text_ptr(&proj->assets_dir, value);
    case BBS_PROJECT_TEXT_DIST_DIR:
      return builders_set_text_ptr(&proj->dist_dir, value);
    case BBS_PROJECT_TEXT_DIST_ARCHIVE_FORMAT:
      return builders_set_text_ptr(&proj->dist_archive_format, value);
    case BBS_PROJECT_TEXT_DIST_ARCHIVE_NAME:
      return builders_set_text_ptr(&proj->dist_archive_name, value);
    case BBS_PROJECT_TEXT_CMAKE_ARGS:
      return builders_set_text_ptr(&proj->cmake_args, value);
    case BBS_PROJECT_TEXT_CMAKE_BUILD_ARGS:
      return builders_set_text_ptr(&proj->cmake_build_args, value);
    case BBS_PROJECT_TEXT_CTEST_ARGS:
      return builders_set_text_ptr(&proj->ctest_args, value);
    default:
      return false;
  }
}

static const char** builders_target_text_slot(bbs_tgt* tgt, bbs_target_text_field field) {
  if (!tgt)
    return NULL;
  switch (field) {
    case BBS_TARGET_TEXT_ID: return &tgt->meta.id;
    case BBS_TARGET_TEXT_NAME: return &tgt->meta.name;
    case BBS_TARGET_TEXT_REPO: return &tgt->meta.repo;
    case BBS_TARGET_TEXT_AUTHORS: return &tgt->meta.authors;
    case BBS_TARGET_TEXT_LICENSE_TYPE: return &tgt->meta.license.type;
    case BBS_TARGET_TEXT_LICENSE_FILE: return &tgt->meta.license.file;
    case BBS_TARGET_TEXT_OUTPUT: return &tgt->output;
    case BBS_TARGET_TEXT_PACKAGE_PATH: return &tgt->package_path;
    case BBS_TARGET_TEXT_PACKAGE_SUBDIR: return &tgt->package_subdir;
    case BBS_TARGET_TEXT_PACKAGE_REPO_LINK: return &tgt->package_repo_link;
    case BBS_TARGET_TEXT_PACKAGE_REPO_TAG: return &tgt->package_repo_tag;
    case BBS_TARGET_TEXT_PACKAGE_REPO_COMMIT: return &tgt->package_repo_commit;
    case BBS_TARGET_TEXT_PACKAGE_CMAKE_TARGET: return &tgt->package_cmake_target;
    case BBS_TARGET_TEXT_PACKAGE_ARCHIVE_LINK: return &tgt->package_archive_link;
    case BBS_TARGET_TEXT_PACKAGE_ARCHIVE_STRIP_PREFIX: return &tgt->package_archive_strip_prefix;
    case BBS_TARGET_TEXT_PACKAGE_PROJECT_CFG_PATH: return &tgt->package_project_cfg_path;
    case BBS_TARGET_TEXT_PACKAGE_RESOLVED_DIR: return &tgt->package_resolved_dir;
    case BBS_TARGET_TEXT_PACKAGE_CACHE_DIR: return &tgt->package_cache_dir;
    case BBS_TARGET_TEXT_PACKAGE_BUILD_DIR: return &tgt->package_build_dir;
    case BBS_TARGET_TEXT_DEFINES: return &tgt->defines;
    case BBS_TARGET_TEXT_ADDITIONAL_COMPILE_ARGS: return &tgt->additional_compile_args;
    case BBS_TARGET_TEXT_ADDITIONAL_LINK_ARGS: return &tgt->additional_link_args;
    case BBS_TARGET_TEXT_STDVER: return &tgt->stdver;
    case BBS_TARGET_TEXT_DIST_ARCHIVE_NAME: return &tgt->dist.archive_name;
    default: return NULL;
  }
}

bool bbs_target_set_text(bbs_tgt* tgt, bbs_target_text_field field, const char* value) {
  return builders_set_text_ptr(builders_target_text_slot(tgt, field), value);
}

bool bbs_target_append_text(bbs_tgt* tgt, bbs_target_text_field field, const char* value, const char* separator) {
  const char** slot = builders_target_text_slot(tgt, field);
  if (!slot)
    return false;
  *slot = builders_join_text(*slot, value, separator ? separator : " ");
  return *slot != NULL || !value || !value[0];
}

int bbs_run_bash(bbs_ctx* ctx, const char* workdir, const char* command) {
  (void)ctx;
  return g_builder_session.tc ? toolchain_run_bash(g_builder_session.tc, workdir, command) : -1;
}

const bbs_tool* bbs_find_tool(bbs_ctx* ctx, const bbs_tool_discover_strat* strat) {
  (void)ctx;
  if (!g_builder_session.tc || !strat) {
    bbs_error("bbs_find_tool requires an active builder toolchain and a non-null strategy.");
    return NULL;
  }
  if (!strat->id || !strat->id[0]) {
    bbs_error("bbs_find_tool requires strategy field 'id'.");
    return NULL;
  }
  if (!strat->exe_name || !strat->exe_name[0]) {
    bbs_error("bbs_find_tool('%s') requires strategy field 'exe_name'.", strat->id);
    return NULL;
  }

  toolchain_discover_tool(g_builder_session.tc, (const tool_discover_strat*)strat);
  toolchain_sort_tools(g_builder_session.tc);
  toolchain_snapshot_current_host_env(g_builder_session.tc);
  toolchain_refresh_runtime_support(g_builder_session.tc);
  return builders_find_tool_result(g_builder_session.tc, strat->id);
}

const bbs_sdk* bbs_find_sdk(bbs_ctx* ctx, const bbs_sdk_discover_strat* strat) {
  (void)ctx;
  if (!g_builder_session.tc || !strat) {
    bbs_error("bbs_find_sdk requires an active builder toolchain and a non-null strategy.");
    return NULL;
  }
  if (!strat->id || !strat->id[0]) {
    bbs_error("bbs_find_sdk requires strategy field 'id'.");
    return NULL;
  }
  if ((!strat->env_vars || !strat->env_vars[0]) && (!strat->root_hints || !strat->root_hints[0])) {
    bbs_error("bbs_find_sdk('%s') requires at least one of 'env_vars' or 'root_hints'.", strat->id);
    return NULL;
  }

  toolchain_discover_sdk(g_builder_session.tc, (const sdk_discover_strat*)strat);
  toolchain_sort_sdks(g_builder_session.tc);
  toolchain_snapshot_current_host_env(g_builder_session.tc);
  toolchain_refresh_runtime_support(g_builder_session.tc);
  return builders_find_sdk_result(g_builder_session.tc, strat->id);
}

static void builders_fill_ctx(cmd c, cmd_ctx* cmdctx, toolchain* tc, bool hotbuild_mode) {
  memset(&g_builder_session.ctx, 0, sizeof(g_builder_session.ctx));
  g_builder_session.ctx.api_version = BBS_BUILD_API_VERSION;
  g_builder_session.ctx.struct_size = (uint32_t)sizeof(g_builder_session.ctx);
  g_builder_session.ctx.command = c;
  g_builder_session.ctx.hotbuild_mode = hotbuild_mode;
  g_builder_session.ctx.flags = hotbuild_mode ? BBS_CTXF_HOTBUILD_MODE : 0u;
  g_builder_session.ctx.project_cfg_path = cmdctx ? cmdctx->cfg_paths[CFG_PROJECT] : NULL;
  g_builder_session.ctx.user_cfg_path = cmdctx ? cmdctx->cfg_paths[CFG_USER] : NULL;
  g_builder_session.ctx.local_cfg_path = cmdctx ? cmdctx->cfg_paths[CFG_LOCAL] : NULL;
  g_builder_session.ctx.toolchain_cfg_path = cmdctx ? cmdctx->cfg_paths[CFG_TOOLCHAIN] : NULL;
  g_builder_session.ctx.command_name = bbs_cmd_name(c);
  g_builder_session.ctx.cwd = project_current_workdir();
  g_builder_session.ctx.exe_path = get_path_exe("bbs.exe");
  g_builder_session.ctx.workdir = g_builder_session.ctx.cwd;
  g_builder_session.ctx.toolchain = (const bbs_toolchain*)tc;
  if (tc) {
    g_builder_session.ctx.host.os = tc->p_os;
    g_builder_session.ctx.host.arch = tc->p_arch;
    g_builder_session.ctx.selected = g_builder_session.ctx.host;
  }
}

static bool builders_begin(cmd c, cmd_ctx* cmdctx, toolchain* tc, bool hotbuild_mode) {
  memset(&g_builder_session, 0, sizeof(g_builder_session));
  if (!builders_is_supported_command(c))
    return true;
  g_builder_session.active = true;
  g_builder_session.command = c;
  g_builder_session.cmdctx = cmdctx;
  g_builder_session.tc = tc;
  g_builder_session.hotbuild_mode = hotbuild_mode;
  builders_fill_ctx(c, cmdctx, tc, hotbuild_mode);
  return true;
}

static void builders_set_selection(const char* target, const char* platform, const char* config) {
  if (!g_builder_session.active)
    return;
  bbs_ctx_set_text(&g_builder_session.ctx, BBS_CTX_TEXT_SELECTED_TARGET, target);
  bbs_ctx_set_text(&g_builder_session.ctx, BBS_CTX_TEXT_SELECTED_PLATFORM, platform);
  bbs_ctx_set_text(&g_builder_session.ctx, BBS_CTX_TEXT_SELECTED_CONFIG, config);
}

static bool builders_project_loaded(project* proj) {
  if (!g_builder_session.active)
    return true;
  if (!builders_ensure_loaded(proj))
    return false;
  if (!g_builder_session.loaded || g_builder_session.builder_c <= 0)
    return true;

  g_builder_session.ctx.selected_config = proj->active_config;
  g_builder_session.ctx.workdir = proj->root_dir ? proj->root_dir : g_builder_session.ctx.cwd;
  if (!builders_dispatch_project_signal(BBS_SIG_PRE_CMD, proj))
    return false;
  if (builders_command_has_build_phase(g_builder_session.command) && g_builder_session.command != CMD_BUILD)
    if (!builders_dispatch_project_signal(BBS_SIG_PRE_BUILD, proj))
      return false;
  return builders_dispatch_project_signal(builders_specific_signal(g_builder_session.command, false), proj);
}

static void builders_project_finished(project* proj, target* tgt, bool ok) {
  (void)tgt;

  if (!g_builder_session.active || !g_builder_session.loaded || g_builder_session.builder_c <= 0)
    return;
  g_builder_session.ctx.command_failed = !ok;
  builders_dispatch_project_signal(builders_specific_signal(g_builder_session.command, true), proj);
  if (builders_command_has_build_phase(g_builder_session.command) && g_builder_session.command != CMD_BUILD)
    builders_dispatch_project_signal(BBS_SIG_POST_BUILD, proj);
  builders_dispatch_project_signal(BBS_SIG_POST_CMD, proj);
}

static int builders_end(int rc) {
  if (!g_builder_session.active)
    return rc;
  g_builder_session.ctx.command_result = rc;
  g_builder_session.ctx.command_failed = rc != 0;
  if (g_builder_session.loaded && g_builder_session.builder_c > 0)
    builders_dispatch_signal(BBS_SIG_QUIT, NULL, NULL);
  for (int i = 0; i < g_builder_session.builder_c; ++i) {
#if defined(_WIN32)
    if (g_builder_session.builders && g_builder_session.builders[i].handle)
      FreeLibrary(g_builder_session.builders[i].handle);
#else
    if (g_builder_session.builders && g_builder_session.builders[i].handle)
      dlclose(g_builder_session.builders[i].handle);
#endif
  }
  memset(&g_builder_session, 0, sizeof(g_builder_session));
  return rc;
}
