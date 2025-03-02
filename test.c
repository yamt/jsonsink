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

#include "jsonsink.h"

int
main(int argc, char **argv)
{
        char buf[1000];
        struct jsonsink s;
        jsonsink_init(&s);
        jsonsink_set_buffer(&s, buf, sizeof(buf));
        jsonsink_object_start(&s);
        JSONSINK_ADD_LITERAL_KEY(&s, "key1");
        jsonsink_object_start(&s);
        JSONSINK_ADD_LITERAL_KEY(&s, "key1");
        jsonsink_add_serialized_value(&s, "100", 3);
        JSONSINK_ADD_LITERAL_KEY(&s, "key2");
        jsonsink_add_serialized_value(&s, "200", 3);
        JSONSINK_ADD_LITERAL_KEY(&s, "array1");
        jsonsink_array_start(&s);
        jsonsink_array_start(&s);
        JSONSINK_ADD_LITERAL(&s, "1");
        JSONSINK_ADD_LITERAL(&s, "1");
        JSONSINK_ADD_LITERAL(&s, "1");
        jsonsink_array_end(&s);
        jsonsink_array_start(&s);
        JSONSINK_ADD_LITERAL(&s, "2");
        JSONSINK_ADD_LITERAL(&s, "2");
        JSONSINK_ADD_LITERAL(&s, "2");
        jsonsink_add_double(&s, -1.2345);
        jsonsink_add_uint32(&s, 54321);
        jsonsink_add_int32(&s, -54321);
        jsonsink_array_end(&s);
        jsonsink_array_end(&s);
        jsonsink_object_end(&s);
        jsonsink_object_end(&s);
        int error = jsonsink_error(&s);
        void *p = jsonsink_pointer(&s);
        size_t sz = jsonsink_size(&s);
        jsonsink_clear(&s);
        if (error != 0) {
                printf("jsonsink error: %d\n", error);
                return 1;
        }
        fwrite(p, sz, 1, stdout);
}
