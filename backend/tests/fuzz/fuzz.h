/*  fuzz.h - common functions for fuzzing libzint */
/*
    libzint - the open source barcode library
    Copyright (C) 2024 Robin Stuart <rstuart114@gmail.com>

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

#ifndef Z_FUZZ_H
#define Z_FUZZ_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zint.h"

#define ZARRAY_SIZE(x) ((int) (sizeof(x) / sizeof((x)[0])))

#define INPUT_MODE_MASK     (ESCAPE_MODE | GS1PARENS_MODE | GS1NOCHECK_MODE | HEIGHTPERROW_MODE | FAST_MODE \
                                | EXTRA_ESCAPE_MODE)

#ifdef Z_FUZZ_SET_OUTPUT_OPTIONS
#define OUTPUT_OPTIONS_MASK (BARCODE_BIND_TOP | BARCODE_BIND | BARCODE_BOX | BARCODE_STDOUT | READER_INIT \
                                | SMALL_TEXT | BOLD_TEXT | CMYK_COLOUR | BARCODE_DOTTY_MODE | GS1_GS_SEPARATOR \
                                | OUT_BUFFER_INTERMEDIATE | BARCODE_QUIET_ZONES | BARCODE_NO_QUIET_ZONES \
                                | COMPLIANT_HEIGHT | EANUPC_GUARD_WHITESPACE | EMBED_VECTOR_FONT)
#endif

/* Based on `is_sane()` flags in "backend/common.h") */
#define IS_CTL_F    (0x00000001)            /* ASCII control (incl. DEL) */
#define IS_PRT_F    (0x00000002)            /* ASCII printable (incl. space) */
#define IS_SPC_F    (0x00000004 | IS_PRT_F) /* Space */
#define IS_HSH_F    (0x00000008 | IS_PRT_F) /* Hash sign # */
#define IS_AST_F    (0x00000010 | IS_PRT_F) /* Asterisk sign * */
#define IS_PLS_F    (0x00000020 | IS_PRT_F) /* Plus sign + */
#define IS_MNS_F    (0x00000040 | IS_PRT_F) /* Minus sign - */
#define IS_NUM_F    (0x00000080 | IS_PRT_F) /* Number 0-9 */
#define IS_UPO_F    (0x00000100 | IS_PRT_F) /* Uppercase letter, apart from A-F, T and X */
#define IS_UAD_F    (0x00000200 | IS_PRT_F) /* Uppercase A, D (hex, CALCIUM/FIM/DAFT) */
#define IS_UBC_F    (0x00000400 | IS_PRT_F) /* Uppercase B, C (hex, CALCIUM/FIM) */
#define IS_UE__F    (0x00000800 | IS_PRT_F) /* Uppercase E (hex, FIM) */
#define IS_UF__F    (0x00001000 | IS_PRT_F) /* Uppercase F (hex, DAFT) */
#define IS_UT__F    (0x00002000 | IS_PRT_F) /* Uppercase T (DAFT) */
#define IS_UX__F    (0x00004000 | IS_PRT_F) /* Uppercase X (SODIUM_X/ISBNX) */
#define IS_LWO_F    (0x00008000 | IS_PRT_F) /* Lowercase letter, apart from a-f, t and x */
#define IS_LAD_F    (0x00010000 | IS_PRT_F) /* Lowercase a, d (hex, CALCIUM/FIM/DAFT) */
#define IS_LBC_F    (0x00020000 | IS_PRT_F) /* Lowercase b, c (hex, CALCIUM/FIM) */
#define IS_LE__F    (0x00040000 | IS_PRT_F) /* Lowercase e (hex, FIM) */
#define IS_LF__F    (0x00080000 | IS_PRT_F) /* Lowercase f (hex, DAFT) */
#define IS_LT__F    (0x00100000 | IS_PRT_F) /* Lowercase t (DAFT) */
#define IS_LX__F    (0x00200000 | IS_PRT_F) /* Lowercase x (SODIUM_X/ISBNX) */
#define IS_C82_F    (0x00400000 | IS_PRT_F) /* CSET82 punctuation (apart from *, + and -) */
#define IS_SIL_F    (0x00800000 | IS_PRT_F) /* SILVER/TECHNETIUM punctuation .$/% (apart from space, + and -) */
#define IS_CLI_F    (0x01000000 | IS_PRT_F) /* CALCIUM INNER punctuation $:/. (apart from + and -) (Codabar) */
#define IS_ARS_F    (0x02000000 | IS_PRT_F) /* ARSENIC uppercase subset (VIN) */

#define IS_UHX_F    (IS_UAD_F | IS_UBC_F | IS_UE__F | IS_UF__F) /* Uppercase hex */
#define IS_LHX_F    (IS_LAD_F | IS_LBC_F | IS_LE__F | IS_LF__F) /* Lowercase hex */
#define IS_UPR_F    (IS_UPO_F | IS_UHX_F | IS_UT__F | IS_UX__F) /* Uppercase letters */
#define IS_LWR_F    (IS_LWO_F | IS_LHX_F | IS_LT__F | IS_LX__F) /* Lowercase letters */

/* Flag table for `is_chr()` and `is_sane()` (taken from "backend/common.c") */
#define IS_CLS_F    (IS_CLI_F | IS_SIL_F)
static const unsigned int flgs[256] = {
    IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, /*00-07*/
    IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, /*08-0F*/
    IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, /*10-17*/
    IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, IS_CTL_F, /*18-1F*/
               IS_SPC_F,            IS_C82_F,            IS_C82_F,            IS_HSH_F, /*20-23*/ /*  !"# */
               IS_CLS_F, IS_SIL_F | IS_C82_F,            IS_C82_F,            IS_C82_F, /*24-27*/ /* $%&' */
               IS_C82_F,            IS_C82_F,            IS_AST_F,            IS_PLS_F, /*28-2B*/ /* ()*+ */
               IS_C82_F,            IS_MNS_F, IS_CLS_F | IS_C82_F, IS_CLS_F | IS_C82_F, /*2C-2F*/ /* ,-./ */
               IS_NUM_F,            IS_NUM_F,            IS_NUM_F,            IS_NUM_F, /*30-33*/ /* 0123 */
               IS_NUM_F,            IS_NUM_F,            IS_NUM_F,            IS_NUM_F, /*34-37*/ /* 4567 */
               IS_NUM_F,            IS_NUM_F, IS_CLI_F | IS_C82_F,            IS_C82_F, /*38-3B*/ /* 89:; */
               IS_C82_F,            IS_C82_F,            IS_C82_F,            IS_C82_F, /*3C-3F*/ /* <=>? */
               IS_PRT_F, IS_ARS_F | IS_UAD_F, IS_ARS_F | IS_UBC_F, IS_ARS_F | IS_UBC_F, /*40-43*/ /* @ABC */
    IS_ARS_F | IS_UAD_F, IS_ARS_F | IS_UE__F, IS_ARS_F | IS_UF__F, IS_ARS_F | IS_UPO_F, /*44-47*/ /* DEFG */
    IS_ARS_F | IS_UPO_F,            IS_UPO_F, IS_ARS_F | IS_UPO_F, IS_ARS_F | IS_UPO_F, /*48-4B*/ /* HIJK */
    IS_ARS_F | IS_UPO_F, IS_ARS_F | IS_UPO_F, IS_ARS_F | IS_UPO_F,            IS_UPO_F, /*4C-4F*/ /* LMNO */
    IS_ARS_F | IS_UPO_F,            IS_UPO_F, IS_ARS_F | IS_UPO_F, IS_ARS_F | IS_UPO_F, /*50-53*/ /* PQRS */
    IS_ARS_F | IS_UT__F, IS_ARS_F | IS_UPO_F, IS_ARS_F | IS_UPO_F, IS_ARS_F | IS_UPO_F, /*53-57*/ /* TUVW */
    IS_ARS_F | IS_UX__F, IS_ARS_F | IS_UPO_F, IS_ARS_F | IS_UPO_F,            IS_PRT_F, /*58-5B*/ /* XYZ[ */
               IS_PRT_F,            IS_PRT_F,            IS_PRT_F,            IS_C82_F, /*5C-5F*/ /* \]^_ */
               IS_PRT_F,            IS_LAD_F,            IS_LBC_F,            IS_LBC_F, /*60-63*/ /* `abc */
               IS_LAD_F,            IS_LE__F,            IS_LF__F,            IS_LWO_F, /*64-67*/ /* defg */
               IS_LWO_F,            IS_LWO_F,            IS_LWO_F,            IS_LWO_F, /*68-6B*/ /* hijk */
               IS_LWO_F,            IS_LWO_F,            IS_LWO_F,            IS_LWO_F, /*6C-6F*/ /* lmno */
               IS_LWO_F,            IS_LWO_F,            IS_LWO_F,            IS_LWO_F, /*70-73*/ /* pqrs */
               IS_LT__F,            IS_LWO_F,            IS_LWO_F,            IS_LWO_F, /*74-77*/ /* tuvw */
               IS_LX__F,            IS_LWO_F,            IS_LWO_F,            IS_PRT_F, /*78-7B*/ /* xyz{ */
               IS_PRT_F,            IS_PRT_F,            IS_PRT_F,            IS_CTL_F, /*7C-7F*/ /* |}~D */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*80-9F*/
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*A0-BF*/
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*C0-DF*/
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*E0-FF*/
};

/* Verifies that a string only uses valid characters */
static int is_sane(const unsigned int flg, const unsigned char source[], const int length) {
    int i;

    for (i = 0; i < length; i++) {
        if (!(flgs[source[i]] & flg)) {
            return 0;
        }
    }
    return 1;
}

#define NEON_F              (IS_NUM_F) /* NEON "0123456789" */
#define SODIUM_MNS_F        (IS_NUM_F | IS_MNS_F) /* SODIUM_MNS "0123456789-" */
#define SODIUM_PLS_F        (IS_NUM_F | IS_PLS_F) /* SODIUM_PLS "0123456789+" */
#define SODIUM_X_F          (IS_NUM_F | IS_UX__F | IS_LX__F) /* SODIUM_X "0123456789Xx" */
#define ISBNX_ADDON_SANE_F  (IS_NUM_F | IS_UX__F | IS_LX__F | IS_PLS_F) /* ISBNX_ADDON_SANE "0123456789Xx+" */
/* SILVER "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%" + lowercase */
#define SILVER_F            (IS_NUM_F | IS_UPR_F | IS_LWR_F | IS_MNS_F | IS_SIL_F | IS_SPC_F | IS_PLS_F)
/* CALCIUM "0123456789-$:/.+ABCD" + lowercase */
#define CALCIUM_F           (IS_NUM_F | IS_MNS_F | IS_CLI_F | IS_PLS_F | IS_UAD_F | IS_UBC_F | IS_LAD_F | IS_LBC_F)
/* FIM "ABCDE" + lowercase */
#define FIM_F               (IS_UAD_F | IS_UBC_F | IS_UE__F | IS_LAD_F | IS_LBC_F | IS_LE__F)
/* GDSET "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz #" */
#define GDSET_F             (IS_NUM_F | IS_UPR_F | IS_LWR_F | IS_SPC_F | IS_HSH_F)
#define KRSET_F             (IS_NUM_F | IS_UPR_F | IS_LWR_F)
/* ARSENIC "0123456789ABCDEFGHJKLMNPRSTUVWXYZ" (no lowercase) */
#define ARSENIC_F           (IS_NUM_F | IS_ARS_F)
/* SHKASUTSET "1234567890-ABCDEFGHIJKLMNOPQRSTUVWXYZ" + lowercase */
#define SHKASUTSET_F        (IS_NUM_F | IS_MNS_F | IS_UPR_F | IS_LWR_F)
#define SSET_F              (IS_NUM_F | IS_UHX_F) /* SSET "0123456789ABCDEF" (no lowercase) */
/* DAFT "FADT" + lowercase */
#define DAFT_F              (IS_UAD_F | IS_UF__F | IS_UT__F | IS_LAD_F | IS_LF__F | IS_LT__F)
/* RUBIDIUM "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ " + lowercase */
#define RUBIDIUM_F          (IS_NUM_F | IS_UPR_F | IS_LWR_F | IS_SPC_F)
#define ASCII_PRT_F         (IS_PRT_F) /* ASCII printable */
#define ASCII_F             (IS_PRT_F | IS_CTL_F) /* ASCII */

struct settings_item {
    const int idx, symbology, sane_flag;
    const int option_1_min, option_1_max, option_2_min, option_2_max, option_3, len_min, len_max;
};

static const struct settings_item settings[] = {
    {   0, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {   1, BARCODE_CODE11,       SODIUM_MNS_F,           0,  -1,           0,   2,           0,             1,  140 },
    {   2, BARCODE_C25STANDARD,        NEON_F,           0,  -1,           0,   2,           0,             1,  112 },
    {   3, BARCODE_C25INTER,           NEON_F,           0,  -1,           0,   2,           0,             1,  125 },
    {   4, BARCODE_C25IATA,            NEON_F,           0,  -1,           0,   2,           0,             1,   80 },
    {   5, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {   6, BARCODE_C25LOGIC,           NEON_F,           0,  -1,           0,   2,           0,             1,  113 },
    {   7, BARCODE_C25IND,             NEON_F,           0,  -1,           0,   2,           0,             1,   79 },
    {   8, BARCODE_CODE39,           SILVER_F,           0,  -1,           0,   2,           0,             1,   86 },
    {   9, BARCODE_EXCODE39,         SILVER_F,           0,  -1,           0,   2,           0,             1,   86 },
    {  10, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  11, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  12, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  13, BARCODE_EANX,         SODIUM_PLS_F,           0,  -1,           0,  -1,           0,             1,   19 },
    {  14, BARCODE_EANX_CHK,     SODIUM_PLS_F,           0,  -1,           0,  -1,           0,             1,   19 },
    {  15, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  16, BARCODE_GS1_128,       ASCII_PRT_F,           0,  -1,           0,  -1,           0,             3,  198 },
    {  17, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  18, BARCODE_CODABAR,         CALCIUM_F,           0,  -1,           0,   2,           0,             3,  103 },
    {  19, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  20, BARCODE_CODE128,                 0,           0,  -1,           0,  -1,           0,             1,  198 },
    {  21, BARCODE_DPLEIT,             NEON_F,           0,  -1,           0,  -1,           0,             1,   13 },
    {  22, BARCODE_DPIDENT,            NEON_F,           0,  -1,           0,  -1,           0,             1,   11 },
    {  23, BARCODE_CODE16K,                 0,          -1,  16,           0,  -1,           0,             1,  154 },
    {  24, BARCODE_CODE49,            ASCII_F,           2,   8,           0,  -1,           0,             1,   81 },
    {  25, BARCODE_CODE93,            ASCII_F,           0,  -1,           0,   1,           0,             1,  123 },
    {  26, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  27, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  28, BARCODE_FLAT,               NEON_F,           0,  -1,           0,  -1,           0,             1,  128 },
    {  29, BARCODE_DBAR_OMN,           NEON_F,           0,  -1,           0,  -1,           0,             1,   14 },
    {  30, BARCODE_DBAR_LTD,           NEON_F,           0,  -1,           0,  -1,           0,             1,   14 },
    {  31, BARCODE_DBAR_EXP,      ASCII_PRT_F,           0,  -1,           0,  -1,           0,             3,   74 },
    {  32, BARCODE_TELEPEN,           ASCII_F,           0,  -1,           0,  -1,           0,             1,   69 },
    {  33, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  34, BARCODE_UPCA,         SODIUM_PLS_F,           0,  -1,           0,  -1,           0,             1,   18 },
    {  35, BARCODE_UPCA_CHK,     SODIUM_PLS_F,           0,  -1,           0,  -1,           0,             1,   18 },
    {  36, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  37, BARCODE_UPCE,         SODIUM_PLS_F,           0,  -1,           0,  -1,           0,             1,   14 },
    {  38, BARCODE_UPCE_CHK,     SODIUM_PLS_F,           0,  -1,           0,  -1,           0,             1,   14 },
    {  39, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  40, BARCODE_POSTNET,            NEON_F,           0,  -1,           0,  -1,           0,             1,   38 },
    {  41, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  42, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  43, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  44, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  45, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  46, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  47, BARCODE_MSI_PLESSEY,        NEON_F,           0,  -1,           0,  16,           0,             1,   92 },
    {  48, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  49, BARCODE_FIM,                 FIM_F,           0,  -1,           0,  -1,           0,             1,    1 },
    {  50, BARCODE_LOGMARS,          SILVER_F,           0,  -1,           0,   2,           0,             1,   30 },
    {  51, BARCODE_PHARMA,             NEON_F,           0,  -1,           0,  -1,           0,             1,    6 },
    {  52, BARCODE_PZN,                NEON_F,           0,  -1,           0,   1,           0,             1,    8 },
    {  53, BARCODE_PHARMA_TWO,         NEON_F,           0,  -1,           0,  -1,           0,             1,    8 },
    {  54, BARCODE_CEPNET,             NEON_F,           0,  -1,           0,  -1,           0,             8,    8 },
    {  55, BARCODE_PDF417,                  0,          -1,   8,           0,  30,           0,             1, 2710 },
    {  56, BARCODE_PDF417COMP,              0,          -1,   8,           0,  30,           0,             1, 2710 },
    {  57, BARCODE_MAXICODE,                0,          -1,   6,           0, 100,           0,             1,  138 },
    {  58, BARCODE_QRCODE,                  0,          -1,   4,           0,  40, ZINT_FULL_MULTIBYTE,     1, 7089 },
    {  59, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  60, BARCODE_CODE128AB,               0,           0,  -1,           0,  -1,           0,             1,   99 },
    {  61, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  62, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  63, BARCODE_AUSPOST,           GDSET_F,           0,  -1,           0,  -1,           0,             8,   23 },
    {  64, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  65, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  66, BARCODE_AUSREPLY,          GDSET_F,           0,  -1,           0,  -1,           0,             1,    8 },
    {  67, BARCODE_AUSROUTE,          GDSET_F,           0,  -1,           0,  -1,           0,             1,    8 },
    {  68, BARCODE_AUSREDIRECT,       GDSET_F,           0,  -1,           0,  -1,           0,             1,    8 },
    {  69, BARCODE_ISBNX,  ISBNX_ADDON_SANE_F,           0,  -1,           0,  -1,           0,             9,   19 },
    {  70, BARCODE_RM4SCC,            KRSET_F,           0,  -1,           0,  -1,           0,             1,   50 },
    {  71, BARCODE_DATAMATRIX,              0,           0,  -1,           0,  48, DM_SQUARE | DM_ISO_144,  1, 3550 },
    {  72, BARCODE_EAN14,              NEON_F,           0,  -1,           0,  -1,           0,             1,   13 },
    {  73, BARCODE_VIN,             ARSENIC_F,           0,  -1,           0,  -1,           0,            17,   17 },
    {  74, BARCODE_CODABLOCKF,              0,          -1,  44,          -1,  67,           0,             1, 2725 },
    {  75, BARCODE_NVE18,              NEON_F,           0,  -1,           0,  -1,           0,             1,   17 },
    {  76, BARCODE_JAPANPOST,    SHKASUTSET_F,           0,  -1,           0,  -1,           0,             1,   20 },
    {  77, BARCODE_KOREAPOST,          NEON_F,           0,  -1,           0,  -1,           0,             1,    6 },
    {  78, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  79, BARCODE_DBAR_STK,           NEON_F,           0,  -1,           0,  -1,           0,             1,   14 },
    {  80, BARCODE_DBAR_OMNSTK,        NEON_F,           0,  -1,           0,  -1,           0,             1,   14 },
    {  81, BARCODE_DBAR_EXPSTK,   ASCII_PRT_F,           0,  -1,           0,  11,           0,             1,   74 },
    {  82, BARCODE_PLANET,             NEON_F,           0,  -1,           0,  -1,           0,             1,   38 },
    {  83, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  84, BARCODE_MICROPDF417,             0,           0,  -1,           0,  30,           0,             1,  366 },
    {  85, BARCODE_USPS_IMAIL,   SODIUM_MNS_F,           0,  -1,           0,  -1,           0,             1,   32 },
    {  86, BARCODE_PLESSEY,            SSET_F,           0,  -1,           0,  -1,           0,             1,   67 },
    {  87, BARCODE_TELEPEN_NUM,    SODIUM_X_F,           0,  -1,           0,  -1,           0,             1,  136 },
    {  88, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  89, BARCODE_ITF14,              NEON_F,           0,  -1,           0,  -1,           0,             1,   13 },
    {  90, BARCODE_KIX,               KRSET_F,           0,  -1,           0,  -1,           0,             1,   18 },
    {  91, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  92, BARCODE_AZTEC,                   0,          -1,   4,           0,  36,           0,             1, 4483 },
    {  93, BARCODE_DAFT,               DAFT_F,           0,  -1,          50, 900,           0,             1,  576 },
    {  94, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  95, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    {  96, BARCODE_DPD,               KRSET_F,           0,  -1,           0,  -1,           0,            27,   28 },
    {  97, BARCODE_MICROQR,                 0,          -1,   4,           0,   4,           0,             1,   35 },
    {  98, BARCODE_HIBC_128,         SILVER_F,           0,  -1,           0,  -1,           0,             1,  110 },
    {  99, BARCODE_HIBC_39,          SILVER_F,           0,  -1,           0,   2,           0,             1,   70 },
    { 100, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 101, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 102, BARCODE_HIBC_DM,          SILVER_F,           0,  -1,           0,  48, DM_SQUARE | DM_ISO_144,  1,  110 },
    { 103, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 104, BARCODE_HIBC_QR,          SILVER_F,          -1,   4,           0,  40, ZINT_FULL_MULTIBYTE,     1,  110 },
    { 105, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 106, BARCODE_HIBC_PDF,         SILVER_F,          -1,   8,           0,  30,           0,             1,  110 },
    { 107, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 108, BARCODE_HIBC_MICPDF,      SILVER_F,           0,  -1,           0,  30,           0,             1,  110 },
    { 109, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 110, BARCODE_HIBC_BLOCKF,      SILVER_F,          -1,  44,          -1,  67,           0,             1,  110 },
    { 111, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 112, BARCODE_HIBC_AZTEC,       SILVER_F,          -1,   4,           0,  36,           0,             1,  110 },
    { 113, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 114, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 115, BARCODE_DOTCODE,                 0,           0,  -1,           0, 200,           0,             1,  900 },
    { 116, BARCODE_HANXIN,                  0,          -1,   5,           0,  84, ZINT_FULL_MULTIBYTE,     1, 7827 },
    { 117, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 118, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 119, BARCODE_MAILMARK_2D,    RUBIDIUM_F,           0,  -1,           0,  30, DM_SQUARE | DM_ISO_144, 28,   90 },
    { 120, BARCODE_UPU_S10,           KRSET_F,           0,  -1,           0,  -1,           0,            12,   13 },
    { 121, BARCODE_MAILMARK_4S,    RUBIDIUM_F,           0,  -1,           0,  -1,           0,            14,   26 },
    { 122, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 123, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 124, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 125, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 126, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 127, /*symbology*/ -1,  /*sane_flag*/ 0, /*opt_1*/ 0,  -1, /*opt_2*/ 0,  -1, /*opt_3*/ 0,     /*len*/ 0,   -1 },
    { 128, BARCODE_AZRUNE,             NEON_F,           0,  -1,           0,  -1,           0,             1,    3 },
    { 129, BARCODE_CODE32,             NEON_F,           0,  -1,           0,  -1,           0,             1,    8 },
    { 130, BARCODE_EANX_CC,      SODIUM_PLS_F,           0,   2,           0,  -1,           0,             1,  338 },
    { 131, BARCODE_GS1_128_CC,    ASCII_PRT_F,           0,   3,           0,  -1,           0,             1, 2361 },
    { 132, BARCODE_DBAR_OMN_CC,        NEON_F,           0,   2,           0,  -1,           0,             1,  338 },
    { 133, BARCODE_DBAR_LTD_CC,        NEON_F,           0,   2,           0,  -1,           0,             1,  338 },
    { 134, BARCODE_DBAR_EXP_CC,   ASCII_PRT_F,           0,   2,           0,  -1,           0,             1,  338 },
    { 135, BARCODE_UPCA_CC,      SODIUM_PLS_F,           0,   2,           0,  -1,           0,             1,  338 },
    { 136, BARCODE_UPCE_CC,      SODIUM_PLS_F,           0,   2,           0,  -1,           0,             1,  338 },
    { 137, BARCODE_DBAR_STK_CC,        NEON_F,           0,   2,           0,  -1,           0,             1,  338 },
    { 138, BARCODE_DBAR_OMNSTK_CC,     NEON_F,           0,   2,           0,  -1,           0,             1,  338 },
    { 139, BARCODE_DBAR_EXPSTK_CC, ASCII_PRT_F,          0,   2,           0,  11,           0,             1,  338 },
    { 140, BARCODE_CHANNEL,            NEON_F,           0,  -1,           3,   8,           0,             1,    7 },
    { 141, BARCODE_CODEONE,                 0,           0,  -1,           0,  10,           0,             1, 3550 },
    { 142, BARCODE_GRIDMATRIX,              0,           0,   5,           0,  13, ZINT_FULL_MULTIBYTE,     1, 1751 },
    { 143, BARCODE_UPNQR,                   0,           0,  -1,           0,  -1, ZINT_FULL_MULTIBYTE,     1,  411 },
    { 144, BARCODE_ULTRA,                   0,          -1,   5,           0,   2, ULTRA_COMPRESSION,       1,  504 },
    { 145, BARCODE_RMQR,                    0,          -1,   4,           0,  38,           0,             1,  361 },
};

/* Make sure value `v` is between `min` and `max` */
static int clamp(const int v, const int min, const int max) {
    return v < min ? min : v > max ? max : v;
}

/* Set `symbol` based on `settings[idx]`, returning new length after consuming some `*p_input` (`data`) */
static int set_symbol(struct zint_symbol *symbol, const int idx, const int chk_sane, const int no_eci,
            const unsigned char **p_input, const size_t size) {
    const unsigned char *input = *p_input;
    int length = (int) size;
    const struct settings_item *si;

    assert(idx >= 0 && idx < ZARRAY_SIZE(settings));
    assert(settings[idx].symbology > 0);
    assert(settings[idx].symbology == idx);

    si = settings + idx;

    ZBarcode_Reset(symbol);

    symbol->symbology = si->symbology;
    if (length < si->len_min) {
        return 0;
    }
    /* `input_mode` */
    if (length > si->len_min) {
        symbol->input_mode = (*input++ << 3); /* Note DATA/UNICODE/GS1_MODE must be set by caller */
        length--;
    }
    /* `option_1` */
    if (length > si->len_min && si->option_1_min <= si->option_1_max) { 
        unsigned char ch = *input++;
        if (ch != 0xFF) { /* Special case 255 as default (-1) */
            if (si->option_1_min + 1 == si->option_1_max) { /* Only one in it? */
                symbol->option_1 = (ch & 1) ? si->option_1_min : si->option_1_max; /* Odd/even */
            } else {
                symbol->option_1 = clamp(ch, si->option_1_min, si->option_1_max);
            }
        }
        length--;
    }
    /* `option_2` */
    if (length > si->len_min && si->option_2_min <= si->option_2_max) { 
        if (si->option_2_min + 1 == si->option_2_max) { /* Only one in it? */
            symbol->option_2 = (*input++ & 1) ? si->option_2_min : si->option_2_max; /* Odd/even */
        } else {
            symbol->option_2 = clamp(*input++, si->option_2_min, si->option_2_max);
        }
        length--;
    }
    /* `eci` */
    if (!no_eci) {
        if (length > si->len_min && (ZBarcode_Cap(symbol->symbology, ZINT_CAP_ECI) & ZINT_CAP_ECI)) {
            symbol->eci = *input++;
            /* Avoid invalid/unconvertible ECIs */
            if (symbol->eci == 1) {
                symbol->eci = 899; /* Binary */
            } else if (symbol->eci > 35 && symbol->eci != 170) {
                symbol->eci = 35; /* UTF-32LE */
            } else if (symbol->eci == 2 || symbol->eci == 15 || symbol->eci == 19) {
                symbol->eci++;
            }
            length--;
        }
    }
    /* `option_3` */
    if (length > si->len_min && si->option_3) {
        if (*input++ & 1) { /* Odd/even */
            symbol->option_3 = si->option_3;
        }
        length--;
    }
#ifdef Z_FUZZ_SET_OUTPUT_OPTIONS
    if (length > si->len_min) {
        symbol->output_options = *input++ & OUTPUT_OPTIONS_MASK;
        length--;
    }
#endif

    if (length > si->len_max) {
        return 0;
    }
    if (chk_sane && si->sane_flag && !is_sane(si->sane_flag, input, length)) {
        return 0;
    }

#ifdef Z_FUZZ_DEBUG
    symbol->debug = ZINT_DEBUG_PRINT;
#endif

    *p_input = input;

    assert(length >= si->len_min && length <= si->len_max);

    return length;
}

/* vim: set ts=4 sw=4 et : */
#endif /* Z_FUZZ_H */
