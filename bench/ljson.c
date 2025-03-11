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
#include <string.h>

#include "bench.h"
#include "json.h"

int
test_ljson(unsigned int n, const double *data_double, const uint32_t *data_u32)
{
        /* XXX error checks */
        int ret = 0;
        json_sax_print_hd h = json_sax_print_unformat_start(0);
        json_sax_print_object(h, NULL, JSON_SAX_START);
        json_string_t key;
#define SET_STR(v, s)                                                         \
        v.str = s;                                                            \
        v.len = sizeof(s) - 1
        SET_STR(key, "array");
        json_sax_print_array(h, &key, JSON_SAX_START);
        uint32_t i;
        for (i = 0; i < n; i++) {
                json_sax_print_object(h, NULL, JSON_SAX_START);
                SET_STR(key, "u32");
                json_sax_print_lint(h, &key, *data_u32++);
                SET_STR(key, "double_array");
                json_sax_print_array(h, &key, JSON_SAX_START);
                json_sax_print_double(h, NULL, *data_double++);
                json_sax_print_double(h, NULL, *data_double++);
                json_sax_print_double(h, NULL, *data_double++);
                json_sax_print_double(h, NULL, *data_double++);
                json_sax_print_array(h, NULL, JSON_SAX_FINISH);
                json_sax_print_object(h, NULL, JSON_SAX_FINISH);
        }
        json_sax_print_array(h, NULL, JSON_SAX_FINISH);
        json_sax_print_object(h, NULL, JSON_SAX_FINISH);
        void *p;
        size_t sz;
        p = json_sax_print_finish(h, &sz);
        if (do_fwrite(p, 1, sz, stdout) != sz) {
                printf("fwrite error\n");
                ret = 1;
        }
        json_memory_free(p);
        return ret;
}

void
run_bench(void)
{
        bench("LJSON SAX", test_ljson);
}
