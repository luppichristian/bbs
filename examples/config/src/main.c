#include <stdio.h>

#include "config_info.h"

int main(void) {
  printf("%s\n", config_info_message());
  return 0;
}
