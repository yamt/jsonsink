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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "rng.h"

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
                *p++ = (double)rng_rand_u32(rng);
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
        const unsigned int n = 100000;
        int ret;

        const unsigned int ndata = 32;
        uint32_t data_u32[ndata];
        double data_double[ndata * 4];
        rng_init(&rng, 0x12345678);
        random_u32(&rng, ndata, data_u32);
        random_double(&rng, ndata * 4, data_double);

        ret = clock_gettime(cid, &start);
        if (ret != 0) {
                fprintf(stderr, "clock_gettime failed\n");
                exit(1);
        }
        for (i = 0; i < n; i++) {
                fn(ndata, data_double, data_u32);
        }
        ret = clock_gettime(cid, &end);
        if (ret != 0) {
                fprintf(stderr, "clock_gettime failed\n");
                exit(1);
        }
        double start_sec = start.tv_sec * 1.0 + start.tv_nsec / 1000000000.0;
        double end_sec = end.tv_sec * 1.0 + end.tv_nsec / 1000000000.0;
        double cps = n / (end_sec - start_sec);
        printf("%s: %g\n", label, cps);
}

size_t
do_fwrite(const void *p, size_t sz, size_t nitems, FILE *fp)
{
        return nitems;
        // return fwrite(p, sz, nitems, fp);
}
