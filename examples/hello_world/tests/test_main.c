#include <stdio.h>
#include <string.h>

#include "app.h"

int main(void) {
  if (strcmp(hello_message(), "Hello, world!") != 0) {
    fprintf(stderr, "unexpected message\n");
    return 1;
  }

  if (hello_add(20, 22) != 42) {
    fprintf(stderr, "unexpected sum\n");
    return 1;
  }

  printf("hello_world test passed\n");
  return 0;
}
