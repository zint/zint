/* common.h - Header for all common functions in common.c */
/*
    libzint - the open source barcode library
    Copyright (C) 2009-2022 Robin Stuart <rstuart114@gmail.com>

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

#ifndef Z_COMMON_H
#define Z_COMMON_H

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

#ifdef _MSC_VER
#  include <malloc.h>
#  define z_alloca(nmemb) _alloca(nmemb)
#else
#  if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199000L /* C89 */
#    include <alloca.h>
#  endif
#  define z_alloca(nmemb) alloca(nmemb)
#endif

#ifdef _MSC_VER
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

/* `is_sane()` flags */
#define IS_SPC_F    0x0001 /* Space */
#define IS_HSH_F    0x0002 /* Hash sign # */
#define IS_AST_F    0x0004 /* Asterisk sign * */
#define IS_PLS_F    0x0008 /* Plus sign + */
#define IS_MNS_F    0x0010 /* Minus sign - */
#define IS_NUM_F    0x0020 /* Number 0-9 */
#define IS_UPO_F    0x0040 /* Uppercase letter, apart from A-F and X */
#define IS_UHX_F    0x0080 /* Uppercase hex A-F */
#define IS_UX__F    0x0100 /* Uppercase X */
#define IS_LWO_F    0x0200 /* Lowercase letter, apart from a-f and x */
#define IS_LHX_F    0x0400 /* Lowercase hex a-f */
#define IS_LX__F    0x0800 /* Lowercase x */
#define IS_C82_F    0x1000 /* CSET82 punctuation (apart from *, + and -) */
#define IS_SIL_F    0x2000 /* SILVER/TECHNETIUM punctuation .$/% (apart from space, + and -) */
#define IS_CLI_F    0x4000 /* CALCIUM INNER punctuation $:/. (apart from + and -) (Codabar) */
#define IS_ARS_F    0x8000 /* ARSENIC uppercase subset (VIN) */

#define IS_UPR_F    (IS_UPO_F | IS_UHX_F | IS_UX__F) /* Uppercase letters */
#define IS_LWR_F    (IS_LWO_F | IS_LHX_F | IS_LX__F) /* Lowercase letters */

/* The most commonly used set */
#define NEON_F      IS_NUM_F /* NEON "0123456789" */

/* Simple versions of <cctype> functions with no dependence on locale */
#define z_isdigit(c) ((c) <= '9' && (c) >= '0')
#define z_isupper(c) ((c) >= 'A' && (c) <= 'Z')
#define z_islower(c) ((c) >= 'a' && (c) <= 'z')

#include "zint.h"
#include "zintconfig.h"
#include <stdlib.h>
#include <string.h>

/* Helpers to cast away char pointer signedness */
#define ustrlen(source) strlen((const char *) (source))
#define ustrcpy(target, source) strcpy((char *) (target), (const char *) (source))
#define ustrcat(target, source) strcat((char *) (target), (const char *) (source))
#define ustrncat(target, source, count) strncat((char *) (target), (const char *) (source), (count))

/* VC6 or C89 */
#if (defined(_MSC_VER) && _MSC_VER == 1200) || (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199000L)
#  define ceilf (float) ceil
#  define floorf (float) floor
#  define fmodf (float) fmod
#endif
/* `round()` (C99) not before MSVC 2013 (C++ 12.0) */
#if (defined(_MSC_VER) && _MSC_VER < 1800) || (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199000L)
#  define round(arg) floor((arg) + 0.5)
#  define roundf(arg) floorf((arg) + 0.5f)
#endif

#ifdef _MSC_VER
#  pragma warning(disable: 4244) /* conversion from int to float */
#  if _MSC_VER != 1200 /* VC6 */
#    pragma warning(disable: 4996) /* function or variable may be unsafe */
#  endif
#endif

/* Is float integral value? (https://stackoverflow.com/a/40404149) */
#define isfintf(arg) (fmodf(arg, 1.0f) == 0.0f)

#if (defined(__GNUC__) || defined(__clang__)) && !defined(ZINT_TEST) && !defined(__MINGW32__)
#  define INTERNAL __attribute__ ((visibility ("hidden")))
#elif defined(ZINT_TEST)
#  define INTERNAL ZINT_EXTERN /* The test suite references INTERNAL functions, so they need to be exported */
#else
#  define INTERNAL
#endif

#if (defined(__GNUC__) || defined(__clang__)) && !defined(__MINGW32__)
#  define INTERNAL_DATA_EXTERN __attribute__ ((visibility ("hidden"))) extern
#  define INTERNAL_DATA __attribute__ ((visibility ("hidden")))
#else
#  define INTERNAL_DATA_EXTERN extern
#  define INTERNAL_DATA
#endif

#define Z_COMMON_INLINE   1

#ifdef Z_COMMON_INLINE
/* Return true (1) if a module is dark/black, otherwise false (0) */
#  define module_is_set(s, y, x) (((s)->encoded_data[(y)][(x) >> 3] >> ((x) & 0x07)) & 1)

/* Set a module to dark/black */
#  define set_module(s, y, x) do { (s)->encoded_data[(y)][(x) >> 3] |= 1 << ((x) & 0x07); } while (0)

/* Return true (1-8) if a module is colour, otherwise false (0) */
#  define module_colour_is_set(s, y, x) ((s)->encoded_data[(y)][(x)])

/* Set a module to a colour */
#  define set_module_colour(s, y, x, c) do { (s)->encoded_data[(y)][(x)] = (c); } while (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    INTERNAL int ctoi(const char source);
    INTERNAL char itoc(const int source);
    INTERNAL int to_int(const unsigned char source[], const int length);
    INTERNAL void to_upper(unsigned char source[], const int length);
    INTERNAL int chr_cnt(const unsigned char string[], const int length, const unsigned char c);

    INTERNAL int is_chr(const unsigned int flg, const unsigned int c);
    INTERNAL int is_sane(const unsigned int flg, const unsigned char source[], const int length);
    INTERNAL int is_sane_lookup(const char test_string[], const int test_length, const unsigned char source[],
                    const int length, int *posns);
    INTERNAL int posn(const char set_string[], const char data);

    INTERNAL int bin_append_posn(const int arg, const int length, char *binary, const int bin_posn);

    #ifndef Z_COMMON_INLINE
    INTERNAL int module_is_set(const struct zint_symbol *symbol, const int y_coord, const int x_coord);
    INTERNAL void set_module(struct zint_symbol *symbol, const int y_coord, const int x_coord);
    INTERNAL int module_colour_is_set(const struct zint_symbol *symbol, const int y_coord, const int x_coord);
    INTERNAL void set_module_colour(struct zint_symbol *symbol, const int y_coord, const int x_coord,
                    const int colour);
    #endif
    INTERNAL void unset_module(struct zint_symbol *symbol, const int y_coord, const int x_coord);

    INTERNAL void expand(struct zint_symbol *symbol, const char data[], const int length);

    INTERNAL int is_stackable(const int symbology);
    INTERNAL int is_extendable(const int symbology);
    INTERNAL int is_composite(const int symbology);
    INTERNAL int is_dotty(const int symbology);
    INTERNAL int is_fixed_ratio(const int symbology);

    INTERNAL int is_twodigits(const unsigned char source[], const int length, const int position);

    INTERNAL unsigned int decode_utf8(unsigned int *state, unsigned int *codep, const unsigned char byte);
    INTERNAL int is_valid_utf8(const unsigned char source[], const int length);
    INTERNAL int utf8_to_unicode(struct zint_symbol *symbol, const unsigned char source[], unsigned int vals[],
                    int *length, const int disallow_4byte);

    INTERNAL int set_height(struct zint_symbol *symbol, const float min_row_height, const float default_height,
                    const float max_height, const int set_errtxt);

    INTERNAL float stripf(const float arg);

    INTERNAL int segs_length(const struct zint_seg segs[], const int seg_count);
    INTERNAL void segs_cpy(const struct zint_symbol *symbol, const struct zint_seg segs[], const int seg_count,
                struct zint_seg local_segs[]);

    INTERNAL int colour_to_red(const int colour);
    INTERNAL int colour_to_green(const int colour);
    INTERNAL int colour_to_blue(const int colour);

    #ifdef ZINT_TEST
    INTERNAL void debug_test_codeword_dump(struct zint_symbol *symbol, const unsigned char *codewords,
                    const int length);
    INTERNAL void debug_test_codeword_dump_int(struct zint_symbol *symbol, const int *codewords, const int length);
    #endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* vim: set ts=4 sw=4 et : */
#endif /* Z_COMMON_H */
