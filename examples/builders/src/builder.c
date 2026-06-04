#include <bbs/build.h>

bool bbs_callback(bbs_sig signal, bbs_ctx* ctx, bbs_proj* prj, bbs_tgt* tgt) {
  (void)ctx;
  (void)tgt;

  if (signal != BBS_SIG_PRE_BUILD || !prj)
    return true;

  for (int i = 0; i < prj->target_c; ++i) {
    bbs_tgt* current = &prj->targets[i];
    if (!bbs_target_has_dependency(current, "preprocessor"))
      continue;
    if (!bbs_target_set_text(current, BBS_TARGET_TEXT_DEFINES, "PREPROCESSOR_ACTIVE"))
      return false;
  }

  return true;
}
