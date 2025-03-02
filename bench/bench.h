#include <stdbool.h>
#include <stdio.h>

void bench(const char *label, int (*fn)(void));
size_t do_fwrite(const void *p, size_t sz, size_t nitems, FILE *fp);

extern bool enable_output;
