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
#include "cJSON.h"

int
test_cjson(unsigned int n, const double *data_double, const uint32_t *data_u32)
{
        /* XXX error checks */
        unsigned int i;
        int ret = 0;
        cJSON *root = cJSON_CreateObject();
        cJSON *array = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "array", array);
        for (i = 0; i < n; i++) {
                cJSON *o = cJSON_CreateObject();
                cJSON *a = cJSON_CreateDoubleArray(data_double, 4);
                data_double += 4;
                cJSON_AddNumberToObject(o, "u32", (double)*data_u32++);
                cJSON_AddItemToObject(o, "double_array", a);
                cJSON_AddItemToArray(array, o);
        }
        char *p = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        size_t sz = strlen(p); /* XXX is there a more efficient way? */
        if (do_fwrite(p, 1, sz, stdout) != sz) {
                printf("fwrite error\n");
                ret = 1;
        }
        free(p);
        return ret;
}

void
run_bench(void)
{
        bench("cjson", test_cjson);
}
