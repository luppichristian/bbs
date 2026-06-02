#include <stdio.h>

#include "strings.h"

int main(void) {
  const char* text = header_lib_message();
  printf("%s\n", text);
  printf("vowels: %zu\n", header_lib_count_vowels(text));
  return 0;
}
