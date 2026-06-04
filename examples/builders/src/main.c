#include <stdio.h>

#ifndef PREPROCESSOR_ACTIVE
#  error PREPROCESSOR_ACTIVE was not injected by the builder
#endif

int main(void) {
  puts("builder define injected successfully");
  return 0;
}
