#pragma once

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#  include <direct.h>
#  include <io.h>
#  include <windows.h>
#  define access _access
#  define F_OK   0
#else
#  include <dirent.h>
#  include <errno.h>
#  include <strings.h>
#  include <sys/stat.h>
#  include <sys/wait.h>
#  include <unistd.h>
#  if defined(__APPLE__)
#    include <mach-o/dyld.h>
#  endif
#endif

#if !defined(_MAX_PATH)
#  define _MAX_PATH 4096
#endif

#if !defined(_countof)
#  define _countof(x) (sizeof(x) / sizeof((x)[0]))
#endif

#if !defined(_WIN32)
#  define _stricmp strcasecmp
#  define _strcmpi strcasecmp
#  define _getcwd  getcwd
#endif

typedef void* (*platform_alloc_fn)(size_t sz);
