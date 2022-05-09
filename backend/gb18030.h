/*  gb18030.h - Unicode to GB 18030

    libzint - the open source barcode library
    Copyright (C) 2019-2022 Robin Stuart <rstuart114@gmail.com>

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

#ifndef Z_GB18030_H
#define Z_GB18030_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

INTERNAL int gbk_wctomb_zint(unsigned int *r, const unsigned int wc);
INTERNAL int gb18030_wctomb_zint(unsigned int *r1, unsigned int *r2, const unsigned int wc);
INTERNAL int gb18030_utf8(struct zint_symbol *symbol, const unsigned char source[], int *p_length,
                unsigned int *ddata);
INTERNAL int gb18030_utf8_to_eci(const int eci, const unsigned char source[], int *p_length, unsigned int *ddata,
                const int full_multibyte);

INTERNAL void gb18030_cpy(const unsigned char source[], int *p_length, unsigned int *ddata,
                const int full_multibyte);
INTERNAL void gb18030_cpy_segs(struct zint_seg segs[], const int seg_count, unsigned int *ddata,
                const int full_multibyte);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* vim: set ts=4 sw=4 et : */
#endif /* Z_GB18030_H */
