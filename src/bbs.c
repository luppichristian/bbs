#include "bbs_base.c"
#include "bbs_cmd.c"
#include "bbs_project.c"
#include "bbs_toolchain.c"
#include "bbs_user.c"

int main(int argc, char** argv) {
  atexit(release);

  if (argc == 1) {
    print_usage();
    return 0;
  }

  if (_strcmpi(argv[1], CMD_INFOS[CMD_HELP].name) == 0) {
    print_help(argc, argv);
    return 0;
  }

  cmd_ctx* cmdctx = init_cmd_ctx(argc, argv);
  for (int i = CMD_CLEAN; i < CMD_MAX; ++i) {
    if (_strcmpi(argv[1], CMD_INFOS[i].name) == 0)
      return run_cmd((cmd)i, cmdctx);
  }

  return print_unrecognized_command(argv[1]);
}
