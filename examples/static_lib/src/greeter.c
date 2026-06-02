#include "greeter.h"

const char* greeter_message(void) {
  return "Hello from a static library.";
}

int greeter_add(int a, int b) {
  return a + b;
}
