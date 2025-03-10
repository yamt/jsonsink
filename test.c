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

#include "jsonsink.h"

static size_t
do_fwrite(const void *p, size_t sz, size_t nitems, FILE *fp)
{
        // return nitems;
        return fwrite(p, sz, nitems, fp);
}

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
build(struct jsonsink *s)
{
        jsonsink_object_start(s);
        JSONSINK_ADD_LITERAL_KEY(s, "key1");
        jsonsink_object_start(s);
        JSONSINK_ADD_LITERAL_KEY(s, "key1");
        jsonsink_add_serialized_value(s, "100", 3);
        JSONSINK_ADD_LITERAL_KEY(s, "key2");
        jsonsink_add_serialized_value(s, "200", 3);
        JSONSINK_ADD_LITERAL_KEY(s, "array1");
        jsonsink_array_start(s);

        /*
         * test a few object/array nesting
         */
        jsonsink_array_start(s);  /* array in array (first) */
        jsonsink_object_start(s); /* object in array (first) */
        JSONSINK_ADD_LITERAL_KEY(s, "o");
        jsonsink_object_start(s); /* object in object (first) */
        jsonsink_object_end(s);
        JSONSINK_ADD_LITERAL_KEY(s, "a");
        jsonsink_array_start(s); /* array in object (non-first) */
        jsonsink_array_end(s);
        jsonsink_object_end(s);
        jsonsink_object_start(s); /* object in array (non-first) */
        JSONSINK_ADD_LITERAL_KEY(s, "a");
        jsonsink_array_start(s); /* array in object (first) */
        jsonsink_array_end(s);
        JSONSINK_ADD_LITERAL_KEY(s, "o");
        jsonsink_object_start(s); /* object in object (non-first) */
        jsonsink_object_end(s);
        jsonsink_object_end(s);
        jsonsink_array_end(s); /* array in array (non-first) */
        jsonsink_array_start(s);
        jsonsink_array_end(s);

        /*
         * test string escaping
         */
        jsonsink_add_string(s, JSONSINK_LITERAL("こんにちは, world"));
        jsonsink_add_string(s,
                            JSONSINK_LITERAL("nul \0 quote \" backslash \\"));
        /* https://util.unicode.org/UnicodeJsps/character.jsp?a=1F977 */
        jsonsink_add_string(s, JSONSINK_LITERAL("ninja \xf0\x9f\xa5\xb7"));

        uint32_t i;
        for (i = 0; i < 100; i++) {
                jsonsink_object_start(s);
                JSONSINK_ADD_LITERAL_KEY(s, "version");
                JSONSINK_ADD_LITERAL(s, "2");
                JSONSINK_ADD_LITERAL_KEY(s, "id");
                jsonsink_add_uint32(s, i);
                JSONSINK_ADD_LITERAL_KEY(s, "double");
                jsonsink_add_double(s, -1.2345);
                JSONSINK_ADD_LITERAL_KEY(s, "int32");
                jsonsink_add_int32(s, -54321);
                JSONSINK_ADD_LITERAL_KEY(s, "jsonsink_add_escaped_string");
                jsonsink_add_escaped_string(s, JSONSINK_LITERAL("foo"));
                JSONSINK_ADD_LITERAL_KEY(s, "jsonsink_add_serialized_value");
                jsonsink_add_serialized_value(s,
                                              JSONSINK_LITERAL_QUOTE("foo"));
                JSONSINK_ADD_LITERAL_KEY(s, "null");
                jsonsink_add_null(s);
                JSONSINK_ADD_LITERAL_KEY(s, "true");
                jsonsink_add_bool(s, true);
                JSONSINK_ADD_LITERAL_KEY(s, "false");
                jsonsink_add_bool(s, false);
                jsonsink_object_end(s);
        }
        jsonsink_array_end(s);
        jsonsink_object_end(s);
        jsonsink_object_end(s);
}

int
test_with_static_buffer(void)
{
        struct sink sink;
        char buf[64];
        struct jsonsink *s = &sink.s;
        jsonsink_init(s);
        jsonsink_set_buffer(s, buf, sizeof(buf));
        sink.fp = stdout;
        s->flush = flush;
        build(s);
        jsonsink_flush(s, 0);
        int error = jsonsink_error(s);
        if (error != 0) {
                printf("jsonsink error: %d\n", error);
                return 1;
        }
        return 0;
}

int
test_with_malloc(void)
{
        struct jsonsink s0;
        struct jsonsink *s = &s0;
        int ret = 0;
        jsonsink_init(s);
        build(s);
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
        build(s);
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
test_with_realloc(void)
{
        struct jsonsink s0;
        struct jsonsink *s = &s0;
        int ret = 0;
        jsonsink_init(s);
        s->flush = realloc_flush;
        build(s);
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

void
bench(const char *label, int (*fn)(void))
{
        clockid_t cid = CLOCK_MONOTONIC;
        struct timespec start;
        struct timespec end;
        unsigned int i;
        unsigned int n = 100000;
        int ret;
        ret = clock_gettime(cid, &start);
        if (ret != 0) {
                fprintf(stderr, "clock_gettime failed\n");
                exit(1);
        }
        for (i = 0; i < n; i++) {
                fn();
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

int
main(int argc, char **argv)
{
#if 0
        bench("test_with_static_buffer", test_with_static_buffer);
        bench("test_with_malloc", test_with_malloc);
        bench("test_with_realloc", test_with_realloc);
#endif
        test_with_static_buffer();
        // test_with_malloc();
        // test_with_realloc();
}
