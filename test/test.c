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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "jsonsink.h"

#define HUNDRED_CHARS                                                         \
        "0123456789"                                                          \
        "0123456789"                                                          \
        "0123456789"                                                          \
        "0123456789"                                                          \
        "0123456789"                                                          \
        "0123456789"                                                          \
        "0123456789"                                                          \
        "0123456789"                                                          \
        "0123456789"                                                          \
        "0123456789"

struct sink {
        struct jsonsink s;
        FILE *fp;
};

static bool
flush(struct jsonsink *s, size_t needed)
{
        assert(needed <= JSONSINK_MAX_RESERVATION);
        struct sink *sink = (void *)s;
        size_t nwritten = fwrite(s->buf, 1, s->bufpos, sink->fp);
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

        jsonsink_add_binary_base64(
                s, JSONSINK_LITERAL("nul \0 quote \" backslash \\"));
        jsonsink_add_binary_base64(s, HUNDRED_CHARS, 100);
        jsonsink_add_escaped_string(s, HUNDRED_CHARS, 100);
        jsonsink_add_serialized_value(s, "\"" HUNDRED_CHARS "\"", 100 + 2);

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
        jsonsink_add_serialized_key(s, "\"" HUNDRED_CHARS "\"", 100 + 2);
        jsonsink_value_start(s);
        jsonsink_add_fragment(s, "\"", 1);
        jsonsink_add_fragment(s, HUNDRED_CHARS, 100);
        jsonsink_add_fragment(s, HUNDRED_CHARS, 100);
        jsonsink_add_fragment(s, HUNDRED_CHARS, 100);
        jsonsink_add_fragment(s, "\"", 1);
        jsonsink_value_end(s);
        jsonsink_object_end(s);
}

int
test_with_static_buffer(void)
{
        struct sink sink;
        char buf[JSONSINK_MAX_RESERVATION];
        struct jsonsink *s = &sink.s;
        jsonsink_init(s);
        jsonsink_set_buffer(s, buf, sizeof(buf));
        sink.fp = stdout;
        s->flush = flush;
        build(s);
        jsonsink_flush(s, 0);
        int error = jsonsink_error(s);
        if (error != 0) {
                fprintf(stderr, "jsonsink error: %d\n", error);
                return 1;
        }
        return 0;
}

int
main(int argc, char **argv)
{
        test_with_static_buffer();
}
