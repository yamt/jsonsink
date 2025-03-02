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

#include "bench.h"
#include "jsonsink.h"

struct sink {
        struct jsonsink s;
        FILE *fp;
};

static bool
flush(struct jsonsink *s, size_t needed)
{
        struct sink *sink = (void *)s;
        size_t nwritten = do_fwrite(s->buf, 1, s->bufpos, sink->fp);
        if (nwritten != s->bufpos) {
                return false;
        }
        s->bufpos = 0;
        return true;
}

static void
build(struct jsonsink *s, unsigned int n, const double *data_double,
      const uint32_t *data_u32)
{
        jsonsink_object_start(s);
        JSONSINK_ADD_LITERAL_KEY(s, "array");
        jsonsink_array_start(s);
        uint32_t i;
        for (i = 0; i < n; i++) {
                jsonsink_object_start(s);
                JSONSINK_ADD_LITERAL_KEY(s, "u32");
                jsonsink_add_uint32(s, *data_u32++);
                JSONSINK_ADD_LITERAL_KEY(s, "double_array");
                jsonsink_array_start(s);
                jsonsink_add_double(s, *data_double++);
                jsonsink_add_double(s, *data_double++);
                jsonsink_add_double(s, *data_double++);
                jsonsink_add_double(s, *data_double++);
                jsonsink_array_end(s);
                jsonsink_object_end(s);
        }
        jsonsink_array_end(s);
        jsonsink_object_end(s);
}

int
test_with_static_buffer(unsigned int n, const double *data_double,
                        const uint32_t *data_u32)
{
        struct sink sink;
        char buf[64];
        struct jsonsink *s = &sink.s;
        jsonsink_init(s);
        jsonsink_set_buffer(s, buf, sizeof(buf));
        sink.fp = stdout;
        s->flush = flush;
        build(s, n, data_double, data_u32);
        jsonsink_flush(s, 0);
        int error = jsonsink_error(s);
        if (error != 0) {
                printf("jsonsink error: %d\n", error);
                return 1;
        }
        return 0;
}

int
test_with_malloc(unsigned int n, const double *data_double,
                 const uint32_t *data_u32)
{
        struct jsonsink s0;
        struct jsonsink *s = &s0;
        int ret = 0;
        jsonsink_init(s);
        build(s, n, data_double, data_u32);
        int error = jsonsink_error(s);
        if (error != JSONSINK_ERROR_NO_BUFFER_SPACE) {
                printf("jsonsink error: %d\n", error);
                return 1;
        }
        size_t sz = jsonsink_size(s);
        void *buf = malloc(sz);
        if (buf == NULL) {
                printf("malloc failure\n");
                return 1;
        }
        jsonsink_init(s);
        jsonsink_set_buffer(s, buf, sz);
        build(s, n, data_double, data_u32);
        error = jsonsink_error(s);
        if (error != 0) {
                printf("jsonsink error: %d\n", error);
                ret = 1;
                goto out;
        }
        if (sz != jsonsink_size(s)) {
                printf("unexpected size: %zu != %zu\n", sz, jsonsink_size(s));
                ret = 1;
                goto out;
        }
        if (do_fwrite(buf, 1, sz, stdout) != sz) {
                printf("fwrite error\n");
                ret = 1;
                goto out;
        }
out:
        free(buf);
        return ret;
}

static bool
realloc_flush(struct jsonsink *s, size_t needed)
{
        size_t newsize = s->bufpos + needed;
        void *p = realloc(s->buf, newsize);
        if (p == NULL) {
                return false;
        }
        s->buf = p;
        s->buflen = newsize;
        return true;
}

int
test_with_realloc(unsigned int n, const double *data_double,
                  const uint32_t *data_u32)
{
        struct jsonsink s0;
        struct jsonsink *s = &s0;
        int ret = 0;
        jsonsink_init(s);
        s->flush = realloc_flush;
        build(s, n, data_double, data_u32);
        int error = jsonsink_error(s);
        if (error != 0) {
                printf("jsonsink error: %d\n", error);
                ret = 1;
                goto out;
        }
        if (do_fwrite(s->buf, 1, s->bufpos, stdout) != s->bufpos) {
                printf("fwrite error\n");
                ret = 1;
                goto out;
        }
out:
        free(s->buf);
        return ret;
}

int
main(int argc, char **argv)
{
        test_run = true;
        bench("test_with_static_buffer", test_with_static_buffer);
        printf("\n");
        test_run = false;
        bench("test_with_static_buffer", test_with_static_buffer);
        bench("test_with_malloc", test_with_malloc);
        bench("test_with_realloc", test_with_realloc);
}
