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
#include <stdio.h>
#include <string.h>

#include "jsonsink.h"

void
jsonsink_add_uint32(struct jsonsink *s, uint32_t v)
{
        char buf[sizeof("4294967295")];
        int ret = snprintf(buf, sizeof(buf), "%" PRIu32, v);
        if (ret < 0) {
                jsonsink_set_error(s, JSONSINK_ERROR_SERIALIZATION);
                return;
        }
        assert(ret < sizeof(buf));
        jsonsink_add_serialized_value(s, buf, ret);
}

void
jsonsink_add_int32(struct jsonsink *s, int32_t v)
{
        char buf[sizeof("-2147483648")];
        int ret = snprintf(buf, sizeof(buf), "%" PRId32, v);
        if (ret < 0) {
                jsonsink_set_error(s, JSONSINK_ERROR_SERIALIZATION);
                return;
        }
        assert(ret < sizeof(buf));
        jsonsink_add_serialized_value(s, buf, ret);
}

void
jsonsink_add_double(struct jsonsink *s, double v)
{
        char buf[64];
        int ret = snprintf(buf, sizeof(buf), "%1.17g", v);
        if (ret < 0) {
                jsonsink_set_error(s, JSONSINK_ERROR_SERIALIZATION);
                return;
        }
        if (ret >= sizeof(buf)) {
                jsonsink_set_error(s, JSONSINK_ERROR_SERIALIZATION);
                return;
        }
        jsonsink_add_serialized_value(s, buf, ret);
}
