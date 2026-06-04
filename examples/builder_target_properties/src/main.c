#include <stdio.h>

#ifndef TARGET_TUNER_ACTIVE
#  error TARGET_TUNER_ACTIVE was not injected by the builder
#endif

#ifndef TARGET_TUNER_LEVEL
#  error TARGET_TUNER_LEVEL was not injected by the builder
#endif

int main(void) {
  puts("builder target properties example");
  puts("compile-time properties were injected successfully");
  printf("TARGET_TUNER_LEVEL=%d\n", TARGET_TUNER_LEVEL);
  return 0;
}
