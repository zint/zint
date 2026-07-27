/* zfont.h - handle fonts */
/*
    libzint - the open source barcode library
    Copyright (C) 2026 Robin Stuart <rstuart114@gmail.com>

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

#ifndef Z_FONT_H
#define Z_FONT_H

#define ZFONT_MAX_CHARS ((int) sizeof(((struct zint_symbol *)0)->text))
#define ZFONT_MAX_LINES ZFONT_MAX_CHARS

/* NOTE: ZFONT_HALIGN_XXX be same as vector_string->halign */
#define ZFONT_HALIGN_CENTRE 0
#define ZFONT_HALIGN_LEFT   1
#define ZFONT_HALIGN_RIGHT  2
#define ZFONT_UPCEAN_TEXT   4   /* Helper flag to indicate dealing with EAN/UPC */
#define ZFONT_UPCO_TEXT     8

#define STBTT_STATIC
#include "stb_truetype_zint.h"

struct zfont_patch_info;

struct zfont {
    stbtt_fontinfo info;

    /* Raster patching */
    const unsigned char *patch_info_idxs;
    const struct zfont_patch_info *patch_infos;
    const unsigned char *patches;
    const char *bases;
    int patch_info_idxs_size;

    const struct zfont_patch_info *upco_patch_infos;
    const unsigned char *upco_patches;

    int upceanflag;
    int grayscale;

    float scale; /* stb scale */
    float upco_scale; /* stb scale for UPC-E/A outside digits */

    int font_height;
    int upco_font_height; /* UPC-E/A outside digits font size */
    float ascent;
    float descent_adj;
    float line_gap;
    float text_height;

    /* Note using "ascender" to mean height above digits as "ascent" usually measured from baseline */
    float digit_ascender;
    float text_gap_antialias;
    float antialias_fudge;

    float hadvances[ZFONT_MAX_CHARS]; /* Horizontal advances */
    float line_widths[ZFONT_MAX_LINES];
    float max_line_width;
    unsigned char line_idxs[ZFONT_MAX_LINES + 1]; /* Line indices into `symbol->text`; last extra index is length */
    unsigned char iso_line_idxs[ZFONT_MAX_LINES + 1]; /* Line indices into `iso_text`; last extra index is length */
    int lines; /* Number of lines */
    int line_advance; /* Size (height) of each line */

    unsigned char iso_text[ZFONT_MAX_CHARS]; /* `symbol->text` converted to ISO/IEC 8859-1 */
    int iso_len; /* Length of `iso_text[]` */

    int glyphs[256]; /* Font glyphs (confined to ISO/IEC 8859-1, i.e. <= 0xFF) */
    unsigned char *bitmaps[256]; /* Rendered bitmaps per glyph */
    int bitmap_dims[256][4]; /* { width, height, `xMin`, `yMin` } */
    int space_glyph;
    int first_ch, last_ch; /* First/last bitmap character used */

    /* UPC-E/A outside stuff */
    int upco_glyphs[10]; /* Digits only */
    unsigned char *upco_bitmaps[10]; /* Rendered bitmaps per UPC-E/A outside glyph */
    int upco_bitmap_dims[10][4]; /* { width, height, `xMin`, `yMin` } */
};

/* Initialize a font */
INTERNAL int zint_font_init(struct zfont *zfnt, struct zint_symbol *symbol, const int upceanflag);

/* Free any resources */
INTERNAL void zint_font_free(struct zfont *zfnt, const int si);

/* Calculate various font metrics, and set total height of text in `zfnt->text_height` */
INTERNAL int zint_font_text_height(struct zfont *zfnt, struct zint_symbol *symbol, const int si,
                const float max_width, const unsigned char addon[6], const int addon_len);

/* Raster rendering */
INTERNAL void zint_font_text_render(struct zfont *zfnt, struct zint_symbol *symbol, unsigned char *pixelbuf,
                const int line, const int main_width, const int xoffset, const int roffset, const int yposn,
                const int textflags, const int image_width, const int image_height, const int si);

/* vim: set ts=4 sw=4 et : */
#endif /* Z_FONT_H */
