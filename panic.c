#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void panic(char *fmt, ...) {
  va_list va;
  char msg[1024];

  va_start(va, fmt);
  vsnprintf(msg, sizeof(msg), fmt, va);
  fprintf(stderr, "%s", msg);

  va_end(va);
  exit(EXIT_FAILURE);
}
