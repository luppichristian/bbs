#include <stdio.h>
#include <string.h>

#include "greeter.h"

int main(void) {
  if (strcmp(greeter_message(), "Hello from a static library.") != 0) {
    fprintf(stderr, "unexpected message\n");
    return 1;
  }

  if (greeter_add(19, 23) != 42) {
    fprintf(stderr, "unexpected sum\n");
    return 1;
  }

  printf("static_lib test passed\n");
  return 0;
}
