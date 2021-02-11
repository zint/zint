/* large.h - Handles binary manipulation of large numbers */

/*
    libzint - the open source barcode library
    Copyright (C) 2008 - 2021 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
/* vim: set ts=4 sw=4 et : */
#ifndef __LARGE_H
#define __LARGE_H

#ifndef _MSC_VER
#include <stdint.h>
#else
#include "ms_stdint.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct { uint64_t lo; uint64_t hi; } large_int;

#define large_lo(s) ((s)->lo)
#define large_hi(s) ((s)->hi)

/* Set 128-bit `t` from 128-bit `s` */
#define large_load(t, s) do { (t)->lo = (s)->lo; (t)->hi = (s)->hi; } while (0)

/* Set 128-bit `t` from 64-bit `s` */
#define large_load_u64(t, s) do { (t)->lo = (s); (t)->hi = 0; } while (0)

INTERNAL void large_load_str_u64(large_int *t, const unsigned char *s, const int length);

INTERNAL void large_add(large_int *t, const large_int *s);
INTERNAL void large_add_u64(large_int *t, const uint64_t s);

INTERNAL void large_sub_u64(large_int *t, const uint64_t s);

INTERNAL void large_mul_u64(large_int *t, const uint64_t s);

INTERNAL uint64_t large_div_u64(large_int *t, uint64_t v);

INTERNAL void large_unset_bit(large_int *t, const int bit);

INTERNAL void large_uint_array(const large_int *t, unsigned int *uint_array, const int size, int bits);
INTERNAL void large_uchar_array(const large_int *t, unsigned char *uchar_array, const int size, int bits);

INTERNAL void large_print(const large_int *t);
INTERNAL char *large_dump(const large_int *t, char *buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LARGE_H */
