#include <stdio.h>

#include "shared.h"

int main(void) {
  int value = common_add(2, 5);
  value = common_scale(value);
  value = feature_offset(value);
  printf("unity result: %d\n", value);
  return 0;
}
