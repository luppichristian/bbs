#include <stdio.h>

#include "greeter.h"

int main(void) {
  printf("%s\n", greeter_message());
  printf("20 + 22 = %d\n", greeter_add(20, 22));
  return 0;
}
