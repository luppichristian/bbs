#pragma once

#include "../pub/bbs/build.h"
#include "bbs_platform.h"

#define VER_MAJOR 0
#define VER_MINOR 2

#define ARENA_DIM (1024 * 1024 * 4)  // 4 megabytes, sizeof each block

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_RESET      "\x1b[0m"
#define ANSI_BOLD       "\x1b[1m"
#define ANSI_FG_INFO    "\x1b[38;5;110m"
#define ANSI_FG_WARN    "\x1b[38;5;180m"
#define ANSI_FG_ERROR   "\x1b[38;5;167m"
#define ANSI_FG_SUCCESS "\x1b[38;5;114m"
#define ANSI_FG_TEXT    "\x1b[38;5;245m"

/* Backward-compatible color aliases */
#define ANSI_BLACK   "\x1b[30m"
#define ANSI_RED     ANSI_FG_ERROR
#define ANSI_GREEN   ANSI_FG_SUCCESS
#define ANSI_YELLOW  ANSI_FG_WARN
#define ANSI_BLUE    ANSI_FG_INFO
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_WHITE   "\x1b[37m"

#define LABEL_WARN  ANSI_BOLD ANSI_FG_WARN "Warn" ANSI_RESET " " ANSI_FG_TEXT
#define LABEL_ERROR ANSI_BOLD ANSI_FG_ERROR "Error" ANSI_RESET " " ANSI_FG_TEXT
#define LABEL_DONE  ANSI_BOLD ANSI_FG_SUCCESS "Done" ANSI_RESET " " ANSI_FG_TEXT

typedef struct {
  const char* short_name;  // "a"
  const char* long_name;   // "all"
  bool present;
  const char* value;
} cmdopt;

typedef struct {
  char** argv;
  int argc;
} cmdline;

typedef bbs_node_type node_type;
typedef bbs_ver ver;
typedef bbs_node node;

#define NODE_TYPE_DEF BBS_NODE_DEF
#define NODE_TYPE_STR BBS_NODE_STR
#define NODE_TYPE_INT BBS_NODE_INT
#define NODE_TYPE_FLT BBS_NODE_FLT
#define NODE_TYPE_VER BBS_NODE_VER
#define NODE_TYPE_IDF BBS_NODE_IDF
#define NODE_TYPE_BOL BBS_NODE_BOL
#define NODE_TYPE_MAX BBS_NODE_MAX

#define NODE_TYPE_NAMES BBS_NODE_TYPE_NAMES

#define _str value._str
#define _int value._int
#define _flt value._flt
#define _ver value._ver
#define _idf value._idf
#define _bol value._bol

typedef struct arena_blk arena_blk;
struct arena_blk {
  arena_blk* next;
  arena_blk* prev;
  void* buff;
  size_t size;
  size_t used;
};

typedef struct {
  arena_blk* blk;
  size_t curr;
} arena;
