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

#include <flatbuffers/flatbuffers.h>

#include "bench.h"
#include "test_generated.h"

using namespace Test;
using namespace flatbuffers;

int
test_flatbuffers(unsigned int n, const double *data_double,
                 const uint32_t *data_u32)
{
        FlatBufferBuilder b;
        std::vector<Offset<Obj>> obj_vec;
        uint32_t i;
        for (i = 0; i < n; i++) {
                auto doubles = b.CreateVector(data_double, 4);
                data_double += 4;
                auto obj = CreateObj(b, *data_u32++, doubles);
                obj_vec.push_back(obj);
        }
        auto objs = b.CreateVector(obj_vec);
        auto root = CreateRoot(b, objs);
        b.Finish(root);
        const void *p = b.GetBufferPointer();
        size_t sz = b.GetSize();
        if (do_fwrite(p, 1, sz, stdout) != sz) {
                printf("fwrite error\n");
                return 1;
        }
        return 0;
}

void
run_bench(void)
{
        bench("flatbuffers", test_flatbuffers);
}
