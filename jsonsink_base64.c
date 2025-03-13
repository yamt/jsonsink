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
#include <stdio.h>
#include <string.h>

#include "jsonsink.h"

#if !defined(__has_builtin)
#define __has_builtin(a) 0
#endif

#if !__has_builtin(__builtin_assume)
#define __builtin_assume(cond)
#endif

#if !defined(LITTLE_ENDIAN)
#if defined(__LITTLE_ENDIAN__) || (defined(__BYTE_ORDER__) &&                 \
                                   __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define LITTLE_ENDIAN 1
#endif
#endif

#if !defined(LITTLE_ENDIAN)
#if defined(__BIG_ENDIAN__) ||                                                \
        (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define LITTLE_ENDIAN 0
#endif
#endif

#if !defined(LITTLE_ENDIAN)
#error endian is not known
#endif

#define BASE64_ASSERT(cond) JSONSINK_ASSERT(cond)

static uint8_t
conv_to_char(uint8_t x)
{
        BASE64_ASSERT(x < 64);
        static const char table[64] = {
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
                'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
                'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
                'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2',
                '3', '4', '5', '6', '7', '8', '9', '+', '/',
        };
        return table[x];
}

static uint32_t
loadbe(const uint8_t p[3])
{
        return ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2];
}

static uint32_t
expand(uint32_t x)
{
        return ((x << 6) & 0x3f000000) | ((x << 4) & 0x003f0000) |
               ((x << 2) & 0x00003f00) | (x & 0x0000003f);
}

static uint32_t
byteswap(uint32_t x)
{
        return ((x << 24) & 0x3f000000) | ((x << 8) & 0x003f0000) |
               ((x >> 8) & 0x00003f00) | ((x >> 24) & 0x0000003f);
}

static uint32_t
convert(uint32_t x)
{
        union {
                uint32_t u32;
                uint8_t u8[4];
        } u;
        u.u32 = x;
        unsigned int j;
        for (j = 0; j < 4; j++) {
                u.u8[j] = conv_to_char(u.u8[j]);
        }
        return u.u32;
}

static uint32_t
pad(uint32_t x, unsigned int srclen)
{
        BASE64_ASSERT(srclen > 0 && srclen <= 3);
        union {
                uint32_t u32;
                uint8_t u8[4];
        } u;
        u.u32 = x;
        if (srclen < 3) {
                if (srclen == 1) {
                        u.u8[2] = '=';
                }
                u.u8[3] = '=';
        }
        return u.u32;
}

static void
enc3(const uint8_t p[3], char dst[4], unsigned int srclen)
{
        BASE64_ASSERT(srclen > 0 && srclen <= 3);

        uint32_t x = loadbe(p);
        x = expand(x);
#if LITTLE_ENDIAN
        x = byteswap(x);
#endif
        x = convert(x);
        x = pad(x, srclen);
        memcpy(dst, &x, 4);
}

static size_t
base64size(size_t srclen)
{
        size_t bsz = (srclen + 2) / 3 * 4;
        BASE64_ASSERT(srclen / 3 == bsz / 4 || srclen / 3 + 1 == bsz / 4);
        return bsz;
}

static void
base64encode(const void *restrict src, size_t srclen, char *restrict dst)
{
        const uint8_t *p = src;
        size_t n = srclen / 3;
        size_t i;

        for (i = 0; i < n; i++) {
                enc3(p, dst, 3);
                p += 3;
                dst += 4;
        }
        size_t tail = srclen - n * 3;
        BASE64_ASSERT(0 <= tail && tail < 3);
        if (tail > 0) {
                uint8_t tmp[3];
                memset(tmp, 0, sizeof(tmp));
                memcpy(tmp, p, tail);
                enc3(tmp, dst, tail);
        }
}

void
jsonsink_add_binary_base64(struct jsonsink *s, const void *p, size_t sz)
{
        const uint8_t *cp = p;
        const uint8_t *ep = cp + sz;
        JSONSINK_ASSERT(cp <= ep);
        const size_t maxchunksize = JSONSINK_MAX_RESERVATION / 4 * 3;

        /*
         * REVISIT: maybe we can add extra spaces here to make the
         * base64 output buffer better aligned.
         */
        jsonsink_value_start(s);
        jsonsink_add_fragment(s, "\"", 1);
        while (cp < ep) {
                size_t len = ep - cp;
                if (len > maxchunksize) {
                        len = maxchunksize;
                }
                size_t bsz = base64size(len);
                JSONSINK_ASSERT(bsz <= JSONSINK_MAX_RESERVATION);
                void *dest = jsonsink_reserve_buffer(s, bsz);
                if (dest != NULL) {
                        base64encode(cp, len, dest);
                }
                jsonsink_commit_buffer(s, bsz);
                cp += len;
        }
        jsonsink_add_fragment(s, "\"", 1);
        jsonsink_value_end(s);
}
