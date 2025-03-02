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
#include "parson.h"

int
test_parson(unsigned int n, const double *data_double,
            const uint32_t *data_u32)
{
        /* XXX error checks */
        unsigned int i;
        int ret = 0;
        JSON_Value *root_value = json_value_init_object();
        JSON_Value *array_value = json_value_init_array();
        JSON_Array *array = json_value_get_array(array_value);
        JSON_Object *root = json_value_get_object(root_value);
        json_object_set_value(root, "array", array_value);
        for (i = 0; i < n; i++) {
                JSON_Value *av = json_value_init_array();
                JSON_Array *a = json_value_get_array(av);
                json_array_append_number(a, *data_double++);
                json_array_append_number(a, *data_double++);
                json_array_append_number(a, *data_double++);
                json_array_append_number(a, *data_double++);
                JSON_Value *ov = json_value_init_object();
                JSON_Object *o = json_value_get_object(ov);
                json_object_set_number(o, "u32", (double)*data_u32++);
                json_object_set_value(o, "double_array", av);
                json_array_append_value(array, ov);
        }
        char *p = json_serialize_to_string(root_value);
        size_t sz = strlen(p); /* XXX is there a more efficient way? */
        if (do_fwrite(p, 1, sz, stdout) != sz) {
                printf("fwrite error\n");
                ret = 1;
        }
        json_value_free(root_value);
        json_free_serialized_string(p);
        return ret;
}

int
main(int argc, char **argv)
{
        test_run = true;
        bench("test_parson", test_parson);
        printf("\n");
        test_run = false;
        bench("test_parson", test_parson);
}
