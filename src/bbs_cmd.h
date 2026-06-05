#pragma once

#include "bbs_base.h"
#include "../pub/bbs/build.h"

typedef bbs_cfg cfg;
typedef bbs_cfg_loc cfg_loc;
typedef bbs_cfg_info cfg_info;
typedef bbs_cmd_info cmd_info;
typedef bbs_cmd cmd;

#define CFG_PROJECT BBS_CFG_PROJECT
#define CFG_GLOBAL BBS_CFG_GLOBAL
#define CFG_LOCAL BBS_CFG_LOCAL
#define CFG_TOOLCHAIN BBS_CFG_TOOLCHAIN
#define CFG_MAX BBS_CFG_MAX

#define CFG_LOC_CWD BBS_CFG_LOC_CWD
#define CFG_LOC_EXE BBS_CFG_LOC_EXE

#define CFG_INFOS BBS_CFG_INFOS

#define CMD_HELP BBS_CMD_HELP
#define CMD_CLEAN BBS_CMD_CLEAN
#define CMD_UPDATE BBS_CMD_UPDATE
#define CMD_GEN BBS_CMD_GEN
#define CMD_CFG BBS_CMD_CFG
#define CMD_BUILD BBS_CMD_BUILD
#define CMD_AUTO BBS_CMD_AUTO
#define CMD_RUN BBS_CMD_RUN
#define CMD_INFO BBS_CMD_INFO
#define CMD_PACKAGE BBS_CMD_PACKAGE
#define CMD_DIST BBS_CMD_DIST
#define CMD_TEST BBS_CMD_TEST
#define CMD_BUMPVER BBS_CMD_BUMPVER
#define CMD_MAX BBS_CMD_MAX

#define CMD_INFOS BBS_CMD_INFOS

typedef struct {
  cmdline* cl;
  const char* cfg_paths[CFG_MAX];
} cmd_ctx;
