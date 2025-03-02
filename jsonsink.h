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

#if defined(__cplusplus)
extern "C" {
#endif

#define JSONSINK_OK 0
#define JSONSINK_ERROR_BUFFEROVERFLOW 1
#define JSONSINK_ERROR_SERIALIZATION 2

struct jsonsink {
        void *buf;
        size_t buflen;
        size_t bufpos;
        int error;
        bool need_comma;
};

/*
 * core api
 */

void jsonsink_init(struct jsonsink *s);
void jsonsink_set_buffer(struct jsonsink *s, void *buf, size_t buflen);
void jsonsink_clear(struct jsonsink *s);
int jsonsink_error(const struct jsonsink *s);
void jsonsink_set_error(struct jsonsink *s, int error);
void *jsonsink_pointer(const struct jsonsink *s);
size_t jsonsink_size(const struct jsonsink *s);

/*
 * the api to create JSON object and array.
 *
 * the library doesn't validate if start/end are balanced well.
 * it's the user's responsibily to call them in a sane way.
 */

void jsonsink_object_start(struct jsonsink *s);
void jsonsink_object_end(struct jsonsink *s);

void jsonsink_array_start(struct jsonsink *s);
void jsonsink_array_end(struct jsonsink *s);

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
 * jsonsink_add_escaped_string only quotes the given string.
 * it's the user's responsibily to pass the string which doesn't
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
 * serialize-and-add style functions.
 *
 * the current implementations of these functions are just
 * snprintf + jsonsink_add_serialized_value.
 *
 * if you care performance of the string conversion, you might
 * want to use something like https://github.com/koendv/FloatToAscii
 * instead of snprintf and then add the result with
 * jsonsink_add_serialized_value.
 */

void jsonsink_add_uint32(struct jsonsink *s, uint32_t v);
void jsonsink_add_int32(struct jsonsink *s, int32_t v);
void jsonsink_add_double(struct jsonsink *s, double v);

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

#if defined(__cplusplus)
}
#endif

#endif /* !defined(_JSONSINK_H) */
