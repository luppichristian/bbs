#define BBS_INTERNAL
#include "bbs_base.c"
#include "bbs_compiler_args.c"
#include "bbs_toolchain.c"
#include "bbs_user.c"
#include "bbs_project.c"
#include "bbs_builders.c"
#include "bbs_cmd.c"

int main(int argc, char** argv) {
  atexit(release);
  platform_timestamp started_ms = now_ms();

  if (argc == 1) {
    print_usage();
    print_done_elapsed(started_ms);
    return 0;
  }

  if (_strcmpi(argv[1], CMD_INFOS[CMD_HELP].name) == 0) {
    print_help(argc, argv);
    print_done_elapsed(started_ms);
    return 0;
  }

  cmd_ctx* cmdctx = init_cmd_ctx(argc, argv);
  for (int i = CMD_CLEAN; i < CMD_MAX; ++i) {
    if (_strcmpi(argv[1], CMD_INFOS[i].name) == 0)
      {
        int rc = run_cmd((cmd)i, cmdctx);
        if (rc == 0)
          print_done_elapsed(started_ms);
        return rc;
      }
  }

  return print_unrecognized_command(argv[1]);
}
