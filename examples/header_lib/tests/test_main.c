#include <stdio.h>
#include <string.h>

#include "strings.h"

int main(void) {
  const char* text = header_lib_message();

  if (strcmp(text, "Hello from a header library.") != 0) {
    fprintf(stderr, "unexpected message\n");
    return 1;
  }

  if (header_lib_count_vowels(text) != 9) {
    fprintf(stderr, "unexpected vowel count\n");
    return 1;
  }

  printf("header_lib test passed\n");
  return 0;
}
