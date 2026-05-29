#pragma once

#define VER_MAJOR 0
#define VER_MINOR 0

#define ARENA_DIM (1024 * 1024 * 4)  // 4 megabytes, sizeof each block

#include <ctype.h>
#include <direct.h>
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

#define LABEL_INFO    ANSI_FG_TEXT
#define LABEL_WARN    ANSI_BOLD ANSI_FG_WARN "[warn]" ANSI_RESET " " ANSI_FG_TEXT
#define LABEL_ERROR   ANSI_BOLD ANSI_FG_ERROR "[error]" ANSI_RESET " " ANSI_FG_TEXT
#define LABEL_SUCCESS ANSI_BOLD ANSI_FG_SUCCESS "[ ok ]" ANSI_RESET " " ANSI_FG_TEXT

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

typedef enum {
  NODE_TYPE_DEF = 0,
  NODE_TYPE_STR,
  NODE_TYPE_INT,
  NODE_TYPE_FLT,
  NODE_TYPE_VER,
  NODE_TYPE_IDF,
  NODE_TYPE_BOL,
  NODE_TYPE_MAX,
} node_type;

const char* NODE_TYPE_NAMES[] = {
    [NODE_TYPE_DEF] = "def",
    [NODE_TYPE_STR] = "str",
    [NODE_TYPE_INT] = "int",
    [NODE_TYPE_FLT] = "flt",
    [NODE_TYPE_VER] = "ver",
    [NODE_TYPE_IDF] = "idf",
    [NODE_TYPE_BOL] = "bol",
};

typedef struct {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
  uint8_t user;
} ver;

typedef struct node node;
struct node {
  node* next;
  node* parent;
  node* children;

  const char* name;
  size_t name_dim;
  node_type type;
  union {
    const char* _str;
    int64_t _int;
    double _flt;
    ver _ver;
    const char* _idf;
    bool _bol;
  };

  size_t txt_offset;
  size_t txt_dim;
};

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
