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
#include <stdio.h>

#include "bench.h"

int
test_snprintf(unsigned int n, const double *data_double,
              const uint32_t *data_u32)
{
        /* XXX error checks */
        char buf[4096]; /* XXX cheating */
        char *cp = buf;
        size_t rest = 4096;
        int ret;

        ret = snprintf(cp, rest, "{\"array\":[");
        cp += ret;
        rest -= ret;

        uint32_t i;
        const char *sep = "";
        for (i = 0; i < n; i++) {
                ret = snprintf(
                        cp, rest,
                        "%s{\"u32\":%" PRIu32
                        ",\"double_array\":[%1.17g,%1.17g,%1.17g,%1.17g]}",
                        sep, *data_u32++, data_double[0], data_double[1],
                        data_double[2], data_double[3]);
                data_double += 4;
                cp += ret;
                rest -= ret;
                sep = ",";
        }

        ret = snprintf(cp, rest, "]}");
        cp += ret;
        rest -= ret;

        void *p = buf;
        size_t sz = cp - buf;

        ret = 0;
        if (do_fwrite(p, 1, sz, stdout) != sz) {
                printf("fwrite error\n");
                ret = 1;
        }
        return ret;
}

void
run_bench(void)
{
        bench("snprintf", test_snprintf);
}
