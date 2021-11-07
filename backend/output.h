/*  output.h - Common routines for raster/vector

    libzint - the open source barcode library
    Copyright (C) 2020 - 2021 Robin Stuart <rstuart114@gmail.com>

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

#ifndef Z_OUTPUT_H
#define Z_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

INTERNAL int out_check_colour_options(struct zint_symbol *symbol);
INTERNAL void out_set_whitespace_offsets(const struct zint_symbol *symbol, const int hide_text,
                float *xoffset, float *yoffset, float *roffset, float *boffset, const float scaler,
                int *xoffset_si, int *yoffset_si, int *roffset_si, int *boffset_si);
INTERNAL int out_process_upcean(const struct zint_symbol *symbol, int *p_main_width, int *p_comp_xoffset,
                unsigned char addon[6], int *p_addon_gap);
INTERNAL float out_large_bar_height(struct zint_symbol *symbol, int si, int *row_heights_si, int *symbol_height_si);
INTERNAL void out_upcean_split_text(int upceanflag, unsigned char text[],
                unsigned char textpart1[5], unsigned char textpart2[7], unsigned char textpart3[7],
                unsigned char textpart4[2]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* Z_OUTPUT_H */
