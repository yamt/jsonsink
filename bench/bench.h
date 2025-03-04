#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

void bench(const char *label,
           int (*fn)(unsigned int n, const double *data_double,
                     const uint32_t *data_u32));
size_t do_fwrite(const void *p, size_t sz, size_t nitems, FILE *fp);
void run_bench(void);

extern bool test_run;

#if defined(__cplusplus)
}
#endif
