#include <bbs/build.h>

static bool tune_target(bbs_tgt* tgt) {
  if (!tgt)
    return false;

  if (!bbs_target_set_text(tgt, BBS_TARGET_TEXT_OUTPUT, "demo_app_tuned"))
    return false;

  if (!bbs_target_set_text(tgt, BBS_TARGET_TEXT_DEFINES, "TARGET_TUNER_ACTIVE TARGET_TUNER_LEVEL=2"))
    return false;

  if (!bbs_target_append_text(tgt, BBS_TARGET_TEXT_DEFINES, "TARGET_TUNER_EXTRA=1", " "))
    return false;

  if (!bbs_target_append_text(tgt, BBS_TARGET_TEXT_ADDITIONAL_COMPILE_ARGS, "-Wno-unused-variable", " "))
    return false;

  if (!bbs_target_set_text(tgt, BBS_TARGET_TEXT_STDVER, "c11"))
    return false;

  return true;
}

bool bbs_callback(bbs_sig signal, bbs_ctx* ctx, bbs_proj* prj, bbs_tgt* tgt) {
  (void)tgt;

  if (!prj)
    return true;

  if (signal == BBS_SIG_PRE_BUILD) {
    for (int i = 0; i < prj->target_c; ++i) {
      bbs_tgt* current = &prj->targets[i];
      if (!bbs_target_has_dependency(current, "target_tuner"))
        continue;
      if (!tune_target(current))
        return false;
    }

    if (!bbs_project_set_text(prj, BBS_PROJECT_TEXT_CMAKE_ARGS, "-DCMAKE_VERBOSE_MAKEFILE=ON"))
      return false;

    if (ctx)
      bbs_print("builder 'target_tuner' adjusted target properties before compilation");
  }

  return true;
}
