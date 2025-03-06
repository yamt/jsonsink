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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "jsonsink.h"

struct surrogates {
        uint16_t high;
        uint16_t low;
};

static struct surrogates
calculate_sarrogates(uint32_t code)
{
        /*
         * https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G31699
         */
        JSONSINK_ASSERT(code >= 0x10000);
        uint16_t w = ((code >> 16) & 0x001f) - 1;
        uint16_t high = 0xd800 | (w << 6) | ((code & 0xffff) >> 10);
        uint16_t low = 0xdc00 | (code & 0x3ff);
        JSONSINK_ASSERT(0xd800 <= high && high <= 0xdbff);
        JSONSINK_ASSERT(0xdc00 <= low && low <= 0xdfff);
        return (struct surrogates){high, low};
}

void
jsonsink_add_string(struct jsonsink *s, const char *cp, size_t sz)
{
        /*
         * https://www.unicode.org/versions/Unicode16.0.0/core-spec/chapter-3/#G31703
         */
        const uint8_t *p = (const void *)cp;
        const uint8_t *ep = p + sz;
        JSONSINK_ASSERT(p <= ep);

        jsonsink_value_start(s);
        jsonsink_add_fragment(s, "\"", 1);
        while (p < ep) {
                /*
                 * decode a character
                 */
                uint8_t u8 = *p++;
                /* these bytes never appear in a valid utf-8 */
                JSONSINK_ASSERT(u8 != 0xc0 && u8 != 0xc1 && u8 < 0xf5);
                /* these bytes never appear at the beginning of a charater */
                JSONSINK_ASSERT(u8 < 0x80 || 0xbf < u8);
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
                        /* reject overlog encodings */
                        JSONSINK_ASSERT(0x80 <= code && code <= 0x7ff);
                } else if (u8 < 0xf0) {
                        /* 3 byte */
                        JSONSINK_ASSERT(p + 2 <= ep);
                        JSONSINK_ASSERT((u8 & 0xf0) == 0xe0);
                        JSONSINK_ASSERT((p[0] & 0xc0) == 0x80);
                        JSONSINK_ASSERT((p[1] & 0xc0) == 0x80);
                        code = ((u8 & 0xf) << 12) | ((p[0] & 0x3f) << 6) |
                               (p[1] & 0x3f);
                        /* reject overlog encodings */
                        JSONSINK_ASSERT(0x800 <= code && code <= 0xffff);
                        p += 2;
                } else {
                        /* 4 byte */
                        JSONSINK_ASSERT(p + 3 <= ep);
                        JSONSINK_ASSERT((u8 & 0xf8) == 0xf0);
                        JSONSINK_ASSERT((p[0] & 0xc0) == 0x80);
                        JSONSINK_ASSERT((p[1] & 0xc0) == 0x80);
                        JSONSINK_ASSERT((p[2] & 0xc0) == 0x80);
                        code = ((u8 & 0x3) << 18) | ((p[0] & 0x3f) << 12) |
                               ((p[1] & 0x3f) << 6) | ((p[2]) & 0x3f);
                        /* reject overlog encodings */
                        JSONSINK_ASSERT(0x10000 <= code && code <= 0x10ffff);
                        p += 3;
                }
                JSONSINK_ASSERT(code <= 0x10ffff);
                /* sarrogate halves should never appear in a utf-8 string */
                JSONSINK_ASSERT(code < 0xd800 || 0xe000 <= code);

                /*
                 * trasnmit the decoded character.
                 * escape if necessary.
                 */
                if (code >= 0x10000) {
                        /* extended character */
                        const size_t len = 12;
                        void *dest = jsonsink_reserve_buffer(s, len + 1);
                        if (dest != NULL) {
                                struct surrogates sarrogates =
                                        calculate_sarrogates(code);
                                int ret = snprintf(
                                        dest, len + 1,
                                        "\\u%04" PRIx16 "\\u%04" PRIx16,
                                        sarrogates.high, sarrogates.low);
                                JSONSINK_ASSERT(ret == len);
                        }
                        jsonsink_commit_buffer(s, len);
                } else if (code == 0x22) {
                        /* " */
                        jsonsink_add_fragment(s, (char[]){0x5c, 0x22}, 2);
                } else if (code == 0x5c) {
                        /* \ */
                        jsonsink_add_fragment(s, (char[]){0x5c, 0x5c}, 2);
                } else if (code <= 0x1f || code >= 0x7f) {
                        /* control character */
                        const size_t len = 6;
                        void *dest = jsonsink_reserve_buffer(s, len + 1);
                        if (dest != NULL) {
                                int ret = snprintf(dest, len + 1, "\\u%04x",
                                                   code);
                                JSONSINK_ASSERT(ret == len);
                        }
                        jsonsink_commit_buffer(s, len);
                } else {
                        /* transmit as it is */
                        uint8_t u8 = (uint8_t)code;
                        jsonsink_add_fragment(s, (const char *)&u8, 1);
                }
        }
        jsonsink_add_fragment(s, "\"", 1);
        jsonsink_value_end(s);
}
