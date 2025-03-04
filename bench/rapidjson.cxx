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
#include <stdlib.h>

#include "bench.h"
#include "rapidjson/writer.h"

using namespace rapidjson;

int
test_rapidjson_writer(unsigned int n, const double *data_double,
                      const uint32_t *data_u32)
{
        StringBuffer s;
        Writer<StringBuffer> writer(s);
        writer.StartObject();
        writer.Key("array");
        writer.StartArray();
        uint32_t i;
        for (i = 0; i < n; i++) {
                writer.StartObject();
                writer.Key("u32");
                writer.Uint(*data_u32++);
                writer.Key("double_array");
                writer.StartArray();
                writer.Double(*data_double++);
                writer.Double(*data_double++);
                writer.Double(*data_double++);
                writer.Double(*data_double++);
                writer.EndArray();
                writer.EndObject();
        }
        writer.EndArray();
        writer.EndObject();
        const char *p = s.GetString();
        size_t sz = s.GetSize();
        if (do_fwrite(p, 1, sz, stdout) != sz) {
                printf("fwrite error\n");
                return 1;
        }
        return 0;
}

void
run_bench(void)
{
        bench("test_rapidjson_writer", test_rapidjson_writer);
}
