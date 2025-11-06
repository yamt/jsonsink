/*-
 * Copyright (c)2025 YAMAMOTO Takashi,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bench.h"
#include "malloc_interposer.h"
#include "rng.h"

bool test_run;

void
random_u32(struct rng *rng, unsigned int n, uint32_t *p)
{
        unsigned int i;
        for (i = 0; i < n; i++) {
                *p++ = rng_rand_u32(rng);
        }
}

void
random_double(struct rng *rng, unsigned int n, double *p)
{
        unsigned int i;
        for (i = 0; i < n; i++) {
                /*
                 * XXX maybe it's better to use a bit more realisitc
                 * distribution.
                 */
#if defined(BENCH_DOUBLE_BITWISE)
                double d;
                do {
                        union {
                                double d;
                                uint64_t u;
                        } u;
                        u.u = ((uint64_t)rng_rand_u32(rng) << 32) |
                              rng_rand_u32(rng);
                        d = u.d;
                } while (isnan(d) || isinf(d));
                *p++ = d;
#elif defined(BENCH_DOUBLE_INTEGER)
                *p++ = (double)rng_rand_u32(rng);
#else
                double d;
                do {
                        d = (double)(int32_t)rng_rand_u32(rng) / 100000;
                } while (isnan(d) || isinf(d));
                *p++ = d;
#endif
        }
}

void
bench(const char *label, int (*fn)(unsigned int n, const double *data_double,
                                   const uint32_t *data_u32))
{
        struct rng rng;
        clockid_t cid = CLOCK_MONOTONIC;
        struct timespec start;
        struct timespec end;
        unsigned int i;
        unsigned int n = 100000;
        int ret;

        if (test_run) {
                n = 1;
        }
#define ndata 32
        uint32_t data_u32[ndata];
        double data_double[ndata * 4];
        rng_init(&rng, 0x12345678);
        random_u32(&rng, ndata, data_u32);
        random_double(&rng, ndata * 4, data_double);

        struct malloc_stat *stat = &malloc_stat;
        size_t peak = 0;
        size_t base = 0;
        if (stat != NULL) {
                /*
                 * the dtoa implementation in macOS libc seems to cache some
                 * heap allocations for bignums. as we are not interested in
                 * them, perform a dry run to populate the cache enough before
                 * getting the baseline of heap alloctaions.
                 */
                int error = fn(ndata, data_double, data_u32);
                if (error) {
                        fprintf(stderr, "callback failed with %d\n", error);
                        exit(1);
                }
                base = stat->allocated_bytes; /* baseline */
                /* reset */
                stat->peak_allocated_bytes = base;
                stat->nalloc = 0;
                stat->nresize = 0;
                stat->nfree = 0;
        }

        ret = clock_gettime(cid, &start);
        if (ret != 0) {
                fprintf(stderr, "clock_gettime failed\n");
                exit(1);
        }
        for (i = 0; i < n; i++) {
                int error = fn(ndata, data_double, data_u32);
                if (error) {
                        fprintf(stderr, "callback failed with %d\n", error);
                        exit(1);
                }
        }
        ret = clock_gettime(cid, &end);
        if (ret != 0) {
                fprintf(stderr, "clock_gettime failed\n");
                exit(1);
        }
        double start_sec = start.tv_sec * 1.0 + start.tv_nsec / 1000000000.0;
        double end_sec = end.tv_sec * 1.0 + end.tv_nsec / 1000000000.0;
        double cps = n / (end_sec - start_sec);
        if (!test_run) {
                uint64_t nalloc = 0;
                uint64_t nresize = 0;
                uint64_t nfree = 0;
                if (stat != NULL) {
                        peak = stat->peak_allocated_bytes;
                        size_t now = stat->allocated_bytes;
                        if (now != base) {
                                fprintf(stderr, "memory leak? %zu != %zu\n",
                                        now, base);
                        }
                        nalloc = stat->nalloc;
                        nresize = stat->nresize;
                        nfree = stat->nfree;
                }
                printf("%s, %g, %zu, %" PRIu64 ", %" PRIu64 ", %" PRIu64 "\n",
                       label, cps, peak - base, nalloc, nresize, nfree);
        }
}

size_t
do_fwrite(const void *p, size_t sz, size_t nitems, FILE *fp)
{
        if (test_run) {
                return fwrite(p, sz, nitems, fp);
        }
        return nitems;
}

int
main(int argc, char **argv)
{
        if (argc == 2 && !strcmp(argv[1], "--test")) {
                test_run = true;
                run_bench();
                test_run = false;
        } else {
                run_bench();
        }
}
