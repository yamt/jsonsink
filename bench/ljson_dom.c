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
        json_mem_t mem;
        pjson_memory_init(&mem);
        json_object *object = pjson_create_object(&mem);
        json_string_t key;
#define SET_STR(v, s)                                                         \
        v.str = s;                                                            \
        v.len = sizeof(s) - 1
        SET_STR(key, "array");
        json_object *array = pjson_add_array_to_object(object, &key, &mem);
        uint32_t i;
        for (i = 0; i < n; i++) {
                json_object *o = pjson_add_object_to_array(array, &mem);
                SET_STR(key, "u32");
                pjson_add_lint_to_object(o, &key, *data_u32++, &mem);
                SET_STR(key, "double_array");
                json_object *double_array = pjson_create_double_array(
                        (void *)data_double, 4, &mem);
                data_double += 4;
                pjson_set_key(double_array, &key, &mem);
                json_add_item_to_object(o, double_array);
        }
        size_t sz;
        void *p = json_print_unformat(object, 0, &sz);
        pjson_memory_free(&mem);
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
        bench("LJSON DOM", test_ljson);
}
