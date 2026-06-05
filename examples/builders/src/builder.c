#include <bbs/build.h>

bool bbs_callback(bbs_sig signal, bbs_ctx* ctx, bbs_proj* prj, bbs_tgt* tgt) {
  (void)ctx;

  if (signal != BBS_SIG_PRE_BUILD || !prj || !tgt)
    return true;

  if (!bbs_target_has_dependency(tgt, "preprocessor"))
    return true;

  if (!bbs_target_set_text(tgt, BBS_TARGET_TEXT_DEFINES, "PREPROCESSOR_ACTIVE"))
    return false;

  return true;
}
