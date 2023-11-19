/*
    libzint - the open source barcode library
    Copyright (C) 2020-2023 Robin Stuart <rstuart114@gmail.com>

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
/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef Z_CODE128_H
#define Z_CODE128_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Allow for a reasonable number of special Code Set escapes and for GS1 AI delimiters */
#define C128_MAX    256

#define C128_LATCHA 'A'
#define C128_LATCHB 'B'
#define C128_LATCHC 'C'
#define C128_SHIFTA 'a'
#define C128_SHIFTB 'b'
#define C128_ABORC  '9'
#define C128_AORB   'Z'

INTERNAL int code128(struct zint_symbol *symbol, unsigned char source[], int length);

INTERNAL int c128_parunmodd(const unsigned char llyth);
INTERNAL void c128_dxsmooth(int list[2][C128_MAX], int *indexliste, const char *manual_set);
INTERNAL void c128_set_a(const unsigned char source, int values[], int *bar_chars);
INTERNAL int c128_set_b(const unsigned char source, int values[], int *bar_chars);
INTERNAL void c128_set_c(const unsigned char source_a, const unsigned char source_b, int values[], int *bar_chars);
INTERNAL void c128_put_in_set(int list[2][C128_MAX], const int indexliste, char set[C128_MAX],
                const unsigned char *source);

INTERNAL_DATA_EXTERN const char C128Table[107][6];

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* vim: set ts=4 sw=4 et : */
#endif /* Z_CODE128_H */
