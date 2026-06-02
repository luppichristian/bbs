#pragma once

#include <stddef.h>

static inline const char* header_lib_message(void) {
  return "Hello from a header library.";
}

static inline size_t header_lib_count_vowels(const char* text) {
  size_t count = 0;

  if (!text)
    return 0;

  for (const char* p = text; *p; ++p) {
    switch (*p) {
      case 'a':
      case 'e':
      case 'i':
      case 'o':
      case 'u':
      case 'A':
      case 'E':
      case 'I':
      case 'O':
      case 'U':
        ++count;
        break;
      default:
        break;
    }
  }

  return count;
}
