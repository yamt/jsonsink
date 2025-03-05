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

#include "jsonsink.h"

void
jsonsink_add_string(struct jsonsink *s, const char *cp, size_t sz)
{
        const uint8_t *p = (const void *)cp;
        const uint8_t *ep = p + sz;
        JSONSINK_ASSERT(p <= ep);

        jsonsink_add_serialized_value(s, "\"", 1);
        s->need_comma = false;
        while (p < ep) {
                uint8_t u8 = *p++;
                uint32_t code;
                if (u8 < 0x80) {
                        /* 1 byte */
                        code = u8 & 0x7f;
                } else if (u8 < 0xe0) {
                        /* 2 byte */
                        JSONSINK_ASSERT(p + 1 <= ep);
                        JSONSINK_ASSERT((u8 & 0xe0) == 0xc0);
                        JSONSINK_ASSERT((u8 & 0xe0) == 0xc0);
                        JSONSINK_ASSERT((p[0] & 0xc0) == 0x80);
                        code = ((u8 & 0x1f) << 6) | ((*p++) & 0x3f);
                } else if (u8 < 0xf0) {
                        /* 3 byte */
                        JSONSINK_ASSERT(p + 2 <= ep);
                        JSONSINK_ASSERT((u8 & 0xf0) == 0xe0);
                        JSONSINK_ASSERT((p[0] & 0xc0) == 0x80);
                        JSONSINK_ASSERT((p[1] & 0xc0) == 0x80);
                        code = ((u8 & 0xf) << 12) | ((p[0] & 0x3f) << 6) |
                               (p[1] & 0x3f);
                        p += 2;
                } else {
                        /* 4 byte */
                        JSONSINK_ASSERT(p + 3 <= ep);
                        JSONSINK_ASSERT((u8 & 0xf0) == 0xe0);
                        JSONSINK_ASSERT((p[0] & 0xc0) == 0x80);
                        JSONSINK_ASSERT((p[1] & 0xc0) == 0x80);
                        JSONSINK_ASSERT((p[2] & 0xc0) == 0x80);
                        code = ((u8 & 0x3) << 18) | ((p[0] & 0x3f) << 12) |
                               ((p[1] & 0x3f) << 6) | ((p[2]) & 0x3f);
                }
                if (code <= 0x1f || code == 0x22 || code == 0x5c ||
                    code >= 0x7f) {
                        const size_t len = 6;
                        void *dest = jsonsink_add_serialized_value_reserve(
                                s, len + 1);
                        if (dest != NULL) {
                                int ret = snprintf(dest, len + 1, "\\u%04x",
                                                   code);
                                JSONSINK_ASSERT(ret == len);
                        }
                        jsonsink_add_serialized_value_commit(s, len);
                } else {
                        uint8_t u8 = (uint8_t)code;
                        jsonsink_add_serialized_value(s, (const char *)&u8, 1);
                }
                s->need_comma = false;
        }
        jsonsink_add_serialized_value(s, "\"", 1);
}
