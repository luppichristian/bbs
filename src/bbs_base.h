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
