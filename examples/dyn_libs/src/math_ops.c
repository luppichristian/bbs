#include <stdio.h>

#include "math_ops.h"
#include "messages.h"

int math_ops_add(int a, int b) {
  return a + b;
}

const char* math_ops_summary(void) {
  static char buffer[96];
  snprintf(buffer, sizeof(buffer), "%s: 20 + 22 = %d", messages_prefix(), math_ops_add(20, 22));
  return buffer;
}
