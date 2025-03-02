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
write_char(struct jsonsink *s, char ch)
{
        char *dest = (char *)s->buf + s->bufpos;
        s->bufpos += 1;
        if (s->bufpos > s->buflen) {
                set_error(s, JSONSINK_ERROR_BUFFEROVERFLOW);
                return;
        }
        *(char *)dest = ch;
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
may_write_comma(struct jsonsink *s)
{
        if (s->need_comma[s->level]) {
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
jsonsink_error(struct jsonsink *s)
{
        return s->error;
}

void
jsonsink_set_error(struct jsonsink *s, int error)
{
        set_error(s, error);
}

void *
jsonsink_pointer(struct jsonsink *s)
{
        return s->buf;
}

size_t
jsonsink_size(struct jsonsink *s)
{
        return s->bufpos;
}

void
jsonsink_object_start(struct jsonsink *s)
{
        may_write_comma(s);
        write_char(s, '{');
        s->need_comma[s->level] = true;
        s->level++;
        s->need_comma[s->level] = false;
}

void
jsonsink_object_end(struct jsonsink *s)
{
        s->level--;
        write_char(s, '}');
}

void
jsonsink_add_serialized_key(struct jsonsink *s, const char *key, size_t keylen)
{
        may_write_comma(s);
        write_serialized(s, key, keylen);
        write_char(s, ':');
        s->need_comma[s->level] = false;
}

void
jsonsink_add_serialized_value(struct jsonsink *s, const char *value,
                              size_t valuelen)
{
        may_write_comma(s);
        write_serialized(s, value, valuelen);
        s->need_comma[s->level] = true;
}

void
jsonsink_array_start(struct jsonsink *s)
{
        may_write_comma(s);
        write_char(s, '[');
        s->need_comma[s->level] = true;
        s->level++;
        s->need_comma[s->level] = false;
}

void
jsonsink_array_end(struct jsonsink *s)
{
        s->level--;
        write_char(s, ']');
}
