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
 * a shared library to hook malloc and friends with a help of dyld.
 *
 * eg.
 *   cc -shared -o malloc_interposer.dylib malloc_interposer.c
 *   DYLD_INSERT_LIBRARIES=malloc_interposer.dylib ./testee
 */

#include <malloc/malloc.h>
#include <stdlib.h>

#define MALLOC_INTERPOSER
#include "malloc_interposer.h"

struct malloc_stat malloc_stat;

static void
update_peak(void)
{
        if (malloc_stat.peak_allocated_bytes < malloc_stat.allocated_bytes) {
                malloc_stat.peak_allocated_bytes = malloc_stat.allocated_bytes;
        }
}

void *
_malloc(size_t sz)
{
        void *p = malloc(sz);
        if (p != NULL) {
                malloc_stat.allocated_bytes += malloc_size(p);
                update_peak();
        }
        return p;
}

void *
_calloc(size_t count, size_t sz)
{
        void *p = calloc(count, sz);
        if (p != NULL) {
                malloc_stat.allocated_bytes += malloc_size(p);
                update_peak();
        }
        return p;
}

void *
_realloc(void *p, size_t sz)
{
        size_t osz = 0;
        if (p != NULL) {
                osz = malloc_size(p);
        }
        p = realloc(p, sz);
        if (p != NULL) {
                malloc_stat.allocated_bytes -= osz;
                malloc_stat.allocated_bytes += malloc_size(p);
                update_peak();
        }
        return p;
}

void *
_reallocf(void *p, size_t sz)
{
        if (p != NULL) {
                malloc_stat.allocated_bytes -= malloc_size(p);
        }
        p = reallocf(p, sz);
        if (p != NULL) {
                malloc_stat.allocated_bytes += malloc_size(p);
                update_peak();
        }
        return p;
}

void *
_aligned_alloc(size_t alignment, size_t sz)
{
        void *p = aligned_alloc(alignment, sz);
        if (p != NULL) {
                malloc_stat.allocated_bytes += malloc_size(p);
                update_peak();
        }
        return p;
}

void
_free(void *p)
{
        if (p != NULL) {
                malloc_stat.allocated_bytes -= malloc_size(p);
        }
        free(p);
}

struct interpose {
        void *new_func;
        void *orig_func;
};

__attribute__((used)) __attribute__((section(
        "__DATA,__interpose"))) static struct interpose interpose_malloc[] = {
        {(void *)_malloc, (void *)malloc},
        {(void *)_calloc, (void *)calloc},
        {(void *)_free, (void *)free},
        {(void *)_realloc, (void *)realloc},
        {(void *)_reallocf, (void *)reallocf},
        {(void *)_aligned_alloc, (void *)aligned_alloc},
};
