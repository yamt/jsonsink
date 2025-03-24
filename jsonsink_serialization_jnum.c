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

/*
 * this file is an example to use alternative itoa/dtoa implementations,
 * namely LJSON jnum.c.
 *
 * note: LJSON jnum.c functions write out the terminating NUL to the buffer.
 * while we don't really need the NUL, we should reserve the extra byte for
 * it with jsonsink_add_serialized_value_reserve to avoid the overrun.
 * on the other hand, they return the number of bytes NOT including the
 * terminating NUL, which is suitable for jsonsink_add_serialized_value_commit.
 *
 * [LJSON]: https://github.com/lengjingzju/json
 */

#if defined(JSONSINK_ENABLE_ASSERTIONS)
#include <math.h>
#endif

#include "jnum.h"
#include "jsonsink.h"

void
jsonsink_add_uint32(struct jsonsink *s, uint32_t v)
{
        const size_t maxlen = sizeof("4294967295");
        char tmp[maxlen];
        void *dest = jsonsink_add_serialized_value_reserve(s, maxlen);
        int ret = jnum_ltoa(v, dest != NULL ? dest : tmp);
        JSONSINK_ASSUME(ret < maxlen);
        jsonsink_add_serialized_value_commit(s, ret);
}

void
jsonsink_add_int32(struct jsonsink *s, int32_t v)
{
        const size_t maxlen = sizeof("-2147483648");
        char tmp[maxlen];
        void *dest = jsonsink_add_serialized_value_reserve(s, maxlen);
        int ret = jnum_itoa(v, dest != NULL ? dest : tmp);
        JSONSINK_ASSUME(ret < maxlen);
        jsonsink_add_serialized_value_commit(s, ret);
}

void
jsonsink_add_double(struct jsonsink *s, double v)
{
        JSONSINK_ASSERT(!isnan(v));
        JSONSINK_ASSERT(!isinf(v));
        /*
         * the maximum length of the scientific notation of IEEE 754 double
         * is 23.
         */
        const size_t maxlen = 23 + 1;
        char tmp[maxlen];
        void *dest = jsonsink_add_serialized_value_reserve(s, maxlen);
        int ret = jnum_dtoa(v, dest != NULL ? dest : tmp);
        JSONSINK_ASSUME(ret < maxlen);
        jsonsink_add_serialized_value_commit(s, ret);
}
