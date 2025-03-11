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

#include <string.h>

#include "jsonsink.h"

static void
set_error(struct jsonsink *s, int error)
{
        /*
         * this function is not expected to be used to clear errors.
         *
         * this function is not expected to be used to set
         * JSONSINK_ERROR_NO_BUFFER_SPACE. it's handled by jsonsink_error().
         */
        JSONSINK_ASSERT(error != JSONSINK_OK);
        JSONSINK_ASSERT(error != JSONSINK_ERROR_NO_BUFFER_SPACE);
        s->error = error;
}

static void *
reserve_buffer(struct jsonsink *s, size_t len)
{
        JSONSINK_ASSERT(s->reserved == 0);
#if defined(JSONSINK_ENABLE_ASSERTIONS)
        s->reserved = len;
#endif
        size_t newbufpos = s->bufpos + len;
        if (newbufpos > s->buflen) {
                if (s->flush != NULL) {
                        if (!jsonsink_flush(s, len)) {
                                return NULL;
                        }
                } else {
                        return NULL;
                }
        }
        return (char *)s->buf + s->bufpos;
}

static void
commit_buffer(struct jsonsink *s, size_t len)
{
        JSONSINK_ASSERT(len <= s->reserved);
        s->bufpos += len;
#if defined(JSONSINK_ENABLE_ASSERTIONS)
        s->reserved = 0;
#endif
}

static void
write_serialized(struct jsonsink *s, const void *value, size_t len)
{
        void *dest = reserve_buffer(s, len);
        if (dest != NULL) {
                memcpy(dest, value, len);
        }
        commit_buffer(s, len);
}

static void
write_char(struct jsonsink *s, char ch)
{
        write_serialized(s, &ch, 1);
}

static void
may_write_comma(struct jsonsink *s)
{
        if (s->need_comma) {
                write_char(s, ',');
        }
}

static void
value_start(struct jsonsink *s)
{
        JSONSINK_ASSERT((s->level > 0 && s->is_obj[s->level - 1]) ==
                        s->has_key);
        may_write_comma(s);
}

static void
value_end(struct jsonsink *s)
{
        s->need_comma = true;
#if defined(JSONSINK_ENABLE_ASSERTIONS)
        s->has_key = false;
#endif
}

void
jsonsink_init(struct jsonsink *s)
{
        memset(s, 0, sizeof(*s));
}

void
jsonsink_set_buffer(struct jsonsink *s, void *buf, size_t buflen)
{
        s->buf = buf;
        s->buflen = buflen;
        s->bufpos = 0;
}

bool
jsonsink_flush(struct jsonsink *s, size_t needed)
{
        if (s->flush == NULL || !s->flush(s, needed)) {
                set_error(s, JSONSINK_ERROR_FLUSH_FAILED);
                return false;
        }
        JSONSINK_ASSERT(s->bufpos + needed <= s->buflen);
        return true;
}

int
jsonsink_error(const struct jsonsink *s)
{
        if (s->error != JSONSINK_OK) {
                return s->error;
        }
        if (s->bufpos > s->buflen) {
                return JSONSINK_ERROR_NO_BUFFER_SPACE;
        }
        return JSONSINK_OK;
}

void
jsonsink_set_error(struct jsonsink *s, int error)
{
        set_error(s, error);
}

const void *
jsonsink_pointer(const struct jsonsink *s)
{
        JSONSINK_ASSERT(s->error == JSONSINK_OK);
        return s->buf;
}

size_t
jsonsink_size(const struct jsonsink *s)
{
        JSONSINK_ASSERT(s->error == JSONSINK_OK);
        return s->bufpos;
}

void
jsonsink_object_start(struct jsonsink *s)
{
        value_start(s);
#if defined(JSONSINK_ENABLE_ASSERTIONS)
        s->is_obj[s->level] = true;
        JSONSINK_ASSERT(++s->level > 0);
        JSONSINK_ASSERT(s->level <= JSONSINK_MAX_NEST);
        s->has_key = false;
#endif
        write_char(s, '{');
        s->need_comma = false;
}

void
jsonsink_object_end(struct jsonsink *s)
{
        JSONSINK_ASSERT(s->level <= JSONSINK_MAX_NEST);
        JSONSINK_ASSERT(s->level-- > 0);
        JSONSINK_ASSERT(s->is_obj[s->level]);
        write_char(s, '}');
        value_end(s);
}

void
jsonsink_array_start(struct jsonsink *s)
{
        value_start(s);
#if defined(JSONSINK_ENABLE_ASSERTIONS)
        s->is_obj[s->level] = false;
        JSONSINK_ASSERT(++s->level > 0);
        JSONSINK_ASSERT(s->level <= JSONSINK_MAX_NEST);
        s->has_key = false;
#endif
        write_char(s, '[');
        s->need_comma = false;
}

void
jsonsink_array_end(struct jsonsink *s)
{
        JSONSINK_ASSERT(s->level <= JSONSINK_MAX_NEST);
        JSONSINK_ASSERT(s->level-- > 0);
        JSONSINK_ASSERT(!s->is_obj[s->level]);
        write_char(s, ']');
        value_end(s);
}

#if defined(JSONSINK_ENABLE_ASSERTIONS)
void
jsonsink_check(const struct jsonsink *s)
{
        JSONSINK_ASSERT(s->level == 0);
}
#endif

void
jsonsink_add_null(struct jsonsink *s)
{
        jsonsink_add_serialized_value(s, JSONSINK_LITERAL("null"));
}

void
jsonsink_add_bool(struct jsonsink *s, bool v)
{
        if (v) {
                jsonsink_add_serialized_value(s, JSONSINK_LITERAL("true"));
        } else {
                jsonsink_add_serialized_value(s, JSONSINK_LITERAL("false"));
        }
}

void *
jsonsink_add_serialized_value_reserve(struct jsonsink *s, size_t len)
{
        value_start(s);
        return reserve_buffer(s, len);
}

void
jsonsink_add_serialized_value_commit(struct jsonsink *s, size_t len)
{
        commit_buffer(s, len);
        value_end(s);
}

void
jsonsink_value_start(struct jsonsink *s)
{
        value_start(s);
}

void
jsonsink_value_end(struct jsonsink *s)
{
        value_end(s);
}

void *
jsonsink_reserve_buffer(struct jsonsink *s, size_t len)
{
        return reserve_buffer(s, len);
}

void
jsonsink_commit_buffer(struct jsonsink *s, size_t len)
{
        commit_buffer(s, len);
}

void
jsonsink_add_fragment(struct jsonsink *s, const char *frag, size_t len)
{
        write_serialized(s, frag, len);
}

void
jsonsink_add_serialized_key(struct jsonsink *s, const char *key, size_t keylen)
{
        JSONSINK_ASSERT(s->level > 0 && s->is_obj[s->level - 1]);
        JSONSINK_ASSERT(!s->has_key);
        may_write_comma(s);
        write_serialized(s, key, keylen);
        write_char(s, ':');
        s->need_comma = false;
#if defined(JSONSINK_ENABLE_ASSERTIONS)
        s->has_key = true;
#endif
}

void
jsonsink_add_serialized_value(struct jsonsink *s, const char *value,
                              size_t valuelen)
{
        value_start(s);
        write_serialized(s, value, valuelen);
        value_end(s);
}

void
jsonsink_add_escaped_string(struct jsonsink *s, const char *value,
                            size_t valuelen)
{
        value_start(s);
        write_char(s, '"');
        write_serialized(s, value, valuelen);
        write_char(s, '"');
        value_end(s);
}
