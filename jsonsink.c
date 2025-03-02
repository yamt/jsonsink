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
#include <inttypes.h>
#include <string.h>

#include "jsonsink.h"

static void
set_error(struct jsonsink *s, int error)
{
        s->error = error;
}

static void
write_serialized(struct jsonsink *s, const void *value, size_t valuelen)
{
        char *dest = (char *)s->buf + s->bufpos;
        s->bufpos += valuelen;
        if (s->bufpos > s->buflen) {
                set_error(s, JSONSINK_ERROR_BUFFEROVERFLOW);
                return;
        }
        memcpy(dest, value, valuelen);
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
}

void
jsonsink_clear(struct jsonsink *s)
{
}

int
jsonsink_error(const struct jsonsink *s)
{
        return s->error;
}

void
jsonsink_set_error(struct jsonsink *s, int error)
{
        set_error(s, error);
}

void *
jsonsink_pointer(const struct jsonsink *s)
{
        return s->buf;
}

size_t
jsonsink_size(const struct jsonsink *s)
{
        return s->bufpos;
}

void
jsonsink_object_start(struct jsonsink *s)
{
        may_write_comma(s);
        write_char(s, '{');
        s->need_comma = false;
}

void
jsonsink_object_end(struct jsonsink *s)
{
        write_char(s, '}');
        s->need_comma = true;
}

void
jsonsink_add_serialized_key(struct jsonsink *s, const char *key, size_t keylen)
{
        may_write_comma(s);
        write_serialized(s, key, keylen);
        write_char(s, ':');
        s->need_comma = false;
}

void
jsonsink_add_serialized_value(struct jsonsink *s, const char *value,
                              size_t valuelen)
{
        may_write_comma(s);
        write_serialized(s, value, valuelen);
        s->need_comma = true;
}

void
jsonsink_array_start(struct jsonsink *s)
{
        may_write_comma(s);
        write_char(s, '[');
        s->need_comma = false;
}

void
jsonsink_array_end(struct jsonsink *s)
{
        write_char(s, ']');
        s->need_comma = true;
}
