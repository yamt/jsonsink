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

#if !defined(_JSONSINK_H)
#define _JSONSINK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(JSONSINK_MAX_RESERVE)
#define JSONSINK_MAX_RESERVATION 64 /* max reservation size in bytes */
#endif

#define JSONSINK_OK 0
#define JSONSINK_ERROR_NO_BUFFER_SPACE 1
#define JSONSINK_ERROR_FLUSH_FAILED 2
#define JSONSINK_ERROR_SERIALIZATION 3

struct jsonsink {
        /*
         *          buf -> +------------+ ^ ^
         *                 |            | | |
         *                 | generated  | | |
         *                 | JSON       | | |
         *                 | fragment   | | |
         *                 |            | | | bufpos
         * buf + bufpos -> +------------+ | v
         *                 |            | |
         *                 | garbage    | |
         *                 |            | |
         *                 |            | |
         *                 |            | | buflen
         * buf + buflen -> +------------+ v
         */

        void *buf;     /* buffer to write */
        size_t buflen; /* the size of the buffer */
        size_t bufpos; /* the next position to write in the buffer */

        /*
         * the `flush` callback is used to make a space in the buffer.
         *
         * it should make at least `needed` bytes available at `s->bufpos`.
         * it returns true on success.
         *
         * there are at least a few implemetation strategies:
         * - process the data in the buffer and empty it. (by `s->bufpos = 0`)
         * - extend the buffer. (eg. realloc)
         * - replace the buffer with a new one.
         *
         * 's->flush = NULL' is an equivalent of a callback which always fails.
         */
        bool (*flush)(struct jsonsink *s, size_t needed);

        /*
         * internal states. do not access them directly.
         */
        int error;
        bool need_comma;

#if defined(JSONSINK_ENABLE_ASSERTIONS)
        /*
         * internal states used for extra validations.
         *
         * see the comments in the "debug stuff" section below.
         */
        size_t reserved;
        unsigned int level;
        bool has_key;
#if !defined(JSONSINK_MAX_NEST)
#define JSONSINK_MAX_NEST 32
#endif
        bool is_obj[JSONSINK_MAX_NEST];
#endif /* defined(JSONSINK_ENABLE_ASSERTIONS) */
};

/**************************************************************************
 * core api
 *
 * note: the idea behind this api is to avoid hardcoding expensive
 * serialization logic in the core part of the library. some users might
 * prefer the slow but small implementation. (eg. system snprintf)
 * others might prefer highly-optimized implementations. it's better to
 * leave the choice to users.
 *
 * implementation: jsonsink.c
 **************************************************************************/

/*
 * initialization
 */

void jsonsink_init(struct jsonsink *s);
void jsonsink_set_buffer(struct jsonsink *s, void *buf, size_t buflen);

/*
 * jsonsink_flush: call the 's->flush' callback and record errors if any
 * so that the user can check it with jsonsink_error() later.
 */

bool jsonsink_flush(struct jsonsink *s, size_t needed);

/*
 * jsonsink_error: query the recorded error.
 *
 * when jsonsink_error() returns JSONSINK_ERROR_NO_BUFFER_SPACE,
 * a user can use jsonsink_size() to query the necessary buffer size.
 */

int jsonsink_error(const struct jsonsink *s);

/*
 * jsonsink_set_error: record an error.
 *
 * this is intended to be used by serialization logic
 * like jsonsink_add_double().
 */

void jsonsink_set_error(struct jsonsink *s, int error);

/*
 * jsonsink_pointer/jsonsink_size: these functions are intended
 * to be used by the api user to access the generated JSON.
 */

const void *jsonsink_pointer(const struct jsonsink *s);
size_t jsonsink_size(const struct jsonsink *s);

/*
 * the api to create JSON object and array.
 *
 * by default, the library doesn't validate if start/end are balanced well.
 * (unless JSONSINK_ENABLE_ASSERTIONS is enabled)
 * it's the user's responsibily to call them in a sane way.
 */

void jsonsink_object_start(struct jsonsink *s);
void jsonsink_object_end(struct jsonsink *s);

void jsonsink_array_start(struct jsonsink *s);
void jsonsink_array_end(struct jsonsink *s);

/*
 * the api to add null and bool values.
 */

void jsonsink_add_null(struct jsonsink *s);
void jsonsink_add_bool(struct jsonsink *s, bool v);

/*
 * reserve/commit api to avoid extra memcpy.
 *
 * see jsonsink_serialization.c for usage examples.
 */

void *jsonsink_add_serialized_value_reserve(struct jsonsink *s, size_t len);
void jsonsink_add_serialized_value_commit(struct jsonsink *s, size_t len);

/*
 * the low-level api to add a value in parts.
 *
 * eg.
 *   jsonsink_value_start(s);
 *   jsonsink_add_fragment(s, "first", 5);
 *   p = jsonsink_reserve_buffer(s, 6);
 *   if (p != NULL) {
 *      memcpy(p, "second", 6);
 *   }
 *   jsonsink_commit_buffer(s, 6);
 *   jsonsink_value_end(s);
 *
 * is an equivalent of
 *
 *   jsonsink_add_serialized_value(s, "first" "second", 5 + 6);
 */

void jsonsink_value_start(struct jsonsink *s);
void jsonsink_value_end(struct jsonsink *s);

/*
 * the low-level api to add raw fragments as they are
 * without affecting the other library states.
 *
 * a fragment here just means a raw byte array.
 * the library doesn't interpret the contents of fragments.
 * it can be used to compose a JSON value with multiple fragments for example.
 */

void *jsonsink_reserve_buffer(struct jsonsink *s, size_t len);
void jsonsink_commit_buffer(struct jsonsink *s, size_t len);
void jsonsink_add_fragment(struct jsonsink *s, const char *frag, size_t len);

/*
 * the api to deal with serialized key/value.
 *
 * these functions add a value (or key) which is already serialized.
 *
 * "already serialized" here means it's just memcpy-able into
 * the buffer to be a part of a serialized JSON.
 * for example, a string should be escaped if necessary and quoted
 * with double quotation marks.
 * the library blindly uses the given serialized value as it is
 * without any validations. it's the user's responsibility to pass
 * a sane value.
 *
 * jsonsink_add_serialized_value is basically an equivalent of
 *   jsonsink_add_serialized_value_reserve
 *   + memcpy
 *   + jsonsink_add_serialized_value_commit.
 *
 * jsonsink_add_escaped_string only quotes the given string.
 * it's the user's responsibily to pass a string which doesn't
 * need further escaping. that is, it doesn't contain '"', '\\', '\0',
 * or control characters.
 */

void jsonsink_add_serialized_key(struct jsonsink *s, const char *key,
                                 size_t keylen);
void jsonsink_add_serialized_value(struct jsonsink *s, const char *value,
                                   size_t valuelen);
void jsonsink_add_escaped_string(struct jsonsink *s, const char *value,
                                 size_t valuelen);

/*
 * convenience macros to use C literals.
 */

#define JSONSINK_LITERAL_QUOTE(cstr) "\"" cstr "\"", sizeof(cstr) + 1
#define JSONSINK_LITERAL(cstr) cstr, sizeof(cstr) - 1
#define JSONSINK_ADD_LITERAL_KEY(s, l)                                        \
        jsonsink_add_serialized_key(s, JSONSINK_LITERAL_QUOTE(l))
#define JSONSINK_ADD_LITERAL(s, l)                                            \
        jsonsink_add_serialized_value(s, JSONSINK_LITERAL(l))
#define JSONSINK_ADD_LITERAL_STRING(s, l)                                     \
        jsonsink_add_serialized_value(s, JSONSINK_LITERAL_QUOTE(l))

/**************************************************************************
 * serialization utility api
 *
 * implementation: jsonsink_serialization.c
 * alternative implementation: jsonsink_serialization_jnum.c
 **************************************************************************/

/*
 * serialize-and-add style functions.
 *
 * the default implementations of these functions found in
 * jsonsink_serialization.c just uses snprintf to convert values
 * to strings.
 *
 * if the performance of the string conversion is important, you might
 * want to use an optimized implemontation instead of snprintf.
 * cf.
 * - https://github.com/miloyip/dtoa-benchmark
 * - https://github.com/miloyip/itoa-benchmark
 *
 * jsonsink_serialization_jnum.c has an alternative implementation which uses
 * itoa/dtoa functions from LJSON jnum.c, which is usually far faster
 * than snprintf.
 *
 * the advantage of the default snprintf implementation is the code size.
 * after all, snprintf is likely already used by the other part of the
 * system.
 * on the other hand, many of highly-optimized dtoa/itoa implementations
 * involve lookup tables, which might be a burden for very small systems.
 *
 * jsonsink_add_double doesn't support nan or inf. it's the user's
 * responsibility to avoid passing them. note that JSON itself doesn't
 * have nan or inf.
 *
 * [LJSON]: https://github.com/lengjingzju/json
 */

void jsonsink_add_uint32(struct jsonsink *s, uint32_t v);
void jsonsink_add_int32(struct jsonsink *s, int32_t v);
void jsonsink_add_double(struct jsonsink *s, double v);

/**************************************************************************
 * utf-8 and string escaping
 *
 * implementation: jsonsink_escape.c
 **************************************************************************/

/*
 * jsonsink_add_string: add a utf-8 string value.
 *
 * this function performs necessary escaping.
 *
 * the input should be a valid utf-8 sequence.
 *   - it should not contain surrogate halves.
 *   - it CAN contain NUL characters. (\u0000)
 * it's users' responsibility to pass a valid utf-8 string.
 * the library doesn't perform any validations.
 *
 * the current implementation is straightforward and slow.
 */

void jsonsink_add_string(struct jsonsink *s, const char *cp, size_t sz);

/**************************************************************************
 * base64
 *
 * implementation: jsonsink_base64.c
 **************************************************************************/

/*
 * jsonsink_add_binary_base64: add a byte array as a JSON string with base64
 * encoding
 */

void jsonsink_add_binary_base64(struct jsonsink *s, const void *p, size_t sz);

/**************************************************************************
 * debug stuff
 **************************************************************************/

#if !defined(__has_builtin)
#define __has_builtin(a) 0
#endif

#if !__has_builtin(__builtin_assume)
#define __builtin_assume(cond)
#endif

/*
 * JSONSINK_ENABLE_ASSERTIONS enables extra sanity checks in the library.
 * the main purpose is to detect typical api usage errors like
 * jsonsink_object_start/jsonsink_object_end mismatches.
 * some of these checks involve extra memory overhead. (see
 * struct jsonsink)
 *
 * jsonsink_check is expected to be called at the end of a json producer
 * logic to ensure the completeness of the json object. it would detect
 * errors like missing the last jsonsink_object_end call.
 *
 * Note: JSONSINK_ENABLE_ASSERTIONS (and the associated constant
 * JSONSINK_MAX_NEST) changes the ABI of this library. please build everything
 * with or without JSONSINK_ENABLE_ASSERTIONS consistently.
 */
#if defined(JSONSINK_ENABLE_ASSERTIONS)
#include <stdio.h>
#define JSONSINK_ASSERT(cond)                                                 \
        do {                                                                  \
                if (!(cond)) {                                                \
                        fprintf(stderr, "assertion (%s) failed at %s:%d\n",   \
                                #cond, __FILE__, __LINE__);                   \
                        __builtin_trap();                                     \
                }                                                             \
        } while (0)
#define JSONSINK_ASSUME(cond) JSONSINK_ASSERT(cond)
void jsonsink_check(const struct jsonsink *s);
#else /* defined(JSONSINK_ENABLE_ASSERTIONS) */
#define JSONSINK_ASSERT(cond)
#define JSONSINK_ASSUME(cond) __builtin_assume(cond)
#define jsonsink_check(s)
#endif /* defined(JSONSINK_ENABLE_ASSERTIONS) */

#if defined(__cplusplus)
}
#endif

#endif /* !defined(_JSONSINK_H) */
