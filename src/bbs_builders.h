#pragma once

static bool builders_begin(cmd c, cmd_ctx* cmdctx, toolchain* tc, bool hotbuild_mode);
static void builders_set_selection(const char* target, const char* platform, const char* config);
static bool builders_project_loaded(project* proj);
static void builders_project_finished(project* proj, target* tgt, bool ok);
static int builders_end(int rc);
