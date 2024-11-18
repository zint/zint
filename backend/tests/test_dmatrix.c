/*
    libzint - the open source barcode library
    Copyright (C) 2019-2024 Robin Stuart <rstuart114@gmail.com>

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

#include "testcommon.h"
#include "../gs1.h"

static void test_large(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int option_2;
        struct zint_structapp structapp;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected_errtxt;
        char *expected_errtxt2;
        char *comment;
    };
    /* ISO/IEC 16022:2006 Table 7 and ISO/IEC 21471:2020 (DMRE) Table 7 */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "1", 3116, 0, 144, 144, "", "", "" },
        /*  1*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "1", 3117, ZINT_ERROR_TOO_LONG, 0, 0, "Error 719: Input length 3117 too long (maximum 3116)", "", "" },
        /*  2*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "1", 10922, ZINT_ERROR_TOO_LONG, 0, 0, "Error 719: Input length 10922 too long (maximum 3116)", "", "Minimal encoding can handle max (10921 + 1) * 6 = 65532 < 65536 (2*16) due to sizeof(previous)" },
        /*  3*/ { BARCODE_DATAMATRIX, -1, { 1, 2, "001001" }, "1", 3108, 0, 144, 144, "", "", "Structured Append 4 codewords overhead == 8 digits" },
        /*  4*/ { BARCODE_DATAMATRIX, -1, { 1, 2, "001001" }, "1", 3109, ZINT_ERROR_TOO_LONG, 0, 0, "Error 729: Input too long, requires too many codewords (maximum 1558)", "Error 520: Input too long, requires too many codewords (maximum 1558)", "" },
        /*  5*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "A", 2335, 0, 144, 144, "", "", "" },
        /*  6*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "A", 2336, ZINT_ERROR_TOO_LONG, 0, 0, "Error 729: Input too long, requires too many codewords (maximum 1558)", "Error 523: Input too long, requires 1560 codewords (maximum 1558)", "" },
        /*  7*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "\200", 1556, 0, 144, 144, "", "", "Spec says 1555 but 1556 correct as only single byte count of 0 required" },
        /*  8*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "\200", 1557, ZINT_ERROR_TOO_LONG, 0, 0, "Error 729: Input too long, requires too many codewords (maximum 1558)", "Error 520: Input too long, requires too many codewords (maximum 1558)", "" },
        /*  9*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "\001", 1558, 0, 144, 144, "", "", "" },
        /* 10*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "\001", 1559, ZINT_ERROR_TOO_LONG, 0, 0, "Error 729: Input too long, requires too many codewords (maximum 1558)", "Error 520: Input too long, requires too many codewords (maximum 1558)", "" },
        /* 11*/ { BARCODE_HIBC_DM, -1, { 0, 0, "" }, "1", 110, 0, 32, 32, "", "", "" },
        /* 12*/ { BARCODE_HIBC_DM, -1, { 0, 0, "" }, "1", 111, ZINT_ERROR_TOO_LONG, 0, 0, "Error 202: Input length 111 too long for HIBC LIC (maximum 110)", "", "" },
        /* 13*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "1", 6, 0, 10, 10, "", "", "" },
        /* 14*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "1", 7, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 1, requires 4 codewords (maximum 3)", "", "" },
        /* 15*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "A", 3, 0, 10, 10, "", "", "" },
        /* 16*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "A", 4, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 1, requires 4 codewords (maximum 3)", "", "" },
        /* 17*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "\200", 1, 0, 10, 10, "", "", "" },
        /* 18*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "\200", 2, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 1, requires 4 codewords (maximum 3)", "", "" },
        /* 19*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "1", 10, 0, 12, 12, "", "", "" },
        /* 20*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "1", 11, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 2, requires 6 codewords (maximum 5)", "", "" },
        /* 21*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "A", 6, 0, 12, 12, "", "", "" },
        /* 22*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "A", 7, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 2, requires 6 codewords (maximum 5)", "", "" },
        /* 23*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "\200", 3, 0, 12, 12, "", "", "" },
        /* 24*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "\200", 4, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 2, requires 6 codewords (maximum 5)", "", "" },
        /* 25*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "1", 16, 0, 14, 14, "", "", "" },
        /* 26*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "1", 17, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 3, requires 9 codewords (maximum 8)", "", "" },
        /* 27*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "A", 10, 0, 14, 14, "", "", "" },
        /* 28*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "A", 11, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 3, requires 9 codewords (maximum 8)", "Error 522: Input too long for Version 3, requires 10 codewords (maximum 8)", "" },
        /* 29*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "\200", 6, 0, 14, 14, "", "", "" },
        /* 30*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "\200", 7, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 3, requires 9 codewords (maximum 8)", "", "" },
        /* 31*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "1", 24, 0, 16, 16, "", "", "" },
        /* 32*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "1", 25, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 4, requires 13 codewords (maximum 12)", "", "" },
        /* 33*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "A", 16, 0, 16, 16, "", "", "" },
        /* 34*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "A", 17, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 4, requires 13 codewords (maximum 12)", "Error 522: Input too long for Version 4, requires 14 codewords (maximum 12)", "" },
        /* 35*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "\200", 10, 0, 16, 16, "", "", "" },
        /* 36*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "\200", 11, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 4, requires 13 codewords (maximum 12)", "", "" },
        /* 37*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "1", 36, 0, 18, 18, "", "", "" },
        /* 38*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "1", 37, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 5, requires 19 codewords (maximum 18)", "", "" },
        /* 39*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "A", 25, 0, 18, 18, "", "", "" },
        /* 40*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "A", 26, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 5, requires 19 codewords (maximum 18)", "Error 522: Input too long for Version 5, requires 20 codewords (maximum 18)", "" },
        /* 41*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "\200", 16, 0, 18, 18, "", "", "" },
        /* 42*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "\200", 17, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 5, requires 19 codewords (maximum 18)", "", "" },
        /* 43*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "1", 44, 0, 20, 20, "", "", "" },
        /* 44*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "1", 45, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 6, requires 23 codewords (maximum 22)", "", "" },
        /* 45*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "A", 31, 0, 20, 20, "", "", "" },
        /* 46*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "A", 32, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 6, requires 23 codewords (maximum 22)", "Error 522: Input too long for Version 6, requires 24 codewords (maximum 22)", "" },
        /* 47*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "\200", 20, 0, 20, 20, "", "", "" },
        /* 48*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "\200", 21, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 6, requires 23 codewords (maximum 22)", "", "" },
        /* 49*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "1", 60, 0, 22, 22, "", "", "" },
        /* 50*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "1", 61, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 7, requires 31 codewords (maximum 30)", "", "" },
        /* 51*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "A", 43, 0, 22, 22, "", "", "" },
        /* 52*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "A", 44, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 7, requires 31 codewords (maximum 30)", "Error 522: Input too long for Version 7, requires 32 codewords (maximum 30)", "" },
        /* 53*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "\200", 28, 0, 22, 22, "", "", "" },
        /* 54*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "\200", 29, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 7, requires 31 codewords (maximum 30)", "", "" },
        /* 55*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "1", 72, 0, 24, 24, "", "", "" },
        /* 56*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "1", 73, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 8, requires 37 codewords (maximum 36)", "", "" },
        /* 57*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "A", 52, 0, 24, 24, "", "", "" },
        /* 58*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "A", 53, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 8, requires 37 codewords (maximum 36)", "Error 522: Input too long for Version 8, requires 38 codewords (maximum 36)", "" },
        /* 59*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "\200", 34, 0, 24, 24, "", "", "" },
        /* 60*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "\200", 35, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 8, requires 37 codewords (maximum 36)", "", "" },
        /* 61*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "1", 88, 0, 26, 26, "", "", "" },
        /* 62*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "1", 89, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 9, requires 45 codewords (maximum 44)", "", "" },
        /* 63*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "A", 64, 0, 26, 26, "", "", "" },
        /* 64*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "A", 65, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 9, requires 45 codewords (maximum 44)", "Error 522: Input too long for Version 9, requires 46 codewords (maximum 44)", "" },
        /* 65*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "\200", 42, 0, 26, 26, "", "", "" },
        /* 66*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "\200", 43, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 9, requires 45 codewords (maximum 44)", "", "" },
        /* 67*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "1", 124, 0, 32, 32, "", "", "" },
        /* 68*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "1", 125, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 10, requires 63 codewords (maximum 62)", "", "" },
        /* 69*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "A", 91, 0, 32, 32, "", "", "" },
        /* 70*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "A", 92, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 10, requires 63 codewords (maximum 62)", "Error 522: Input too long for Version 10, requires 64 codewords (maximum 62)", "" },
        /* 71*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "\200", 60, 0, 32, 32, "", "", "" },
        /* 72*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "\200", 61, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 10, requires 63 codewords (maximum 62)", "", "" },
        /* 73*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "1", 172, 0, 36, 36, "", "", "" },
        /* 74*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "1", 173, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 11, requires 87 codewords (maximum 86)", "", "" },
        /* 75*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "A", 127, 0, 36, 36, "", "", "" },
        /* 76*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "A", 128, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 11, requires 87 codewords (maximum 86)", "Error 522: Input too long for Version 11, requires 88 codewords (maximum 86)", "" },
        /* 77*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "\200", 84, 0, 36, 36, "", "", "" },
        /* 78*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "\200", 85, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 11, requires 87 codewords (maximum 86)", "", "" },
        /* 79*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "1", 228, 0, 40, 40, "", "", "" },
        /* 80*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "1", 229, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 12, requires 115 codewords (maximum 114)", "", "" },
        /* 81*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "A", 169, 0, 40, 40, "", "", "" },
        /* 82*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "A", 170, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 12, requires 115 codewords (maximum 114)", "Error 522: Input too long for Version 12, requires 116 codewords (maximum 114)", "" },
        /* 83*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "\200", 112, 0, 40, 40, "", "", "" },
        /* 84*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "\200", 113, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 12, requires 115 codewords (maximum 114)", "", "" },
        /* 85*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "1", 288, 0, 44, 44, "", "", "" },
        /* 86*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "1", 289, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 13, requires 145 codewords (maximum 144)", "", "" },
        /* 87*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "A", 214, 0, 44, 44, "", "", "" },
        /* 88*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "A", 215, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 13, requires 145 codewords (maximum 144)", "Error 522: Input too long for Version 13, requires 146 codewords (maximum 144)", "" },
        /* 89*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "\200", 142, 0, 44, 44, "", "", "" },
        /* 90*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "\200", 143, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 13, requires 145 codewords (maximum 144)", "", "" },
        /* 91*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "1", 348, 0, 48, 48, "", "", "" },
        /* 92*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "1", 349, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 14, requires 175 codewords (maximum 174)", "", "" },
        /* 93*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "A", 259, 0, 48, 48, "", "", "" },
        /* 94*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "A", 260, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 14, requires 175 codewords (maximum 174)", "Error 522: Input too long for Version 14, requires 176 codewords (maximum 174)", "" },
        /* 95*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "\200", 172, 0, 48, 48, "", "", "" },
        /* 96*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "\200", 173, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 14, requires 175 codewords (maximum 174)", "", "" },
        /* 97*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "1", 408, 0, 52, 52, "", "", "" },
        /* 98*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "1", 409, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 15, requires 205 codewords (maximum 204)", "", "" },
        /* 99*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "A", 304, 0, 52, 52, "", "", "" },
        /*100*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "A", 305, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 15, requires 205 codewords (maximum 204)", "Error 522: Input too long for Version 15, requires 206 codewords (maximum 204)", "" },
        /*101*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "\200", 202, 0, 52, 52, "", "", "" },
        /*102*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "\200", 203, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 15, requires 205 codewords (maximum 204)", "", "" },
        /*103*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "1", 560, 0, 64, 64, "", "", "" },
        /*104*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "1", 561, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 16, requires 281 codewords (maximum 280)", "", "" },
        /*105*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "A", 418, 0, 64, 64, "", "", "" },
        /*106*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "A", 419, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 16, requires 281 codewords (maximum 280)", "Error 522: Input too long for Version 16, requires 282 codewords (maximum 280)", "" },
        /*107*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "\200", 278, 0, 64, 64, "", "", "Spec says 277 but 278 correct as only single byte count of 0 required" },
        /*108*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "\200", 279, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 16, requires 281 codewords (maximum 280)", "", "" },
        /*109*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "1", 736, 0, 72, 72, "", "", "" },
        /*110*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "1", 737, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 17, requires 369 codewords (maximum 368)", "", "" },
        /*111*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "A", 550, 0, 72, 72, "", "", "" },
        /*112*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "A", 551, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 17, requires 369 codewords (maximum 368)", "Error 522: Input too long for Version 17, requires 370 codewords (maximum 368)", "" },
        /*113*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "\200", 366, 0, 72, 72, "", "", "Spec says 365 but 366 correct as only single byte count of 0 required" },
        /*114*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "\200", 367, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 17, requires 369 codewords (maximum 368)", "", "" },
        /*115*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "1", 912, 0, 80, 80, "", "", "" },
        /*116*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "1", 913, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 18, requires 457 codewords (maximum 456)", "", "" },
        /*117*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "A", 682, 0, 80, 80, "", "", "" },
        /*118*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "A", 683, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 18, requires 457 codewords (maximum 456)", "Error 522: Input too long for Version 18, requires 458 codewords (maximum 456)", "" },
        /*119*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "\200", 454, 0, 80, 80, "", "", "Spec says 453 but 454 correct as only single byte count of 0 required" },
        /*120*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "\200", 455, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 18, requires 457 codewords (maximum 456)", "", "" },
        /*121*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "1", 1152, 0, 88, 88, "", "", "" },
        /*122*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "1", 1153, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 19, requires 577 codewords (maximum 576)", "", "" },
        /*123*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "A", 862, 0, 88, 88, "", "", "" },
        /*124*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "A", 863, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 19, requires 577 codewords (maximum 576)", "Error 522: Input too long for Version 19, requires 578 codewords (maximum 576)", "" },
        /*125*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "\200", 574, 0, 88, 88, "", "", "Spec says 573 but 574 correct as only single byte count of 0 required" },
        /*126*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "\200", 575, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 19, requires 577 codewords (maximum 576)", "", "" },
        /*127*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "1", 1392, 0, 96, 96, "", "", "" },
        /*128*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "1", 1393, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 20, requires 697 codewords (maximum 696)", "", "" },
        /*129*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "A", 1042, 0, 96, 96, "", "", "" },
        /*130*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "A", 1043, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 20, requires 697 codewords (maximum 696)", "Error 522: Input too long for Version 20, requires 698 codewords (maximum 696)", "" },
        /*131*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "\200", 694, 0, 96, 96, "", "", "Spec says 693 but 694 correct as only single byte count of 0 required" },
        /*132*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "\200", 695, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 20, requires 697 codewords (maximum 696)", "", "" },
        /*133*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "1", 1632, 0, 104, 104, "", "", "" },
        /*134*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "1", 1633, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 21, requires 817 codewords (maximum 816)", "", "" },
        /*135*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "A", 1222, 0, 104, 104, "", "", "" },
        /*136*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "A", 1223, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 21, requires 817 codewords (maximum 816)", "Error 522: Input too long for Version 21, requires 818 codewords (maximum 816)", "" },
        /*137*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "\200", 814, 0, 104, 104, "", "", "Spec says 813 but 814 correct as only single byte count of 0 required" },
        /*138*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "\200", 815, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 21, requires 817 codewords (maximum 816)", "", "" },
        /*139*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "1", 2100, 0, 120, 120, "", "", "" },
        /*140*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "1", 2101, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 22, requires 1051 codewords (maximum 1050)", "", "" },
        /*141*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "A", 1573, 0, 120, 120, "", "", "" },
        /*142*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "A", 1574, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 22, requires 1051 codewords (maximum 1050)", "Error 522: Input too long for Version 22, requires 1052 codewords (maximum 1050)", "" },
        /*143*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "\200", 1048, 0, 120, 120, "", "", "Spec says 1047 but 1048 correct as only single byte count of 0 required" },
        /*144*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "\200", 1049, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 22, requires 1051 codewords (maximum 1050)", "", "" },
        /*145*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "1", 2608, 0, 132, 132, "", "", "" },
        /*146*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "1", 2609, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 23, requires 1305 codewords (maximum 1304)", "", "" },
        /*147*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "A", 1954, 0, 132, 132, "", "", "" },
        /*148*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "A", 1955, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 23, requires 1305 codewords (maximum 1304)", "Error 522: Input too long for Version 23, requires 1306 codewords (maximum 1304)", "" },
        /*149*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "\200", 1302, 0, 132, 132, "", "", "Spec says 1301 but 1302 correct as only single byte count of 0 required" },
        /*150*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "\200", 1303, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 23, requires 1305 codewords (maximum 1304)", "", "" },
        /*151*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "1", 3116, 0, 144, 144, "", "", "" },
        /*152*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "1", 3117, ZINT_ERROR_TOO_LONG, 0, 0, "Error 719: Input length 3117 too long (maximum 3116)", "", "" },
        /*153*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "A", 2335, 0, 144, 144, "", "", "" },
        /*154*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "A", 2336, ZINT_ERROR_TOO_LONG, 0, 0, "Error 729: Input too long, requires too many codewords (maximum 1558)", "Error 522: Input too long for Version 24, requires 1560 codewords (maximum 1558)", "" },
        /*155*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "\200", 1556, 0, 144, 144, "", "", "Spec says 1555 but 1556 correct as only single byte count of 0 required" },
        /*156*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "\200", 1557, ZINT_ERROR_TOO_LONG, 0, 0, "Error 729: Input too long, requires too many codewords (maximum 1558)", "Error 520: Input too long, requires too many codewords (maximum 1558)", "" },
        /*157*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "1", 10, 0, 8, 18, "", "", "" },
        /*158*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "1", 11, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 25, requires 6 codewords (maximum 5)", "", "" },
        /*159*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "A", 6, 0, 8, 18, "", "", "" },
        /*160*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "A", 7, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 25, requires 6 codewords (maximum 5)", "", "" },
        /*161*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "\200", 3, 0, 8, 18, "", "", "" },
        /*162*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "\200", 4, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 25, requires 6 codewords (maximum 5)", "", "" },
        /*163*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "1", 20, 0, 8, 32, "", "", "" },
        /*164*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "1", 21, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 26, requires 11 codewords (maximum 10)", "", "" },
        /*165*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "A", 13, 0, 8, 32, "", "", "" },
        /*166*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "A", 14, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 26, requires 11 codewords (maximum 10)", "Error 522: Input too long for Version 26, requires 12 codewords (maximum 10)", "" },
        /*167*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "\200", 8, 0, 8, 32, "", "", "" },
        /*168*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "\200", 9, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 26, requires 11 codewords (maximum 10)", "", "" },
        /*169*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "1", 32, 0, 12, 26, "", "", "" },
        /*170*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "1", 33, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 27, requires 17 codewords (maximum 16)", "", "" },
        /*171*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "A", 22, 0, 12, 26, "", "", "" },
        /*172*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "A", 23, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 27, requires 17 codewords (maximum 16)", "Error 522: Input too long for Version 27, requires 18 codewords (maximum 16)", "" },
        /*173*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "\200", 14, 0, 12, 26, "", "", "" },
        /*174*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "\200", 15, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 27, requires 17 codewords (maximum 16)", "", "" },
        /*175*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "1", 44, 0, 12, 36, "", "", "" },
        /*176*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "1", 45, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 28, requires 23 codewords (maximum 22)", "", "" },
        /*177*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "A", 31, 0, 12, 36, "", "", "" },
        /*178*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "A", 32, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 28, requires 23 codewords (maximum 22)", "Error 522: Input too long for Version 28, requires 24 codewords (maximum 22)", "" },
        /*179*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "\200", 20, 0, 12, 36, "", "", "" },
        /*180*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "\200", 21, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 28, requires 23 codewords (maximum 22)", "", "" },
        /*181*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "1", 64, 0, 16, 36, "", "", "" },
        /*182*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "1", 65, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 29, requires 33 codewords (maximum 32)", "", "" },
        /*183*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "A", 46, 0, 16, 36, "", "", "" },
        /*184*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "A", 47, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 29, requires 33 codewords (maximum 32)", "Error 522: Input too long for Version 29, requires 34 codewords (maximum 32)", "" },
        /*185*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "\200", 30, 0, 16, 36, "", "", "" },
        /*186*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "\200", 31, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 29, requires 33 codewords (maximum 32)", "", "" },
        /*187*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "1", 98, 0, 16, 48, "", "", "" },
        /*188*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "1", 99, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 30, requires 50 codewords (maximum 49)", "", "" },
        /*189*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "A", 72, 0, 16, 48, "", "", "" },
        /*190*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "A", 73, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 30, requires 50 codewords (maximum 49)", "", "" },
        /*191*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "\200", 47, 0, 16, 48, "", "", "" },
        /*192*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "\200", 48, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 30, requires 50 codewords (maximum 49)", "", "" },
        /*193*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "1", 36, 0, 8, 48, "", "", "" },
        /*194*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "1", 37, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 31, requires 19 codewords (maximum 18)", "", "" },
        /*195*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "A", 25, 0, 8, 48, "", "", "" },
        /*196*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "A", 26, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 31, requires 19 codewords (maximum 18)", "Error 522: Input too long for Version 31, requires 20 codewords (maximum 18)", "" },
        /*197*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "\200", 16, 0, 8, 48, "", "", "" },
        /*198*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "\200", 17, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 31, requires 19 codewords (maximum 18)", "", "" },
        /*199*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "1", 48, 0, 8, 64, "", "", "" },
        /*200*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "1", 49, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 32, requires 25 codewords (maximum 24)", "", "" },
        /*201*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "A", 34, 0, 8, 64, "", "", "" },
        /*202*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "A", 35, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 32, requires 25 codewords (maximum 24)", "Error 522: Input too long for Version 32, requires 26 codewords (maximum 24)", "" },
        /*203*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "\200", 22, 0, 8, 64, "", "", "" },
        /*204*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "\200", 23, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 32, requires 25 codewords (maximum 24)", "", "" },
        /*205*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "1", 64, 0, 8, 80, "", "", "" },
        /*206*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "1", 65, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 33, requires 33 codewords (maximum 32)", "", "" },
        /*207*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "A", 46, 0, 8, 80, "", "", "" },
        /*208*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "A", 47, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 33, requires 33 codewords (maximum 32)", "Error 522: Input too long for Version 33, requires 34 codewords (maximum 32)", "" },
        /*209*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "\200", 30, 0, 8, 80, "", "", "" },
        /*210*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "\200", 31, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 33, requires 33 codewords (maximum 32)", "", "" },
        /*211*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "1", 76, 0, 8, 96, "", "", "" },
        /*212*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "1", 77, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 34, requires 39 codewords (maximum 38)", "", "" },
        /*213*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "A", 55, 0, 8, 96, "", "", "" },
        /*214*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "A", 56, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 34, requires 39 codewords (maximum 38)", "Error 522: Input too long for Version 34, requires 40 codewords (maximum 38)", "" },
        /*215*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "\200", 36, 0, 8, 96, "", "", "" },
        /*216*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "\200", 37, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 34, requires 39 codewords (maximum 38)", "", "" },
        /*217*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "1", 98, 0, 8, 120, "", "", "" },
        /*218*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "1", 99, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 35, requires 50 codewords (maximum 49)", "", "" },
        /*219*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "A", 72, 0, 8, 120, "", "", "" },
        /*220*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "A", 73, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 35, requires 50 codewords (maximum 49)", "", "" },
        /*221*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "\200", 47, 0, 8, 120, "", "", "" },
        /*222*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "\200", 48, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 35, requires 50 codewords (maximum 49)", "", "" },
        /*223*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "1", 126, 0, 8, 144, "", "", "" },
        /*224*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "1", 127, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 36, requires 64 codewords (maximum 63)", "", "" },
        /*225*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "A", 93, 0, 8, 144, "", "", "" },
        /*226*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "A", 94, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 36, requires 64 codewords (maximum 63)", "", "" },
        /*227*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "\200", 61, 0, 8, 144, "", "", "" },
        /*228*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "\200", 62, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 36, requires 64 codewords (maximum 63)", "", "" },
        /*229*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "1", 86, 0, 12, 64, "", "", "" },
        /*230*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "1", 87, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 37, requires 44 codewords (maximum 43)", "", "" },
        /*231*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "A", 63, 0, 12, 64, "", "", "" },
        /*232*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "A", 64, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 37, requires 44 codewords (maximum 43)", "", "" },
        /*233*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "\200", 41, 0, 12, 64, "", "", "" },
        /*234*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "\200", 42, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 37, requires 44 codewords (maximum 43)", "", "" },
        /*235*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "1", 128, 0, 12, 88, "", "", "" },
        /*236*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "1", 129, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 38, requires 65 codewords (maximum 64)", "", "" },
        /*237*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "A", 94, 0, 12, 88, "", "", "" },
        /*238*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "A", 95, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 38, requires 65 codewords (maximum 64)", "Error 522: Input too long for Version 38, requires 66 codewords (maximum 64)", "" },
        /*239*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "\200", 62, 0, 12, 88, "", "", "" },
        /*240*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "\200", 63, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 38, requires 65 codewords (maximum 64)", "", "" },
        /*241*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "1", 124, 0, 16, 64, "", "", "" },
        /*242*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "1", 125, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 39, requires 63 codewords (maximum 62)", "", "" },
        /*243*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "A", 91, 0, 16, 64, "", "", "" },
        /*244*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "A", 92, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 39, requires 63 codewords (maximum 62)", "Error 522: Input too long for Version 39, requires 64 codewords (maximum 62)", "" },
        /*245*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "\200", 60, 0, 16, 64, "", "", "" },
        /*246*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "\200", 61, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 39, requires 63 codewords (maximum 62)", "", "" },
        /*247*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "1", 88, 0, 20, 36, "", "", "" },
        /*248*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "1", 89, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 40, requires 45 codewords (maximum 44)", "", "" },
        /*249*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "A", 64, 0, 20, 36, "", "", "" },
        /*250*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "A", 65, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 40, requires 45 codewords (maximum 44)", "Error 522: Input too long for Version 40, requires 46 codewords (maximum 44)", "" },
        /*251*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "\200", 42, 0, 20, 36, "", "", "" },
        /*252*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "\200", 43, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 40, requires 45 codewords (maximum 44)", "", "" },
        /*253*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "1", 112, 0, 20, 44, "", "", "" },
        /*254*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "1", 113, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 41, requires 57 codewords (maximum 56)", "", "" },
        /*255*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "A", 82, 0, 20, 44, "", "", "" },
        /*256*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "A", 83, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 41, requires 57 codewords (maximum 56)", "Error 522: Input too long for Version 41, requires 58 codewords (maximum 56)", "" },
        /*257*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "\200", 54, 0, 20, 44, "", "", "" },
        /*258*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "\200", 55, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 41, requires 57 codewords (maximum 56)", "", "" },
        /*259*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "1", 168, 0, 20, 64, "", "", "Spec says 186 but typo" },
        /*260*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "1", 169, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 42, requires 85 codewords (maximum 84)", "", "" },
        /*261*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "A", 124, 0, 20, 64, "", "", "" },
        /*262*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "A", 125, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 42, requires 85 codewords (maximum 84)", "Error 522: Input too long for Version 42, requires 86 codewords (maximum 84)", "" },
        /*263*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "\200", 82, 0, 20, 64, "", "", "" },
        /*264*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "\200", 83, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 42, requires 85 codewords (maximum 84)", "", "" },
        /*265*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "1", 144, 0, 22, 48, "", "", "" },
        /*266*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "1", 145, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 43, requires 73 codewords (maximum 72)", "", "" },
        /*267*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "A", 106, 0, 22, 48, "", "", "" },
        /*268*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "A", 107, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 43, requires 73 codewords (maximum 72)", "Error 522: Input too long for Version 43, requires 74 codewords (maximum 72)", "" },
        /*269*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "\200", 70, 0, 22, 48, "", "", "" },
        /*270*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "\200", 71, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 43, requires 73 codewords (maximum 72)", "", "" },
        /*271*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "1", 160, 0, 24, 48, "", "", "" },
        /*272*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "1", 161, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 44, requires 81 codewords (maximum 80)", "", "" },
        /*273*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "A", 118, 0, 24, 48, "", "", "" },
        /*274*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "A", 119, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 44, requires 81 codewords (maximum 80)", "Error 522: Input too long for Version 44, requires 82 codewords (maximum 80)", "" },
        /*275*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "\200", 78, 0, 24, 48, "", "", "" },
        /*276*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "\200", 79, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 44, requires 81 codewords (maximum 80)", "", "" },
        /*277*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "1", 216, 0, 24, 64, "", "", "" },
        /*278*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "1", 217, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 45, requires 109 codewords (maximum 108)", "", "" },
        /*279*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "A", 160, 0, 24, 64, "", "", "" },
        /*280*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "A", 161, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 45, requires 109 codewords (maximum 108)", "Error 522: Input too long for Version 45, requires 110 codewords (maximum 108)", "" },
        /*281*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "\200", 106, 0, 24, 64, "", "", "" },
        /*282*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "\200", 107, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 45, requires 109 codewords (maximum 108)", "", "" },
        /*283*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "1", 140, 0, 26, 40, "", "", "" },
        /*284*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "1", 141, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 46, requires 71 codewords (maximum 70)", "", "" },
        /*285*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "A", 103, 0, 26, 40, "", "", "" },
        /*286*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "A", 104, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 46, requires 71 codewords (maximum 70)", "Error 522: Input too long for Version 46, requires 72 codewords (maximum 70)", "" },
        /*287*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "\200", 68, 0, 26, 40, "", "", "" },
        /*288*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "\200", 69, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 46, requires 71 codewords (maximum 70)", "", "" },
        /*289*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "1", 180, 0, 26, 48, "", "", "" },
        /*290*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "1", 181, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 47, requires 91 codewords (maximum 90)", "", "" },
        /*291*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "A", 133, 0, 26, 48, "", "", "" },
        /*292*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "A", 134, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 47, requires 91 codewords (maximum 90)", "Error 522: Input too long for Version 47, requires 92 codewords (maximum 90)", "" },
        /*293*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "\200", 88, 0, 26, 48, "", "", "" },
        /*294*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "\200", 89, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 47, requires 91 codewords (maximum 90)", "", "" },
        /*295*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "1", 236, 0, 26, 64, "", "", "" },
        /*296*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "1", 237, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 48, requires 119 codewords (maximum 118)", "", "" },
        /*297*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "A", 175, 0, 26, 64, "", "", "" },
        /*298*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "A", 176, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 48, requires 119 codewords (maximum 118)", "Error 522: Input too long for Version 48, requires 120 codewords (maximum 118)", "" },
        /*299*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "\200", 116, 0, 26, 64, "", "", "" },
        /*300*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "\200", 117, ZINT_ERROR_TOO_LONG, 0, 0, "Error 522: Input too long for Version 48, requires 119 codewords (maximum 118)", "", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[ZINT_MAX_DATA_LEN];
    char escaped[64];

    testStartSymbol("test_large", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);

        if (p_ctx->generate) {
            char errtxt[sizeof(symbol->errtxt)];
            strcpy(errtxt, symbol->errtxt);
            printf("        /*%3d*/ { %s, %d, { %d, %d, \"%s\" }, \"%s\", %d, %s, %d, %d, \"%s\",",
                        i, testUtilBarcodeName(data[i].symbology), data[i].option_2,
                        data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                        testUtilEscape(data[i].pattern, (int) strlen(data[i].pattern), escaped, sizeof(escaped)), data[i].length,
                        testUtilErrorName(ret), symbol->rows, symbol->width, errtxt);
            symbol->input_mode |= FAST_MODE;
            (void) ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
            printf(" \"%s\", \"%s\" },\n", strcmp(errtxt, symbol->errtxt) != 0 ? symbol->errtxt : "", data[i].comment);
        } else {
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
            assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);
            assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
            }

            symbol->input_mode |= FAST_MODE;
            ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
            assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);

            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
            } else {
                if (data[i].expected_errtxt2[0]) {
                    assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt2), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt2);
                } else {
                    assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

/* Note need ZINT_SANITIZE set for these */
static void test_buffer(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int eci;
        int input_mode;
        int output_options;
        char *data;
        int ret;
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { 16383, UNICODE_MODE, READER_INIT, "1", 0, "" },
        /*  1*/ { 3, UNICODE_MODE, 0, "000106j 05 Galeria A NaÃ§Ã£o0000000000", 0, "From Okapi, consecutive use of upper shift; #176" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_buffer", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_DATAMATRIX, data[i].input_mode, data[i].eci, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_options(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int option_1;
        int option_2;
        int option_3;
        int output_options;
        struct zint_structapp structapp;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "1", 0, 10, 10, "" },
        /*  1*/ { BARCODE_DATAMATRIX, -1, 2, -1, -1, -1, { 0, 0, "" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 524: Older Data Matrix standards are no longer supported" },
        /*  2*/ { BARCODE_DATAMATRIX, -1, -1, 1, -1, -1, { 0, 0, "" }, "1", 0, 10, 10, "" },
        /*  3*/ { BARCODE_DATAMATRIX, -1, -1, 2, -1, -1, { 0, 0, "" }, "1", 0, 12, 12, "" },
        /*  4*/ { BARCODE_DATAMATRIX, -1, -1, 48, -1, -1, { 0, 0, "" }, "1", 0, 26, 64, "" },
        /*  5*/ { BARCODE_DATAMATRIX, -1, -1, 49, -1, -1, { 0, 0, "" }, "1", 0, 10, 10, "" }, /* Ignored */
        /*  6*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "____", 0, 12, 12, "" }, /* 4 data */
        /*  7*/ { BARCODE_DATAMATRIX, -1, -1, 1, -1, -1, { 0, 0, "" }, "____", ZINT_ERROR_TOO_LONG, -1, -1, "Error 522: Input too long for Version 1, requires 4 codewords (maximum 3)" },
        /*  8*/ { BARCODE_DATAMATRIX, -1, -1, 25, -1, -1, { 0, 0, "" }, "____", 0, 8, 18, "" },
        /*  9*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "__________", 0, 8, 32, "" }, /* 10 data */
        /* 10*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "__________", 0, 8, 32, "" },
        /* 11*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE | DM_ISO_144, -1, { 0, 0, "" }, "__________", 0, 8, 32, "" },
        /* 12*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "__________", 0, 16, 16, "" },
        /* 13*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE | DM_ISO_144, -1, { 0, 0, "" }, "__________", 0, 16, 16, "" },
        /* 14*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_______________", 0, 12, 26, "" }, /* 15 data */
        /* 15*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "_______________", 0, 12, 26, "" },
        /* 16*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "_______________", 0, 18, 18, "" },
        /* 17*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "__________________", 0, 18, 18, "" }, /* 18 data */
        /* 18*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "___________________", 0, 20, 20, "" }, /* 19 data */
        /* 19*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_____________________", 0, 20, 20, "" }, /* 21 data */
        /* 20*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_______________________", 0, 22, 22, "" }, /* 23 data */
        /* 21*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "_______________________", 0, 8, 64, "" },
        /* 22*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "_______________________", 0, 22, 22, "" },
        /* 23*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_______________________________", 0, 16, 36, "" }, /* 31 data */
        /* 24*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "_______________________________", 0, 16, 36, "" },
        /* 25*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "_______________________________", 0, 24, 24, "" },
        /* 26*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_____________________________________", 0, 26, 26, "" }, /* 37 data */
        /* 27*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "_____________________________________", 0, 8, 96, "" },
        /* 28*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "_____________________________________", 0, 26, 26, "" },
        /* 29*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_______________________________________", 0, 26, 26, "" }, /* 39 data */
        /* 30*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "_______________________________________", 0, 12, 64, "" },
        /* 31*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "_______________________________________", 0, 26, 26, "" },
        /* 32*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "___________________________________________", 0, 26, 26, "" }, /* 43 data */
        /* 33*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "___________________________________________", 0, 12, 64, "" },
        /* 34*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "___________________________________________", 0, 26, 26, "" },
        /* 35*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "____________________________________________", 0, 26, 26, "" }, /* 44 data */
        /* 36*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_____________________________________________", 0, 16, 48, "" }, /* 45 data */
        /* 37*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "_____________________________________________", 0, 16, 48, "" },
        /* 38*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "_____________________________________________", 0, 32, 32, "" },
        /* 39*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_________________________________________________", 0, 16, 48, "" }, /* 49 data */
        /* 40*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "_________________________________________________", 0, 16, 48, "" },
        /* 41*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "_________________________________________________", 0, 32, 32, "" },
        /* 42*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "__________________________________________________", 0, 32, 32, "" }, /* 50 data */
        /* 43*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "__________________________________________________", 0, 20, 44, "" },
        /* 44*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "__________________________________________________", 0, 32, 32, "" },
        /* 45*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU", 0, 32, 32, "" }, /* 51 data */
        /* 46*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU", 0, 20, 44, "" },
        /* 47*/ { BARCODE_DATAMATRIX, -1, -1, -1, 9999, -1, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU", 0, 32, 32, "" }, /* Ignored */
        /* 48*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_____________________________________________________________", 0, 32, 32, "" }, /* 61 data */
        /* 49*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "______________________________________________________________", 0, 32, 32, "" }, /* 62 data */
        /* 50*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_______________________________________________________________", 0, 36, 36, "" }, /* 63 data */
        /* 51*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "_______________________________________________________________", 0, 8, 144, "" },
        /* 52*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "_______________________________________________________________", 0, 36, 36, "" },
        /* 53*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "________________________________________________________________", 0, 36, 36, "" }, /* 64 data */
        /* 54*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "________________________________________________________________", 0, 12, 88, "" },
        /* 55*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "________________________________________________________________", 0, 36, 36, "" },
        /* 56*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_________________________________________________________________", 0, 36, 36, "" }, /* 65 data */
        /* 57*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "_________________________________________________________________", 0, 26, 40, "" },
        /* 58*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "_________________________________________________________________", 0, 36, 36, "" },
        /* 59*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "______________________________________________________________________", 0, 36, 36, "" }, /* 70 data */
        /* 60*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "______________________________________________________________________", 0, 26, 40, "" },
        /* 61*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "______________________________________________________________________", 0, 36, 36, "" },
        /* 62*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "_______________________________________________________________________", 0, 36, 36, "" }, /* 71 data */
        /* 63*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "_______________________________________________________________________", 0, 22, 48, "" },
        /* 64*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "_______________________________________________________________________", 0, 36, 36, "" },
        /* 65*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "________________________________________________________________________________", 0, 36, 36, "" }, /* 80 data */
        /* 66*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "________________________________________________________________________________", 0, 24, 48, "" },
        /* 67*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "________________________________________________________________________________", 0, 36, 36, "" },
        /* 68*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "____________________________________________________________________________________", 0, 36, 36, "" }, /* 84 data */
        /* 69*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "____________________________________________________________________________________", 0, 20, 64, "" },
        /* 70*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "____________________________________________________________________________________", 0, 36, 36, "" },
        /* 71*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "__________________________________________________________________________________________", 0, 40, 40, "" }, /* 90 data */
        /* 72*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "__________________________________________________________________________________________", 0, 26, 48, "" },
        /* 73*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "__________________________________________________________________________________________", 0, 40, 40, "" },
        /* 74*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "___________________________________________________________________________________________", 0, 40, 40, "" }, /* 91 data */
        /* 75*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "___________________________________________________________________________________________", 0, 24, 64, "" },
        /* 76*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "___________________________________________________________________________________________", 0, 40, 40, "" },
        /* 77*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "______________________________________________________________________________________________________________________", 0, 44, 44, "" }, /* 118 data */
        /* 78*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "______________________________________________________________________________________________________________________", 0, 26, 64, "" }, /* 118 data */
        /* 79*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "______________________________________________________________________________________________________________________", 0, 44, 44, "" }, /* 118 data */
        /* 80*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, { 0, 0, "" }, "[90]12", 0, 10, 10, "" },
        /* 81*/ { BARCODE_DATAMATRIX, GS1_MODE | GS1PARENS_MODE, -1, -1, -1, -1, { 0, 0, "" }, "(90)12", 0, 10, 10, "" },
        /* 82*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 1, 2, "" }, "1", 0, 12, 12, "" },
        /* 83*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 16, 16, "" }, "1", 0, 12, 12, "" },
        /* 84*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 1, 1, "" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 720: Structured Append count '1' out of range (2 to 16)" },
        /* 85*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 1, 17, "" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 720: Structured Append count '17' out of range (2 to 16)" },
        /* 86*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 16, "" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 721: Structured Append index '0' out of range (1 to count 16)" },
        /* 87*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 17, 16, "" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 721: Structured Append index '17' out of range (1 to count 16)" },
        /* 88*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "1001" }, "1", 0, 12, 12, "" },
        /* 89*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "A" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 723: Invalid Structured Append ID (digits only)" },
        /* 90*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "0" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 724: Structured Append ID1 '000' and ID2 '000' out of range (001 to 254) (ID \"000000\")" },
        /* 91*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "1" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 725: Structured Append ID1 '000' out of range (001 to 254) (ID \"000001\")" },
        /* 92*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "1000" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 726: Structured Append ID2 '000' out of range (001 to 254) (ID \"001000\")" },
        /* 93*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "001255" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 726: Structured Append ID2 '255' out of range (001 to 254) (ID \"001255\")" },
        /* 94*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "255001" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 725: Structured Append ID1 '255' out of range (001 to 254) (ID \"255001\")" },
        /* 95*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "255255" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 724: Structured Append ID1 '255' and ID2 '255' out of range (001 to 254) (ID \"255255\")" },
        /* 96*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "1234567" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 722: Structured Append ID length 7 too long (6 digit maximum)" },
        /* 97*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, READER_INIT, { 2, 3, "1001" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 727: Cannot have Structured Append and Reader Initialisation at the same time" },
        /* 98*/ { BARCODE_DATAMATRIX, ESCAPE_MODE, -1, -1, -1, -1, { 2, 3, "1001" }, "[)>\\R05\\GA\\R\\E", 0, 12, 26, "" }, /* Macro05/06 ignored if have Structured Append TODO: error/warning */
        /* 99*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, { 0, 0, "" }, "1234,67", ZINT_ERROR_INVALID_DATA, -1, -1, "Error 203: Invalid character at position 5 in input (alphanumerics, space and \"-.$/+%\" only)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol("test_options", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        debug &= ~ZINT_DEBUG_TEST; /* Want real errtxt */
        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, data[i].option_1, data[i].option_2, data[i].option_3, data[i].output_options, data[i].data, -1, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, symbol->errtxt);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, symbol->errtxt);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d symbol->errtxt %s != %s\n", i, symbol->errtxt, data[i].expected_errtxt);

        symbol->input_mode |= FAST_MODE;
        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode FAST_MODE ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d symbol->errtxt %s != %s\n", i, symbol->errtxt, data[i].expected_errtxt);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_reader_init(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int output_options;
        char *data;
        int ret;
        int expected_rows;
        int expected_width;
        char *expected;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, UNICODE_MODE, READER_INIT, "A", 0, 10, 10, "EA 42 81 19 A4 53 21 DF", "TODO: Check this" },
        /*  1*/ { BARCODE_DATAMATRIX, GS1_MODE, READER_INIT, "[91]A", ZINT_ERROR_INVALID_OPTION, 0, 0, "Error 521: Cannot use Reader Initialisation in GS1 mode", "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[1024];

    testStartSymbol("test_reader_init", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %s, \"%s\", %s, %d, %d, \"%s\", \"%s\" },\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), testUtilOutputOptionsName(data[i].output_options),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, symbol->errtxt, data[i].comment);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#define ZINT_TEST_ENCODING
#ifdef ZINT_TEST_ENCODING
INTERNAL int dm_encode_test(struct zint_symbol *symbol, const unsigned char source[], const int length, const int eci,
            const int gs1, unsigned char target[], int *p_tp);
#endif

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_2;
        int option_3;
        int output_options;
        char *data;
        int ret;

        int expected_eci;
        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *expected;
        char *comment;

        int expected_diff; /* Difference between default minimal encodation and ISO encodation (FAST_MODE) */
    };
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY80", 0, 0, 18, 18, 1, "(32) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 93 B0 1C 3C 76 FB D4 AB 16 11", "#208", 0 },
        /*  1*/ { UNICODE_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY80", 0, 0, 18, 18, 1, "(32) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 93 B0 1C 3C 76 FB D4 AB 16 11", "#208", 0 },
        /*  2*/ { UNICODE_MODE | FAST_MODE, 0, 5, -1, -1, "0466010592130100000k*AGUATY80", 0, 0, 18, 18, 1, "(32) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 93 B0 1C 3C 76 FB D4 AB 16 11", "", 0 },
        /*  3*/ { UNICODE_MODE, 0, 5, -1, -1, "0466010592130100000k*AGUATY80", 0, 0, 18, 18, 1, "(32) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 93 B0 1C 3C 76 FB D4 AB 16 11", "", 0 },
        /*  4*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY8", 0, 0, 18, 18, 0, "(32) 86 C4 83 87 DE 8F 83 82 82 31 6C E6 07 B7 82 5F D4 3D 08 EB 60 DA B1 82 72 50 A9 5B", "BWIPP different encodation (earlier change to C40)", 0 },
        /*  5*/ { UNICODE_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY8", 0, 0, 18, 18, 0, "(32) 86 C4 83 87 DE 8F 83 82 82 31 6C 2B 42 E6 82 5F D4 3D 0A 34 D7 21 4E D2 8D C5 9C D7", "AAAAAAAAAAAAAAAAAAAAAACCCCCC; BWIPP different encodation", 0 },
        /*  6*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY80U", 0, 0, 20, 20, 1, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 FE 56 81 76 4F AB 22 B8 6F 0A", "", 0 },
        /*  7*/ { UNICODE_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY80U", 0, 0, 20, 20, 0, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C F0 A8 11 D5 05 46 5F D2 56 81 76 3C D6 92 14 9F E2", "AAAAAAAAAAAAAAAAAAAAEEEEEEEAAA; BWIPP same as FAST_MODE", 0 },
        /*  8*/ { UNICODE_MODE | FAST_MODE, 0, 5, -1, -1, "0466010592130100000k*AGUATY80U", ZINT_ERROR_TOO_LONG, -1, 0, 0, 0, "Error 522: Input too long for Version 5, requires 19 codewords (maximum 18)", "", 0 },
        /*  9*/ { UNICODE_MODE, 0, 5, -1, -1, "0466010592130100000k*AGUATY80U", ZINT_ERROR_TOO_LONG, -1, 0, 0, 0, "Error 522: Input too long for Version 5, requires 19 codewords (maximum 18)", "", 0 },
        /* 10*/ { UNICODE_MODE | FAST_MODE, 0, 6, -1, -1, "0466010592130100000k*AGUATY80U", 0, 0, 20, 20, 1, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 FE 56 81 76 4F AB 22 B8 6F 0A", "", 0 },
        /* 11*/ { UNICODE_MODE, 0, 6, -1, -1, "0466010592130100000k*AGUATY80U", 0, 0, 20, 20, 0, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C F0 A8 11 D5 05 46 5F D2 56 81 76 3C D6 92 14 9F E2", "AAAAAAAAAAAAAAAAAAAAEEEEEEEAAA; BWIPP same as FAST_MODE", 0 },
        /* 12*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY80UA", 0, 0, 20, 20, 1, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C E6 07 B7 82 5F D4 3D 1E 5F FE 81 1E 1B B0 FE E7 54", "", 0 },
        /* 13*/ { UNICODE_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY80UA", 0, 0, 20, 20, 0, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C F0 A8 11 D5 05 46 78 C1 50 5F 81 3B 50 EE DB AE A4", "AAAAAAAAAAAAAAAAAAAAEEEEEEEEEEE; BWIPP same as FAST_MODE", 0 },
        /* 14*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, ">*\015>*\015>", 0, 0, 14, 14, 1, "EE 0C A9 0C A9 FE 3F 81 42 B2 11 A8 F9 0A EC C1 1E 41", "X12 symbols_left 3, process_p 1", 0 },
        /* 15*/ { UNICODE_MODE, 0, -1, -1, -1, ">*\015>*\015>", 0, 0, 14, 14, 0, "3F 2B 0E 3F 2B 0E 3F 81 37 3F 04 F9 CC 02 2D 70 E0 FF", "AAAAAAA; BWIPP same as FAST_MODE", 0 },
        /* 16*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, ">*\015>*\015>*", 0, 0, 14, 14, 1, "EE 0C A9 0C A9 FE 3F 2B 3F 05 D2 10 1B 9A 55 2F 68 C5", "X12 symbols_left 3, process_p 2", 0 },
        /* 17*/ { UNICODE_MODE, 0, -1, -1, -1, ">*\015>*\015>*", 0, 0, 14, 14, 0, "3F 2B 0E 3F 2B 0E 3F 2B 4A 88 C7 41 2E 92 94 9E 96 7B", "AAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /* 18*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, ">*\015>*\015>*\015", 0, 0, 14, 14, 1, "EE 0C A9 0C A9 0C A9 FE 1F 30 3F EE 45 C1 1C D7 5F 7E", "X12 symbols_left 1, process_p 0", 0 },
        /* 19*/ { UNICODE_MODE, 0, -1, -1, -1, ">*\015>*\015>*\015", 0, 0, 14, 14, 1, "EE 0C A9 0C A9 0C A9 FE 1F 30 3F EE 45 C1 1C D7 5F 7E", "X12 symbols_left 1, process_p 0", 0 },
        /* 20*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEF", 0, 0, 12, 12, 1, "E6 59 E9 6D 24 3D 15 EF AA 21 F9 59", "C40 symbols_left 0, process_p 0", 0 },
        /* 21*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEF", 0, 0, 12, 12, 1, "E6 59 E9 6D 24 3D 15 EF AA 21 F9 59", "CCCCCC", 0 },
        /* 22*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFG", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 FE 48 81 8C 7E 09 5E 10 64 BC 5F 4C 91", "C40 symbols_left 3, process_p 1", 0 },
        /* 23*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFG", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 DF 81 AB 07 40 5C C8 C2 F5 59 B3 10", "EEEEEEE; BWIPP same as FAST_MODE", 0 },
        /* 24*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGH", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 FE 48 49 2E 31 00 73 3B 8F 4B 55 93 19", "C40 symbols_left 3, process_p 2", 0 },
        /* 25*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 C8 81 D1 4D 5F 96 32 9F B2 A0 C7 8D", "EEEEEEEE; BWIPP same as FAST_MODE", 1 },
        /* 26*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHI", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 80 5F FE 01 DE 20 9F AA C2 FF 8F 08 97", "C40 symbols_left 1, process_p 0", 0 },
        /* 27*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHI", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 C8 4A 34 B0 7A 27 3C 38 82 4C 30 21", "EEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /* 28*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGH12", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 80 4E 33 B3 46 F3 D5 F5 C0 4F 12 F2 ED", "C40 symbols_left 1, process_p 1", 0 },
        /* 29*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH12", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 C8 8E A7 6D C3 20 83 CE 1E A9 4F 39", "EEEEEEEEAA; BWIPP same as FAST_MODE", 0 },
        /* 30*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHé", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 80 4A BB DA FE E1 A4 57 73 2B 4E 06 10 FB EE 83", "C40 symbols_left 1, process_p 0", 0 },
        /* 31*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHé", 0, 0, 8, 32, 0, "42 F0 08 31 05 18 72 1F EB 6A 80 70 2B 11 BD 91 6B 5F C9 AC 43", "AEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /* 32*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHIJ", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 80 5F 4B AD 47 09 12 FF 2F 95 CA 5B 4A", "C40 symbols_left 1, process_p 1", 0 },
        /* 33*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJ", 0, 0, 14, 14, 0, "42 E6 60 52 73 8D 86 C8 67 A3 F9 29 73 9C 15 90 7E 16", "ACCCCCCCCC; BWIPP same as FAST_MODE", 0 },
        /* 34*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHIJK", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 80 5F FE 4B 4C D8 69 88 60 B9 33 B9 31 E6 BF CA", "C40 symbols_left 3, process_p 2", 0 },
        /* 35*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJK", 0, 0, 8, 32, 0, "F0 04 20 C4 14 61 C8 24 A2 DF B6 77 90 C2 39 AA 9B 61 7E F1 E3", "EEEEEEEEEEE; BWIPP same as FAST_MODE", 0 },
        /* 36*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEF\001G", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 00 3D FE 5D 5A F5 0A 8A 4E 1D 63 07 B9", "C40 symbols_left 1, process_p 0", 0 },
        /* 37*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEF\001G", 0, 0, 14, 14, 0, "42 43 F0 0C 41 46 02 48 57 A1 80 C5 8F EC 72 33 0A 26", "AAEEEEAA; BWIPP same as FAST_MODE", 0 },
        /* 38*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFG\001", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 02 FE 14 A3 27 63 01 2F B1 94 FE FA", "C40 symbols_left 1, process_p 0", 0 },
        /* 39*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFG\001", 0, 0, 14, 14, 0, "42 43 44 F0 10 51 87 02 50 0C 22 23 7E A3 2B DC 8A 62", "AAAEEEEA; BWIPP same as FAST_MODE", 0 },
        /* 40*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFG\001H", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 02 49 C2 E6 DD 06 89 51 BA 8E 9D 1F", "C40 symbols_left 1, process_p 1", 0 },
        /* 41*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFG\001H", 0, 0, 14, 14, 0, "42 E6 60 52 73 8D 00 3E 66 13 C0 89 73 A8 53 20 F7 6F", "ACCCCCCCC; BWIPP same as FAST_MODE", 0 },
        /* 42*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGH\001", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 02 81 BD 6D F3 94 FF 82 A6 BF BB F1 4F", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 43*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH\001", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 C8 02 37 8F B4 A6 27 9E FA CF 1E 02", "EEEEEEEEA; BWIPP same as FAST_MODE", 1 },
        /* 44*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH\001", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 02 81 FB 93 AE 8B 1C 90 DF FE EB C5 A0 2A 6A 4F", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 45*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH\001", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 C8 02 37 8F B4 A6 27 9E FA CF 1E 02", "EEEEEEEEA; BWIPP same as FAST_MODE", 1 },
        /* 46*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGH\001I", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 02 4A E1 0D DD BC 56 E4 66 52 E6 AE 02", "C40 symbols_left 3, process_p 2, backtracks", 0 },
        /* 47*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH\001I", 0, 0, 8, 32, 0, "42 43 44 45 F0 14 61 C8 02 4A 5F 21 C6 61 E1 7B 1C 13 FF 0D BA", "AAAAEEEEAA; BWIPP same as FAST_MODE", 0 },
        /* 48*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH\001I", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 02 4A 81 93 51 DF C0 0C D3 F9 72 13 17 52 5B 7E", "C40 symbols_left 5, process_p 2, backtracks", 0 },
        /* 49*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH\001I", 0, 0, 16, 16, 0, "42 F0 08 31 05 18 72 1F 02 4A 81 93 FD 86 6C AF 6E C6 7C 99 74 21 C4 EF", "AEEEEEEEAA; BWIPP same as FAST_MODE", 0 },
        /* 50*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHI\001", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 80 5F FE 02 81 47 6C 3E 49 D3 FA 46 47 53 6E E5", "Switches to ASC for last char", 0 },
        /* 51*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHI\001", 0, 0, 8, 32, 0, "42 F0 08 31 05 18 72 09 02 81 FA 72 8B 50 63 F2 BD D9 2E 1D CD", "AEEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /* 52*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHI\001", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F FE 02 81 FB 93 33 E3 4F F7 2D 08 8A BF 64 C3 B0 26", "Switches to ASC for last char", 0 },
        /* 53*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHI\001", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F FE 02 81 FB 93 33 E3 4F F7 2D 08 8A BF 64 C3 B0 26", "CCCCCCCCCA", 0 },
        /* 54*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGH\001I\001", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 02 4A 02 81 BD 5D C0 B9 09 25 87 3A 09 23 9D C0", "C40 symbols_left 1, process_p 1, backtracks 2", 0 },
        /* 55*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH\001I\001", 0, 0, 8, 32, 0, "42 E6 60 52 73 8D 83 42 89 82 5F 2E 1D 6C 18 BB 25 C3 BF 7F E1", "ACCCCCCCCCC; BWIPP same as FAST_MODE", 1 },
        /* 56*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEF+G", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 07 E5 FE 6B 35 71 7F 3D 57 59 46 F7 B9", "C40 symbols_left 1, process_p 0", 0 },
        /* 57*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEF+G", 0, 0, 14, 14, 0, "F0 04 20 C4 14 6A C7 81 28 87 C6 C4 D6 50 53 DE 8D 80", "EEEEEEEE; BWIPP same as FAST_MODE", 1 },
        /* 58*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFG+", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 33 FE 33 F5 97 60 73 48 13 2E E5 74", "C40 symbols_left 1, process_p 0", 0 },
        /* 59*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFG+", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 EB 81 E8 04 8F 69 BD F5 01 28 03 0A", "EEEEEEEE; BWIPP same as FAST_MODE", 1 },
        /* 60*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFG+H", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 33 49 E5 B0 6D 05 FB 36 18 34 86 91", "C40 symbols_left 1, process_p 1", 0 },
        /* 61*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFG+H", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 EB 49 4A 4B 86 44 96 1E F6 22 DC 82", "EEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /* 62*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGH+", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 2C 81 02 BD 40 CF 3B 06 C2 DF 36 E0 48", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 63*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH+", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 C8 2C 1F DD 3B 4F 62 48 0E 16 F9 5D", "EEEEEEEEA; BWIPP same as FAST_MODE", 1 },
        /* 64*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH+", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 2C 81 FB 93 F6 78 B5 69 0B 83 C6 32 62 1A D2 FF", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 65*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH+", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 C8 2C 1F DD 3B 4F 62 48 0E 16 F9 5D", "EEEEEEEEA; BWIPP same as FAST_MODE", 1 },
        /* 66*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGH+I", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 2C 4A 5E DD 6E E7 92 60 02 32 6B BF 05", "C40 symbols_left 3, process_p 2, backtracks", 0 },
        /* 67*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH+I", 0, 0, 8, 32, 0, "42 F0 08 31 05 18 72 2B 4A 81 E2 22 C5 B6 9C AB 0E F1 63 E5 EE", "AEEEEEEEEA; BWIPP same as FAST_MODE", 1 },
        /* 68*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH+I", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 2C 4A 81 93 09 2C 69 F5 07 84 5F E4 D5 62 E3 CE", "C40 symbols_left 5, process_p 2, backtracks", 0 },
        /* 69*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH+I", 0, 0, 16, 16, 0, "42 43 F0 0C 41 46 1C 8A C9 7C 81 93 A8 91 6E F7 89 AB E8 A5 8E CA F0 34", "AAEEEEEEEE; BWIPP same as FAST_MODE", 0 },
        /* 70*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHI+", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 80 5F FE 2C 81 F8 BC 8D 12 17 7E 22 27 DE 7F E2", "C40 symbols_left 3, process_p 2, backtracks", 0 },
        /* 71*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHI+", 0, 0, 8, 32, 0, "42 F0 08 31 05 18 72 09 2C 81 45 A2 38 0B A7 76 D9 B9 A3 0C CA", "AEEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /* 72*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHI+", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F FE 2C 81 FB 93 6B 10 E6 0E F9 75 A7 48 A6 F3 08 96", "Switches to ASC for last char", 0 },
        /* 73*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHI+", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F FE 2C 81 FB 93 6B 10 E6 0E F9 75 A7 48 A6 F3 08 96", "CCCCCCCCCA", 0 },
        /* 74*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFjG", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 0E 25 FE DA 14 D7 15 47 69 9D 4A 54 6D", "C40 symbols_left 1, process_p 0", 0 },
        /* 75*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFjG", 0, 0, 14, 14, 0, "42 43 F0 0C 41 46 6B 48 77 B6 92 A1 DC 92 42 54 39 84", "AAEEEEAA; BWIPP same as FAST_MODE", 0 },
        /* 76*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGj", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 5B FE B5 F3 24 0A 99 26 D6 CC A8 40", "C40 symbols_left 1, process_p 0", 0 },
        /* 77*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGj", 0, 0, 14, 14, 0, "42 43 44 F0 10 51 87 6B 0D 41 FF FD 91 82 0B 8F CB AA", "AAAEEEEA; BWIPP same as FAST_MODE", 0 },
        /* 78*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGjH", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 5B 49 63 B6 DE 6F 11 58 DD D6 CB A5", "C40 symbols_left 1, process_p 1", 0 },
        /* 79*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGjH", 0, 0, 14, 14, 0, "42 E6 60 52 73 8D 0E 26 0E F4 AD 89 C6 72 8A 26 34 65", "ACCCCCCCC; BWIPP same as FAST_MODE", 0 },
        /* 80*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHj", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 6B 81 ED 78 CB 9F 52 EE 52 88 91 67 96", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 81*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHj", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 C8 6B 6A C2 69 78 C8 BF DA 9C 5F CA", "EEEEEEEEA; BWIPP same as FAST_MODE", 1 },
        /* 82*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHj", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 6B 81 FB 93 BF 72 03 35 09 37 98 FF 39 A7 E3 6D", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 83*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHj", 0, 0, 14, 14, 0, "F0 04 20 C4 14 61 C8 6B 6A C2 69 78 C8 BF DA 9C 5F CA", "EEEEEEEEA; BWIPP same as FAST_MODE", 1 },
        /* 84*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHjI", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 6B 4A B1 18 E5 B7 FB 88 92 65 CC 38 DB", "C40 symbols_left 3, process_p 2, backtracks", 0 },
        /* 85*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHjI", 0, 0, 8, 32, 0, "42 43 44 45 F0 14 61 C8 6B 4A 0F 34 FE 6A 4C 17 E8 24 D5 9B 63", "AAAAEEEEAA; BWIPP same as FAST_MODE", 0 },
        /* 86*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHjI", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 6B 4A 81 93 40 26 DF A9 05 30 01 29 8E DF D2 5C", "C40 symbols_left 5, process_p 2, backtracks", 0 },
        /* 87*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHjI", 0, 0, 16, 16, 0, "42 F0 08 31 05 18 72 1F 6B 4A 81 93 EC 7F 73 0A B8 0F 0F A3 ED AC 4D CD", "AEEEEEEEAA; BWIPP same as FAST_MODE", 0 },
        /* 88*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHIj", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 80 5F FE 6B 81 17 79 06 42 7E 96 B2 70 79 F8 3C", "Switches to ASC for last char", 0 },
        /* 89*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIj", 0, 0, 8, 32, 0, "42 F0 08 31 05 18 72 09 6B 81 AA 67 B3 5B CE 9E 49 EE 04 8B 14", "AEEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /* 90*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHIJÊ", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F FE 4B EB 4B 81 DD D9 F9 C9 C5 38 F3 4B DB 80 92 A7", "Switches to ASC for last 2 chars", 0 },
        /* 91*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJÊ", 0, 0, 8, 32, 0, "42 E6 60 52 73 8D 86 C8 0B 08 72 65 72 59 41 70 1B CE 76 D7 1F", "ACCCCCCCCCC; BWIPP same as FAST_MODE", 0 },
        /* 92*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHIJKÊ", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F FE 4B 4C EB 4B 15 17 46 06 70 F3 15 74 45 26 72 2D", "C40 symbols_left 3, process_p 2, backtracks", 0 },
        /* 93*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJKÊ", 0, 0, 16, 16, 0, "F0 04 20 C4 14 61 C8 24 A2 DF EB 4B A1 42 E9 2A A8 3B 79 8B E9 40 E9 58", "EEEEEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /* 94*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHIJKª", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F 93 82 BB B2 FE 11 5C 60 32 A6 DE FC 7B 30 F1 03 56", "C40 symbols_left 1, process_p 0", 0 },
        /* 95*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJKª", 0, 0, 16, 16, 0, "F0 04 20 C4 14 61 C8 24 A2 DF EB 2B 02 D1 85 74 F7 92 3F 2E 1F 07 A3 BE", "EEEEEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /* 96*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHIJKê", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F 93 82 BB DB FE 78 43 69 3C C2 FE F5 2E 1B 4F B6 04", "C40 symbols_left 1, process_p 0", 0 },
        /* 97*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJKê", 0, 0, 16, 16, 0, "F0 04 20 C4 14 61 C8 24 A2 DF EB 6B C0 33 CD FB 9D 5C A0 E8 BB 7D 34 E1", "EEEEEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /* 98*/ { GS1_MODE | FAST_MODE, 0, -1, -1, -1, "[10]ABCDEFGH[10]ABc", 0, 0, 12, 26, 1, "E8 8C E6 59 E9 6D 24 80 4A A9 8D FE 42 43 64 81 83 B4 8F 6B 95 F6 CE A6 3C 5C 77 86 08 50", "C40 symbols_left 3, process_p 1, backtracks", 0 },
        /* 99*/ { GS1_MODE, 0, -1, -1, -1, "[10]ABCDEFGH[10]ABc", 0, 0, 12, 26, 0, "E8 8C 42 F0 08 31 05 18 72 1F E8 8C 42 43 64 81 12 DE 1C 0E B2 93 B3 C5 3C 58 A8 4A 83 4C", "AAAEEEEEEEAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*100*/ { GS1_MODE | FAST_MODE, 0, -1, -1, GS1_GS_SEPARATOR, "[10]ABCDEFGH[10]ABc", 0, 0, 12, 26, 1, "E8 8C E6 59 E9 6D 24 80 49 B6 0D FE 42 43 64 81 79 E4 20 33 76 5C C7 23 E6 C5 FA 4C FF 88", "C40 symbols_left 3, process_p 1, backtracks", 0 },
        /*101*/ { GS1_MODE, 0, -1, -1, GS1_GS_SEPARATOR, "[10]ABCDEFGH[10]ABc", 0, 0, 12, 26, 0, "E8 8C 42 F0 08 31 05 18 72 1F 1E 8C 42 43 64 81 4E 3D 4C 86 78 D4 97 78 85 01 48 D5 B9 29", "AAAEEEEEEEAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*102*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdef", 0, 0, 12, 12, 1, "EF 59 E9 6D 24 E2 CC D9 B4 55 E2 6A", "TEX symbols_left 0, process_p 0", 0 },
        /*103*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdef", 0, 0, 12, 12, 1, "EF 59 E9 6D 24 E2 CC D9 B4 55 E2 6A", "TTTTTT", 0 },
        /*104*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefg", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 FE 68 81 A9 65 CD 3A A2 E9 E0 B7 E1 E5", "TEX symbols_left 3, process_p 1", 0 },
        /*105*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefg", 0, 0, 14, 14, 0, "62 63 64 65 66 67 68 81 4A 9E 59 19 83 14 93 AD 5E 19", "AAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*106*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefgh", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 FE 68 69 68 36 28 3C 85 5A E9 D4 49 9A", "TEX symbols_left 3, process_p 2", 0 },
        /*107*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgh", 0, 0, 14, 14, 0, "62 63 64 65 66 67 68 69 8B CD BC 1F A4 A7 9A CE F6 66", "AAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*108*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefghi", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 80 5F FE DA BF FA 16 71 15 22 4D E3 F3", "TEX symbols_left 1, process_p 0", 0 },
        /*109*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghi", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 80 5F FE DA BF FA 16 71 15 22 4D E3 F3", "TTTTTTTTT", 0 },
        /*110*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdef\001g", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 00 3D FE 86 3B 2F 83 51 99 C0 A1 EC DD", "TEX symbols_left 1, process_p 0", 0 },
        /*111*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdef\001g", 0, 0, 14, 14, 0, "62 63 64 65 66 67 02 68 51 87 9F 1D E9 36 3E B2 5F 4F", "AAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*112*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefg\001", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 7D 02 FE CF C2 FD EA DA F8 6C 56 15 9E", "TEX symbols_left 1, process_p 0", 0 },
        /*113*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefg\001", 0, 0, 14, 14, 0, "62 63 64 65 66 67 68 02 AC 5C B2 29 96 15 DB C2 87 96", "AAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*114*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefg\001h", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 7D 02 69 7A 9B EB A4 5E DE 99 25 01 8C", "TEX symbols_left 1, process_p 1", 0 },
        /*115*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefg\001h", 0, 0, 14, 14, 0, "62 EF 60 52 73 8D 00 3E DC 69 2C DB 8D 1B C4 DF 8C 9A", "ATTTTTTTT; BWIPP same as FAST_MODE", 0 },
        /*116*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefgh\001", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 FE 68 69 02 81 EB 84 25 32 6E 1B 5A FB 1D 25 4A", "TEX symbols_left 1, process_p 1, backtracks", 0 },
        /*117*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgh\001", 0, 0, 14, 14, 0, "62 EF 60 52 73 8D 83 42 A7 27 45 DE 64 B7 93 23 AF 04", "ATTTTTTTT; BWIPP same as FAST_MODE", 1 },
        /*118*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "abcdefgh\001", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 FE 68 69 02 81 FB 93 93 FD 1E 3B BA 1D 16 4D 59 41 EC B9", "TEX symbols_left 1, process_p 1, backtracks", 0 },
        /*119*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "abcdefgh\001", 0, 0, 14, 14, 0, "62 EF 60 52 73 8D 83 42 A7 27 45 DE 64 B7 93 23 AF 04", "ATTTTTTTT; BWIPP same as FAST_MODE", 1 },
        /*120*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefgh\001i", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 FE 68 69 02 6A 31 35 48 9B 93 6E 15 BB 02 9D F4", "TEX symbols_left 3, process_p 2, backtracks", 0 },
        /*121*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgh\001i", 0, 0, 8, 32, 0, "62 63 64 65 66 67 68 69 02 6A D2 78 3E D1 EC CE 72 33 20 0E F2", "AAAAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*122*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "abcdefgh\001i", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 FE 68 69 02 6A 81 93 DE D7 EC 9B 7D 72 9C 68 B8 6E CF 31", "TEX symbols_left 3, process_p 2, backtracks", 0 },
        /*123*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "abcdefgh\001i", 0, 0, 16, 16, 0, "62 63 64 65 66 67 68 69 02 6A 81 93 45 9D 38 FC 6B CA 1C 9B DF 3F C8 30", "AAAAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*124*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefghi\001", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 80 5F FE 02 81 4D AB 30 86 CD D1 9D F3 15 F5 B1", "Switches to ASC for last char", 0 },
        /*125*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghi\001", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 80 5F FE 02 81 4D AB 30 86 CD D1 9D F3 15 F5 B1", "TTTTTTTTTA", 0 },
        /*126*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefgh\001i\001", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 FE 68 69 02 6A 02 81 32 55 EC 2E A7 AE 69 41 A6 1F 09 8F", "TEX symbols_left 1, process_p 1, backtracks 2", 0 },
        /*127*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgh\001i\001", 0, 0, 8, 32, 0, "62 EF 60 52 73 8D 83 42 89 82 80 A6 CC 72 8E 8C C5 75 D3 0D C2", "ATTTTTTTTTT; BWIPP same as FAST_MODE", 1 },
        /*128*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefJg", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 0E 25 FE 01 75 0D 9C 9C BE 40 88 BF 09", "TEX symbols_left 1, process_p 0", 0 },
        /*129*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefJg", 0, 0, 14, 14, 0, "62 63 64 65 66 67 4B 68 8F EA 93 94 D3 12 8F FF 2A FD", "AAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*130*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefgJ", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 7D 5B FE 6E 92 FE 83 42 F1 0B 0E 43 24", "TEX symbols_left 1, process_p 0", 0 },
        /*131*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgJ", 0, 0, 14, 14, 0, "62 63 64 65 66 67 68 4B 92 0D 83 DC 75 6C 05 F8 B1 A9", "AAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*132*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefgJh", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 7D 5B 69 DB CB E8 CD C6 D7 FE 7D 57 36", "TEX symbols_left 1, process_p 1", 0 },
        /*133*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgJh", 0, 0, 14, 14, 0, "62 EF 60 52 73 8D 0E 26 B4 8E 41 DB 38 C1 1D D9 4F 90", "ATTTTTTTT; BWIPP same as FAST_MODE", 0 },
        /*134*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefghJ", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 FE 68 69 4B 81 15 8A 35 57 7F 33 B3 48 01 E0 BD", "TEX symbols_left 1, process_p 1, backtracks", 0 },
        /*135*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghJ", 0, 0, 14, 14, 0, "62 EF 60 52 73 8D 83 9B 9F 08 C5 04 B1 AF BD 32 C5 7D", "ATTTTTTTT; BWIPP same as FAST_MODE", 1 },
        /*136*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "abcdefghJ", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 FE 68 69 4B 81 FB 93 5B D4 D2 8B EE 85 F2 3E 3F 8E E5 04", "TEX symbols_left 1, process_p 1, backtracks", 0 },
        /*137*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "abcdefghJ", 0, 0, 14, 14, 0, "62 EF 60 52 73 8D 83 9B 9F 08 C5 04 B1 AF BD 32 C5 7D", "ATTTTTTTT; BWIPP same as FAST_MODE", 1 },
        /*138*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefghJi", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 FE 68 69 4B 6A CF 3B 58 FE 82 46 FC 08 1E 58 03", "TEX symbols_left 3, process_p 2, backtracks", 0 },
        /*139*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghJi", 0, 0, 8, 32, 0, "62 63 64 65 66 67 68 69 4B 6A 2C 76 2E B4 FD E6 9B 80 3C CB 05", "AAAAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*140*/ { UNICODE_MODE | FAST_MODE, 0, -1, DM_SQUARE, -1, "abcdefghJi", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 FE 68 69 4B 6A 81 93 16 FE 20 2B 29 EA 78 1B DE A1 C6 8C", "TEX symbols_left 3, process_p 2, backtracks", 0 },
        /*141*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "abcdefghJi", 0, 0, 16, 16, 0, "62 63 64 65 66 67 68 69 4B 6A 81 93 8D B4 F4 4C 3F 52 F8 E8 B9 F0 C1 8D", "AAAAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*142*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefghiJ", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 80 5F FE 4B 81 B3 A5 20 E3 DC F9 74 40 09 30 46", "Switches to ASC for last char", 0 },
        /*143*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghiJ", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 80 5F FE 4B 81 B3 A5 20 E3 DC F9 74 40 09 30 46", "TTTTTTTTTA", 0 },
        /*144*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefghijkÊ", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 80 5F 93 82 BB DB FE 3E C8 EC 73 58 A7 42 46 10 49 25 99", "TEX symbols_left 1, process_p 0", 0 },
        /*145*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghijkÊ", 0, 0, 16, 16, 0, "EF 59 E9 6D 24 80 5F FE 6B 6C EB 4B 38 32 3E 60 A3 93 26 A6 E7 35 73 96", "TTTTTTTTTAAA; BWIPP same as FAST_MODE", 0 },
        /*146*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefghijkª", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 80 5F 93 82 BB B2 FE 57 D7 E5 7D 3C 87 4B 13 3B F7 90 CB", "TEX symbols_left 1, process_p 0", 0 },
        /*147*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghijkª", 0, 0, 16, 16, 0, "EF 59 E9 6D 24 80 5F FE 6B 6C EB 2B 9B A1 52 3E FC 3A 60 03 11 72 39 70", "TTTTTTTTTAAA; BWIPP same as FAST_MODE", 0 },
        /*148*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "abcdefghijkê", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 80 5F FE 6B 6C EB 6B 59 43 1A B1 96 F4 FF C5 B5 08 AE 2F", "TEX symbols_left 3, process_p 2, backtracks", 0 },
        /*149*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghijkê", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 80 5F FE 6B 6C EB 6B 59 43 1A B1 96 F4 FF C5 B5 08 AE 2F", "TEX symbols_left 3, process_p 2, backtracks", 0 },
        /*150*/ { GS1_MODE | FAST_MODE, 0, -1, -1, -1, "[10]abcdefgh[10]abC", 0, 0, 12, 26, 1, "E8 8C EF 59 E9 6D 24 80 4A A9 8D FE 62 63 44 81 88 DC 73 33 70 A1 83 EA 50 CB 4E 17 90 DB", "TEX symbols left 3, process_p 1, backtracks", 0 },
        /*151*/ { GS1_MODE, 0, -1, -1, -1, "[10]abcdefgh[10]abC", 0, 0, 12, 26, 0, "E8 8C 62 63 64 65 66 67 68 69 E8 8C 62 63 44 81 43 79 B0 35 F6 1E AB 24 19 85 40 E7 B2 49", "AAAAAAAAAAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*152*/ { GS1_MODE | FAST_MODE, 0, -1, -1, GS1_GS_SEPARATOR, "[10]abcdefgh[10]abC", 0, 0, 12, 26, 1, "E8 8C EF 59 E9 6D 24 80 49 B6 0D FE 62 63 44 81 72 8C DC 6B 93 0B 8A 6F 8A 52 C3 DD 67 03", "TEX symbols left 3, process_p 1, backtracks", 0 },
        /*153*/ { GS1_MODE, 0, -1, -1, GS1_GS_SEPARATOR, "[10]abcdefgh[10]abC", 0, 0, 12, 26, 0, "E8 8C 62 63 64 65 66 67 68 69 1E 8C 62 63 44 81 1F 9A E0 BD 3C 59 8F 99 A0 DC A0 78 88 2C", "AAAAAAAAAAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*154*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "\015*>\015*>", 0, 0, 12, 12, 1, "EE 00 2B 00 2B 83 3B 0A CE 32 36 65", "X12 symbols_left 0, process_p 0", 0 },
        /*155*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>", 0, 0, 12, 12, 1, "EE 00 2B 00 2B 83 3B 0A CE 32 36 65", "X12 symbols_left 0, process_p 0", 0 },
        /*156*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "\015*>\015*>\015", 0, 0, 14, 14, 1, "EE 00 2B 00 2B FE 0E 81 C0 6C BF 37 F6 D6 48 71 E2 38", "Switches to ASC for last char", 0 },
        /*157*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015", 0, 0, 14, 14, 0, "0E 2B 3F 0E 2B 3F 0E 81 33 CC F8 AA 23 B8 F3 40 BC DB", "AAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*158*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "\015*>\015*>\015*", 0, 0, 14, 14, 1, "EE 00 2B 00 2B FE 0E 2B BD DB 7C 8F 14 46 F1 9F 94 BC", "Switches to ASC for last 2 chars", 0 },
        /*159*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015*", 0, 0, 14, 14, 0, "0E 2B 3F 0E 2B 3F 0E 2B 4E 7B 3B 12 C1 28 4A AE CA 5F", "AAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*160*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>", 0, 0, 14, 14, 1, "EE 00 2B 00 2B 00 2B FE BF 81 70 74 1C 65 10 0C 06 38", "X12 symbols_left 1, process_p 0, ASC unlatch at end", 0 },
        /*161*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>", 0, 0, 14, 14, 1, "EE 00 2B 00 2B 00 2B FE BF 81 70 74 1C 65 10 0C 06 38", "X12 symbols_left 1, process_p 0, ASC unlatch at end", 0 },
        /*162*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>\015", 0, 0, 14, 14, 1, "EE 00 2B 00 2B 00 2B 0E 1C DB D8 26 3E EC CF 9C C3 4A", "X12 symbols_left 1, process_p 1, ASC no latch at end", 0 },
        /*163*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>\015", 0, 0, 14, 14, 0, "0E EE 06 91 06 91 06 91 71 69 39 1E 01 32 D5 57 81 8E", "AXXXXXXXXX; BWIPP same as FAST_MODE", 0 },
        /*164*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>\015*", 0, 0, 8, 32, 1, "EE 00 2B 00 2B 00 2B FE 0E 2B 65 37 5F 2F F3 96 BE 9A 03 55 68", "X12 symbols_left 3, process_p 2, ASC last 2 chars", 0 },
        /*165*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>\015*", 0, 0, 8, 32, 0, "0E 2B EE 0C 82 0C 82 0C 82 FE E7 2D 83 16 AA 07 19 34 F4 F8 7B", "AAXXXXXXXXX; BWIPP same as FAST_MODE", 0 },
        /*166*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>\015*>", 0, 0, 8, 32, 1, "EE 00 2B 00 2B 00 2B 00 2B FE 6E 95 3A 10 58 4E 96 06 79 09 94", "X12 symbols_left 1, process_p 0, ASC unlatch at end", 0 },
        /*167*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>\015*>", 0, 0, 8, 32, 1, "EE 00 2B 00 2B 00 2B 00 2B FE 6E 95 3A 10 58 4E 96 06 79 09 94", "X12 symbols_left 1, process_p 0, ASC unlatch at end", 0 },
        /*168*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "@A1^B2?C", 0, 0, 14, 14, 1, "F0 00 1C 5E 0B 2F C3 81 2D 71 45 13 9B FF A1 B0 0B E2", "EDIFACT symbols_left 1, process_p 0", 0 },
        /*169*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C", 0, 0, 14, 14, 1, "F0 00 1C 5E 0B 2F C3 81 2D 71 45 13 9B FF A1 B0 0B E2", "EDIFACT symbols_left 1, process_p 0", 0 },
        /*170*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "@A1^B2?C3", 0, 0, 14, 14, 1, "F0 00 1C 5E 0B 2F C3 34 81 E8 6C 9E CE 12 CB F5 58 3F", "EDIFACT symbols_left 1, process_p 1", 0 },
        /*171*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3", 0, 0, 14, 14, 1, "F0 00 1C 5E 0B 2F C3 34 81 E8 6C 9E CE 12 CB F5 58 3F", "EEEEEEEEA", 0 },
        /*172*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "@A1^B2?C3+", 0, 0, 8, 32, 1, "F0 00 1C 5E 0B 2F C3 CE B7 C0 33 C6 81 E1 63 6E 5E B4 27 30 C9", "EDIFACT symbols_left 3, process_p 2", 0 },
        /*173*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3+", 0, 0, 8, 32, 0, "41 F0 07 17 82 CB F0 F3 2C 81 38 6D 00 EB 77 36 F8 D8 99 B4 1C", "AEEEEEEEEA; BWIPP same as FAST_MODE", 1 },
        /*174*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "@A1^B2?C3+D", 0, 0, 8, 32, 1, "F0 00 1C 5E 0B 2F C3 CE B1 1F 4D E1 79 04 2B BC 05 6C 38 73 39", "EDIFACT symbols_left 3, process_p 3", 0 },
        /*175*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3+D", 0, 0, 8, 32, 1, "F0 00 1C 5E 0B 2F C3 CE B1 1F 4D E1 79 04 2B BC 05 6C 38 73 39", "EEEEEEEEEEE", 0 },
        /*176*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "@A1^B2?C3+D4", 0, 0, 8, 32, 1, "F0 00 1C 5E 0B 2F C3 CE B1 34 F4 EC B3 DC 03 A3 1F B5 86 C3 F7", "EDIFACT symbols_left 0, process_p 0", 0 },
        /*177*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3+D4", 0, 0, 8, 32, 1, "F0 00 1C 5E 0B 2F C3 CE B1 34 F4 EC B3 DC 03 A3 1F B5 86 C3 F7", "EDIFACT symbols_left 0, process_p 0", 0 },
        /*178*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "@A1^B2?C3+D4=", 0, 0, 16, 16, 1, "F0 00 1C 5E 0B 2F C3 CE B1 34 3E 81 42 96 43 6E 92 0D A9 B1 65 3C CF 9B", "EDIFACT symbols_left 2, process_p 1", 0 },
        /*179*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3+D4=", 0, 0, 16, 16, 0, "F0 00 1C 5E 0B 2F C3 CE B1 34 3E 81 42 96 43 6E 92 0D A9 B1 65 3C CF 9B", "EEEEEEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /*180*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "@A1^B2?C3+D4=E", 0, 0, 16, 16, 1, "F0 00 1C 5E 0B 2F C3 CE B1 34 3E 46 AD 8C F2 D8 5D AF F3 65 08 1F E3 A5", "EDIFACT symbols_left 2, process_p 2", 0 },
        /*181*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3+D4=E", 0, 0, 16, 16, 0, "41 F0 07 17 82 CB F0 F3 AC 4D 3D 46 16 18 75 8F 52 46 7B 18 F6 68 F1 43", "AEEEEEEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /*182*/ { DATA_MODE | FAST_MODE, 0, -1, -1, -1, "\377\376", 0, 0, 12, 12, 1, "EB 80 EB 7F 81 6F A8 0F 21 6F 5F 88", "FN4 A7F FN4 A7E, 1 pad", 0 },
        /*183*/ { DATA_MODE, 0, -1, -1, -1, "\377\376", 0, 0, 12, 12, 1, "EB 80 EB 7F 81 6F A8 0F 21 6F 5F 88", "FN4 A7F FN4 A7E, 1 pad", 0 },
        /*184*/ { DATA_MODE | FAST_MODE, 0, -1, -1, -1, "\377\376\375", 0, 0, 12, 12, 1, "E7 2C C0 55 E9 67 45 8A D2 7E A9 23", "BAS BFF BFE BFD, no padding", 0 },
        /*185*/ { DATA_MODE, 0, -1, -1, -1, "\377\376\375", 0, 0, 12, 12, 1, "E7 2C C0 55 E9 67 45 8A D2 7E A9 23", "BAS BFF BFE BFD, no padding", 0 },
        /*186*/ { DATA_MODE | FAST_MODE, 3, -1, -1, -1, "\101\102\103\104\300\105\310", 0, 3, 16, 16, 0, "F1 04 42 43 E7 87 5B F1 03 1D 36 81 2C E3 87 24 2D FD 69 9C 87 FA 8A 73", "ECI 4 ASC A41 A42 BAS B43 B44 BC0 B45 BC8; BWIPP different encodation (uses only B256, same no. of codewords)", 0 },
        /*187*/ { DATA_MODE, 3, -1, -1, -1, "\101\102\103\104\300\105\310", 0, 3, 16, 16, 0, "F1 04 42 43 44 45 EB 41 46 EB 49 81 6E 28 91 70 A5 CC E6 32 B6 3F 15 1C", "ECI 4 ASC A41 A42 BAS B43 B44 BC0 B45 BC8; BWIPP different encodation (uses only B256, same no. of codewords)", 0 },
        /*188*/ { UNICODE_MODE | FAST_MODE, 26, -1, -1, -1, "ABCDÀEÈ", 0, 26, 12, 26, 1, "F1 1B E7 60 2D C4 5B F1 06 58 B3 C7 21 81 57 ED 3D C0 12 2E 6C 80 58 CC 2C 05 0D 31 FC 2D", "ECI 27 BAS B41 B42 B43 B44 BC3 B80 B45 BC3 B88", 0 },
        /*189*/ { UNICODE_MODE, 26, -1, -1, -1, "ABCDÀEÈ", 0, 26, 12, 26, 0, "F1 1B 42 43 44 45 E7 B2 06 58 B3 C7 21 81 57 ED A8 34 CC 27 DA F6 B0 4D B7 FC AF 96 56 57", "AAAABBBBB; BWIPP same as FAST_MODE", 0 },
        /*190*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "β", ZINT_WARN_USES_ECI, 9, 12, 12, 1, "Warning F1 0A EB 63 81 41 56 DA C0 3D 2D CC", "ECI 10 FN4 A62", 0 },
        /*191*/ { UNICODE_MODE, 0, -1, -1, -1, "β", ZINT_WARN_USES_ECI, 9, 12, 12, 1, "Warning F1 0A EB 63 81 41 56 DA C0 3D 2D CC", "ECI 10 FN4 A62", 0 },
        /*192*/ { UNICODE_MODE | FAST_MODE, 127, -1, -1, -1, "A", 0, 127, 12, 12, 1, "F1 80 01 42 81 14 A2 86 07 F5 27 30", "ECI 128 A41", 0 },
        /*193*/ { UNICODE_MODE, 127, -1, -1, -1, "A", 0, 127, 12, 12, 1, "F1 80 01 42 81 14 A2 86 07 F5 27 30", "ECI 128 A41", 0 },
        /*194*/ { UNICODE_MODE | FAST_MODE, 16382, -1, -1, -1, "A", 0, 16382, 12, 12, 1, "F1 BF FE 42 81 29 57 AA A0 92 B2 45", "ECI 16383 A41", 0 },
        /*195*/ { UNICODE_MODE, 16382, -1, -1, -1, "A", 0, 16382, 12, 12, 1, "F1 BF FE 42 81 29 57 AA A0 92 B2 45", "ECI 16383 A41", 0 },
        /*196*/ { UNICODE_MODE | FAST_MODE, 810899, -1, -1, -1, "A", 0, 810899, 12, 12, 1, "F1 CC 51 05 42 BB A5 A7 8A C6 6E 0F", "ECI 810900 A41", 0 },
        /*197*/ { UNICODE_MODE, 810899, -1, -1, -1, "A", 0, 810899, 12, 12, 1, "F1 CC 51 05 42 BB A5 A7 8A C6 6E 0F", "ECI 810900 A41", 0 },
        /*198*/ { UNICODE_MODE | ESCAPE_MODE | FAST_MODE, -1, -1, -1, -1, "[)>\\R05\\GA\\R\\E", 0, 0, 10, 10, 1, "EC 42 81 5D 17 49 F6 B6", "Macro05 A41", 0 },
        /*199*/ { UNICODE_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R05\\GA\\R\\E", 0, 0, 10, 10, 1, "EC 42 81 5D 17 49 F6 B6", "Macro05 A41", 0 },
        /*200*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHIJKLM*", 0, 0, 16, 16, 1, "EE 59 E9 6D 24 80 5F 93 9A FE 4E 2B 09 FF 50 A2 83 BE 32 E1 2F 17 1E F3", "C40 == X12, p_r_6_2_1 true", 0 },
        /*201*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJKLM*", 0, 0, 16, 16, 0, "42 F0 08 31 05 18 72 09 28 B3 0D 2B 63 64 96 E4 6C CE D3 47 9A 5F E8 70", "AEEEEEEEEEEEEA; BWIPP same as FAST_MODE", 0 },
        /*202*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "\015\015\015\015\015\015\015\015\015a\015\015\015\015\015\015\015", 0, 0, 12, 26, 1, "EE 00 01 00 01 00 01 FE 62 EE 00 01 00 01 FE 0E B5 9A 73 85 83 20 23 2C E0 EC EC BF 71 E0", "a not X12 encodable", 0 },
        /*203*/ { UNICODE_MODE, 0, -1, -1, -1, "\015\015\015\015\015\015\015\015\015a\015\015\015\015\015\015\015", 0, 0, 12, 26, 0, "EE 00 01 00 01 00 01 FE 62 0E 0E 0E 0E 0E 0E 0E F3 21 61 11 C7 BA 48 DE 65 86 FE 7E A2 F5", "XXXXXXXXXAAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*204*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, ".........a.......", 0, 0, 18, 18, 0, "(32) F0 BA EB AE BA EB AE B9 F0 62 2F 2F 2F 2F 2F 2F 2F 81 78 BE 1F 90 B8 89 73 66 DC BD", "a not EDIFACT encodable; BWIPP different encodation (switches to ASCII one dot before)", 0 },
        /*205*/ { UNICODE_MODE, 0, -1, -1, -1, ".........a.......", 0, 0, 18, 18, 0, "(32) 2F 2F F0 BA EB AE BA EB 9F 62 F0 BA EB AE BA EB 9F 81 F4 71 C0 AF 4F 8C 04 D9 F3 E3", "AAEEEEEEEAEEEEEEE; BWIPP different encodation", 0 },
        /*206*/ { GS1_MODE | FAST_MODE, 0, -1, -1, -1, "[90]........[91]....", 0, 0, 12, 26, 1, "E8 DC 2F 2F 2F 2F 2F 2F 2F 2F E8 DD 2F 2F 2F 2F C6 CC 13 68 0D 9D A9 A5 B8 D5 5A F3 7B 18", "Can't use GS1 EDIFACT if contains FNC1/GS", 0 },
        /*207*/ { GS1_MODE, 0, -1, -1, -1, "[90]........[91]....", 0, 0, 12, 26, 0, "E8 DC 2F F0 BA EB AE BA EB 9F E8 DD 2F 2F 2F 2F FE B0 C3 BC 8E AA AC CF 01 84 72 DF 59 6A", "AAAEEEEEEEAAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*208*/ { GS1_MODE | FAST_MODE, 0, -1, -1, -1, "[90]........", 0, 0, 8, 32, 1, "E8 DC F0 BA EB AE BA EB AE 81 B1 C0 AB DA A5 92 AF E2 05 DE 56", "Can use GS1 EDIFACT if no FNC1/GS", 0 },
        /*209*/ { GS1_MODE, 0, -1, -1, -1, "[90]........", 0, 0, 8, 32, 1, "E8 DC F0 BA EB AE BA EB AE 81 B1 C0 AB DA A5 92 AF E2 05 DE 56", "Can use GS1 EDIFACT if no FNC1/GS", 0 },
        /*210*/ { GS1_MODE | FAST_MODE, 0, -1, -1, -1, "[90]ABCDEFGH[91]ABCD", 0, 0, 12, 26, 1, "E8 DC E6 59 E9 6D 24 80 4A AA CE 59 E9 FE 45 81 6A 05 49 36 67 C8 00 DE 35 29 C5 9A 17 EA", "GS1 C40 ok", 0 },
        /*211*/ { GS1_MODE, 0, -1, -1, -1, "[90]ABCDEFGH[91]ABCD", 0, 0, 12, 26, 0, "E8 DC 42 E6 60 52 73 8D 83 84 52 17 60 52 FE 81 25 A6 B1 A9 6D 2E 30 9E A7 F7 F8 99 55 11", "AAACCCCCCCCCCCCCC; BWIPP same as FAST_MODE", 0 },
        /*212*/ { GS1_MODE | FAST_MODE, 0, -1, -1, -1, "[90]ABCD", 0, 0, 14, 14, 1, "E8 DC 42 43 44 45 81 38 98 32 8C 23 4D 87 5A 95 04 A7", "Final ASC unlatch", 0 },
        /*213*/ { GS1_MODE, 0, -1, -1, -1, "[90]ABCD", 0, 0, 14, 14, 1, "E8 DC 42 43 44 45 81 38 98 32 8C 23 4D 87 5A 95 04 A7", "AAAAAA", 0 },
        /*214*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, ">*\015>*\015>......", 0, 0, 12, 26, 0, "EE 0C A9 0C A9 FE 3F 2F 2F 2F 2F 2F 2F 81 57 ED 0F 29 C7 9D 5D 64 61 94 14 CD A8 DF 65 8D", "X12 then ASC; BWIPP switches to ASCII earlier", 1 },
        /*215*/ { UNICODE_MODE, 0, -1, -1, -1, ">*\015>*\015>......", 0, 0, 12, 26, 0, "3F 2B 0E 3F 2B 0E F0 FA EB AE BA EB 9F 81 57 ED E8 73 07 E0 2D 09 CD 94 36 26 E4 EB 34 6E", "AAAAAAEEEEEEE; BWIPP different encodation", 0 },
        /*216*/ { UNICODE_MODE | FAST_MODE, 0, 4, -1, -1, "ABCDEFGH", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 81 65 FB 93 F7 A4 EA B0 37 00 4F 4C 8A 99 0B 69", "", 0 },
        /*217*/ { UNICODE_MODE, 0, 4, -1, -1, "ABCDEFGH", 0, 0, 16, 16, 0, "F0 04 20 C4 14 61 C8 7C 81 65 FB 93 D7 12 24 0A 2D B8 CF C8 33 D8 87 D1", "EEEEEEEE; BWIPP same as FAST_MODE", 0 },
        /*218*/ { UNICODE_MODE | FAST_MODE, 0, 26, -1, -1, "@@@@@@@@@_", 0, 0, 8, 32, 0, "F0 00 00 00 00 00 1F 41 41 60 63 50 E0 FE 8B 46 16 38 FE 58 AD", "BWIPP switches to ASCII later", 0 },
        /*219*/ { UNICODE_MODE, 0, 26, -1, -1, "@@@@@@@@@_", 0, 0, 8, 32, 0, "41 F0 00 00 00 00 00 00 60 81 39 60 6A 49 C1 4A CD 3A 69 51 E0", "AEEEEEEEEA; BWIPP different encodation", 1 },
        /*220*/ { UNICODE_MODE | FAST_MODE, 0, 4, -1, -1, "@@@@@@@@@_", 0, 0, 16, 16, 0, "F0 00 00 00 00 00 1F 41 41 60 81 93 F9 B0 1A A1 B9 6C D3 01 38 CD 51 75", "BWIPP switches to ASCII later", 0 },
        /*221*/ { UNICODE_MODE, 0, 4, -1, -1, "@@@@@@@@@_", 0, 0, 16, 16, 0, "41 41 F0 00 00 00 00 00 1F 60 81 93 F3 2C 45 BF 89 58 D4 46 51 57 01 F3", "AAEEEEEEEA; BWIPP different encodation", 0 },
        /*222*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", 0, 0, 26, 26, 1, "(72) F0 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", "", 0 },
        /*223*/ { UNICODE_MODE, 0, -1, -1, -1, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", 0, 0, 26, 26, 1, "(72) F0 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE", 0 },
        /*224*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "ABCDEFGHIJK", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 80 5F FE 4B 4C D8 69 88 60 B9 33 B9 31 E6 BF CA", "", 0 },
        /*225*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJK", 0, 0, 8, 32, 0, "F0 04 20 C4 14 61 C8 24 A2 DF B6 77 90 C2 39 AA 9B 61 7E F1 E3", "EEEEEEEEEEE; BWIPP same as FAST_MODE", 0 },
        /*226*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "\001AAAAA", 0, 0, 14, 14, 1, "02 42 42 42 42 42 81 38 80 00 99 9E F5 8F 04 71 FB 4E", "", 0 },
        /*227*/ { UNICODE_MODE, 0, -1, -1, -1, "\001AAAAA", 0, 0, 14, 14, 0, "02 42 42 42 42 42 81 38 80 00 99 9E F5 8F 04 71 FB 4E", "AAAAAA; BWIPP same as FAST_MODE", 0 },
        /*228*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "\001AAAAAA", 0, 0, 14, 14, 1, "02 E6 59 BF 59 BF FE 81 EC 06 F3 00 0B 1F AA CB FC EB", "", 0 },
        /*229*/ { UNICODE_MODE, 0, -1, -1, -1, "\001AAAAAA", 0, 0, 14, 14, 0, "02 42 F0 04 10 41 42 81 9C E9 2B 19 D3 07 3C 11 0E 63", "AAEEEEA; BWIPP same as FAST_MODE", 0 },
        /*230*/ { DATA_MODE | FAST_MODE, 0, -1, -1, -1, "\200\076\076\076\076\076\076\076", 0, 0, 8, 32, 0, "E7 2D 41 EE 0C D3 0C D3 FE 3F 81 93 1A AE 50 D6 E0 FB 18 EA 2E", "BAS X12; BWIPP different encodation", 0 },
        /*231*/ { DATA_MODE, 0, -1, -1, -1, "\200\076\076\076\076\076\076\076", 0, 0, 14, 14, 0, "EB 01 3F EE 0C D3 0C D3 C3 98 B8 7F 21 0C 23 45 A9 27", "AAXXXXXX; BWIPP different encodation", 2 },
        /*232*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, ".....12", 0, 0, 14, 14, 1, "2F 2F 2F 2F 2F 8E 81 38 18 5E 9A 70 18 19 D0 35 7E 82", "ASC", 0 },
        /*233*/ { UNICODE_MODE, 0, -1, -1, -1, ".....12", 0, 0, 14, 14, 0, "2F 2F 2F 2F 2F 8E 81 38 18 5E 9A 70 18 19 D0 35 7E 82", "AAAAAAA; BWIPP same as FAST_MODE", 0 },
        /*234*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "aaaaaa", 0, 0, 12, 12, 1, "EF 59 BF 59 BF 1B 98 38 0A E1 70 F9", "ASC", 0 },
        /*235*/ { UNICODE_MODE, 0, -1, -1, -1, "aaaaaa", 0, 0, 12, 12, 1, "EF 59 BF 59 BF 1B 98 38 0A E1 70 F9", "TTTTTT", 0 },
        /*236*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "AAAAAAAAAAAAAA12a", 0, 0, 12, 26, 1, "E6 59 BF 59 BF 59 BF 59 BF 59 B6 25 D2 FE 81 ED 63 5A 30 1D C5 77 70 66 93 2B 6A 40 23 22", "", 0 },
        /*237*/ { UNICODE_MODE, 0, -1, -1, -1, "AAAAAAAAAAAAAA12a", 0, 0, 12, 26, 0, "E6 59 BF 59 BF 59 BF 59 BF FE 42 42 8E 62 81 ED 19 C5 3F C8 AC F1 E9 F5 68 75 AA 6C 14 72", "CCCCCCCCCCCCAAAAA; BWIPP same as FAST_MODE", 0 },
        /*238*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "1234ééé12", 0, 0, 8, 32, 0, "8E A4 E7 59 D5 6B EB 6A 8E 81 22 4D 35 A9 CA 58 B3 1B 33 B7 DD", "N12 N34 BAS BE9 BE9 ASC FN4 A69 N12; BWIPP same as non-FAST_MODE", 0 },
        /*239*/ { UNICODE_MODE, 0, -1, -1, -1, "1234ééé12", 0, 0, 14, 14, 1, "8E A4 E7 5A D5 6B 01 8E 1C 5C 58 A7 11 8D DD A8 D6 B9", "", 1 },
        /*240*/ { UNICODE_MODE, 0, -1, -1, -1, "A*B>C 1A*B>C 1\013*B>C 1A*B>C 1A*", 0, 0, 22, 22, 1, "(50) EE 57 B8 0F 04 21 72 5E 21 FE 21 32 0C EE 08 9B 64 7E 57 B8 0F 04 21 72 FE 81 68 FE", "Okapi datamatrix/ascii-within-x12.png", 1 },
        /*241*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "A*B>C 1A*B>C 1\013*B>C 1A*B>C 1A*", 0, 0, 22, 22, 1, "(50) EE 57 B8 0F 04 21 72 5E 21 FE 21 32 0C EE 08 9B 64 7E 57 B8 0F 04 21 72 FE 81 68 FE", "Okapi datamatrix/ascii-within-x12.png, process_p 2", 1 },
        /*242*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "A*B>C 1A*B>C \0131*B>C 1A*B>C 1A*", 0, 0, 22, 22, 1, "(50) EE 57 B8 0F 04 21 72 5E 21 FE 21 0C EE 1F 78 0F 04 21 72 5E 21 13 97 FE 2B 81 68 FE", "process_p 1", 1 },
        /*243*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "A*B>C 1A*B>C\013 1*B>C 1A*B>C 1A*", 0, 0, 22, 22, 1, "(50) EE 57 B8 0F 04 21 72 5E 21 FE 0C EE 13 8A 5E 21 13 97 08 9B 64 7E FE 42 2B 81 68 FE", "process_p 0", 1 },
        /*244*/ { UNICODE_MODE | FAST_MODE, 0, -1, -1, -1, "A*B>C 1A*B>C 1*\013B>C 1A*B>C 1A*", 0, 0, 22, 22, 1, "(50) EE 57 B8 0F 04 21 72 5E 21 13 8A FE 0C EE 5E 21 13 97 08 9B 64 7E FE 42 2B 81 68 FE", "process_p 0", 1 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[8192];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_input", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; /* Needed to get codeword dump in errtxt */

        length = testUtilSetSymbol(symbol, BARCODE_DATAMATRIX, data[i].input_mode, data[i].eci, -1 /*option_1*/, data[i].option_2, data[i].option_3, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, %s, \"%s\", %s, %d, %d, %d, %d, \"%s\", \"%s\", %d },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_2,
                    testUtilOption3Name(BARCODE_DATAMATRIX, data[i].option_3),
                    testUtilOutputOptionsName(data[i].output_options), testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
                    testUtilErrorName(data[i].ret), ret < ZINT_ERROR ? symbol->eci : -1, symbol->rows, symbol->width,
                    data[i].bwipp_cmp, symbol->errtxt, data[i].comment, data[i].expected_diff);
        } else {
            if (ret < ZINT_ERROR) {
                assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n", i, symbol->eci, data[i].expected_eci);
                assert_equal(symbol->rows, data[i].expected_rows, "i:%d rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d width %d != %d\n", i, symbol->width, data[i].expected_width);
            }
            assert_zero(strcmp(symbol->errtxt, data[i].expected), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected);

            if (ret < ZINT_ERROR) {
                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        char modules_dump[144 * 144 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, modules_dump);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, modules_dump);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[144 * 144 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }
            }

#ifdef ZINT_TEST_ENCODING
            if (ret < ZINT_ERROR) {
                if (i && (data[i].input_mode & 0x07) == (data[i - 1].input_mode & 0x07) && !(data[i].input_mode & FAST_MODE) && (data[i - 1].input_mode & FAST_MODE)
                        && data[i].eci == data[i - 1].eci && data[i].option_2 == data[i - 1].option_2
                        && data[i].option_3 == data[i - 1].option_3 && data[i].output_options == data[i - 1].output_options
                        && strcmp(data[i].data, data[i - 1].data) == 0) {
                    unsigned char binary[2][2200];
                    int gs1;
                    int binlen;
                    int binlens[2] = {0};
                    unsigned char reduced[1000];
                    unsigned char *text;

                    assert_equal(data[i].expected_rows * data[i].expected_width <= data[i - 1].expected_rows * data[i - 1].expected_width, 1,
                        "i:%d data[i].expected_rows * data[i].expected_width %d > data[i - 1].expected_rows * data[i - 1].expected_width %d\n", i,
                        data[i].expected_rows * data[i].expected_width, data[i - 1].expected_rows * data[i - 1].expected_width);

                    if ((data[i].input_mode & 0x07) == GS1_MODE) {
                        ret = gs1_verify(symbol, (unsigned char *) data[i].data, length, reduced);
                        assert_zero(ret, "i:%d gs1_verify() ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
                        length = (int) ustrlen(reduced);
                        text = reduced;
                    } else {
                        text = (unsigned char *) data[i].data;
                    }
                    binlen = 0;
                    symbol->input_mode = data[i - 1].input_mode;
                    gs1 = (symbol->input_mode & 0x07) != GS1_MODE ? 0 : (symbol->output_options & GS1_GS_SEPARATOR) ? 2 : 1;
                    ret = dm_encode_test(symbol, text, length, symbol->eci, gs1, binary[0], &binlen);
                    assert_zero(ret, "i:%d dm_encode() FAST_MODE ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

                    binlens[0] = binlen;

                    binlen = 0;
                    symbol->input_mode = data[i].input_mode;
                    gs1 = (symbol->input_mode & 0x07) != GS1_MODE ? 0 : (symbol->output_options & GS1_GS_SEPARATOR) ? 2 : 1;
                    ret = dm_encode_test(symbol, text, length, symbol->eci, gs1, binary[1], &binlen);
                    assert_zero(ret, "i:%d dm_encode() minimal ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

                    binlens[1] = binlen;

                    assert_equal(binlens[0], binlens[1] + data[i].expected_diff, "i:%d binlens[0] %d != %d binlens[1] (%d) + expected_diff (%d)\n",
                                i, binlens[0], binlens[1] + data[i].expected_diff, binlens[1], data[i].expected_diff);
                    if (data[i].expected_diff >= 0) {
                        assert_equal(binlens[1] <= binlens[0], 1, "i:%d binlens[1] %d > binlens[0] %d\n", i, binlens[1], binlens[0]);
                    }
                }
            }
#endif
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int eci;
        int output_options;
        int option_2;
        int option_3;
        char *data;
        int length;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        int expected_diff; /* Difference between default minimal encodation and ISO encodation (FAST_MODE) */
        char *expected;
    };
    /* Verified manually against ISO/IEC 16022:2006, ISO/IEC 21471:2020, GS1 General Specifications 21.0.1 (GGS), ANSI/HIBC LIC 2.6-2016 (HIBC/LIC),
       ANSI/HIBC PAS 1.3-2010 (HIBC/PAS) and AIM ITS/04-023:2022 (ECI Part 3: Register), with noted exceptions
    */
    static const struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, -1, "1234abcd", -1, 0, 14, 14, 1, "", 0,
                    "10101010101010"
                    "11001010001111"
                    "11000101100100"
                    "11001001100001"
                    "11011001110000"
                    "10100101011001"
                    "10101110011000"
                    "10011101100101"
                    "10100001001000"
                    "10101000001111"
                    "11101100000010"
                    "11010010100101"
                    "10011111000100"
                    "11111111111111"
                },
        /*  1*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "1234abcd", -1, 0, 14, 14, 1, "", 0,
                    "10101010101010"
                    "11001010001111"
                    "11000101100100"
                    "11001001100001"
                    "11011001110000"
                    "10100101011001"
                    "10101110011000"
                    "10011101100101"
                    "10100001001000"
                    "10101000001111"
                    "11101100000010"
                    "11010010100101"
                    "10011111000100"
                    "11111111111111"
                },
        /*  2*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, -1, "A1B2C3D4E5F6G7H8I9J0K1L2", -1, 0, 18, 18, 1, "16022:2006 Figure 1", 0,
                    "101010101010101010"
                    "101000101010001111"
                    "101100000111000010"
                    "100000101110100111"
                    "100100000100011100"
                    "101011111010101111"
                    "100110011111110110"
                    "110011101111111101"
                    "111111111101111100"
                    "101110110100101101"
                    "100010110101001110"
                    "101101111001100001"
                    "110001101010011110"
                    "110110100000100011"
                    "101101001101011010"
                    "100010011001011011"
                    "100011000000100100"
                    "111111111111111111"
                },
        /*  3*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "A1B2C3D4E5F6G7H8I9J0K1L2", -1, 0, 18, 18, 1, "16022:2006 Figure 1", 0,
                    "101010101010101010"
                    "101000101010001111"
                    "101100000111000010"
                    "100000101110100111"
                    "100100000100011100"
                    "101011111010101111"
                    "100110011111110110"
                    "110011101111111101"
                    "111111111101111100"
                    "101110110100101101"
                    "100010110101001110"
                    "101101111001100001"
                    "110001101010011110"
                    "110110100000100011"
                    "101101001101011010"
                    "100010011001011011"
                    "100011000000100100"
                    "111111111111111111"
                },
        /*  4*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, -1, "123456", -1, 0, 10, 10, 1, "16022:2006 Figure O.2", 0,
                    "1010101010"
                    "1100101101"
                    "1100000100"
                    "1100011101"
                    "1100001000"
                    "1000001111"
                    "1110110000"
                    "1111011001"
                    "1001110100"
                    "1111111111"
                },
        /*  5*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "123456", -1, 0, 10, 10, 1, "16022:2006 Figure O.2", 0,
                    "1010101010"
                    "1100101101"
                    "1100000100"
                    "1100011101"
                    "1100001000"
                    "1000001111"
                    "1110110000"
                    "1111011001"
                    "1001110100"
                    "1111111111"
                },
        /*  6*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, -1, "30Q324343430794<OQQ", -1, 0, 16, 16, 1, "16022:2006 Figure R.1", 0,
                    "1010101010101010"
                    "1010101010000001"
                    "1010101011101100"
                    "1010101010110011"
                    "1010101010001100"
                    "1010101010001101"
                    "1010101010000100"
                    "1010101001101001"
                    "1010101010000110"
                    "1000001001011001"
                    "1111111110000100"
                    "1101100110010101"
                    "1111111001100100"
                    "1110010111100101"
                    "1110010010100010"
                    "1111111111111111"
                },
        /*  7*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "30Q324343430794<OQQ", -1, 0, 16, 16, 1, "16022:2006 Figure R.1", 0,
                    "1010101010101010"
                    "1010101010000001"
                    "1010101011101100"
                    "1010101010110011"
                    "1010101010001100"
                    "1010101010001101"
                    "1010101010000100"
                    "1010101001101001"
                    "1010101010000110"
                    "1000001001011001"
                    "1111111110000100"
                    "1101100110010101"
                    "1111111001100100"
                    "1110010111100101"
                    "1110010010100010"
                    "1111111111111111"
                },
        /*  8*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, 32, -1, "A1B2C3D4E5F6G7H8I9J0K1L2", -1, 0, 8, 64, 1, "21471:2020 Figure 1", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1010001010111001101010111011111110001110000000011101010100010101"
                    "1011000000000110110111111110111010110000101001101101001000010110"
                    "1000001000110011110001111011111110001100101010111011111111001111"
                    "1001000111011100101010110001011010010111001000101111101011101100"
                    "1010101011111011100000000001000110101110011111011000101011000111"
                    "1101101100110110111100110000001011100011001001101011001001001000"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /*  9*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, 32, -1, "A1B2C3D4E5F6G7H8I9J0K1L2", -1, 0, 8, 64, 1, "21471:2020 Figure 1", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1010001010111001101010111011111110001110000000011101010100010101"
                    "1011000000000110110111111110111010110000101001101101001000010110"
                    "1000001000110011110001111011111110001100101010111011111111001111"
                    "1001000111011100101010110001011010010111001000101111101011101100"
                    "1010101011111011100000000001000110101110011111011000101011000111"
                    "1101101100110110111100110000001011100011001001101011001001001000"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /* 10*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, 31, -1, "123456789012345678901234567890123456", -1, 0, 8, 48, 1, "21471:2020 Figure H.3", 0,
                    "101010101010101010101010101010101010101010101010"
                    "110010010100000111110001101001010110100110001011"
                    "110011000111101000101010110111001110011011011010"
                    "110001110010001100001001100011001100110111000111"
                    "111010010010110111101000110001110001000011000000"
                    "100011010010011010111111111101010010100111000001"
                    "101101111110001011010010111010001111110101101110"
                    "111111111111111111111111111111111111111111111111"
                },
        /* 11*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, 31, -1, "123456789012345678901234567890123456", -1, 0, 8, 48, 1, "21471:2020 Figure H.3", 0,
                    "101010101010101010101010101010101010101010101010"
                    "110010010100000111110001101001010110100110001011"
                    "110011000111101000101010110111001110011011011010"
                    "110001110010001100001001100011001100110111000111"
                    "111010010010110111101000110001110001000011000000"
                    "100011010010011010111111111101010010100111000001"
                    "101101111110001011010010111010001111110101101110"
                    "111111111111111111111111111111111111111111111111"
                },
        /* 12*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, 29, -1, "30Q324343430794<OQ", -1, 0, 16, 36, 1, "21471:2020 Figure J.1 NOTE: single Q at end, not 2; also not DMRE", 0,
                    "101010101010101010101010101010101010"
                    "101010101000000101111110101011001101"
                    "101010101101111110111111101110001100"
                    "101010110010101011111000000010111101"
                    "101010100010101100110101100100100100"
                    "101010100100011001100110110111011001"
                    "101010100010101000110100010111010110"
                    "101010100110000111101110001110111001"
                    "101010101001110110110000111010000100"
                    "100000110100111111111110101010100001"
                    "111101010100100110110011100001011000"
                    "110100100100010001100111011111001111"
                    "101100100100000000111011111010010100"
                    "100111011010111111100011010011001001"
                    "101111001001100000110110100000010110"
                    "111111111111111111111111111111111111"
                },
        /* 13*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, 29, -1, "30Q324343430794<OQ", -1, 0, 16, 36, 1, "21471:2020 Figure J.1 NOTE: single Q at end, not 2; also not DMRE", 0,
                    "101010101010101010101010101010101010"
                    "101010101000000101111110101011001101"
                    "101010101101111110111111101110001100"
                    "101010110010101011111000000010111101"
                    "101010100010101100110101100100100100"
                    "101010100100011001100110110111011001"
                    "101010100010101000110100010111010110"
                    "101010100110000111101110001110111001"
                    "101010101001110110110000111010000100"
                    "100000110100111111111110101010100001"
                    "111101010100100110110011100001011000"
                    "110100100100010001100111011111001111"
                    "101100100100000000111011111010010100"
                    "100111011010111111100011010011001001"
                    "101111001001100000110110100000010110"
                    "111111111111111111111111111111111111"
                },
        /* 14*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, -1, "https://example.com/01/09506000134369", -1, 0, 22, 22, 0, "GGS Figure 2.1.13.1 (and 5.1-9) same; note not GS1; BWIPP different encodation", 0,
                    "1010101010101010101010"
                    "1100101110000001111101"
                    "1001010101111000101010"
                    "1000000100111100110001"
                    "1101010111000111100100"
                    "1101110001010000001101"
                    "1110100101010101011010"
                    "1101101101110101100111"
                    "1111010000000001100100"
                    "1100011001100101001111"
                    "1000111100001111010010"
                    "1011111101110101111011"
                    "1100110101001101101010"
                    "1011010101000110011101"
                    "1000000110110000011110"
                    "1011011100011000011001"
                    "1110000001110111001000"
                    "1100101000000101110001"
                    "1111101100001110001010"
                    "1110110101100110000101"
                    "1101100001100010010100"
                    "1111111111111111111111"
                },
        /* 15*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "https://example.com/01/09506000134369", -1, 0, 22, 22, 0, "GGS Figure 2.1.13.1 (and 5.1-9) **NOT SAME** (see FAST_MODE); BWIPP different encodation", 0,
                    "1010101010101010101010"
                    "1100101110011010111101"
                    "1001010101110111101110"
                    "1000000100111000010001"
                    "1101010111011000110100"
                    "1101110001110000100101"
                    "1110100101000101000000"
                    "1101101110110101011111"
                    "1111010011000001111100"
                    "1100011101100100110111"
                    "1000100000000000011010"
                    "1011000101110000101111"
                    "1100011101000100000010"
                    "1011010101010011111101"
                    "1000000100010111001110"
                    "1011011100010000110001"
                    "1110001101011000000000"
                    "1100101101101011001001"
                    "1111110111001111110010"
                    "1110000101101111110001"
                    "1101110001101001110000"
                    "1111111111111111111111"
                },
        /* 16*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, -1, -1, -1, "[01]09501101530003[17]150119[10]AB-123", -1, 0, 20, 20, 1, "GGS Figure 2.6.14-3", 0,
                    "10101010101010101010"
                    "11001111010100000111"
                    "10001010001001010100"
                    "10110011010100010001"
                    "11101010000001101010"
                    "10000100111011010111"
                    "10011010101101010110"
                    "11010001001110101001"
                    "11101000110100101100"
                    "11001111010111001101"
                    "10001010000001100000"
                    "11010000100010111011"
                    "10110010011000001000"
                    "10011010000011010011"
                    "11111010101110100110"
                    "11010010111011100001"
                    "11010100101100111110"
                    "11000001110010010101"
                    "10011011100101011010"
                    "11111111111111111111"
                },
        /* 17*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]09501101530003[17]150119[10]AB-123", -1, 0, 20, 20, 0, "GGS Figure 2.6.14-3 **NOT SAME** (see FAST_MODE); BWIPP same as FAST_MODE", 0,
                    "10101010101010101010"
                    "11001111010100110111"
                    "10001010001000000100"
                    "10110011010100010001"
                    "11101010000010000110"
                    "10000100111010000011"
                    "10011010010110110110"
                    "11010001110011000111"
                    "11101001001111011000"
                    "11001111110001101101"
                    "10001010011001111000"
                    "11010110100010110011"
                    "10100011111111110000"
                    "10000111011001111111"
                    "11001010111100001010"
                    "11010111111001011001"
                    "11011000000010110010"
                    "11000001110001000101"
                    "10011010100111101010"
                    "11111111111111111111"
                },
        /* 18*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, -1, -1, -1, "[01]04012345678901[21]ABCDEFG123456789", -1, 0, 20, 20, 1, "GGS Figure 2.6.14-4", 0,
                    "10101010101010101010"
                    "11011000001101000111"
                    "10001001100001110100"
                    "10110110110000010001"
                    "11100010000111110110"
                    "10101010110011101101"
                    "11111000100000100010"
                    "10010010001011110001"
                    "10101010110011010110"
                    "11011110011010001111"
                    "10001010011101010010"
                    "10111111011110110011"
                    "11110100101101011000"
                    "11010111011100100111"
                    "10000000011001100000"
                    "11101111110100001011"
                    "11010001001000101010"
                    "11010011101000100101"
                    "10001000100001111010"
                    "11111111111111111111"
                },
        /* 19*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]04012345678901[21]ABCDEFG123456789", -1, 0, 20, 20, 0, "GGS Figure 2.6.14-4 **NOT SAME** (see FAST_MODE); BWIPP same as FAST_MODE", 0,
                    "10101010101010101010"
                    "11011000000001000111"
                    "10001001100100101100"
                    "10110111100001100001"
                    "11100011100100010010"
                    "10101010001110001101"
                    "11111000011000001010"
                    "10010010011111000111"
                    "10101010111000110000"
                    "11011101100110100111"
                    "10001000101110110010"
                    "10111010011110011011"
                    "11111111110111101110"
                    "11000010011001101001"
                    "11101100000101001000"
                    "11101001011100110001"
                    "11100100011000111100"
                    "11001010100011000101"
                    "10001000111000110010"
                    "11111111111111111111"
                },
        /* 20*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, -1, -1, -1, "[01]04012345678901[17]170101[10]ABC123", -1, 0, 20, 20, 1, "GGS Figure 4.15-1 (and 5.1-6)", 0,
                    "10101010101010101010"
                    "11011000010100000111"
                    "10001001100001010100"
                    "10110111001100000001"
                    "11100010000101101110"
                    "10101100110001010101"
                    "11111010101000100110"
                    "10010011001000100011"
                    "10101000110010111010"
                    "11001111001010101111"
                    "10001010000111000010"
                    "10110000010101000011"
                    "11110010000001011000"
                    "11011000000110101111"
                    "11111010111011110110"
                    "11001001001110101111"
                    "11011010000001110010"
                    "11010111010111101101"
                    "10001000000101111010"
                    "11111111111111111111"
                },
        /* 21*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]04012345678901[17]170101[10]ABC123", -1, 0, 20, 20, 0, "GGS Figure 4.15-1 (and 5.1-6) **NOT SAME** (see FAST_MODE); BWIPP same as FAST_MODE", 0,
                    "10101010101010101010"
                    "11011000010100110111"
                    "10001001100000000100"
                    "10110111001100001001"
                    "11100010000010010010"
                    "10101100111000111001"
                    "11111010110001000010"
                    "10010011110100101101"
                    "10101001001100100000"
                    "11001111101110100111"
                    "10001010010101101010"
                    "10110110111100101011"
                    "11100011111111010000"
                    "11000010100100111011"
                    "11001101000111010110"
                    "11011110010111110111"
                    "11011001111011011110"
                    "11010110010111111101"
                    "10001010100111001010"
                    "11111111111111111111"
                },
        /* 22*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, GS1_GS_SEPARATOR, -1, -1, "[01]09504000059101[21]12345678p901[10]1234567p[17]141120[8200]http://www.gs1.org/demo/", -1, 0, 32, 32, 1, "GGS Figure 4.15.1-1 **NOT SAME**, uses 0-padded final TEXT triplet", 0,
                    "10101010101010101010101010101010"
                    "11001111010000111101100000101001"
                    "10001010011111001011011001000010"
                    "10111011001001111101111101000101"
                    "11100101000010001000011011011110"
                    "10000101001101111010111000100101"
                    "10010001000100101000000010011110"
                    "10010110011101011000101100101111"
                    "11101010110010001100011100110100"
                    "11011100110110111101010000010001"
                    "10001010000101001100001111001110"
                    "10110011010101111000010011100001"
                    "11101100100110101110101000010110"
                    "11100100000000111000000111111001"
                    "10100010111011101001110100100010"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11100011100010011011000101111111"
                    "11001100001101001110011000000010"
                    "10111011011110111100010111011101"
                    "11101011011111101101101100000110"
                    "10010000011010011111000000111111"
                    "11000101110000101100110001100100"
                    "10000010111011011111010001010011"
                    "10110011100011001101101100110110"
                    "10111010111101011000100010111001"
                    "10010011101110101110010011010110"
                    "11101011001100011011001001000001"
                    "10001011111000001010010011110000"
                    "11001011000110111111100011000001"
                    "10110110011000001010011010011000"
                    "11111111111111111111111111111111"
                },
        /* 23*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, GS1_GS_SEPARATOR, -1, -1, "[01]09504000059101[21]12345678p901[10]1234567p[17]141120[8200]http://www.gs1.org/demo/", -1, 0, 32, 32, 1, "GGS Figure 4.15.1-1 **NOT SAME**, uses 0-padded final TEXT triplet", 0,
                    "10101010101010101010101010101010"
                    "11001111010000111101100000101001"
                    "10001010011111001011011001000010"
                    "10111011001001111101111101000101"
                    "11100101000010001000011011011110"
                    "10000101001101111010111000100101"
                    "10010001000100101000000010011110"
                    "10010110011101011000101100101111"
                    "11101010110010001100011100110100"
                    "11011100110110111101010000010001"
                    "10001010000101001100001111001110"
                    "10110011010101111000010011100001"
                    "11101100100110101110101000010110"
                    "11100100000000111000000111111001"
                    "10100010111011101001110100100010"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11100011100010011011000101111111"
                    "11001100001101001110011000000010"
                    "10111011011110111100010111011101"
                    "11101011011111101101101100000110"
                    "10010000011010011111000000111111"
                    "11000101110000101100110001100100"
                    "10000010111011011111010001010011"
                    "10110011100011001101101100110110"
                    "10111010111101011000100010111001"
                    "10010011101110101110010011010110"
                    "11101011001100011011001001000001"
                    "10001011111000001010010011110000"
                    "11001011000110111111100011000001"
                    "10110110011000001010011010011000"
                    "11111111111111111111111111111111"
                },
        /* 24*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, -1, -1, -1, "[01]09504000059101[21]12345678p901[10]1234567p[17]141120[8200]http://www.gs1.org/demo/", -1, 0, 32, 32, 1, "GGS Figure 4.15.1-2 (and 4.15.1-3) **NOT SAME**, uses 0-padded final TEXT triplet (as does tec-it)", 0,
                    "10101010101010101010101010101010"
                    "11001111010000111101100000101001"
                    "10001010011111001011011001000010"
                    "10111011001001111101111101000101"
                    "11100101000010001000011011011110"
                    "10000101001101111010111000100101"
                    "10010001110100101000000010011110"
                    "10010110101101011000101100101111"
                    "11101010000010001100011100110100"
                    "11011100110110111101010000011001"
                    "10001010000101001100000111001010"
                    "10110011010101111000010011010101"
                    "11101100100110101110101111011110"
                    "11100100000000111000011101100111"
                    "10100010111011101010010111001010"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11111011100010011000001101110111"
                    "11010100001101101010111001101010"
                    "10100011011110111011111010110101"
                    "11101011011111001001000000111110"
                    "10010000011001011101001101100011"
                    "11000101110110001010111111110000"
                    "10000010110011111100011111010101"
                    "10110011011011001011010010110100"
                    "10111010110110111011000101100001"
                    "10010010000011101101011111110110"
                    "11101000010110011011011101111001"
                    "10001100011101101000101011001100"
                    "11000001011001011111011001010001"
                    "10110000011010101010011010011000"
                    "11111111111111111111111111111111"
                },
        /* 25*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]09504000059101[21]12345678p901[10]1234567p[17]141120[8200]http://www.gs1.org/demo/", -1, 0, 32, 32, 1, "GGS Figure 4.15.1-2 (and 4.15.1-3) **NOT SAME**, uses 0-padded final TEXT triplet (as does tec-it)", 0,
                    "10101010101010101010101010101010"
                    "11001111010000111101100000101001"
                    "10001010011111001011011001000010"
                    "10111011001001111101111101000101"
                    "11100101000010001000011011011110"
                    "10000101001101111010111000100101"
                    "10010001110100101000000010011110"
                    "10010110101101011000101100101111"
                    "11101010000010001100011100110100"
                    "11011100110110111101010000011001"
                    "10001010000101001100000111001010"
                    "10110011010101111000010011010101"
                    "11101100100110101110101111011110"
                    "11100100000000111000011101100111"
                    "10100010111011101010010111001010"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11111011100010011000001101110111"
                    "11010100001101101010111001101010"
                    "10100011011110111011111010110101"
                    "11101011011111001001000000111110"
                    "10010000011001011101001101100011"
                    "11000101110110001010111111110000"
                    "10000010110011111100011111010101"
                    "10110011011011001011010010110100"
                    "10111010110110111011000101100001"
                    "10010010000011101101011111110110"
                    "11101000010110011011011101111001"
                    "10001100011101101000101011001100"
                    "11000001011001011111011001010001"
                    "10110000011010101010011010011000"
                    "11111111111111111111111111111111"
                },
        /* 26*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]09524000059109[21]12345678p901[10]1234567p[17]271120", -1, 0, 22, 22, 1, "GGS v23 Figure 4.15.1-1 (and 4.15.1-3 and 4.15.1-4)", 0,
                    "1010101010101010101010"
                    "1100111101000011000111"
                    "1000101101111100100100"
                    "1011101100100110001001"
                    "1110010100001000011100"
                    "1000010100110100101101"
                    "1001000111010000011000"
                    "1001011010110001101111"
                    "1110101000001000100100"
                    "1101110011011000101001"
                    "1001101000010110100010"
                    "1011001101011011001001"
                    "1110110011101011011010"
                    "1110010101001100110001"
                    "1010001000011110011100"
                    "1111101111110110001111"
                    "1101011110011111010010"
                    "1010001011101101101101"
                    "1110010011011110100010"
                    "1001011100110011100111"
                    "1001110001100110100100"
                    "1111111111111111111111"
                },
        /* 27*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, -1, -1, -1, "[01]09512345678901[15]170810[21]abcde", -1, 0, 20, 20, 1, "GGS Figure 5.6.2-1", 0,
                    "10101010101010101010"
                    "11001111010111100111"
                    "10001010100101010100"
                    "10110111010010011001"
                    "11100010010101101110"
                    "10101100101000111101"
                    "11111010011000000110"
                    "10010011100000101101"
                    "10101001101011000010"
                    "11000110000100011111"
                    "10001011010001100010"
                    "10110000101001000011"
                    "11110011011001011100"
                    "11011101001000101011"
                    "11111101001000110110"
                    "11011100110000101111"
                    "11000100001111110000"
                    "11011111110010101101"
                    "10011010101001110010"
                    "11111111111111111111"
                },
        /* 28*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]09512345678901[15]170810[21]abcde", -1, 0, 20, 20, 1, "GGS Figure 5.6.2-1", 0,
                    "10101010101010101010"
                    "11001111010111100111"
                    "10001010100101010100"
                    "10110111010010011001"
                    "11100010010101101110"
                    "10101100101000111101"
                    "11111010011000000110"
                    "10010011100000101101"
                    "10101001101011000010"
                    "11000110000100011111"
                    "10001011010001100010"
                    "10110000101001000011"
                    "11110011011001011100"
                    "11011101001000101011"
                    "11111101001000110110"
                    "11011100110000101111"
                    "11000100001111110000"
                    "11011111110010101101"
                    "10011010101001110010"
                    "11111111111111111111"
                },
        /* 29*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, -1, -1, -1, "[01]00012345678905[17]040115", -1, 0, 12, 26, 1, "GGS Figure 5.6.3.1-1 (left)", 0,
                    "10101010101010101010101010"
                    "11001000010011010100111111"
                    "10001001100010001111001010"
                    "10110111011000001001001111"
                    "11100010100100100010001100"
                    "10101100110101011101101001"
                    "11111000111110101001010010"
                    "10011111101010111001100001"
                    "10101110101000110000010010"
                    "11001101010110000111100111"
                    "10001001100010100010100000"
                    "11111111111111111111111111"
                },
        /* 30*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]00012345678905[17]040115", -1, 0, 12, 26, 1, "GGS Figure 5.6.3.1-1 (left)", 0,
                    "10101010101010101010101010"
                    "11001000010011010100111111"
                    "10001001100010001111001010"
                    "10110111011000001001001111"
                    "11100010100100100010001100"
                    "10101100110101011101101001"
                    "11111000111110101001010010"
                    "10011111101010111001100001"
                    "10101110101000110000010010"
                    "11001101010110000111100111"
                    "10001001100010100010100000"
                    "11111111111111111111111111"
                },
        /* 31*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, -1, -1, DM_SQUARE, "[01]00012345678905[17]040115", -1, 0, 18, 18, 1, "GGS Figure 5.6.3.1-1 (right)", 0,
                    "101010101010101010"
                    "110010000100010101"
                    "100010011010111110"
                    "101101110001101111"
                    "111000100010101100"
                    "101011001100010001"
                    "111110000010101010"
                    "100101100110101101"
                    "101010101001110110"
                    "110011110001110011"
                    "100000111010111100"
                    "111100000000010111"
                    "111011010101010010"
                    "111111111001001011"
                    "100011100101010000"
                    "111100011101000011"
                    "100000101110000100"
                    "111111111111111111"
                },
        /* 32*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, DM_SQUARE, "[01]00012345678905[17]040115", -1, 0, 18, 18, 1, "GGS Figure 5.6.3.1-1 (right)", 0,
                    "101010101010101010"
                    "110010000100010101"
                    "100010011010111110"
                    "101101110001101111"
                    "111000100010101100"
                    "101011001100010001"
                    "111110000010101010"
                    "100101100110101101"
                    "101010101001110110"
                    "110011110001110011"
                    "100000111010111100"
                    "111100000000010111"
                    "111011010101010010"
                    "111111111001001011"
                    "100011100101010000"
                    "111100011101000011"
                    "100000101110000100"
                    "111111111111111111"
                },
        /* 33*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, -1, -1, -1, "[01]00012345678905[17]180401[21]ABCDEFGHIJKL12345678[91]ABCDEFGHI123456789[92]abcdefghi", -1, 0, 32, 32, 0, "GGS Figure 5.6.3.2-3 (left) **NOT SAME** different encodation; BWIPP different encodation", 0,
                    "10101010101010101010101010101010"
                    "11001000010111111000100110101011"
                    "10001001100001101100110010100010"
                    "10110111001101111110011001000111"
                    "11100010001100101100101001011110"
                    "10101101101011111110000000100101"
                    "11111010010010101101000010011110"
                    "10010100101111011101101100101111"
                    "10101000101101101111111100110100"
                    "11001110011000111111110000001001"
                    "10001010001010101101001111001110"
                    "11110001000000111101011100101101"
                    "11010001111011001000011010000010"
                    "11011100101001111001000111111111"
                    "10111100101001101111011101000010"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11010100111011111001101111100111"
                    "11100111010011001011100001001010"
                    "11111001010111011101111000110011"
                    "11110000000010101101001110000110"
                    "11101110000001111011101000010101"
                    "11110101001101101101110000001000"
                    "10101011000111111010111001100111"
                    "11000101010010001100000011101010"
                    "11111101110111011001111011001101"
                    "11010111011010001000011101001010"
                    "10100111111110111101010111100011"
                    "10111011111010001001001100101110"
                    "10010101001110111101000101111101"
                    "11110110001001001010110111010110"
                    "11111111111111111111111111111111"
                },
        /* 34*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]00012345678905[17]180401[21]ABCDEFGHIJKL12345678[91]ABCDEFGHI123456789[92]abcdefghi", -1, 0, 32, 32, 0, "GGS Figure 5.6.3.2-3 (left) **NOT SAME** different encodation; BWIPP different encodation", 2,
                    "10101010101010101010101010101010"
                    "11001000010111111001100001001011"
                    "10001001100001101101000101000010"
                    "10110111001101111011101101000111"
                    "11100010001100101000100111011110"
                    "10101101101011111010001000100101"
                    "11111010010010101110101010011110"
                    "10010100101111011100001100101111"
                    "10101000101101101011011100110100"
                    "11001110011000111101110000001001"
                    "10001010001011101101001111010110"
                    "11110001000011111101011001001101"
                    "11010001111011001000010001011010"
                    "11011100100111111001010011011101"
                    "10111100100011101100111011001100"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "11010100101011111001011111001111"
                    "11100111111011101011111011100010"
                    "11111000010111111101111111010011"
                    "11111110100011001011001111101110"
                    "11100110000000111010010011110001"
                    "11110111000011001010111110001100"
                    "10101011001110111001110110001011"
                    "11000101100111101010111001100010"
                    "11111101001100011001010010000101"
                    "11010101100010001011111011111010"
                    "10100100100100111100001111011011"
                    "10111011101111001100101011101110"
                    "10011101001101011100111000111001"
                    "11110000001011001010110001000110"
                    "11111111111111111111111111111111"
                },
        /* 35*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, -1, 30, -1, "[01]00012345678905[17]180401[21]ABCDEFGHIJKL12345678[91]abcdefghi", -1, 0, 16, 48, 1, "GGS Figure 5.6.3.2-3 (right) **NOT SAME** different encodation", 0,
                    "101010101010101010101010101010101010101010101010"
                    "110010000101111001000011101101100100111011001111"
                    "100010011000011101111100100100011000110010111100"
                    "101101110011011100100111100111101110111110100011"
                    "111000100011001100111010101000011110001110111110"
                    "101011011010110100101101100100010101101110101111"
                    "111110100100100101111000101001100001101001001010"
                    "100101001011111001111001110110100101101100010111"
                    "101010001011111100111100100010000000011110001100"
                    "110011100101101010111011110111110111100111011001"
                    "100010100011010010111100100100101000001000011110"
                    "111100010010101101110001100001000001010110001001"
                    "110100001000101000011100101010101100011001001010"
                    "110111001110010000011111101111000110100011011011"
                    "101110001010001011101010101101111111111000000100"
                    "111111111111111111111111111111111111111111111111"
                },
        /* 36*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, 30, -1, "[01]00012345678905[17]180401[21]ABCDEFGHIJKL12345678[91]abcdefghi", -1, 0, 16, 48, 0, "GGS Figure 5.6.3.2-3 (right) **NOT SAME** different encodation; BWIPP same as FAST_MODE", 2,
                    "101010101010101010101010101010101010101010101010"
                    "110010000101111001000011101101100100111011001111"
                    "100010011000011101111100100100011000110010111100"
                    "101101110011011100100111100111101110111110100011"
                    "111000100011001100111010101000011110001110111110"
                    "101011011010110100101101100100010101101110101111"
                    "111110100100100101111000101001100001101001001010"
                    "100101001011111001111001110110100101101100010111"
                    "101010001011111100111100100010000000011110001100"
                    "110011100101101010111011110111110111100111011001"
                    "100010100011010010111100100100101000001000011110"
                    "111100010010101101110001100001000001010110001001"
                    "110100001000101000011100101010101100011001001010"
                    "110111001110010000011111101111000110100011011011"
                    "101110001010001011101010101101111111111000000100"
                    "111111111111111111111111111111111111111111111111"
                },
        /* 37*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, -1, -1, DM_SQUARE, "[00]395011010013000129[403]123+1021JK+0320+12[421]5281500KM", -1, 0, 24, 24, 1, "GGS Figure 6.6.5-6 **NOT SAME** figure has unnecessary FNC1 at end of data", 0,
                    "101010101010101010101010"
                    "110001110100011010101101"
                    "100010100100101000011000"
                    "101000110001001011100001"
                    "111010110110100001100010"
                    "100001001001010100001111"
                    "100110110111100000000100"
                    "100101110011001001100001"
                    "110000010110101011100010"
                    "110011001100011101110101"
                    "100000100011111010000000"
                    "101010110110011011000001"
                    "111010001001101000110000"
                    "110001000100011110101101"
                    "101011011101110000101000"
                    "110111000100011101011111"
                    "110101001010111101000010"
                    "110000111110111111111001"
                    "100101110010001010110110"
                    "111011010010000000100111"
                    "100110111101001000000100"
                    "101111000010010011111101"
                    "111001011011101100011010"
                    "111111111111111111111111"
                },
        /* 38*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, DM_SQUARE, "[00]395011010013000129[403]123+1021JK+0320+12[421]5281500KM", -1, 0, 24, 24, 1, "GGS Figure 6.6.5-6 **NOT SAME** figure has unnecessary FNC1 at end of data", 0,
                    "101010101010101010101010"
                    "110001110100011010101101"
                    "100010100100101000011000"
                    "101000110001001011100001"
                    "111010110110100001100010"
                    "100001001001010100001111"
                    "100110110111100000000100"
                    "100101110011001001100001"
                    "110000010110101011100010"
                    "110011001100011101110101"
                    "100000100011111010000000"
                    "101010110110011011000001"
                    "111010001001101000110000"
                    "110001000100011110101101"
                    "101011011101110000101000"
                    "110111000100011101011111"
                    "110101001010111101000010"
                    "110000111110111111111001"
                    "100101110010001010110110"
                    "111011010010000000100111"
                    "100110111101001000000100"
                    "101111000010010011111101"
                    "111001011011101100011010"
                    "111111111111111111111111"
                },
        /* 39*/ { BARCODE_DATAMATRIX, GS1_MODE | FAST_MODE, -1, -1, -1, -1, "[00]093123450000000012[421]0362770[401]931234518430GR[403]MEL", -1, 0, 24, 24, 1, "GGS Figure 6.6.5-7 **NOT SAME** different encodation", 0,
                    "101010101010101010101010"
                    "110011100101100110110101"
                    "100010001001111010000100"
                    "101001110100001011100011"
                    "111000110111000001101010"
                    "101011010010100110000111"
                    "101110011000011000001100"
                    "100101101011010001101001"
                    "110000100101011011101110"
                    "110010010110101101110001"
                    "100010101101001011010000"
                    "101010001000100000011001"
                    "101000101010000010000100"
                    "100110101110101100010101"
                    "100010010000111001000000"
                    "101000001001111000011011"
                    "111110000111001110000010"
                    "110001110101001101101111"
                    "110111110101100111101110"
                    "110100010010101011110101"
                    "110011000001011110100010"
                    "100111010001010011000101"
                    "101110011001110010101010"
                    "111111111111111111111111"
                },
        /* 40*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[00]093123450000000012[421]0362770[401]931234518430GR[403]MEL", -1, 0, 24, 24, 1, "GGS Figure 6.6.5-7 **NOT SAME** different encodation", 0,
                    "101010101010101010101010"
                    "110011100101100110110101"
                    "100010001001111010000100"
                    "101001110100001011100011"
                    "111000110111000001101010"
                    "101011010010100110000111"
                    "101110011000011000001100"
                    "100101101011010001101001"
                    "110000100101011011101110"
                    "110010010110101101110001"
                    "100010101101001011010000"
                    "101010001000100000011001"
                    "101000101010000010000100"
                    "100110101110101100010101"
                    "100010010000111001000000"
                    "101000001001111000011011"
                    "111110000111001110000010"
                    "110001110101001101101111"
                    "110111110101100111101110"
                    "110100010010101011110101"
                    "110011000001011110100010"
                    "100111010001010011000101"
                    "101110011001110010101010"
                    "111111111111111111111111"
                },
        /* 41*/ { BARCODE_HIBC_DM, FAST_MODE, -1, -1, -1, -1, "A123BJC5D6E71", -1, 0, 16, 16, 0, "HIBC/LIC Figure 3 same; BWIPP different encodation, same no. of codewords", 0,
                    "1010101010101010"
                    "1011101001111011"
                    "1000001100010100"
                    "1010101011001111"
                    "1100110100101100"
                    "1011000001011001"
                    "1100010011110100"
                    "1000101001100101"
                    "1010111011100000"
                    "1011000011100011"
                    "1000101100111010"
                    "1001011100001101"
                    "1010010001110100"
                    "1101010010001101"
                    "1111100111000010"
                    "1111111111111111"
                },
        /* 42*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "A123BJC5D6E71", -1, 0, 16, 16, 0, "HIBC/LIC Figure 3 **NOT SAME** (see FAST_MODE); BWIPP different encodation", 0,
                    "1010101010101010"
                    "1011101100110101"
                    "1000110001101100"
                    "1010001011110101"
                    "1100100001000100"
                    "1010011000100001"
                    "1001000111111100"
                    "1001000100111101"
                    "1110000011010110"
                    "1011010000000001"
                    "1001101100100000"
                    "1101101100100111"
                    "1101001110100100"
                    "1101110011110101"
                    "1000100111000010"
                    "1111111111111111"
                },
        /* 43*/ { BARCODE_HIBC_DM, FAST_MODE, -1, -1, -1, -1, "A123BJC5D6E71/$$52001510X3", -1, 0, 20, 20, 0, "HIBC/LIC Section 4.3.3 **NOT SAME** different encodation; also figure has weird CRLF after check digit; BWIPP different encodation", 0,
                    "10101010101010101010"
                    "10111011000101100001"
                    "10000110011010101100"
                    "10100001100011010101"
                    "11010110011011000000"
                    "10001010010000010001"
                    "11011000100100111110"
                    "10000110010001101111"
                    "10011010001111111010"
                    "10111100100101111001"
                    "10000110000111010110"
                    "11010111000101001111"
                    "10000011101001011100"
                    "10110001100101000001"
                    "11100001000111011110"
                    "10001010000000000101"
                    "10010101101111110110"
                    "11011011000010010101"
                    "10010010010000100010"
                    "11111111111111111111"
                },
        /* 44*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "A123BJC5D6E71/$$52001510X3", -1, 0, 20, 20, 0, "HIBC/LIC Section 4.3.3 **NOT SAME** different encodation; also figure has weird CRLF after check digit; BWIPP different encodation", 0,
                    "10101010101010101010"
                    "10111011011011011001"
                    "10001100010010100100"
                    "10100010010001101101"
                    "11001000111011101010"
                    "10100110110001011101"
                    "10010011100100100100"
                    "10010111010000000011"
                    "11100010001011101110"
                    "10110100100011101111"
                    "10010110001000010010"
                    "11010110110001101101"
                    "10000011111100100100"
                    "10110011010111011001"
                    "11100010111111111010"
                    "10000110011101011111"
                    "10011000110011110100"
                    "11000101010100010101"
                    "10011111101001100010"
                    "11111111111111111111"
                },
        /* 45*/ { BARCODE_HIBC_DM, FAST_MODE, -1, -1, -1, -1, "H123ABC01234567890", -1, 0, 12, 26, 1, "HIBC/LIC Figure C2, same", 0,
                    "10101010101010101010101010"
                    "10111011011011110101001101"
                    "10010110000001001100110100"
                    "10010001010100001011110001"
                    "11010101011010110100111100"
                    "10000101110000001110001101"
                    "11011011110011001011100000"
                    "10010001101011100010001001"
                    "10000001101101100110101010"
                    "11001111011110011111010001"
                    "10010010001100110000011010"
                    "11111111111111111111111111"
                },
        /* 46*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "H123ABC01234567890", -1, 0, 12, 26, 1, "HIBC/LIC Figure C2, same", 0,
                    "10101010101010101010101010"
                    "10111011011011110101001101"
                    "10010110000001001100110100"
                    "10010001010100001011110001"
                    "11010101011010110100111100"
                    "10000101110000001110001101"
                    "11011011110011001011100000"
                    "10010001101011100010001001"
                    "10000001101101100110101010"
                    "11001111011110011111010001"
                    "10010010001100110000011010"
                    "11111111111111111111111111"
                },
        /* 47*/ { BARCODE_HIBC_DM, FAST_MODE, -1, -1, -1, DM_SQUARE, "/ACMRN123456/V200912190833", -1, 0, 20, 20, 1, "HIBC/PAS Section 2.2 Patient Id, same", 0,
                    "10101010101010101010"
                    "10001000010011001001"
                    "11100110001010110100"
                    "10000010111001010101"
                    "11011100101010111100"
                    "10010001110010100001"
                    "11011110100100100110"
                    "10110000100000101111"
                    "11111100011100001000"
                    "11001011011010001101"
                    "11001010111110110000"
                    "11000001111100001111"
                    "11110010001100000100"
                    "10011011100010110011"
                    "11111100110000111110"
                    "11000110111111110001"
                    "11000001011001100110"
                    "10101010010101100101"
                    "10000100100110010010"
                    "11111111111111111111"
                },
        /* 48*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, DM_SQUARE, "/ACMRN123456/V200912190833", -1, 0, 20, 20, 0, "HIBC/PAS Section 2.2 Patient Id **NOT SAME** (see FAST_MODE); BWIPP same as FAST_MODE", 0,
                    "10101010101010101010"
                    "11000000010011001111"
                    "11010001101010110110"
                    "11101100111001000001"
                    "10001010101010111010"
                    "10100001110011010001"
                    "10011110100100111110"
                    "11110000100001110011"
                    "11111100011101010100"
                    "11001011000010111101"
                    "11001010110111011000"
                    "11000001100110010111"
                    "11110010111001001100"
                    "10011101100110011111"
                    "11111010100111110110"
                    "11011101010001001011"
                    "11011101110001110010"
                    "11110001110101111101"
                    "11100110100110001010"
                    "11111111111111111111"
                },
        /* 49*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE | FAST_MODE, -1, -1, -1, -1, "[)>\\R06\\G+/ACMRN123456/V2009121908334\\R\\E", -1, 0, 20, 20, 1, "HIBC/PAS 1.3-2010 Section 2.2 Patient Id Macro, same", 0,
                    "10101010101010101010"
                    "10000000001110001111"
                    "11010101001010011100"
                    "11000000011100110101"
                    "11011001101011001100"
                    "11001100000100010001"
                    "11110111101011000100"
                    "11010010001101100001"
                    "11110010010110011110"
                    "11010010010000010011"
                    "10010001100010110000"
                    "11101100100001000111"
                    "11101010000011111100"
                    "11000010000101001011"
                    "11001110111110010010"
                    "11000010110100011101"
                    "11001011001001011100"
                    "10010110010000010101"
                    "11100110001010111010"
                    "11111111111111111111"
                },
        /* 50*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R06\\G+/ACMRN123456/V2009121908334\\R\\E", -1, 0, 20, 20, 0, "HIBC/PAS 1.3-2010 Section 2.2 Patient Id Macro **NOT SAME** (see FAST_MODE); BWIPP same as FAST_MODE", 0,
                    "10101010101010101010"
                    "11111011001110001111"
                    "11100100001010011100"
                    "10000000011100111101"
                    "11000111101011011100"
                    "11110100000101011011"
                    "10101111101010010000"
                    "10010010001010101111"
                    "11110010010110111010"
                    "11010010011010110101"
                    "10010001110101110010"
                    "11101101100101000111"
                    "11101010010100101100"
                    "11000110100010000111"
                    "11001111001101111110"
                    "11010111100010100101"
                    "11011001111000110010"
                    "11011001110001011101"
                    "11011111001010100010"
                    "11111111111111111111"
                },
        /* 51*/ { BARCODE_HIBC_DM, FAST_MODE, -1, -1, -1, -1, "/EO523201", -1, 0, 14, 14, 1, "HIBC/PAS Section 2.2 Purchase Order, same", 0,
                    "10101010101010"
                    "10011001010101"
                    "11101000011010"
                    "10001100011101"
                    "11101100101100"
                    "10100001101111"
                    "10010001010110"
                    "10000001011001"
                    "11100000010100"
                    "11011010100101"
                    "10111110101110"
                    "11110000101101"
                    "10010010000100"
                    "11111111111111"
                },
        /* 52*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/EO523201", -1, 0, 14, 14, 1, "HIBC/PAS Section 2.2 Purchase Order, same", 0,
                    "10101010101010"
                    "10011001010101"
                    "11101000011010"
                    "10001100011101"
                    "11101100101100"
                    "10100001101111"
                    "10010001010110"
                    "10000001011001"
                    "11100000010100"
                    "11011010100101"
                    "10111110101110"
                    "11110000101101"
                    "10010010000100"
                    "11111111111111"
                },
        /* 53*/ { BARCODE_HIBC_DM, FAST_MODE, -1, -1, -1, DM_SQUARE, "/EU720060FF0/O523201", -1, 0, 18, 18, 1, "HIBC/PAS Section 2.2 2nd Purchase Order, same", 0,
                    "101010101010101010"
                    "100110010100100001"
                    "111011110110010110"
                    "100000101110011001"
                    "111001001010000100"
                    "100000000000011101"
                    "100101100000101110"
                    "111000000111111011"
                    "110110111000101010"
                    "101001000111000111"
                    "100011110101010110"
                    "111111001101010011"
                    "100000000001101000"
                    "110100100011011111"
                    "111000100110101110"
                    "111010100101000011"
                    "111000010011001010"
                    "111111111111111111"
                },
        /* 54*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, DM_SQUARE, "/EU720060FF0/O523201", -1, 0, 18, 18, 1, "HIBC/PAS Section 2.2 2nd Purchase Order, same", 0,
                    "101010101010101010"
                    "100110010100100001"
                    "111011110110010110"
                    "100000101110011001"
                    "111001001010000100"
                    "100000000000011101"
                    "100101100000101110"
                    "111000000111111011"
                    "110110111000101010"
                    "101001000111000111"
                    "100011110101010110"
                    "111111001101010011"
                    "100000000001101000"
                    "110100100011011111"
                    "111000100110101110"
                    "111010100101000011"
                    "111000010011001010"
                    "111111111111111111"
                },
        /* 55*/ { BARCODE_HIBC_DM, FAST_MODE, -1, -1, -1, -1, "/EU720060FF0/O523201/Z34H159/M9842431340", -1, 0, 22, 22, 1, "HIBC/PAS Section 2.2 3rd Purchase Order (left), same", 0,
                    "1010101010101010101010"
                    "1001100101001000000011"
                    "1110111101100001111010"
                    "1000001011101100111111"
                    "1110010010010000111100"
                    "1000000000011100000111"
                    "1001011010011000001110"
                    "1110000010001001101001"
                    "1101100110001010100100"
                    "1010010011011101000101"
                    "1000100011010000001110"
                    "1111010100101000010111"
                    "1000001001011011101110"
                    "1111110111111101100011"
                    "1001010110011010000000"
                    "1101010100110100010011"
                    "1001010011000110000000"
                    "1111001010100101110111"
                    "1100110010110011010000"
                    "1100001011100001000111"
                    "1010110000010001001000"
                    "1111111111111111111111"
                },
        /* 56*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/EU720060FF0/O523201/Z34H159/M9842431340", -1, 0, 22, 22, 1, "HIBC/PAS Section 2.2 3rd Purchase Order (left), same", 0,
                    "1010101010101010101010"
                    "1001100101001000000011"
                    "1110111101100001111010"
                    "1000001011101100111111"
                    "1110010010010000111100"
                    "1000000000011100000111"
                    "1001011010011000001110"
                    "1110000010001001101001"
                    "1101100110001010100100"
                    "1010010011011101000101"
                    "1000100011010000001110"
                    "1111010100101000010111"
                    "1000001001011011101110"
                    "1111110111111101100011"
                    "1001010110011010000000"
                    "1101010100110100010011"
                    "1001010011000110000000"
                    "1111001010100101110111"
                    "1100110010110011010000"
                    "1100001011100001000111"
                    "1010110000010001001000"
                    "1111111111111111111111"
                },
        /* 57*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE | FAST_MODE, -1, -1, -1, -1, "[)>\\R06\\G+/EU720060FF0/O523201/Z34H159/M9842431340V\\R\\E", -1, 0, 22, 22, 1, "HIBC/PAS Section 2.2 3rd Purchase Order (right), same", 0,
                    "1010101010101010101010"
                    "1000000000111010011101"
                    "1101011100101001011100"
                    "1100010000000001101001"
                    "1111110110000111100000"
                    "1100100000110011001101"
                    "1001011001000010000110"
                    "1000100101110111110111"
                    "1100001001110111111100"
                    "1011111001001010001101"
                    "1000011000010100101010"
                    "1111001101110100101101"
                    "1110001101101100001100"
                    "1001010101111010110011"
                    "1000110111011100101010"
                    "1111110011011111010101"
                    "1101000011100111101110"
                    "1011000010010100110111"
                    "1001110101111101000000"
                    "1110101001011011000111"
                    "1001110110011101101000"
                    "1111111111111111111111"
                },
        /* 58*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R06\\G+/EU720060FF0/O523201/Z34H159/M9842431340V\\R\\E", -1, 0, 22, 22, 1, "HIBC/PAS Section 2.2 3rd Purchase Order (right), same", 0,
                    "1010101010101010101010"
                    "1000000000111010011101"
                    "1101011100101001011100"
                    "1100010000000001101001"
                    "1111110110000111100000"
                    "1100100000110011001101"
                    "1001011001000010000110"
                    "1000100101110111110111"
                    "1100001001110111111100"
                    "1011111001001010001101"
                    "1000011000010100101010"
                    "1111001101110100101101"
                    "1110001101101100001100"
                    "1001010101111010110011"
                    "1000110111011100101010"
                    "1111110011011111010101"
                    "1101000011100111101110"
                    "1011000010010100110111"
                    "1001110101111101000000"
                    "1110101001011011000111"
                    "1001110110011101101000"
                    "1111111111111111111111"
                },
        /* 59*/ { BARCODE_HIBC_DM, FAST_MODE, -1, -1, -1, -1, "/E+/KN12345", -1, 0, 16, 16, 1, "HIBC/PAS Section 2.2 Asset Tag **NOT SAME** check digit 'A' in figure is for '/KN12345', but actual data is as given here, when check digit is 'J'", 0,
                    "1010101010101010"
                    "1001101010001111"
                    "1110001000101100"
                    "1000110100101101"
                    "1101000000110010"
                    "1000101001000001"
                    "1110000111001100"
                    "1010001101111101"
                    "1111101010101000"
                    "1101100101010001"
                    "1100001011010010"
                    "1100001111001001"
                    "1100010100000110"
                    "1010001101001101"
                    "1001000000000010"
                    "1111111111111111"
                },
        /* 60*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/E+/KN12345", -1, 0, 16, 16, 0, "HIBC/PAS Section 2.2 Asset Tag **NOT SAME** see above; BWIPP same as FAST_MODE", 0,
                    "1010101010101010"
                    "1100011011001011"
                    "1101100111000110"
                    "1110111101000011"
                    "1010100000010010"
                    "1011001001000111"
                    "1001100110111110"
                    "1100001111011001"
                    "1011101101000110"
                    "1001011010011111"
                    "1111001001000000"
                    "1101100000100011"
                    "1100110101110000"
                    "1100011111111101"
                    "1001111001100010"
                    "1111111111111111"
                },
        /* 61*/ { BARCODE_HIBC_DM, FAST_MODE, -1, -1, -1, -1, "/LAH123/NC903", -1, 0, 16, 16, 1, "HIBC/PAS Section 2.2 Surgical Instrument, same", 0,
                    "1010101010101010"
                    "1001010001010001"
                    "1110010100000100"
                    "1000001100000011"
                    "1110001100101000"
                    "1000111111100001"
                    "1011001110000100"
                    "1100110000001101"
                    "1000001110010000"
                    "1011001110111111"
                    "1001011010011010"
                    "1111000110111011"
                    "1010010101000100"
                    "1011001110110101"
                    "1100000101010010"
                    "1111111111111111"
                },
        /* 62*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/LAH123/NC903", -1, 0, 16, 16, 0, "HIBC/PAS Section 2.2 Surgical Instrument **NOT SAME** (see FAST_MODE); BWIPP same as FAST_MODE", 0,
                    "1010101010101010"
                    "1111000001001111"
                    "1110010001010110"
                    "1000001101100111"
                    "1111000100100100"
                    "1000001111111001"
                    "1011111000101100"
                    "1010000000111101"
                    "1001111100000100"
                    "1001101000011111"
                    "1111010001010110"
                    "1000001001111101"
                    "1111101010000010"
                    "1101010100000101"
                    "1110011111101010"
                    "1111111111111111"
                },
        /* 63*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE | FAST_MODE, -1, -1, 7, -1, "[)>\\R06\\G18VD89536\\G1P8902A\\GS3122A02965\\R\\E", -1, 0, 22, 22, 1, "ANSI MH10.8.17-2017 Figure 4 Macro06 **NOT SAME** 253-state randomising of padding in figure seems incorrect", 0,
                    "1010101010101010101010"
                    "1101110000111001011011"
                    "1010111010001010001110"
                    "1100011100101001000111"
                    "1110011000100010001100"
                    "1111011100011001000101"
                    "1011101101000101111010"
                    "1100101100010101010111"
                    "1110101001001000001100"
                    "1000010001111000110101"
                    "1110111001110000001000"
                    "1010100011101000011011"
                    "1100010101011110111010"
                    "1011011000011100011111"
                    "1011001001000101100110"
                    "1000000111001100000001"
                    "1011110001100011000010"
                    "1100110100000110100111"
                    "1100011111110000001110"
                    "1100110010010010001101"
                    "1000001010010010110100"
                    "1111111111111111111111"
                },
        /* 64*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, 7, -1, "[)>\\R06\\G18VD89536\\G1P8902A\\GS3122A02965\\R\\E", -1, 0, 22, 22, 1, "ANSI MH10.8.17-2017 Figure 4 Macro06 **NOT SAME** 253-state randomising of padding in figure seems incorrect", 0,
                    "1010101010101010101010"
                    "1101110000111001011011"
                    "1010111010001010001110"
                    "1100011100101001000111"
                    "1110011000100010001100"
                    "1111011100011001000101"
                    "1011101101000101111010"
                    "1100101100010101010111"
                    "1110101001001000001100"
                    "1000010001111000110101"
                    "1110111001110000001000"
                    "1010100011101000011011"
                    "1100010101011110111010"
                    "1011011000011100011111"
                    "1011001001000101100110"
                    "1000000111001100000001"
                    "1011110001100011000010"
                    "1100110100000110100111"
                    "1100011111110000001110"
                    "1100110010010010001101"
                    "1000001010010010110100"
                    "1111111111111111111111"
                },
        /* 65*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE | FAST_MODE, -1, -1, -1, -1, "[)>\\R06\\G25S0614141MH80312\\R\\E", -1, 0, 16, 16, 1, "ANSI MH10.8.17-2017 Table B.1 B7", 0,
                    "1010101010101010"
                    "1101000010101111"
                    "1011100001011100"
                    "1011010001010101"
                    "1110000110111010"
                    "1101010011011111"
                    "1000010001111100"
                    "1101100111110101"
                    "1100101101001100"
                    "1010100000001111"
                    "1001100010010100"
                    "1001000000000101"
                    "1011110011000010"
                    "1110101111010101"
                    "1010101010001010"
                    "1111111111111111"
                },
        /* 66*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R06\\G25S0614141MH80312\\R\\E", -1, 0, 16, 16, 1, "ANSI MH10.8.17-2017 Table B.1 B7", 0,
                    "1010101010101010"
                    "1101000010101111"
                    "1011100001011100"
                    "1011010001010101"
                    "1110000110111010"
                    "1101010011011111"
                    "1000010001111100"
                    "1101100111110101"
                    "1100101101001100"
                    "1010100000001111"
                    "1001100010010100"
                    "1001000000000101"
                    "1011110011000010"
                    "1110101111010101"
                    "1010101010001010"
                    "1111111111111111"
                },
        /* 67*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE | FAST_MODE, -1, -1, -1, -1, "[)>\\R05\\G80040614141MH80312\\R\\E", -1, 0, 16, 16, 1, "ANSI MH10.8.17-2017 Table B.1 B8", 0,
                    "1010101010101010"
                    "1111100010001111"
                    "1010100001100100"
                    "1010010001011001"
                    "1110000110000110"
                    "1001010011101111"
                    "1000010010001100"
                    "1101100111110101"
                    "1100101000101100"
                    "1010100001010011"
                    "1001001100111100"
                    "1001110010011101"
                    "1001011000010100"
                    "1100110000010101"
                    "1010000010101010"
                    "1111111111111111"
                },
        /* 68*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R05\\G80040614141MH80312\\R\\E", -1, 0, 16, 16, 1, "ANSI MH10.8.17-2017 Table B.1 B8", 0,
                    "1010101010101010"
                    "1111100010001111"
                    "1010100001100100"
                    "1010010001011001"
                    "1110000110000110"
                    "1001010011101111"
                    "1000010010001100"
                    "1101100111110101"
                    "1100101000101100"
                    "1010100001010011"
                    "1001001100111100"
                    "1001110010011101"
                    "1001011000010100"
                    "1100110000010101"
                    "1010000010101010"
                    "1111111111111111"
                },
        /* 69*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 3, -1, -1, -1, "sn:7QPB4MN", -1, 0, 16, 16, 1, "AIM ITS/04-023:2022 ECI 3 Example 1", 0,
                    "1010101010101010"
                    "1001001011100001"
                    "1000111110110000"
                    "1100100110000001"
                    "1000110111101010"
                    "1100100110001111"
                    "1000010110000110"
                    "1000010000000001"
                    "1001101001111110"
                    "1001100011010111"
                    "1011100110011110"
                    "1000010111011101"
                    "1101000010001110"
                    "1110101001000101"
                    "1001011001111010"
                    "1111111111111111"
                },
        /* 70*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 3, -1, -1, -1, "price:£20.00", -1, 0, 12, 26, 1, "AIM ITS/04-023:2022 ECI 3 Example 2", 0,
                    "10101010101010101010101010"
                    "10000111010111000110110001"
                    "10000101101010010011011000"
                    "11001010011000000101110111"
                    "10010101000100111011110110"
                    "11100111000100111100000001"
                    "11100101111110001101011100"
                    "10000011101101110011110011"
                    "11011100101111100110010110"
                    "10101101100110000001001011"
                    "11100101010110100111011000"
                    "11111111111111111111111111"
                },
        /* 71*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 3, -1, -1, -1, "C:\\DOCS\\EXAMPLE.TXT", -1, 0, 18, 18, 1, "AIM ITS/04-023:2022 ECI 3 Example 3", 0,
                    "101010101010101010"
                    "100000001010011011"
                    "100010111000011100"
                    "110010001000010111"
                    "100011100110001110"
                    "110001010100010001"
                    "101000000010110100"
                    "111000000111100111"
                    "100010101110101110"
                    "111101011100000111"
                    "101101110100111010"
                    "110000101011100001"
                    "100110010100111110"
                    "100000000100010101"
                    "111011010100011000"
                    "101111000111101111"
                    "100110001010000100"
                    "111111111111111111"
                },
        /* 72*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 4, -1, -1, -1, "Študentska št. 2198390", -1, 0, 20, 20, 0, "AIM ITS/04-023:2022 ECI 4 Example 1; BWIPP same as FAST_MODE", 0,
                    "10101010101010101010"
                    "10001110100110101111"
                    "10001101000001010110"
                    "11011010011101011001"
                    "10111110101000101110"
                    "11010110111111100111"
                    "10011001111111100000"
                    "10011111111111010001"
                    "11000110110101100100"
                    "11101101010011000011"
                    "10111110101011001110"
                    "10100100100110000101"
                    "10111010000001010010"
                    "10011010111010000001"
                    "10100010001110111000"
                    "10001101001000100111"
                    "10110101100001100110"
                    "11111110001010111101"
                    "11010000001001011010"
                    "11111111111111111111"
                },
        /* 73*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 4, -1, 10, -1, "Szczegółowe dane kontaktowe:+48 22 694 60 00", -1, 0, 32, 32, 0, "AIM ITS/04-023:2022 ECI 4 Example 2 **NOT SAME** different encodation; BWIPP different encodation", 0,
                    "10101010101010101010101010101010"
                    "10010010111100111011110110001111"
                    "10001111110010101000001000100000"
                    "11011101001100111001111111000101"
                    "10011101100110001100001101011110"
                    "11101101011101011001100000100101"
                    "11000111000010101110001010011110"
                    "11010100010000011000001100101111"
                    "11100010001100101001011100110110"
                    "11101011010001011001110000000001"
                    "10000101111001001011000111011000"
                    "10000110001011111011010001001111"
                    "10010101010000101001101110111000"
                    "10011001000101011111000011101101"
                    "11011111100011101100010000000010"
                    "11111111111111111111111111111111"
                    "10101010101010101010101010101010"
                    "10011011100000011001000100011101"
                    "11011100111100001111011111100100"
                    "11011011110010011111110101100011"
                    "11101111000110101101001000001000"
                    "10110101101110011111100100010011"
                    "10011001101101101001001000100000"
                    "10101010111010111100000011000111"
                    "10011011001000101010001001000010"
                    "10011101101000111010100010100101"
                    "11100000000001001110111100101110"
                    "10011011100010111011101110011101"
                    "11101101111111101110111001010100"
                    "10001010101011111111110010011101"
                    "11001100101101101010010101100000"
                    "11111111111111111111111111111111"
                },
        /* 74*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 5, -1, -1, DM_SQUARE, "Liĥtenŝtejno", -1, 0, 18, 18, 0, "AIM ITS/04-023:2022 ECI 5 Example 1; BWIPP different encodation", 0,
                    "101010101010101010"
                    "100101101010111011"
                    "100011010100011110"
                    "111010101110110111"
                    "100001111010000110"
                    "111100110100011001"
                    "111111110000101000"
                    "110111110111011111"
                    "101101101001101110"
                    "101011010001100011"
                    "110100011000000110"
                    "111101010011111011"
                    "101010100110001100"
                    "100010001100010111"
                    "110101100101101010"
                    "100010011100100101"
                    "110001011101010110"
                    "111111111111111111"
                },
        /* 75*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 6, -1, -1, DM_SQUARE, "Lietuvą", -1, 0, 16, 16, 1, "AIM ITS/04-023:2022 ECI 6 Example 1", 0,
                    "1010101010101010"
                    "1001011011110111"
                    "1000010100001110"
                    "1111100101001101"
                    "1001110010001110"
                    "1111010011100111"
                    "1110100001010110"
                    "1100000101010001"
                    "1101101001001100"
                    "1110101111100011"
                    "1101111011100110"
                    "1011100001101101"
                    "1011000111011100"
                    "1100101001001101"
                    "1100100101110010"
                    "1111111111111111"
                },
        /* 76*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 7, -1, -1, DM_SQUARE, "Россия", -1, 0, 16, 16, 1, "AIM ITS/04-023:2022 ECI 7 Example 1", 0,
                    "1010101010101010"
                    "1001110110001111"
                    "1001101010110100"
                    "1000101100100101"
                    "1001100010101000"
                    "1110010011110111"
                    "1100000001110110"
                    "1111100110000001"
                    "1010001010001110"
                    "1001110101110011"
                    "1011111000110000"
                    "1011011010100111"
                    "1101110011100110"
                    "1000011010101101"
                    "1010110001000010"
                    "1111111111111111"
                },
        /* 77*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 7, -1, -1, DM_SQUARE, "Монголулс", -1, 0, 18, 18, 1, "AIM ITS/04-023:2022 ECI 7 Example 2", 0,
                    "101010101010101010"
                    "100111100010010101"
                    "100110000111111110"
                    "100010111010101111"
                    "100100001110101110"
                    "111000111100001001"
                    "110000100010111110"
                    "110100010111111111"
                    "101110001001110010"
                    "100011100101100111"
                    "110001001010011100"
                    "100101010001110101"
                    "100111110110111000"
                    "101001010101111001"
                    "110011000110000000"
                    "101111110101011001"
                    "110100011110000100"
                    "111111111111111111"
                },
        /* 78*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 8, -1, -1, DM_SQUARE, "جواز السفر", -1, 0, 18, 18, 1, "AIM ITS/04-023:2022 ECI 8 Example 1", 0,
                    "101010101010101010"
                    "100111100010000001"
                    "100110001111110010"
                    "100111111010101111"
                    "100100001010101110"
                    "111010111100010001"
                    "110100100010111010"
                    "111100100111011001"
                    "111011111001110010"
                    "101111001111111001"
                    "110010100101010100"
                    "101110100110100011"
                    "111100100010101110"
                    "110010000001110111"
                    "100000010011010100"
                    "101111111100011101"
                    "101100011100001010"
                    "111111111111111111"
                },
        /* 79*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 8, -1, -1, -1, "المنشأ: المملكة العربية السعودية", -1, 0, 24, 24, 1, "AIM ITS/04-023:2022 ECI 8 Example 2 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "101010101010101010101010"
                    "100111010010111011001011"
                    "100110111100000010010000"
                    "100111000000111111001101"
                    "100101110011111110100010"
                    "111001010001001001101111"
                    "111101010101110101101110"
                    "111100001011010001101001"
                    "101011110001111111000110"
                    "100011100010010100001011"
                    "101011100111000111010010"
                    "111101010101001011010111"
                    "100000001011111111110110"
                    "110010000011010101000101"
                    "101010110001010010010010"
                    "111011000011110111111011"
                    "111100001100000111000110"
                    "111100010101100000100011"
                    "101111110010011001101110"
                    "100001110110010001100101"
                    "101010101000001101111100"
                    "110101100100100111001101"
                    "100111000101101010110010"
                    "111111111111111111111111"
                },
        /* 80*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 9, -1, -1, DM_SQUARE, "Μέρος #. α123", -1, 0, 18, 18, 1, "AIM ITS/04-023:2022 ECI 9 Example 1", 0,
                    "101010101010101010"
                    "100111100100000011"
                    "100110001111001100"
                    "101011100001110111"
                    "100100011010100110"
                    "110111001000011001"
                    "101110100000101000"
                    "101110010110111101"
                    "101111101000101100"
                    "100000110010100111"
                    "111010011011010100"
                    "110101010110001101"
                    "111110011101000010"
                    "101100110111110101"
                    "101011110001111000"
                    "111110011110011011"
                    "111100011110101000"
                    "111111111111111111"
                },
        /* 81*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 10, -1, -1, -1, "דרכון", -1, 0, 8, 32, 1, "AIM ITS/04-023:2022 ECI 10 Example 1", 0,
                    "10101010101010101010101010101010"
                    "10011101110010011001011101110101"
                    "10011110001001001011101011110100"
                    "10110010001000011011100111101101"
                    "11011110000110101111101011010010"
                    "11111110111110011001000100101111"
                    "11010100010001101010000011101010"
                    "11111111111111111111111111111111"
                },
        /* 82*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 10, -1, -1, -1, "מספר חלק: A20200715001", -1, 0, 20, 20, 0, "AIM ITS/04-023:2022 ECI 10 Example 2 **NOT SAME** different encodation; BWIPP same as FAST_MODE", 0,
                    "10101010101010101010"
                    "10011110011111000111"
                    "10011101101010101110"
                    "10110111101100111001"
                    "10010101111010010100"
                    "11110010110001000001"
                    "10011011101000111110"
                    "10110010000111000011"
                    "10011111010101010000"
                    "10111100110011010101"
                    "11001000110110100100"
                    "10111100100000111101"
                    "10101001000011001110"
                    "11000110100101000001"
                    "10011110101101011000"
                    "10000110110111011111"
                    "10110001001101110100"
                    "11100100101110010101"
                    "11000111100100101010"
                    "11111111111111111111"
                },
        /* 83*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 10, -1, -1, -1, "מספר חלק: A20200715001", -1, 0, 20, 20, 1, "AIM ITS/04-023:2022 ECI 10 Example 2 **NOT SAME** different encodation", 0,
                    "10101010101010101010"
                    "10011101100001000111"
                    "10011111111110010110"
                    "10110111101101001001"
                    "10010101111011010100"
                    "11110010110100101101"
                    "10011011101101111010"
                    "10110010001101100011"
                    "10011110001010100010"
                    "10111101001110011101"
                    "11001000111010000100"
                    "10110001001011000101"
                    "10000101101011001010"
                    "11110110001101010101"
                    "10010111111001100100"
                    "10011101010100110111"
                    "10111011001110011010"
                    "11110101100100001101"
                    "11000111010001111010"
                    "11111111111111111111"
                },
        /* 84*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 11, -1, -1, -1, "Amerika Birleşik Devletleri", -1, 0, 22, 22, 0, "AIM ITS/04-023:2022 ECI 11 Example 1 **NOT SAME** different encodation; BWIPP same as FAST_MODE", 0,
                    "1010101010101010101010"
                    "1000101011001011000001"
                    "1001111101111001101100"
                    "1100101110111111111011"
                    "1001111000001110111000"
                    "1111011101001001000011"
                    "1110110000001101101010"
                    "1010011101010101100111"
                    "1110010001101000110110"
                    "1001000101111010101001"
                    "1110011011110111010000"
                    "1100110101100001100011"
                    "1100110000000000101010"
                    "1001011001011010011101"
                    "1101011110000110010010"
                    "1100111000110011110101"
                    "1011010110110111000010"
                    "1011111110101100100001"
                    "1110100000001110110100"
                    "1101100010010110011111"
                    "1110010000010000011110"
                    "1111111111111111111111"
                },
        /* 85*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 11, -1, -1, -1, "Amerika Birleşik Devletleri", -1, 0, 22, 22, 1, "AIM ITS/04-023:2022 ECI 11 Example 1 **NOT SAME** different encodation", 0,
                    "1010101010101010101010"
                    "1001110011001011001001"
                    "1001111001111001110100"
                    "1100000110111110100011"
                    "1001011000001110110000"
                    "1111011101001000110011"
                    "1110110000001101001000"
                    "1010011101010101110111"
                    "1110010001101011101110"
                    "1001000101111000001001"
                    "1110011011110011000100"
                    "1100110101100100101111"
                    "1100110000000101011010"
                    "1001011001100001100001"
                    "1101011110100001100100"
                    "1100111001000111011101"
                    "1011011100010000111010"
                    "1011110101000111110001"
                    "1110110111010111100000"
                    "1101110111000000010011"
                    "1110111101000010011110"
                    "1111111111111111111111"
                },
        /* 86*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 11, -1, -1, -1, "Biniş kartı #120921039", -1, 0, 20, 20, 0, "AIM ITS/04-023:2022 ECI 11 Example 2; BWIPP different encodation", 0,
                    "10101010101010101010"
                    "10001110101011111111"
                    "10010101011000110110"
                    "11001010110110010001"
                    "10011111100000111010"
                    "11101010111000111011"
                    "11010100101001000000"
                    "10101010001011000001"
                    "11110110110010001010"
                    "10010000100110111011"
                    "11001001111000001110"
                    "10010100000101101111"
                    "11101111000001110100"
                    "11000001110100111111"
                    "10101110011101100000"
                    "11111011111011101101"
                    "11010011001001110000"
                    "10101100011011001101"
                    "10000100110101011010"
                    "11111111111111111111"
                },
        /* 87*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 12, -1, -1, -1, "Kūrybiškumą", -1, 0, 12, 26, 0, "AIM ITS/04-023:2022 ECI 12 Example 1 **NOT SAME** different encodation; BWIPP same as FAST_MODE", 0,
                    "10101010101010101010101010"
                    "10010010000111001101100011"
                    "10011111110110110110111000"
                    "11011100100001101101111111"
                    "10011100000110010100001110"
                    "11000100011101011100010001"
                    "10000100101011100011011010"
                    "10100101000001111110101001"
                    "11010001001001001000011100"
                    "10111001001111010000000101"
                    "10011101110100000011000100"
                    "11111111111111111111111111"
                },
        /* 88*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 12, -1, -1, -1, "Kūrybiškumą", -1, 0, 12, 26, 1, "AIM ITS/04-023:2022 ECI 12 Example 1 **NOT SAME** different encodation", 0,
                    "10101010101010101010101010"
                    "10011110000111001010011111"
                    "10010001010110110011110000"
                    "11011100100001101000000111"
                    "10011110000110000000100110"
                    "11000100011100111011101001"
                    "10000100101011000010101010"
                    "10100101000100100110000001"
                    "11010010001010110101011000"
                    "11111001101111110000010001"
                    "11000101010100110000011010"
                    "11111111111111111111111111"
                },
        /* 89*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 13, -1, -1, -1, "บาร๋แค่ด", -1, 0, 16, 16, 1, "AIM ITS/04-023:2022 ECI 13 Example 1 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "1010101010101010"
                    "1001110100011001"
                    "1001101111101000"
                    "1110100100111111"
                    "1001110111011110"
                    "1101001000100111"
                    "1110001010011110"
                    "1110111010010001"
                    "1111111001100010"
                    "1001000000100101"
                    "1100101010000100"
                    "1100010011100011"
                    "1100000011101000"
                    "1000011111010101"
                    "1001010001001010"
                    "1111111111111111"
                },
        /* 90*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 15, -1, -1, -1, "uzņēmums", -1, 0, 16, 16, 0, "AIM ITS/04-023:2022 ECI 15 Example 1 **NOT SAME** different encodation; BWIPP different encodation", 0,
                    "1010101010101010"
                    "1001101111101001"
                    "1010110111010100"
                    "1000101010000001"
                    "1001011110010000"
                    "1111001100011111"
                    "1101110101111110"
                    "1010111001100001"
                    "1111100011000010"
                    "1011100000101111"
                    "1101000010001000"
                    "1110000111000111"
                    "1001010000011010"
                    "1101001001100101"
                    "1110011001011010"
                    "1111111111111111"
                },
        /* 91*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 16, -1, -1, -1, "ṁórṡáċ", -1, 0, 8, 32, 1, "AIM ITS/04-023:2022 ECI 16 Example 1 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "10101010101010101010101010101010"
                    "10011101001100111111001100100101"
                    "10101011110110101011000000111100"
                    "10011001010000111001101100000001"
                    "11010010010000001010011001001110"
                    "11110110101100011111111110011101"
                    "11101100010001001001011110100100"
                    "11111111111111111111111111111111"
                },
        /* 92*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 17, -1, -1, -1, "Price: €13.50", -1, 0, 12, 26, 1, "AIM ITS/04-023:2022 ECI 17 Example 1", 0,
                    "10101010101010101010101010"
                    "10000111000111101110001001"
                    "10100101110010111101000010"
                    "10101011110111010110100111"
                    "10010101011010010000100110"
                    "11100000110001000100010001"
                    "11100100110010110111101100"
                    "10000001101110111010011011"
                    "11011100101111110001001110"
                    "10101101110001111000111101"
                    "10100111000100011110100110"
                    "11111111111111111111111111"
                },
        /* 93*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 18, -1, -1, -1, "Te słowa są głębokie", -1, 0, 22, 22, 0, "AIM ITS/04-023:2022 ECI 18 Example 1 **NOT SAME** different encodation; BWIPP different encodation", 0,
                    "1010101010101010101010"
                    "1001011001001010011011"
                    "1010001101110111011000"
                    "1011100010011110110111"
                    "1001001100010100000100"
                    "1111011101010000101101"
                    "1110010000110101100100"
                    "1000110111110101011111"
                    "1110000000001000110110"
                    "1111110111111001001001"
                    "1110010001111100101010"
                    "1110100011100100111111"
                    "1101011101010010001010"
                    "1011011000011101110111"
                    "1100001001111010101110"
                    "1011000111000111100101"
                    "1000001001000010111110"
                    "1010010010111111100001"
                    "1100110001011001111000"
                    "1110101010111001011011"
                    "1011000010010101000110"
                    "1111111111111111111111"
                },
        /* 94*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 20, -1, -1, -1, "バーコード", -1, 0, 12, 26, 0, "AIM ITS/04-023:2022 ECI 20 Example 1 **NOT SAME** Zint switches to ASCII 1 char before end; BWIPP same as example", 0,
                    "10101010101010101010101010"
                    "10011110011111011010110101"
                    "10100100010101100010110110"
                    "11011010011110110101110111"
                    "10111111011000110101101110"
                    "11110110100001110010111001"
                    "10001000110010011001110110"
                    "11100110101110111000101101"
                    "10100100101011100010101100"
                    "11100010001011010011110111"
                    "11000110001100101101001010"
                    "11111111111111111111111111"
                },
        /* 95*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 20, -1, -1, DM_SQUARE, "東京都", -1, 0, 16, 16, 0, "AIM ITS/04-023:2022 ECI 20 Example 2 **NOT SAME** Zint switches to ASCII 1 char before end; BWIPP same as example", 0,
                    "1010101010101010"
                    "1001110111101011"
                    "1010011001101010"
                    "1101111100001101"
                    "1000111010111110"
                    "1100110011010111"
                    "1011000011111110"
                    "1000100111111001"
                    "1111100000001010"
                    "1111000101101011"
                    "1010110001100000"
                    "1110010000001001"
                    "1001011000001110"
                    "1110111011010101"
                    "1010010001000010"
                    "1111111111111111"
                },
        /* 96*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 21, -1, -1, -1, "Študentska št. 2198390", -1, 0, 20, 20, 0, "AIM ITS/04-023:2022 ECI 21 Example 1 **NOT SAME** different encodation; BWIPP different encodation", 0,
                    "10101010101010101010"
                    "10001100100001100111"
                    "10100101100110111110"
                    "11101100001010110001"
                    "10111011100101011110"
                    "11101101000100110001"
                    "11111011100011001110"
                    "11000000111110011001"
                    "11100111111110101000"
                    "11000111111000011001"
                    "11011011010111011110"
                    "11100101100111011011"
                    "11111011111100110010"
                    "11010100101010011001"
                    "11101100100010111110"
                    "11110000011100100001"
                    "10011011000111010100"
                    "11111100011000100101"
                    "11010010011010100010"
                    "11111111111111111111"
                },
        /* 97*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 22, -1, -1, DM_SQUARE, "Россия", -1, 0, 16, 16, 1, "AIM ITS/04-023:2022 ECI 22 Example 1", 0,
                    "1010101010101010"
                    "1001110111011111"
                    "1010101011100000"
                    "1111111101000011"
                    "1001100010101000"
                    "1111010011011111"
                    "1000000001100110"
                    "1011100100000001"
                    "1010101000111000"
                    "1001111010011001"
                    "1101100010010000"
                    "1011011010001001"
                    "1110100000111100"
                    "1001011100110101"
                    "1011010001101010"
                    "1111111111111111"
                },
        /* 98*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 22, -1, -1, DM_SQUARE, "Монголулс", -1, 0, 18, 18, 1, "AIM ITS/04-023:2022 ECI 22 Example 2", 0,
                    "101010101010101010"
                    "100111100100010101"
                    "101010000001111110"
                    "111111111010101111"
                    "100100010110101110"
                    "111100111100011001"
                    "100001000010111100"
                    "100110010111100101"
                    "101000111000100010"
                    "100011110100101111"
                    "111011111000101000"
                    "100111100001100101"
                    "100011110010101010"
                    "100000110010101001"
                    "110111010111100000"
                    "100110011101011001"
                    "101100011000000110"
                    "111111111111111111"
                },
        /* 99*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 23, -1, -1, -1, "bœuf", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 23 Example 1", 0,
                    "10101010101010"
                    "10001110110001"
                    "10110001110100"
                    "10000110111101"
                    "10011010110010"
                    "11110101110101"
                    "11110010010000"
                    "10001111000111"
                    "11110111110110"
                    "10000001100001"
                    "11010110100000"
                    "10010000100111"
                    "11000001001110"
                    "11111111111111"
                },
        /*100*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 24, -1, -1, DM_SQUARE, "جواز السفر", -1, 0, 18, 18, 1, "AIM ITS/04-023:2022 ECI 24 Example 1", 0,
                    "101010101010101010"
                    "100111100010000001"
                    "101110001110110010"
                    "100111111110101111"
                    "100100001010101110"
                    "111010111100011001"
                    "110000010010100010"
                    "111101110110110001"
                    "111011011001101010"
                    "101111100101000111"
                    "110000010001000010"
                    "101111110011000011"
                    "110001010110100110"
                    "110010011100010111"
                    "111110101000110000"
                    "101111100010001011"
                    "110100011110001010"
                    "111111111111111111"
                },
        /*101*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 24, -1, -1, -1, "المنشأ: المملكة العربية السعودية", -1, 0, 24, 24, 1, "AIM ITS/04-023:2022 ECI 24 Example 2", 0,
                    "101010101010101010101010"
                    "100111010010001001001011"
                    "101110111100001110010010"
                    "100111000000111111011011"
                    "100101110011101110101110"
                    "111001010001001001101111"
                    "110111010100010101101110"
                    "111100001001000110001001"
                    "111011111001111100101010"
                    "100011100010010101011111"
                    "101011100111100110010100"
                    "111101010101101010010011"
                    "100000001011110011010100"
                    "110010000001000011100101"
                    "101010110101010110001010"
                    "111011110111011101000001"
                    "100100011011100100110010"
                    "111100001100111100001111"
                    "101110101011010110101110"
                    "100001100100001100100001"
                    "101110110011010001010110"
                    "110001111011100111011101"
                    "101111000110001010111010"
                    "111111111111111111111111"
                },
        /*102*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 25, -1, -1, -1, "条码", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 25 Example 1", 0,
                    "10101010101010"
                    "10000010011111"
                    "10110101011100"
                    "10101111001101"
                    "10000011101100"
                    "11000000010111"
                    "10010000010100"
                    "10000100010111"
                    "10101101011110"
                    "11001100011001"
                    "11111010000010"
                    "10011010101101"
                    "10011101010100"
                    "11111111111111"
                },
        /*103*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 25, -1, -1, -1, "バーコード", -1, 0, 12, 26, 1, "AIM ITS/04-023:2022 ECI 25 Example 2", 0,
                    "10101010101010101010101010"
                    "10011110010000001001101001"
                    "10110000101111001011001010"
                    "10100111011011101110100111"
                    "10011000011000010000110110"
                    "11010010110001100111100001"
                    "11010110110011000110001100"
                    "10110011101011011111000001"
                    "10010111101001011101001100"
                    "11100100000101010101100011"
                    "11000100100011011100111100"
                    "11111111111111111111111111"
                },
        /*104*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 25, -1, -1, -1, "바코드", -1, 0, 8, 32, 1, "AIM ITS/04-023:2022 ECI 25 Example 3 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "10101010101010101010101010101010"
                    "10011101000111111101100100000101"
                    "10111011100010101000011111001100"
                    "10101011100111111110100111110101"
                    "11100001000110001100000000111010"
                    "11010111110011111111000101100111"
                    "11110100011001101010111111010110"
                    "11111111111111111111111111111111"
                },
        /*105*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 26, -1, -1, DM_SQUARE, "条码", -1, 0, 16, 16, 1, "AIM ITS/04-023:2022 ECI 26 Example 1", 0,
                    "1010101010101010"
                    "1001110111110101"
                    "1011111011000100"
                    "1011010100111001"
                    "1000010010010100"
                    "1101110011001111"
                    "1011100000010110"
                    "1110100111001001"
                    "1010111100111110"
                    "1110010111011111"
                    "1100111010000100"
                    "1011111101000011"
                    "1111111101100100"
                    "1000011100010101"
                    "1001110001111010"
                    "1111111111111111"
                },
        /*106*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 26, -1, -1, -1, "バーコード", -1, 0, 20, 20, 1, "AIM ITS/04-023:2022 ECI 26 Example 2", 0,
                    "10101010101010101010"
                    "10011110001010000111"
                    "10111111000100111110"
                    "10110011010001010001"
                    "10001110001111011010"
                    "11000011100100001011"
                    "10101010000101000000"
                    "10110001001001100101"
                    "10001010000000001000"
                    "11110010000011010101"
                    "10001010010001011010"
                    "11100001110010000001"
                    "10110010011000110000"
                    "10011010010111000111"
                    "11111111110100000110"
                    "11010100111101001001"
                    "11001010010001101000"
                    "11101000000101101101"
                    "11000101001001010010"
                    "11111111111111111111"
                },
        /*107*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 26, -1, -1, DM_SQUARE, "바코드", -1, 0, 18, 18, 1, "AIM ITS/04-023:2022 ECI 26 Example 3", 0,
                    "101010101010101010"
                    "100111100000010101"
                    "101111000110111110"
                    "101101010101101111"
                    "100011101010101110"
                    "111100111100001001"
                    "100100110010110010"
                    "101010010110000111"
                    "100101011000001010"
                    "100100001110010001"
                    "100011010111100000"
                    "100011100010101101"
                    "111111010100001010"
                    "110010001001001001"
                    "100111011011000110"
                    "100110110111011111"
                    "101100010110000100"
                    "111111111111111111"
                },
        /*108*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 27, -1, -1, -1, "sn:7QPB4MN", -1, 0, 16, 16, 1, "AIM ITS/04-023:2022 ECI 27 Example 1", 0,
                    "1010101010101010"
                    "1001001011000001"
                    "1011111111011000"
                    "1100100111101001"
                    "1000110111011100"
                    "1100100110000111"
                    "1000010110101110"
                    "1000010011010001"
                    "1001101001110110"
                    "1001100011110011"
                    "1011000000011110"
                    "1000110001100001"
                    "1110101111011010"
                    "1110001011011101"
                    "1001011001110010"
                    "1111111111111111"
                },
        /*109*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 28, -1, -1, -1, "條碼", -1, 0, 14, 14, 0, "AIM ITS/04-023:2022 ECI 28 Example 1 **NOT SAME** Zint switches to ASCII 1 char before end; BWIPP same as example except does not set explicit BASE256 byte count", 0,
                    "10101010101010"
                    "10011101101111"
                    "10111001001100"
                    "11010111110001"
                    "10011010100010"
                    "11111111100101"
                    "11010001010100"
                    "11000100010111"
                    "10110010111110"
                    "10011001011001"
                    "10000111110010"
                    "11110001100001"
                    "10101101000110"
                    "11111111111111"
                },
        /*110*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 29, -1, -1, -1, "条码", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 29 Example 1 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "10101010101010"
                    "10011101000111"
                    "10111011101100"
                    "11101111110101"
                    "10010001000000"
                    "11110000101001"
                    "11111100101110"
                    "11100000001111"
                    "11011110100110"
                    "10000000101001"
                    "10111010101110"
                    "10100011110101"
                    "11001101000100"
                    "11111111111111"
                },
        /*111*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 29, -1, -1, -1, "北京", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 29 Example 2 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "10101010101010"
                    "10011101000101"
                    "10111011100110"
                    "11100110001111"
                    "10001011110100"
                    "11110000010111"
                    "11011111000000"
                    "11000100101111"
                    "11001101001110"
                    "10100100101001"
                    "10000101101110"
                    "10011001110001"
                    "10101101000110"
                    "11111111111111"
                },
        /*112*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 30, -1, -1, -1, "바코드", -1, 0, 8, 32, 1, "AIM ITS/04-023:2022 ECI 30 Example 1 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "10101010101010101010101010101010"
                    "10011101010000111110011110001101"
                    "10111011100001101011001100110000"
                    "11111001111110111010010000010001"
                    "11011010110111101010001000101010"
                    "11011111001011111001111111100111"
                    "11011100010101101110100010101010"
                    "11111111111111111111111111111111"
                },
        /*113*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 30, -1, -1, -1, "서울", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 30 Example 2 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "10101010101010"
                    "10011101010111"
                    "10111011100110"
                    "11111011101101"
                    "10000000001100"
                    "11101010010111"
                    "11111000111000"
                    "11000111001111"
                    "11101101011110"
                    "11110110000001"
                    "10110010011110"
                    "11011101100001"
                    "11001101000110"
                    "11111111111111"
                },
        /*114*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 31, -1, -1, -1, "条码", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 31 Example 1 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "10101010101010"
                    "10011101010111"
                    "11001011110100"
                    "10001111000101"
                    "10010001011100"
                    "11110011111001"
                    "11111010101000"
                    "11110100011111"
                    "11001001011110"
                    "10011000100001"
                    "11100001000110"
                    "11000111110101"
                    "11000111000100"
                    "11111111111111"
                },
        /*115*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 31, -1, -1, -1, "北京", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 31 Example 2 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "10101010101010"
                    "10011101010101"
                    "11001011111110"
                    "10000110111111"
                    "10001011101000"
                    "11110011000111"
                    "11011001000110"
                    "11010000111111"
                    "11011010110110"
                    "10111100100001"
                    "11011110000110"
                    "11111101110001"
                    "10100111000110"
                    "11111111111111"
                },
        /*116*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 31, -1, -1, -1, "條碼", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 31 Example 3 **NOT SAME** different encodation (example uses binary)", 0,
                    "10101010101010"
                    "10001101110001"
                    "11000100010000"
                    "10001011110011"
                    "10111011000000"
                    "11101111000101"
                    "10011100000110"
                    "11010101100111"
                    "10101011010110"
                    "10000111111001"
                    "11000110110000"
                    "11001111101011"
                    "10110111010010"
                    "11111111111111"
                },
        /*117*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 32, -1, -1, -1, "条码", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 32 Example 1 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "10101010101010"
                    "10011101011111"
                    "11001011111000"
                    "10011110010101"
                    "10010000011000"
                    "11110011000001"
                    "11111001000010"
                    "11100101010111"
                    "11011110110110"
                    "10010011011001"
                    "10001110010010"
                    "10110101101101"
                    "11000001000100"
                    "11111111111111"
                },
        /*118*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 32, -1, -1, -1, "北京", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 32 Example 2 **NOT SAME** example sets explicit BASE256 byte count", 0,
                    "10101010101010"
                    "10011101011101"
                    "11001011110010"
                    "10010111101111"
                    "10001010101100"
                    "11110011111111"
                    "11011010101100"
                    "11000001110111"
                    "11001101011110"
                    "10110111011001"
                    "10110001010010"
                    "10001111101001"
                    "10100001000110"
                    "11111111111111"
                },
        /*119*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 32, -1, -1, -1, "條碼", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 32 Example 3 **NOT SAME** different encodation (example uses binary)", 0,
                    "10101010101010"
                    "10001101111001"
                    "11000100011100"
                    "10011010100011"
                    "10111010000100"
                    "11101111111101"
                    "10011111101100"
                    "11000100101111"
                    "10111100111110"
                    "10001100000001"
                    "10101001100100"
                    "10111101110011"
                    "10110001010010"
                    "11111111111111"
                },
        /*120*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 32, -1, -1, DM_SQUARE, "པེ་ཅིང།", -1, 0, 24, 24, 0, "AIM ITS/04-023:2022 ECI 32 Example 4 **NOT SAME** Zint switches to ASCII 1 char before end; BWIPP same as example", 0,
                    "101010101010101010101010"
                    "100111110000011010000101"
                    "110001010011101011111000"
                    "100110100010001000000001"
                    "101010111101111000100010"
                    "111100101011010111001111"
                    "101001001000011100000110"
                    "101000101100101001110001"
                    "100001001111101011100100"
                    "111010001100111101101101"
                    "100011010001011011110000"
                    "110010010111110001101011"
                    "101101111000101100010010"
                    "110000111010011001110111"
                    "101111000010001111001100"
                    "101001000110000010101101"
                    "101011101110010001100100"
                    "100101000100111011001101"
                    "111101101010011000011010"
                    "111110011100101001001111"
                    "100010111101101000111100"
                    "111001111000001100100101"
                    "110001000101110010010010"
                    "111111111111111111111111"
                },
        /*121*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 32, -1, -1, DM_SQUARE, "バーコード", -1, 0, 18, 18, 1, "AIM ITS/04-023:2022 ECI 32 Example 5", 0,
                    "101010101010101010"
                    "100111100000000001"
                    "110010001111110010"
                    "100101001110101111"
                    "100100110010101110"
                    "110100010000010001"
                    "110100100010100110"
                    "100101110111001001"
                    "101001001001110100"
                    "111011101000110011"
                    "110101001000010110"
                    "100010101000110001"
                    "110111110111101000"
                    "111011011001100101"
                    "101001011011110000"
                    "101111010001101001"
                    "101100011101111000"
                    "111111111111111111"
                },
        /*122*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 32, -1, -1, DM_SQUARE, "바코드", -1, 0, 18, 18, 0, "AIM ITS/04-023:2022 ECI 32 Example 6 **NOT SAME** Zint switches to ASCII 1 char before end; BWIPP same as example", 0,
                    "101010101010101010"
                    "100111100000001111"
                    "110001010011000100"
                    "100110100011110111"
                    "101011111110100110"
                    "111100110100011001"
                    "100100100000100000"
                    "101000110111110001"
                    "101001001000001000"
                    "111100011010000111"
                    "100001001010100000"
                    "111011011010000111"
                    "111010101010001010"
                    "101001100101100111"
                    "101101011010111010"
                    "110111110000100111"
                    "101100010101010110"
                    "111111111111111111"
                },
        /*123*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 33, -1, -1, -1, "条码", -1, 0, 14, 14, 1, "AIM ITS/04-023:2022 ECI 33 Example 1", 0,
                    "10101010101010"
                    "10001010110111"
                    "11000000010100"
                    "10100000001001"
                    "10010101100000"
                    "11111000100001"
                    "10001100000010"
                    "10011110011111"
                    "10101010010110"
                    "10101011001001"
                    "10110010011110"
                    "11110100111101"
                    "10010001000100"
                    "11111111111111"
                },
        /*124*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 33, -1, -1, DM_SQUARE, "バーコード", -1, 0, 18, 18, 0, "AIM ITS/04-023:2022 ECI 33 Example 2 **NOT SAME** Zint switches to ASCII 1 char before end; BWIPP same as example", 0,
                    "101010101010101010"
                    "100111100010010001"
                    "110010000100110010"
                    "101011100010101111"
                    "101010011010101110"
                    "111100110000011001"
                    "100101010010100100"
                    "110000100110100001"
                    "100001011001010010"
                    "111000011111011101"
                    "111001011000110110"
                    "111001011110000111"
                    "111110001110011010"
                    "111101110110011101"
                    "110111001100001000"
                    "110111010110011001"
                    "111100010111101010"
                    "111111111111111111"
                },
        /*125*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 33, -1, -1, -1, "바코드", -1, 0, 8, 32, 0, "AIM ITS/04-023:2022 ECI 33 Example 3 **NOT SAME** different encodation; BWIPP same as FAST_MODE", 0,
                    "10101010101010101010101010101010"
                    "10010110001111011011011000110101"
                    "11001111111100101000100000101000"
                    "10101010110011011010101011101001"
                    "11001001010111101111001001100010"
                    "11111001001000011000100000000011"
                    "11110010110110101110001111110110"
                    "11111111111111111111111111111111"
                },
        /*126*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 33, -1, -1, -1, "바코드", -1, 0, 8, 32, 1, "AIM ITS/04-023:2022 ECI 33 Example 3 **NOT SAME** different encodation", 0,
                    "10101010101010101010101010101010"
                    "10011101001111011010111110010101"
                    "11000011111111101000110111000100"
                    "10100000110010111111011010110001"
                    "11000001010111001000101111100010"
                    "11111111000000011001110101101111"
                    "11110100011100101110011010101000"
                    "11111111111111111111111111111111"
                },
        /*127*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 34, -1, -1, -1, "条码", -1, 0, 8, 32, 1, "AIM ITS/04-023:2022 ECI 34 Example 1", 0,
                    "10101010101010101010101010101010"
                    "10000100000001011001100100101101"
                    "11000100100001001101100101100100"
                    "10111010000110111100110000101101"
                    "11010000001011001000011010110010"
                    "11100000010100111100110101101011"
                    "11010000001111101100000001100110"
                    "11111111111111111111111111111111"
                },
        /*128*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 34, -1, -1, -1, "バーコード", -1, 0, 22, 22, 0, "AIM ITS/04-023:2022 ECI 34 Example 2 **NOT SAME** different encodation; BWIPP different encodation", 0,
                    "1010101010101010101010"
                    "1000010001011111000001"
                    "1100000010110010010000"
                    "1011110000010011001001"
                    "1011001000111010010010"
                    "1110010000101001001011"
                    "1011101110111101010000"
                    "1011111000010101100111"
                    "1010110110101000110110"
                    "1010000011111001101001"
                    "1000100111111111110100"
                    "1011101011101111100011"
                    "1101100101100000011000"
                    "1100011000010100101011"
                    "1111001010000011111100"
                    "1111000101010110000011"
                    "1110001011000011100110"
                    "1110010101000001110101"
                    "1010101000111011110110"
                    "1000001001000110011101"
                    "1101110000000101011100"
                    "1111111111111111111111"
                },
        /*129*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 34, -1, -1, DM_SQUARE, "바코드", -1, 0, 18, 18, 0, "AIM ITS/04-023:2022 ECI 34 Example 3 **NOT SAME** different encodation; BWIPP same as FAST_MODE", 0,
                    "101010101010101010"
                    "100001000101111111"
                    "110011001011000000"
                    "101110001001110111"
                    "101011101110110110"
                    "110010000000010001"
                    "111001110000111010"
                    "110111010111011001"
                    "100011011001110010"
                    "101000000111010001"
                    "100000101011011110"
                    "101100011100110111"
                    "101100010100110000"
                    "111111100111000001"
                    "100001000011011000"
                    "100000101110010011"
                    "101000000011010110"
                    "111111111111111111"
                },
        /*130*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 34, -1, -1, DM_SQUARE, "바코드", -1, 0, 18, 18, 0, "AIM ITS/04-023:2022 ECI 34 Example 3 **NOT SAME** different encodation", 0,
                    "101010101010101010"
                    "100111100101111111"
                    "110011011011000000"
                    "101110101001110111"
                    "101010001110110110"
                    "110000000000011001"
                    "110101110000111010"
                    "110111010110100001"
                    "100011011000011110"
                    "101000011000101111"
                    "100011011111111000"
                    "101110111100011111"
                    "101101011010001000"
                    "110111011110011001"
                    "111000001110001000"
                    "100111100000010111"
                    "101100011111010100"
                    "111111111111111111"
                },
        /*131*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 35, -1, -1, -1, "条码", -1, 0, 8, 32, 1, "AIM ITS/04-023:2022 ECI 35 Example 1", 0,
                    "10101010101010101010101010101010"
                    "10001010101001011001011111111101"
                    "11000000011111001011101110100000"
                    "11000000000110111000110111011101"
                    "11000010001110001111001100111110"
                    "11000010101100011100010000001011"
                    "11001100011110001000101111100010"
                    "11111111111111111111111111111111"
                },
        /*132*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 35, -1, -1, -1, "バーコード", -1, 0, 22, 22, 0, "AIM ITS/04-023:2022 ECI 35 Example 2 **NOT SAME** different encodation; BWIPP different encodation", 0,
                    "1010101010101010101010"
                    "1001111010111100010001"
                    "1100100000010110000000"
                    "1100111001000110010101"
                    "1010100000110100011010"
                    "1111001100101000111011"
                    "1001010110011101101100"
                    "1110011011010101011111"
                    "1000010010101011011110"
                    "1000000111111000011001"
                    "1111100111110011001000"
                    "1111110011101010000001"
                    "1100000101000111111000"
                    "1100111000001001100101"
                    "1011101010011011100000"
                    "1101000100111000010101"
                    "1010000001011110010100"
                    "1000010110001001000101"
                    "1000011101101100111000"
                    "1001011111010100100111"
                    "1000011100010000111100"
                    "1111111111111111111111"
                },
        /*133*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 35, -1, -1, DM_SQUARE, "바코드", -1, 0, 18, 18, 0, "AIM ITS/04-023:2022 ECI 35 Example 3 **NOT SAME** different encodation; BWIPP different encodation", 0,
                    "101010101010101010"
                    "100101100010110001"
                    "110011111110000010"
                    "110011000101000111"
                    "100010100010001110"
                    "111110110000011001"
                    "101101010000110000"
                    "111101100111001101"
                    "100100011001111100"
                    "110111100000101011"
                    "101001011010101000"
                    "111111001010001101"
                    "110001011011010100"
                    "111100111001011101"
                    "111011010101100100"
                    "110000000111111111"
                    "110010110111000000"
                    "111111111111111111"
                },
        /*134*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 170, -1, -1, -1, "sn:7QPB4MN", -1, 0, 16, 16, 1, "AIM ITS/04-023:2022 ECI 170 Example 1", 0,
                    "1010101010101010"
                    "1101001101000101"
                    "1000011000100000"
                    "1000101000101001"
                    "1011111101000010"
                    "1110000100111111"
                    "1011011000011110"
                    "1010011011010001"
                    "1000111011000100"
                    "1110001110110001"
                    "1000111001111000"
                    "1000100110001001"
                    "1101001101010000"
                    "1010000010100101"
                    "1101110101101010"
                    "1111111111111111"
                },
        /*135*/ { BARCODE_DATAMATRIX, DATA_MODE, 899, -1, -1, -1, "\000\001\002\133\134\135\375\376\377", 9, 0, 12, 26, 0, "AIM ITS/04-023:2022 ECI 899 Example 1 **NOT SAME** different encodation; BWIPP different encodation", 0,
                    "10101010101010101010101010"
                    "11001100001001010101010111"
                    "10000000111000111010100110"
                    "10110001100110100000111111"
                    "10000101001011001101100110"
                    "11000011110001000000110001"
                    "11011011110011111011110000"
                    "11101110101110100010110101"
                    "10001101101001001110101110"
                    "10010110100111110011110111"
                    "10010010000010011010010100"
                    "11111111111111111111111111"
                },
        /*136*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, 3, -1, -1, -1, "\101\300", -1, 0, 12, 12, 1, "AÀ", 0,
                    "101010101010"
                    "100010101111"
                    "100001011110"
                    "110000010001"
                    "101100110000"
                    "110010100111"
                    "101011011100"
                    "110100111101"
                    "101100110100"
                    "101011100101"
                    "100011011010"
                    "111111111111"
                },
        /*137*/ { BARCODE_DATAMATRIX, DATA_MODE, 3, -1, -1, -1, "\101\300", -1, 0, 12, 12, 1, "AÀ", 0,
                    "101010101010"
                    "100010101111"
                    "100001011110"
                    "110000010001"
                    "101100110000"
                    "110010100111"
                    "101011011100"
                    "110100111101"
                    "101100110100"
                    "101011100101"
                    "100011011010"
                    "111111111111"
                },
        /*138*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, 26, -1, -1, -1, "AÀ", -1, 0, 14, 14, 1, "AÀ", 0,
                    "10101010101010"
                    "10001010100001"
                    "10110101100100"
                    "10110001000101"
                    "10111000100010"
                    "11101011110011"
                    "10011100001100"
                    "10001100101111"
                    "10110110111110"
                    "10000111010001"
                    "10000001111000"
                    "11110100110001"
                    "11000110001100"
                    "11111111111111"
                },
        /*139*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 26, -1, -1, -1, "AÀ", -1, 0, 14, 14, 1, "AÀ", 0,
                    "10101010101010"
                    "10001010100001"
                    "10110101100100"
                    "10110001000101"
                    "10111000100010"
                    "11101011110011"
                    "10011100001100"
                    "10001100101111"
                    "10110110111110"
                    "10000111010001"
                    "10000001111000"
                    "11110100110001"
                    "11000110001100"
                    "11111111111111"
                },
        /*140*/ { BARCODE_DATAMATRIX, UNICODE_MODE | FAST_MODE, -1, -1, -1, DM_SQUARE, "abcdefgh+", -1, 0, 16, 16, 1, "TEX last_shift 2, symbols_left 1, process_p 1", 0,
                    "1010101010101010"
                    "1010011011101001"
                    "1011001010010010"
                    "1001100100110011"
                    "1111100010101100"
                    "1111111011110111"
                    "1111011111111100"
                    "1011001100001111"
                    "1000000101011000"
                    "1000011011000111"
                    "1101011100110100"
                    "1100100100110101"
                    "1000000111001000"
                    "1111111010001101"
                    "1101110101001010"
                    "1111111111111111"
                },
        /*141*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, -1, -1, DM_SQUARE, "abcdefgh+", -1, 0, 14, 14, 0, "ATTTTTTTT; BWIPP same as FAST_MODE", 1,
                    "10101010101010"
                    "11100001010101"
                    "11010101001000"
                    "11111100011011"
                    "10100110000010"
                    "10001000100111"
                    "10101100101000"
                    "10001000110011"
                    "11101101100100"
                    "10101101000011"
                    "10110100100010"
                    "11101010101001"
                    "10111011000100"
                    "11111111111111"
                },
        /*142*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200", -1, 0, 8, 32, 1, "7 BASE256s, 1 pad", 0,
                    "10101010101010101010101010101010"
                    "10000101000011011000110100100001"
                    "11100111110101001011101110100010"
                    "10111011010100111110010110001011"
                    "11001000110001101000001111000010"
                    "11000010000001111000100101001011"
                    "11010000111100001010011101100100"
                    "11111111111111111111111111111111"
                },
        /*143*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200", -1, 0, 8, 32, 1, "7 BASE256s, 1 pad", 0,
                    "10101010101010101010101010101010"
                    "10000101000011011000110100100001"
                    "11100111110101001011101110100010"
                    "10111011010100111110010110001011"
                    "11001000110001101000001111000010"
                    "11000010000001111000100101001011"
                    "11010000111100001010011101100100"
                    "11111111111111111111111111111111"
                },
        /*144*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200", -1, 0, 8, 32, 1, "8 BASE256s, no padding", 0,
                    "10101010101010101010101010101010"
                    "10000101000011011111001101000001"
                    "11010111110101001001011001100010"
                    "11001011010111111010001100100011"
                    "11001000110000101011101100011010"
                    "11000010000001111000010010110011"
                    "11010000110010001001010001111000"
                    "11111111111111111111111111111111"
                },
        /*145*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200", -1, 0, 8, 32, 1, "8 BASE256s, no padding", 0,
                    "10101010101010101010101010101010"
                    "10000101000011011111001101000001"
                    "11010111110101001001011001100010"
                    "11001011010111111010001100100011"
                    "11001000110000101011101100011010"
                    "11000010000001111000010010110011"
                    "11010000110010001001010001111000"
                    "11111111111111111111111111111111"
                },
        /*146*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, DM_SQUARE, "\200\200\200\200\200\200\200\200\200\200", -1, 0, 16, 16, 1, "8 BASE256s, square, no padding", 0,
                    "1010101010101010"
                    "1000010100001101"
                    "1101011111101110"
                    "1100101101000101"
                    "1000100000110000"
                    "1100011100010111"
                    "1001010100101100"
                    "1110111010010111"
                    "1000111000010110"
                    "1110001101001001"
                    "1000110011010000"
                    "1011110101001101"
                    "1000000010101100"
                    "1001001000100101"
                    "1111000011111010"
                    "1111111111111111"
                },
        /*147*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, DM_SQUARE, "\200\200\200\200\200\200\200\200\200\200", -1, 0, 16, 16, 1, "8 BASE256s, square, no padding", 0,
                    "1010101010101010"
                    "1000010100001101"
                    "1101011111101110"
                    "1100101101000101"
                    "1000100000110000"
                    "1100011100010111"
                    "1001010100101100"
                    "1110111010010111"
                    "1000111000010110"
                    "1110001101001001"
                    "1000110011010000"
                    "1011110101001101"
                    "1000000010101100"
                    "1001001000100101"
                    "1111000011111010"
                    "1111111111111111"
                },
        /*148*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200", -1, 0, 16, 16, 1, "9 BASE256s, 1 pad", 0,
                    "1010101010101010"
                    "1000010101001101"
                    "1110011111000010"
                    "1101101101010101"
                    "1000100000001010"
                    "1100011001101111"
                    "1001010111100100"
                    "1110111001100111"
                    "1000111101000010"
                    "1110001101001101"
                    "1000011111001000"
                    "1011001101010111"
                    "1010101000000000"
                    "1011001001011101"
                    "1100000011011010"
                    "1111111111111111"
                },
        /*149*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200", -1, 0, 16, 16, 1, "9 BASE256s, 1 pad", 0,
                    "1010101010101010"
                    "1000010101001101"
                    "1110011111000010"
                    "1101101101010101"
                    "1000100000001010"
                    "1100011001101111"
                    "1001010111100100"
                    "1110111001100111"
                    "1000111101000010"
                    "1110001101001101"
                    "1000011111001000"
                    "1011001101010111"
                    "1010101000000000"
                    "1011001001011101"
                    "1100000011011010"
                    "1111111111111111"
                },
        /*150*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200", -1, 0, 22, 22, 1, "22 BASE256s, 6 pads", 0,
                    "1010101010101010101010"
                    "1010010100011100010101"
                    "1000011110111110001100"
                    "1010101100010111000101"
                    "1000100000010100110110"
                    "1100011100101000100101"
                    "1001010100111101110100"
                    "1110111000010101110111"
                    "1000111010101000101100"
                    "1110000111111001100111"
                    "1000000111110100001010"
                    "1011110011101111101001"
                    "1111000101101110010000"
                    "1100011000111101111001"
                    "1111001010100110101110"
                    "1111000100111010000001"
                    "1110000100011110101100"
                    "1100010001111011110101"
                    "1000101001101111011100"
                    "1111110010000111001001"
                    "1111101000110111010100"
                    "1111111111111111111111"
                },
        /*151*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200", -1, 0, 22, 22, 1, "22 BASE256s, 6 pads", 0,
                    "1010101010101010101010"
                    "1010010100011100010101"
                    "1000011110111110001100"
                    "1010101100010111000101"
                    "1000100000010100110110"
                    "1100011100101000100101"
                    "1001010100111101110100"
                    "1110111000010101110111"
                    "1000111010101000101100"
                    "1110000111111001100111"
                    "1000000111110100001010"
                    "1011110011101111101001"
                    "1111000101101110010000"
                    "1100011000111101111001"
                    "1111001010100110101110"
                    "1111000100111010000001"
                    "1110000100011110101100"
                    "1100010001111011110101"
                    "1000101001101111011100"
                    "1111110010000111001001"
                    "1111101000110111010100"
                    "1111111111111111111111"
                },
        /*152*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, DM_DMRE, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200", -1, 0, 8, 64, 1, "22 BASE256s, no padding", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1000010101100011101010101011101111110100100110011100010011010111"
                    "1101011110001010110000001110001010001011010111001010101101100000"
                    "1100101000110001110100000001100110010100111101111110000010011111"
                    "1000100101001000110110101110011011110110111010101110010111001100"
                    "1100001101011011111101111000110110110101110110111111011010011111"
                    "1101000011001010111101101101110010111100111101001010010011001000"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /*153*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, DM_DMRE, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200", -1, 0, 8, 64, 1, "22 BASE256s, no padding", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1000010101100011101010101011101111110100100110011100010011010111"
                    "1101011110001010110000001110001010001011010111001010101101100000"
                    "1100101000110001110100000001100110010100111101111110000010011111"
                    "1000100101001000110110101110011011110110111010101110010111001100"
                    "1100001101011011111101111000110110110101110110111111011010011111"
                    "1101000011001010111101101101110010111100111101001010010011001000"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /*154*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066", -1, 0, 64, 64, 1, "249 BASE256s + 6 ASCII (3 double-digits)", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1000010100011101100000010111100110100010110111011011111010000001"
                    "1100011110111110100110111101111010101101000010001101001011001100"
                    "1101101100010111110011110010101111111101101111111000111010011111"
                    "1000100000010100101110100100010011101111110111101111000010011100"
                    "1100011100101001111000111100010110100000001000111000010111011111"
                    "1001010100111000110001000111000010000110010001101001010100110110"
                    "1110111000001001100110011110010110100111101011111101110000100001"
                    "1000111010111010101111110111010011110010010000101011111100100100"
                    "1110000111101011110100011110010111101001010010011010011100101111"
                    "1000000110110110101100000110101010111110101110001001100111001010"
                    "1011110011010011100100010101110110010111100011011011000101010001"
                    "1111000000110100100110001000001010110000101101001000110010011100"
                    "1100010111100011111010011010011110001011111110011001011110110101"
                    "1111011001000110110011110110010010011101001110101110100000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110001011011001101110110011001111000100100101111110101111000101"
                    "1110110010110010111000100100011010010000000010101111111101011000"
                    "1101000010101101110111100001111110100100111100111100010010010011"
                    "1000100110010100110000111011001011101101110101001001100110110100"
                    "1111100101011001100001011010011111100110001111111010101010010111"
                    "1000001111111110110101011001100011101101000010101100111010001110"
                    "1111000000111101101111010011010111000001100001111110000100000001"
                    "1111010010101010100000011000001010001110111110101110000001111010"
                    "1011001001000111100000011010011110111101001101011110111011000001"
                    "1000010001100010101101001011010011100111000010101001111100101110"
                    "1111011010100001111100001110110111001001101010111111110010101101"
                    "1110100001010010100001011000100010001010100100001101110101001100"
                    "1111111110101101111011010111110111101000001011011110100000000011"
                    "1001110010110010111011001000101011011110110101101001001010101100"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1101000101000011110110000010110111101010011000011011010101000001"
                    "1101111101111010110111110101100011001010001010101100011011100010"
                    "1011011101111111111111010000101111011110000101011000011000100011"
                    "1011011110000100101000101100100011100101110101001110000101001100"
                    "1000101100001001110001110111011111000100111010111111011110001001"
                    "1010100101101000110000011111110011001100101111101000111000101100"
                    "1100001101001011101001110000100110000100011001011100010001000011"
                    "1100000011000000110100100100111011000111110110001101011110011110"
                    "1001100111111111101100110100011110111000010011111000001000111111"
                    "1010100111101010101001000101100011100100011101001000011100011010"
                    "1100100101001111101001111111011111100110000101111001111100010011"
                    "1111100101100000100001110001101011001111010110101000001110010100"
                    "1110111011000011110010010010111111001001010110011011010111010011"
                    "1011110001011110101100100110011010111110100001001010011101010110"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110111000110011111011100100010111000011100101011011101100010011"
                    "1110110010010110100100001110111010010110110001101111010100110000"
                    "1001001101100001111011100010000110011101011001111010001001111001"
                    "1010011111000110111100001110010010011010011010101000100111101000"
                    "1011111111010001100011010010011111110100000001111000101111110011"
                    "1011011011010000111100000011000010100000011010001100111100001100"
                    "1010100010001111100111010000110110101100110010011011101100010101"
                    "1010001000011110101101010000111011011001111110001100011011110010"
                    "1100100011011111101101110100101111011111010101011000011100100101"
                    "1110001110100110101011000111000011111100100000001001111000001110"
                    "1111011111010111111111110100001110110110001101011111010010010111"
                    "1111110000011000100110001110001010111110000000001010111101111100"
                    "1010101010110001100001110010110111111100001000011001001011101101"
                    "1000001101010100110010010110101010000000001010101100100011101010"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /*155*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066", -1, 0, 64, 64, 1, "249 BASE256s + 6 ASCII (3 double-digits)", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1000010100011101100000010111100110100010110111011011111010000001"
                    "1100011110111110100110111101111010101101000010001101001011001100"
                    "1101101100010111110011110010101111111101101111111000111010011111"
                    "1000100000010100101110100100010011101111110111101111000010011100"
                    "1100011100101001111000111100010110100000001000111000010111011111"
                    "1001010100111000110001000111000010000110010001101001010100110110"
                    "1110111000001001100110011110010110100111101011111101110000100001"
                    "1000111010111010101111110111010011110010010000101011111100100100"
                    "1110000111101011110100011110010111101001010010011010011100101111"
                    "1000000110110110101100000110101010111110101110001001100111001010"
                    "1011110011010011100100010101110110010111100011011011000101010001"
                    "1111000000110100100110001000001010110000101101001000110010011100"
                    "1100010111100011111010011010011110001011111110011001011110110101"
                    "1111011001000110110011110110010010011101001110101110100000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110001011011001101110110011001111000100100101111110101111000101"
                    "1110110010110010111000100100011010010000000010101111111101011000"
                    "1101000010101101110111100001111110100100111100111100010010010011"
                    "1000100110010100110000111011001011101101110101001001100110110100"
                    "1111100101011001100001011010011111100110001111111010101010010111"
                    "1000001111111110110101011001100011101101000010101100111010001110"
                    "1111000000111101101111010011010111000001100001111110000100000001"
                    "1111010010101010100000011000001010001110111110101110000001111010"
                    "1011001001000111100000011010011110111101001101011110111011000001"
                    "1000010001100010101101001011010011100111000010101001111100101110"
                    "1111011010100001111100001110110111001001101010111111110010101101"
                    "1110100001010010100001011000100010001010100100001101110101001100"
                    "1111111110101101111011010111110111101000001011011110100000000011"
                    "1001110010110010111011001000101011011110110101101001001010101100"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1101000101000011110110000010110111101010011000011011010101000001"
                    "1101111101111010110111110101100011001010001010101100011011100010"
                    "1011011101111111111111010000101111011110000101011000011000100011"
                    "1011011110000100101000101100100011100101110101001110000101001100"
                    "1000101100001001110001110111011111000100111010111111011110001001"
                    "1010100101101000110000011111110011001100101111101000111000101100"
                    "1100001101001011101001110000100110000100011001011100010001000011"
                    "1100000011000000110100100100111011000111110110001101011110011110"
                    "1001100111111111101100110100011110111000010011111000001000111111"
                    "1010100111101010101001000101100011100100011101001000011100011010"
                    "1100100101001111101001111111011111100110000101111001111100010011"
                    "1111100101100000100001110001101011001111010110101000001110010100"
                    "1110111011000011110010010010111111001001010110011011010111010011"
                    "1011110001011110101100100110011010111110100001001010011101010110"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110111000110011111011100100010111000011100101011011101100010011"
                    "1110110010010110100100001110111010010110110001101111010100110000"
                    "1001001101100001111011100010000110011101011001111010001001111001"
                    "1010011111000110111100001110010010011010011010101000100111101000"
                    "1011111111010001100011010010011111110100000001111000101111110011"
                    "1011011011010000111100000011000010100000011010001100111100001100"
                    "1010100010001111100111010000110110101100110010011011101100010101"
                    "1010001000011110101101010000111011011001111110001100011011110010"
                    "1100100011011111101101110100101111011111010101011000011100100101"
                    "1110001110100110101011000111000011111100100000001001111000001110"
                    "1111011111010111111111110100001110110110001101011111010010010111"
                    "1111110000011000100110001110001010111110000000001010111101111100"
                    "1010101010110001100001110010110111111100001000011001001011101101"
                    "1000001101010100110010010110101010000000001010101100100011101010"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /*156*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066", -1, 0, 64, 64, 0, "249 BASE256s + 8 ASCII (Sh A80 + 3 double-digits); BWIPP uses 2nd B256 length byte instead of ASCII shift (same no. of codewords)", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1000010100011101100000010111100110100010110111011011111010000001"
                    "1100011110111110100110111101111010101101000010001101001011001100"
                    "1101101100010111110011110010101111111101101111111000111010011111"
                    "1000100000010100101110100100010011101111110111101111000010011100"
                    "1100011100101001111000111100010110100000001000111000010111011111"
                    "1001010100111000110001000111000010000110010001101001010100110110"
                    "1110111000001001100110011110010110100111101011111101110000100001"
                    "1000111010111010101111110111010011110010010000101011111100100100"
                    "1110000111101011110100011110010111101001010010011010011100101111"
                    "1000000110110110101100000110101010111110101110001001100111001010"
                    "1011110011010011100100010101110110010111100011011011000101010001"
                    "1111000000110100100110001000001010110000101101001000110010011100"
                    "1100010111100011111010011010011110001011111110011001011110100101"
                    "1111011001000110110011110110010010011101001110101110100000111100"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110001011011001101110110011001111000100100101111110101110001101"
                    "1110110010110010111000100100011010010000000010101111111100110100"
                    "1101000010101101110111100001111110100100111100111100010000000001"
                    "1000100110010100110000111011001011101101110101001001100000001100"
                    "1111100101011001100001011010011111100110001111111010101001000111"
                    "1000001111111110110101011001100011101101000010101100111101110110"
                    "1111000000111101101111010011010111000001100001111110000111011001"
                    "1111010010101010100000011000001010001110111110101111011000110110"
                    "1011001001000111100000011010011110111101001101011100111100001001"
                    "1000010001100010101101001011010011100111000010101110100100111110"
                    "1111011010100001111100001110110111001001101011111100111010000111"
                    "1110100001010010100001011000100010001010110101101010010100110100"
                    "1111111110101101111011010111110111101000101010111010111100010011"
                    "1001110010110010111011001000101011011100011101101100100011101100"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1101000101000011110110000010110111101000011000111110111010011001"
                    "1101111101111010110111110101100011001000101011101100100111110010"
                    "1011011101111111111111010000101111010010000110111110001111100111"
                    "1011011110000100101000101100100011101101110010101111000100001000"
                    "1000101100001001110001110111011111100100111110111000100100000101"
                    "1010100101101000110000011111111010100100001011101001001011010010"
                    "1100001101001011101001110000101111100100000011111111101010100011"
                    "1100000011000000110100100100100011000101101010101101011011101110"
                    "1001100111111111101100110100000110111010110011011010001010001111"
                    "1010100111101010101001000101001011101010101011101100000011001110"
                    "1100100101001111101001111111011111101011101001011001101101011011"
                    "1111100101100000100001110001101011011101110000001010101100110100"
                    "1110111011000011110010010010111111011111110111111110001001101011"
                    "1011110001011110101100100110011010001101000111001110011110001110"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110111000110011111011100100010111111011010010011101000000001011"
                    "1110110010010110100100001110100011101000100110101001000110101000"
                    "1001001101100001111011100010111111111010011111011010100111110001"
                    "1010011111000110111100001110111011011000000101101101011000001000"
                    "1011111111010001100011010011011111001000000100011011100011101111"
                    "1011011011010000111100001000111010101111011101101101010111001000"
                    "1010100010001111100111010001110110110101011001011101010011101001"
                    "1010001000011110101101011010000011001010110101101000110111100000"
                    "1100100011011111101101011101110110001000001011111000100101111101"
                    "1110001110100110101001100110100010111111000101001110100100110110"
                    "1111011111010111111110100001101111100010101100111101010001000111"
                    "1111110000011000101010110101101010000111111000001011100010000000"
                    "1010101010110001101001110010010111001010001001011001111101101001"
                    "1000001101010100110100010100101010001100001000101100101001101010"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /*157*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066", -1, 0, 64, 64, 0, "249 BASE256s + 8 ASCII (Sh A80 + 3 double-digits); BWIPP uses 2nd B256 length byte instead of ASCII shift (same no. of codewords)", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1000010100011101100000010111100110100010110111011011111010000001"
                    "1100011110111110100110111101111010101101000010001101001011001100"
                    "1101101100010111110011110010101111111101101111111000111010011111"
                    "1000100000010100101110100100010011101111110111101111000010011100"
                    "1100011100101001111000111100010110100000001000111000010111011111"
                    "1001010100111000110001000111000010000110010001101001010100110110"
                    "1110111000001001100110011110010110100111101011111101110000100001"
                    "1000111010111010101111110111010011110010010000101011111100100100"
                    "1110000111101011110100011110010111101001010010011010011100101111"
                    "1000000110110110101100000110101010111110101110001001100111001010"
                    "1011110011010011100100010101110110010111100011011011000101010001"
                    "1111000000110100100110001000001010110000101101001000110010011100"
                    "1100010111100011111010011010011110001011111110011001011110100101"
                    "1111011001000110110011110110010010011101001110101110100000111100"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110001011011001101110110011001111000100100101111110101110001101"
                    "1110110010110010111000100100011010010000000010101111111100110100"
                    "1101000010101101110111100001111110100100111100111100010000000001"
                    "1000100110010100110000111011001011101101110101001001100000001100"
                    "1111100101011001100001011010011111100110001111111010101001000111"
                    "1000001111111110110101011001100011101101000010101100111101110110"
                    "1111000000111101101111010011010111000001100001111110000111011001"
                    "1111010010101010100000011000001010001110111110101111011000110110"
                    "1011001001000111100000011010011110111101001101011100111100001001"
                    "1000010001100010101101001011010011100111000010101110100100111110"
                    "1111011010100001111100001110110111001001101011111100111010000111"
                    "1110100001010010100001011000100010001010110101101010010100110100"
                    "1111111110101101111011010111110111101000101010111010111100010011"
                    "1001110010110010111011001000101011011100011101101100100011101100"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1101000101000011110110000010110111101000011000111110111010011001"
                    "1101111101111010110111110101100011001000101011101100100111110010"
                    "1011011101111111111111010000101111010010000110111110001111100111"
                    "1011011110000100101000101100100011101101110010101111000100001000"
                    "1000101100001001110001110111011111100100111110111000100100000101"
                    "1010100101101000110000011111111010100100001011101001001011010010"
                    "1100001101001011101001110000101111100100000011111111101010100011"
                    "1100000011000000110100100100100011000101101010101101011011101110"
                    "1001100111111111101100110100000110111010110011011010001010001111"
                    "1010100111101010101001000101001011101010101011101100000011001110"
                    "1100100101001111101001111111011111101011101001011001101101011011"
                    "1111100101100000100001110001101011011101110000001010101100110100"
                    "1110111011000011110010010010111111011111110111111110001001101011"
                    "1011110001011110101100100110011010001101000111001110011110001110"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110111000110011111011100100010111111011010010011101000000001011"
                    "1110110010010110100100001110100011101000100110101001000110101000"
                    "1001001101100001111011100010111111111010011111011010100111110001"
                    "1010011111000110111100001110111011011000000101101101011000001000"
                    "1011111111010001100011010011011111001000000100011011100011101111"
                    "1011011011010000111100001000111010101111011101101101010111001000"
                    "1010100010001111100111010001110110110101011001011101010011101001"
                    "1010001000011110101101011010000011001010110101101000110111100000"
                    "1100100011011111101101011101110110001000001011111000100101111101"
                    "1110001110100110101001100110100010111111000101001110100100110110"
                    "1111011111010111111110100001101111100010101100111101010001000111"
                    "1111110000011000101010110101101010000111111000001011100010000000"
                    "1010101010110001101001110010010111001010001001011001111101101001"
                    "1000001101010100110100010100101010001100001000101100101001101010"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /*158*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, -1, "\101\102\103\104\105\106\107\110\111\112\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066", -1, 0, 64, 64, 1, "10 ASCII + 251 BASE256s + 6 ASCII", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1010011010011101100000010111100110100010110111011011111010000001"
                    "1011001010111110100110111101111010101101000010001101001011001100"
                    "1001100010010111110011110010101111111101101111111000111010011111"
                    "1010100001010100101110100100010011101111110111101111000010011100"
                    "1000001110101001111000111100010110100000001000111000010111011111"
                    "1100011000111000110001000111000010000110010001101001010100110100"
                    "1111100000001001100110011110010110100111101011111101110000100001"
                    "1110101010111010101111110111010011110010010000101011111100100110"
                    "1110100111101011110100011110010111101001010010011010011100101111"
                    "1100000110110110101100000110101010111110101110001001100111001010"
                    "1111110011010011100100010101110110010111100011011011000101010111"
                    "1111000000110100100110001000001010110000101101001000110010011110"
                    "1000010111100011111010011010011110001011111110011001011110100111"
                    "1111011001000110110011110110010010011101001110101110100000101000"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110001011011001101110110011001111000100100101111110101111000001"
                    "1110110010110010111000100100011010010000000010101111111100010100"
                    "1101000010101101110111100001111110100100111100111100010100011011"
                    "1000100110010100110000111011001011101101110101001001100011101110"
                    "1111100101011001100001011010011111100110001111111010110111111111"
                    "1000001111111110110101011001100011101101000010101100110011000110"
                    "1111000000111101101111010011010111000001100001111110000000101001"
                    "1111010010101010100000011000001010001110111110101111000001010010"
                    "1011001001000111100000011010011110111101001101011011010101100001"
                    "1000010001100010101101001011010011100111000010101000000000111110"
                    "1111011010100001111100001110110111001001101010111000100011100111"
                    "1110100001010010100001011000100010001010110100101010000001111100"
                    "1111111110101101111011010111110111101000000011111010000111001011"
                    "1001110010110010111011001000101011011101000101001100110000010100"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1101000101000011110110000010110111101001011000111100010011010001"
                    "1101111101111010110111110101100011001110101011001010000000010110"
                    "1011011101111111111111010000101111011010000100111110111001010011"
                    "1011011110000100101000101100100011100111110011001111110100000000"
                    "1000101100001001110001110111011111000100110101111011010110000011"
                    "1010100101101000110000011111110010001100110000001001101010011010"
                    "1100001101001011101001110000100111000100100111111111011000011011"
                    "1100000011000000110100100100101011000111011101101001000000110110"
                    "1001100111111111101100110100101110111010010111011011011001111111"
                    "1010100111101010101001000101100011101100010100001111010110111110"
                    "1100100101001111101001111100011111101100001000111010011110100111"
                    "1111100101100000100001111100101011011100011111101010000100001100"
                    "1110111011000011110010010100111111001101110101011011101100110001"
                    "1011110001011110101100011110011011000000100000001111100001111110"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110111000110011111011101100010111001111000011011001011111010011"
                    "1110110010010110100100100110001010110011000011101001101000101000"
                    "1001001101100001111000000010010110101111101001011010001101010001"
                    "1010011111000110111001001100001010000000011111101000001111111100"
                    "1011111111010001100111010000111110111110101101111110101011100011"
                    "1011011011010000100000000000001011001011001110001000100100100000"
                    "1010100010001111101111011011100110010010101001111111100001101111"
                    "1010001000011010101101100001111010010101101111001101000111100100"
                    "1100100011010011101101101000000111010011100001011011101110111101"
                    "1110001110101100101010000000001011111110110100101001000111100110"
                    "1111011111100101111110011110011110100000010010011001010011111111"
                    "1111110010100000101110000101001011111010000011001100110100101100"
                    "1011101011110001110011111110010111100110000000011001001101100001"
                    "1001010101010100111010010100101010000010001011001100101011101010"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /*159*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\101\102\103\104\105\106\107\110\111\112\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066", -1, 0, 64, 64, 0, "10 ASCII + 251 BASE256s + 6 ASCII; BWIPP same as FAST_MODE", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1010011010011101100000010111100110100010110111011011111010000001"
                    "1011001010111110100110111101111010101101000010001101001011001100"
                    "1001100000010111110011110010101111111101101111111000111010011111"
                    "1010100000010100101110100100010011101111110111101111000010011100"
                    "1000001101101001111000111100010110100000001000111000010111011111"
                    "1100010100111000110001000111000010000110010001101001010100110100"
                    "1111100000001001100110011110010110100111101011111101110000100001"
                    "1111001010111010101111110111010011110010010000101011111100100110"
                    "1011110111101011110100011110010111101001010010011010011100101111"
                    "1001000110110110101100000110101010111110101110001001100111001010"
                    "1011110011010011100100010101110110010111100011011011000101010111"
                    "1111000000110100100110001000001010110000101101001000110010011110"
                    "1000010111100011111010011010011110001011111110011001011110111111"
                    "1111011001000110110011110110010010011101001110101110100000101000"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110001011011001101110110011001111000100100101111110101110010001"
                    "1110110010110010111000100100011010010000000010101111111101110100"
                    "1101000010101101110111100001111110100100111100111100010000001011"
                    "1000100110010100110000111011001011101101110101001001100001111100"
                    "1111100101011001100001011010011111100110001111111010101010010111"
                    "1000001111111110110101011001100011101101000010101100100110110110"
                    "1111000000111101101111010011010111000001100001111111011001100001"
                    "1111010010101010100000011000001010001110111110101111100111000110"
                    "1011001001000111100000011010011110111101001101011101110100010101"
                    "1000010001100010101101001011010011100111000010101001100011110110"
                    "1111011010100001111100001110110111001001101010011111110101111001"
                    "1110100001010010100001011000100010001010110101001000001100001010"
                    "1111111110101101111011010111110111101000000011111111001100100011"
                    "1001110010110010111011001000101011011101000101001001011010011100"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1101000101000011110110000010110111101001011000111110101110110001"
                    "1101111101111010110111110101100011001110101001101010001110111010"
                    "1011011101111111111111010000101111011010000100111111100001111011"
                    "1011011110000100101000101100100011100111110100001100110100001000"
                    "1000101100001001110001110111011111000100111110011110101001001111"
                    "1010100101101000110000011111110010001100111010001101000111010100"
                    "1100001101001011101001110000100111000100110011111101010011101011"
                    "1100000011000000110100100100101011000110110111101100011111110110"
                    "1001100111111111101100110100101110111011011111111100010000101111"
                    "1010100111101010101001000101100011100010010110001111010101010010"
                    "1100100101001111101001111100011111101100010000011110000010110011"
                    "1111100101100000100001111100101011101011100110101111011011101000"
                    "1110111011000011110010010100111111100101101111011011100100011001"
                    "1011110001011110101100011110010010010011000111001110100010001100"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1110111000110011111011101100011111111010011111011110010000101011"
                    "1110110010010110100100100110000010111011000010101000101111011000"
                    "1001001101100001111000000010001111011000111000011100010111001001"
                    "1010011111000110111001001100000011111100100101101100011100001100"
                    "1011111111010001100111010011000111001011010111011010110000101111"
                    "1011011011010000100000001101010011100111111100101100011001000000"
                    "1010100010001111101111010101101110101110101001111000101111100001"
                    "1010001000011010101101000001010011111100111010101101001111010110"
                    "1100100011010011101101010000000111111100010100111011001010101101"
                    "1110001110101100101011010010011011001011000010101000001101000110"
                    "1111011111100101111110100111011110010111100010111111101000101111"
                    "1111110010100000100101001001110010011011001100001001000000000000"
                    "1011101011110001111111111011010110010010000001011001000101111001"
                    "1001010101010100111110010100001010001100001000001100100011101010"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /*160*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, -1, "\101\102\103\104\105\106\107\110\111\112\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066\067\070\071\060\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066", -1, 0, 88, 88, 1, "10 ASCII + 252 BASE256s + 10 ASCII + 253 BASE256 + 6 ASCII", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1010011010011100000001110111100010001011011111001111101000000110011111111000100110101111"
                    "1011001010111110011010111101111010110100001010010100101100110010101011111110101001100000"
                    "1001100010010111001111110010101111110110111111100011101001100100011100101011100101001111"
                    "1010100001010100111010100100010110111111011011111100001001111100101011000101111000110100"
                    "1000001110101001100011111100010010000000100110100001011101011010111111000101011110001111"
                    "1100011000111001000100100111000000011001000011100101010011011101001001110001001001110000"
                    "1111100100001000011001111110010010011110101111110111000000010000011111100000101111100001"
                    "1110101010111010111110110111010111001001000010101111110010010001101101110001010010011110"
                    "1110100111101011010001111110010110100101001110001001101011010100011111101100001000111101"
                    "1100000110110110110000100110101011111010111010000110011111101100001001111011110110101100"
                    "1111110011010010010001110101110001011110001111001101110110100100011101111010010101001111"
                    "1111000000110100011000101000001011000010110011000011011001100110001011000110010000100110"
                    "1000010111100011101001111010011000101111111110000010000000111010011010011110000010000111"
                    "1111011001000111001110110110010001110100111010111010000011110011101001000010011000110100"
                    "1110001011011000111011110011001100010010010111110010010110001111011011001010101101010101"
                    "1110110010110011100010100100011001000000001010111110110100111001101010111110011110101100"
                    "1101000010101101011111100001111010010011110110010110101011010001111010011100001111100001"
                    "1000100110010101000010111011001110110111010011000110000101010100001010101111100111010110"
                    "1111100101011000000101111010011110011000110111110001010110010000111110111110001110011111"
                    "1000001111111111010100111001100110110100001010110011111111100111001100010101010111010110"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1111000000111100111101110011010100000111101110010000001111101101011100011101100011000001"
                    "1111010010101010000000111000001000111011110011111100101011100100001110001110101000011010"
                    "1011001001000110000001111010011011110011001110110010010011011000011100001000010010101101"
                    "1000010001100010110100101011010110011100001010000100011011111010101110101111000001010010"
                    "1111011010100001110001101110110100111110000111110110111111111101111010010011011011011001"
                    "1110100001010010000100111000100000101010010011101000000110011001001001110010111100111010"
                    "1111111110101101101101110111110110000101101111111110001001011001111111010011010100101011"
                    "1001110010110011101100101000101101110000010010011101100000111011001000000110100011000100"
                    "1101000101000011011001100010110100111011100111010101001000101100111100010010100111001001"
                    "1101111101111011011110110101100100111010011011011011101100110111001111111000010011111010"
                    "1011011101111111111101110000100110000001001110100111111111011001011010100101111100001011"
                    "1011011110000100100010101100101010011110010010100101100001110111001001010111001010000000"
                    "1000101100001001000111110110100000110110111110100101000000111101011011101000010010011011"
                    "1010100101101001000000111111111011010000110011101100111010001001001000101111110001101010"
                    "1100001101001010100111110110101000000101011110111001001000101110011101100011110100001011"
                    "1100000011000001010010100100101000011010001011011110110011100010001010111111100011110110"
                    "1001100111111110110011101101111010111010111110111001011001010010011001011111111001100111"
                    "1010100111101010100100100111010110000100010010100000101001010011101001100100000000001010"
                    "1100100101001110100101111101011101111100000111001111111001111110011100101010000001010011"
                    "1111100101100000000110111100010011111110100011011001010011111100101010100100101010010000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1110111011000011000011100110100001000011101111100100110001010001011000011011100111110001"
                    "1011110001011110110010111000010110101100000010111111100001100111101100111000101001100100"
                    "1110111000110011010011100100101011010111101111001001110010101000111110100000111110011011"
                    "1110110010010110010110111110000100011110100010011011010001100111101101000010111011100000"
                    "1001001101100111110101100101000110000101011110111011100001110110111001101000010000100001"
                    "1010011111000111111000101110000110010111100010010011100110100111001001010110010011000100"
                    "1011111111001001110101111101001010101000010111011000011111100111011000010010010100110111"
                    "1011011011010000010000110011001111000011100010000000001110111001101100010000011000110000"
                    "1010100011101100011011101011110011110011100110001111000000101000011110010010011010111011"
                    "1010001000011010111010110110001011000000110011011100000000100010001001100000110110001110"
                    "1100100001100110110001100001110101111011110111010000001110000011111000111000000000011101"
                    "1110001111000110110100101110001001101101101011011100011100011010001110000101100100100110"
                    "1111010000110111100101110001101010000100000110011110010110010001111011000100000111100111"
                    "1111110011000001111100101110001011010010001011101111011001001111001011100011001001110100"
                    "1010001100000001101001110011111010100010101111001101101010101110011011011111110001001001"
                    "1000010111011010100000100111100011110101100011111000111101011110101000010110011111001110"
                    "1011010111001100001111100100010111111100001110101000110101100001011111010100111000110101"
                    "1110110110110001011010111001011010001000011011010100110101001111101001100111110001010100"
                    "1111110001100101100011101101110100111110000111000000000000101001011111111011011110000111"
                    "1010001001101110110100110011000100001001011010111110010111000110001101010100100000101100"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1000001000101010111111100010100100011111100111111001010000110111111110011011011110111011"
                    "1011111001101000001110100010111101010001001011011011100111011000101110111011010100011000"
                    "1111101000010101010011110011100100001101001110001011010000001000011000010111011000100001"
                    "1000111101011010101100110110101011111111110010011100110000100001101000110110001000101010"
                    "1011010110000010111111100101011010000011000111111101010000001001111101010101000111000101"
                    "1000010000110101101110110100000110110000101011001010111110101001101000010100010001000010"
                    "1111101100100000100111101110011111010101011110111101101010000000011001001011011011110101"
                    "1000101000110000000100100101001000011010011010101101100111101101101100111010010010011110"
                    "1000011101010100111111101011010111101000100110001100101100101000011010110111010010011011"
                    "1001110101110101001010101010100110010011100011101110011000110100001101000101101101110110"
                    "1110111110100110010111110110101110010100101111010011011000001100011010110111000001001001"
                    "1010110001101110011000100101010001110100011010110110100111110101001100000110010011001000"
                    "1110010001000010100001100001100011000001011111111010001110100010011100111001100100100001"
                    "1000100010000000110110110010011101011111000010001010111110101001101010000011000101010000"
                    "1100010110111111001001110000010001011010100110010111110001000011111100101101010000001001"
                    "1100111101111100100110111110111100111011011011110010110000001001101011011100110100100100"
                    "1010101101010111011011110011001001010101101111111011110110101011011001010111101011000111"
                    "1110001001100111001100101100111100010100100011110101011111010101001000101001111001111010"
                    "1001111110110111000111110010110001110001110111100111011110000010111010010101111110111001"
                    "1101010100110101110000110100001000100000101010010010101110100111001111100010001001001100"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                },
        /*161*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\101\102\103\104\105\106\107\110\111\112\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066\067\070\071\060\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200" "\061\062\063\064\065\066", -1, 0, 88, 88, 0, "10 ASCII + 252 BASE256s + 10 ASCII + 253 BASE256 + 6 ASCII; BWIPP same as FAST_MODE", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1010011010011100000001110111100010001011011111001111101000000110011111111000100110101111"
                    "1011001010111110011010111101111010110100001010010100101100110010101011111110101001100000"
                    "1001100000010111001111110010101111110110111111100011101001100100011100101011100101001111"
                    "1010100000010100111010100100010110111111011011111100001001111100101011000101111000110100"
                    "1000001110101001100011111100010010000000100110100001011101011010111111000101011110001111"
                    "1100010100111001000100100111000000011001000011100101010011011101001001110001001001110000"
                    "1111100000001000011001111110010010011110101111110111000000010000011111100000101111100001"
                    "1111001010111010111110110111010111001001000010101111110010010001101101110001010010011110"
                    "1011110111101011010001111110010110100101001110001001101011010100011111101100001000111101"
                    "1001000110110110110000100110101011111010111010000110011111101100001001111011110110101100"
                    "1011110011010010010001110101110001011110001111001101110110100100011101111010010101001111"
                    "1111000000110100011000101000001011000010110011000011011001100110001011000110010000100110"
                    "1000010111100011101001111010011000101111111110000010000000111010011010011110000010000111"
                    "1111011001000111001110110110010001110100111010111010000011110011101001000010011000110100"
                    "1110001011011000111011110011001100010010010111110010010110001111011011001010101101010101"
                    "1110110010110011100010100100011001000000001010111110110100111001101010111110011110101100"
                    "1101000010101101011111100001111010010011110110010110101011010001111010011100001111100001"
                    "1000100110010101000010111011001110110111010011000110000101010100001010101111100111010110"
                    "1111100101011000000101111010011110011000110111110001010110010000111110111110001110011111"
                    "1000001111111111010100111001100110110100001010110011111111100111001100010101010111010110"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1111000000111100111101110011010100000111101110010000001111101101011100011101100011000001"
                    "1111010010101010000000111000001000111011110011111100101011100100001110001110101000001110"
                    "1011001001000110000001111010011011110011001110110010010011011000011100001000010010001001"
                    "1000010001100010110100101011010110011100001010000100011011111010101110101111000000100110"
                    "1111011010100001110001101110110100111110000111110110111111111101111010010011011001010011"
                    "1110100001010010000100111000100000101010010011101000000110011001001001110010111010110110"
                    "1111111110101101101101110111110110000101101111111110001001011001111111010011001100010011"
                    "1001110010110011101100101000101101110000010010011101100000111011001000000110110001111100"
                    "1101000101000011011001100010110100111011100111010101001000101100111100010010110001110001"
                    "1101111101111011011110110101100100111010011011011011101100110111001111111000100001000110"
                    "1011011101111111111101110000100110000001001110100111111111011001011010100101011101111011"
                    "1011011110000100100010101100101010011110010010100101100001110111001001010000001001010000"
                    "1000101100001001000111110110100000110110111110100101000000111101011011111010000000011111"
                    "1010100101101001000000111111111011010000110011101100111010001001001000101111010001000110"
                    "1100001101001010100111110110101000000101011110111001001000101110011100011000010011111011"
                    "1100000011000001010010100100101000011010001011011110110011100010001011000011001011100110"
                    "1001100111111110110011101101111010111010111110111001011001010010011110111101100111110111"
                    "1010100111101010100100100111010110000100010010100000101001010011101001100110011100001110"
                    "1100100101001110100101111101011101111100000111001111111001111111111010110101000110001011"
                    "1111100101100000000110111100010011111110100011011001010011111101101101111000011111100100"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1110111011000011000011100110100001000011101111100100110001010001111011011111000010100101"
                    "1011110001011110110010111000010110101100000010111111100001100010101000011111110101110110"
                    "1110111000110011010011100100101011010111101111001001110010100000111101001100111000110011"
                    "1110110010010110010110111110000100011110100010011011010001110101101111111011101100111000"
                    "1001001101100111110101100101000110000101011110111011100000011101111111000001011001000001"
                    "1010011111000111111000101110000110010111100010010011100110111111101000111101011000100000"
                    "1011111111001001110101111101001010101000010111011000011100110001011110010000001100000111"
                    "1011011011010000010000110011001111000011100010000000001101010100101111011010110100011100"
                    "1010100011101100011011101011110011110011100110001111000000000010011010011111100010100011"
                    "1010001000011010111010110110001011000000110011011100011101110111001011000110101000011000"
                    "1100100001100110110001100001110101111011110111010001010100101011011001000101010100111101"
                    "1110001111000110110100101110001001101101101011011100101011110011101001110001111010100110"
                    "1111010000110111100101110001101010000100000110011111110010101110011011111000101000001111"
                    "1111110011000001111100101110001011010010001011101011010001010000101100100110101101100100"
                    "1010001100000001101001110011111010100010101111010000011111000110011001000011011001011101"
                    "1000010111011010100000100111100011110101100011110010101111111011101100110101101011101110"
                    "1011010111001100001111100100010111111100001111000001011110000001011010101011110111100101"
                    "1110110110110001011010111001011010001000011011010011001111100101101010100011100001101010"
                    "1111110001100101100011101101110100111110001110001101001000011100011001010111101000010111"
                    "1010001001101110110100110011000100001001000010110010100011100101001010001111111111000100"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1000001000101010111111100010100100011110100110100110100110110100111010111010001101101011"
                    "1011111001101000001110100010111101010000011011111010110111111100001101000100110000101100"
                    "1111101000010101010011110011100100001101011111000000000111011011011000011001000101011101"
                    "1000111101011010101100110110101011111111111011001011010000000010001000001001001010001010"
                    "1011010110000010111111100101011010000011010110110111101001111100011001101011111100011111"
                    "1000010000110101101110110100000110110001000011101110000010100100001111011011000001000110"
                    "1111101100100000100111101110011111010110001111010111011001110001011111000110010111011101"
                    "1000101000110000000100100101001000010010111011100000100000111100101101000101010100110110"
                    "1000011101010100111111101011010111101010010110001010110110110111011011010111010000100011"
                    "1001110101110101001010101010100110011000000010000010000111100001101000100011101001101110"
                    "1110111110100110010111110110101110110000101111011100101011111111111111100111011000110101"
                    "1010110001101110011000100101010010100011100010100001011111000000001101011100101000010000"
                    "1110010001000010100001100001100000001111000111101100100110110001111100010110011010011101"
                    "1000100010000000110110110010010100000101101010100010011100010110101101001111101111011110"
                    "1100010110111111001001110000010101001011011110011011011001001010111001110100101000010001"
                    "1100111101111100100110111110101111110010011010010111010111100000001101010001011010000100"
                    "1010101101010111011011110011110111000010110111111000000000010001111101011110011110110111"
                    "1110001001100111001100101110010110100011001011110001111110111001001000111011100111010010"
                    "1001111110110111000111110000110000011001010111000100001111101000111110011101010110101001"
                    "1101010100110101110000110100101000010000100010110010111110101011001111100010000101001100"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                },
        /*162*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, -1, "\101\102\103\104\105\106\107\110\111\112\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066\067\070\071\060\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200", -1, 0, 88, 88, 1, "10 ASCII + 252 BASE256s + 10 ASCII + 304 BASE256, no padding", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1010011010011100000001110111100010001011011111001111101000000110011111111000100110101111"
                    "1011001010111110011010111101111010110100001010010100101100110010101011111110101001100000"
                    "1001100010010111001111110010101111110110111111100011101001100100011100101011100101001111"
                    "1010100001010100111010100100010110111111011011111100001001111100101011000101111000110100"
                    "1000001110101001100011111100010010000000100110100001011101011010111111000101011110001111"
                    "1100011000111001000100100111000000011001000011100101010011011101001001110001001001110000"
                    "1111100100001000011001111110010010011110101111110111000000010000011111100000101111100001"
                    "1110101010111010111110110111010111001001000010101111110010010001101101110001010010011110"
                    "1110100111101011010001111110010110100101001110001001101011010100011111101100001000111101"
                    "1100000110110110110000100110101011111010111010000110011111101100001001111011110110101100"
                    "1111110011010010010001110101110001011110001111001101110110100100011101111010010101001111"
                    "1111000000110100011000101000001011000010110011000011011001100110001011000110010000100110"
                    "1000010111100011101001111010011000101111111110000010000000111010011010011110000010010111"
                    "1111011001000111001110110110010001110100111010111010000011110011101001000010011000111000"
                    "1110001011011000111011110011001100010010010111110010010110001111011011001010101100001001"
                    "1110110010110011100010100100011001000000001010111110110100111001101010111110011110110100"
                    "1101000010101101011111100001111010010011110110010110101011010001111010011100001101000001"
                    "1000100110010101000010111011001110110111010011000110000101010100001010101111100000111110"
                    "1111100101011000000101111010011110011000110111110001010110010000111110111110011111011111"
                    "1000001111111111010100111001100110110100001010110011111111100111001100010101011001101110"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1111000000111100111101110011010100000111101110010000001111101101011100011100100111001001"
                    "1111010010101010000000111000001000111011110011111100101011100100001110001110110001001110"
                    "1011001001000110000001111010011011110011001110110010010011011000011100001110110011111001"
                    "1000010001100010110100101011010110011100001010000100011011111010101110101000100001100110"
                    "1111011010100001110001101110110100111110000111110110111111111101111010000110011011011111"
                    "1110100001010010000100111000100000101010010011101000000110011001001001111000110000001010"
                    "1111111110101101101101110111110110000101101111111110001001011001111111000011011101111011"
                    "1001110010110011101100101000101101110000010010011101100000111011001000111101111101001100"
                    "1101000101000011011001100010110100111011100111010101001000101100111000110010101011010001"
                    "1101111101111011011110110101100100111010011011011011101100110111001000010011100100011010"
                    "1011011101111111111101110000100110000001001110100111111111011000111101011111111010010111"
                    "1011011110000100100010101100101010011110010010100101100001110111001111110101011010110000"
                    "1000101100001001000111110110100000110110111110100101000000111111111111001001000011000101"
                    "1010100101101001000000111111111011010000110011101100111010001011101010000101111001101100"
                    "1100001101001010100111110110101000000101011110111001001000101010011011001011001111101011"
                    "1100000011000001010010100100101000011010001011011110110011100000001101100001100011010110"
                    "1001100111111110110011101101111010111010111110111001011001010001111101011110100111001111"
                    "1010100111101010100100100111010110000100010010100000101001011011101000011100010100100110"
                    "1100100101001110100101111101011101111100000111001111111001111101011101010000100010110011"
                    "1111100101100000000110111100010011111110100011011001010011011011101100000001010000110000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1110111011000011000011100110100001000011101111100100110001001110011000000001000001111101"
                    "1011110001011110110010111000010110101100000010111111100011010000001100111010011101011110"
                    "1110111000110011010011100100101011010111101111001001110010000000011111110111111010000011"
                    "1110110010010110010110111110000100011110100010011011010100011000101110110101100000000000"
                    "1001001101100111110101100101000110000101011110111011101100001010011110111100001101010001"
                    "1010011111000111001000101110000110010111100010010011001010101100001010000101101001101000"
                    "1011111111001001100101111101001010101000010111011000001001000001111101001011000001000011"
                    "1011011011010000101000110011001111000011100010000010011011111110001000111101000110010100"
                    "1010100011101100011011101011110011110011100110001110000000111111111011101011111111101111"
                    "1010001000011010111010110110001011000000110011010000101011000001001010100000101011100010"
                    "1100100001100110110001100001110101111011110111011101100000010010011001011001111001001101"
                    "1110001111000110110100101110001001101101101011111101111000110001001001100000100000001110"
                    "1111010000110111100101110001101010000100000110001101010110100010111100100101011100011111"
                    "1111110011000001111100101110001011010010000011100001110011100110001000000101100010100100"
                    "1010001100000001101001110011111010100010101110010111111000110101011100110010101111100001"
                    "1000010111011010100000100111100011110101110010110110000110001100101101001111000001111010"
                    "1011010111001100001111100100010111111100110110100000011010110110111110100011100110010011"
                    "1110110110110001011010111001011010001010111010001010101001110100101010110101100011110000"
                    "1111110001100101100011101101110100111100101110111011010111011110111010101011110011010111"
                    "1010001001101110110100110011000100000010110010011110011101101001001101001001010011000100"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1000001000101010111111100010100100011001111111000011111100010010111110000100000100101011"
                    "1011111001101000001110100010111101100101011011010001000100111101101100101010001011101100"
                    "1111101000010101010011110011100100111000101110000011000001010011111011000110101100110101"
                    "1000111101011010101100110110101001000010101010001010111111110101001010001001001010010110"
                    "1011010110000010111111100101011000101001010110001101010110100000111110110000111101110001"
                    "1000010000110101101110110100001110111101001011010010000111011111101111011101001000100110"
                    "1111101100100000100111101110011001101110101110100001011100101001011110010100111111100101"
                    "1000101000110000000100100101010111001110010010011011001010001000101000100100100101001110"
                    "1000011101010100111111101011111011101010110111111110000000111001111100011101100110100011"
                    "1001110101110101001010101000000010000010110011100010010000111011101111111100101110011110"
                    "1110111110100110010111110100110000100101000110001000000010101101111010000001011110010001"
                    "1010110001101110011000101011011111001111011010011011011000010100001011100101011010111100"
                    "1110010001000010100001101000001111111101000110101000001000010010011001110111101011100011"
                    "1000100010000000110100101001001001000100110010110000000001011100001001100001000110001110"
                    "1100010110111111001011111101011101110010100110111111010100001010111100011011100011100001"
                    "1100111101111100101100101010011100111101001010011110000111110011101100100000001011011100"
                    "1010101101010111010011111000011100100011011110011011010001011000111010101000010111010111"
                    "1110001001100111011110111101000101001011000011100101001111000111101100100000000000011110"
                    "1001111110110111100111101011110100110001000110000111011111101000111100001101011010100001"
                    "1101010100110101100000110110001000001000101011110010001110100101001111000010001111001100"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                },
        /*163*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\101\102\103\104\105\106\107\110\111\112\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066\067\070\071\060\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200", -1, 0, 88, 88, 0, "10 ASCII + 252 BASE256s + 10 ASCII + 304 BASE256, no padding; BWIPP same as FAST_MODE", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1010011010011100000001110111100010001011011111001111101000000110011111111000100110101111"
                    "1011001010111110011010111101111010110100001010010100101100110010101011111110101001100000"
                    "1001100000010111001111110010101111110110111111100011101001100100011100101011100101001111"
                    "1010100000010100111010100100010110111111011011111100001001111100101011000101111000110100"
                    "1000001110101001100011111100010010000000100110100001011101011010111111000101011110001111"
                    "1100010100111001000100100111000000011001000011100101010011011101001001110001001001110000"
                    "1111100000001000011001111110010010011110101111110111000000010000011111100000101111100001"
                    "1111001010111010111110110111010111001001000010101111110010010001101101110001010010011110"
                    "1011110111101011010001111110010110100101001110001001101011010100011111101100001000111101"
                    "1001000110110110110000100110101011111010111010000110011111101100001001111011110110101100"
                    "1011110011010010010001110101110001011110001111001101110110100100011101111010010101001111"
                    "1111000000110100011000101000001011000010110011000011011001100110001011000110010000100110"
                    "1000010111100011101001111010011000101111111110000010000000111010011010011110000010010111"
                    "1111011001000111001110110110010001110100111010111010000011110011101001000010011000111000"
                    "1110001011011000111011110011001100010010010111110010010110001111011011001010101100001001"
                    "1110110010110011100010100100011001000000001010111110110100111001101010111110011110110100"
                    "1101000010101101011111100001111010010011110110010110101011010001111010011100001101000001"
                    "1000100110010101000010111011001110110111010011000110000101010100001010101111100000111110"
                    "1111100101011000000101111010011110011000110111110001010110010000111110111110011111011111"
                    "1000001111111111010100111001100110110100001010110011111111100111001100010101011001101110"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1111000000111100111101110011010100000111101110010000001111101101011100011100100111001001"
                    "1111010010101010000000111000001000111011110011111100101011100100001110001110110001011010"
                    "1011001001000110000001111010011011110011001110110010010011011000011100001110110011011101"
                    "1000010001100010110100101011010110011100001010000100011011111010101110101000100000010010"
                    "1111011010100001110001101110110100111110000111110110111111111101111010000110011001010101"
                    "1110100001010010000100111000100000101010010011101000000110011001001001111000110110000110"
                    "1111111110101101101101110111110110000101101111111110001001011001111111000011000101000011"
                    "1001110010110011101100101000101101110000010010011101100000111011001000111101101111110100"
                    "1101000101000011011001100010110100111011100111010101001000101100111000110010111101101001"
                    "1101111101111011011110110101100100111010011011011011101100110111001000010011010110100110"
                    "1011011101111111111101110000100110000001001110100111111111011000111101011111011011100111"
                    "1011011110000100100010101100101010011110010010100101100001110111001111110010011001100000"
                    "1000101100001001000111110110100000110110111110100101000000111111111111011011010001000001"
                    "1010100101101001000000111111111011010000110011101100111010001011101010000101011001000000"
                    "1100001101001010100111110110101000000101011110111001001000101010011010110000101000011011"
                    "1100000011000001010010100100101000011010001011011110110011100000001100011101001011000110"
                    "1001100111111110110011101101111010111010111110111001011001010001111010111100111001011111"
                    "1010100111101010100100100111010110000100010010100000101001011011101000011110001000100010"
                    "1100100101001110100101111101011101111100000111001111111001111100111011001111100101101011"
                    "1111100101100000000110111100010011111110100011011001010011011010101011011101100101000100"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1110111011000011000011100110100001000011101111100100110001001110111011000101100100101001"
                    "1011110001011110110010111000010110101100000010111111100011010101001000011101000001001100"
                    "1110111000110011010011100100101011010111101111001001110010001000011100011011111100101011"
                    "1110110010010110010110111110000100011110100010011011010100001010101100001100110111011000"
                    "1001001101100111110101100101000110000101011110111011101101100001011000010101000100110001"
                    "1010011111000111001000101110000110010111100010010011001010110100101011101110100010001100"
                    "1011111111001001100101111101001010101000010111011000001010010111111011001001011001110011"
                    "1011011011010000101000110011001111000011100010000010011000010011001011110111101010111000"
                    "1010100011101100011011101011110011110011100110001110000000010101111111100110000111110111"
                    "1010001000011010111010110110001011000000110011010000110110010100001000000110110101110100"
                    "1100100001100110110001100001110101111011110111011100111010111010111000100100101101101101"
                    "1110001111000110110100101110001001101101101011111101001111011000101110010100111110001110"
                    "1111010000110111100101110001101010000100000110001100110010011101011100011001110011110111"
                    "1111110011000001111100101110001011010010000011100101111011111001101111000000000110110100"
                    "1010001100000001101001110011111010100010101110001010001101011101011110101110000111110101"
                    "1000010111011010100000100111100011110101110010111100010100101001101001101100110101011010"
                    "1011010111001100001111100100010111111100110111001001110001010110111011011100101001000011"
                    "1110110110110001011010111001011010001010111010001101010011011110101001110001110011001110"
                    "1111110001100101100011101101110100111100100111110110011111101011111100000111000101000111"
                    "1010001001101110110100110011000100000010101010010010101001001010001010010010001100101100"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "1000001000101010111111100010100100011000111110011100001010010001111010100101010111111011"
                    "1011111001101000001110100010111101100100001011110000010100011001001111010101101111011000"
                    "1111101000010101010011110011100100111000111111001000010110000000111011001000110001001001"
                    "1000111101011010101100110110101001000010100011011101011111010110101010110110001000110110"
                    "1011010110000010111111100101011000101001000111000111101111010101011010001110000110101011"
                    "1000010000110101101110110100001110111100100011110110111011010010001000010010011000100010"
                    "1111101100100000100111101110011001101101111111001011101111011000011000011001110011001101"
                    "1000101000110000000100100101010111000110110011010110001101011001101001011011100011100110"
                    "1000011101010100111111101011111011101000000111111000011010100110111101111101100100011011"
                    "1001110101110101001010101000000010001001010010001110001111101110001010011010101010000110"
                    "1110111110100110010111110100110000000001000110000111110001011110011111010001000111101101"
                    "1010110001101110011000101011011100011000100010001100100000100001001010111111100001100100"
                    "1110010001000010100001101000001100110011011110111110100000000001111001011000010101011111"
                    "1000100010000000110100101001000000011110011010011000100011100011001110101101101100000000"
                    "1100010110111111001011111101011001100011011110110011111100000011111001000010011011111001"
                    "1100111101111100101100101010001111110100001011111011100000011010001010101101100101111100"
                    "1010101101010111010011111000100010110100000110011000100111100010011110100001100010100111"
                    "1110001001100111011110111111101111111100101011100001101110101011101100110010011110110110"
                    "1001111110110111100111101001110101011001100110100100001110000010111000000101110010110001"
                    "1101010100110101100000110110101000111000100011010010011110101001001111000010000011001100"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                },
        /*164*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, -1, "@@@@@@@@@_", -1, 0, 8, 32, 0, "EDI; BWIPP uses different encodation, switching to ASC for last 2 chars, not last 3 like Zint", 0,
                    "10101010101010101010101010101010"
                    "10000000001001111001101100001101"
                    "10000000000001001001110011001100"
                    "10000000000110011111100101100001"
                    "11000000110101101100001101111000"
                    "11000001110100111000111101101011"
                    "11000000000000001001000001011010"
                    "11111111111111111111111111111111"
                },
        /*165*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "@@@@@@@@@_", -1, 0, 8, 32, 0, "EDI **NOT SAME** (see FAST_MODE); BWIPP uses different encodation", 0,
                    "10101010101010101010101010101010"
                    "11100000000000011000100100101001"
                    "11100000000000001010011101001000"
                    "10000000000011111010010010100101"
                    "10000000000100101101001011011110"
                    "11000000001000111010111110011001"
                    "10000000000001001010010000010000"
                    "11111111111111111111111111111111"
                },
        /*166*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, -1, "@@@@@@@@@@_", -1, 0, 16, 16, 0, "EDI; BWIPP uses different encodation, switching to ASC for last 3 chars, not last 4 like Zint", 0,
                    "1010101010101010"
                    "1000000001000001"
                    "1000000000111110"
                    "1000000100010001"
                    "1000000000101110"
                    "1000001001110111"
                    "1000010001010110"
                    "1110100011110001"
                    "1110001110011000"
                    "1010010110010101"
                    "1000111010010110"
                    "1001011001000001"
                    "1000110101100010"
                    "1110100110001101"
                    "1010100000010010"
                    "1111111111111111"
                },
        /*167*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "@@@@@@@@@@_", -1, 0, 8, 32, 0, "AAEEEEEEEEA; BWIPP uses different encodation, see above", 1,
                    "10101010101010101010101010101010"
                    "10100000000000111000110101111001"
                    "10000000000001001000100100011000"
                    "10010000000011011010001100000101"
                    "10000000001111001001110110011110"
                    "11000110001010011011101010011011"
                    "10000110001100001001010000001110"
                    "11111111111111111111111111111111"
                },
        /*168*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, -1, "@@@@@@@@@@@_", -1, 0, 16, 16, 0, "EDI; BWIPP uses different encodation, switching to ASC for last 4 chars, not last 1 like Zint", 0,
                    "1010101010101010"
                    "1000000001000001"
                    "1000000000001100"
                    "1000000100100001"
                    "1000000000000010"
                    "1000001001100111"
                    "1000010001000110"
                    "1000000010101001"
                    "1000111011011000"
                    "1001110001101101"
                    "1000111000000000"
                    "1000100111110111"
                    "1011100111000100"
                    "1011000110111101"
                    "1000100000100010"
                    "1111111111111111"
        },
        /*169*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "@@@@@@@@@@@_", -1, 0, 16, 16, 0, "AAAEEEEEEEEA; BWIPP uses different encodation, see above", 0,
                    "1010101010101010"
                    "1010011101000001"
                    "1000000000101000"
                    "1001000100101111"
                    "1000000000000100"
                    "1100001001010011"
                    "1000010000001000"
                    "1000000000100001"
                    "1000001011010100"
                    "1000000001111001"
                    "1000000110011000"
                    "1000011111101101"
                    "1001011010100110"
                    "1011001101111101"
                    "1011000011010010"
                    "1111111111111111"
                },
        /*170*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, -1, "@@@@@@@@@@@@_", -1, 0, 16, 16, 0, "EDI; BWIPP uses different encodation, switching to ASC for last 5 chars, not last 2 like Zint", 0,
                    "1010101010101010"
                    "1000000000100001"
                    "1000000000111000"
                    "1000000010111001"
                    "1000000100100100"
                    "1000001000100111"
                    "1000000011111110"
                    "1000000110111001"
                    "1000110010010100"
                    "1001111110000101"
                    "1000000010010000"
                    "1000101101011001"
                    "1001010001011110"
                    "1000000111000101"
                    "1011100000010010"
                    "1111111111111111"
                },
        /*171*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "@@@@@@@@@@@@_", -1, 0, 16, 16, 0, "EEEEEEEEEEEEA; BWIPP uses different encodation, see above", 1,
                    "1010101010101010"
                    "1000000001100001"
                    "1000000000110100"
                    "1000000100001011"
                    "1000000000001100"
                    "1000001001101111"
                    "1000010001000110"
                    "1000000001101001"
                    "1000001111100100"
                    "1000001011000101"
                    "1000001001010000"
                    "1000011111011001"
                    "1010111001010110"
                    "1001100010001101"
                    "1000000000100010"
                    "1111111111111111"
                },
        /*172*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, -1, "@@@@@@@@@@@@@_", -1, 0, 12, 26, 0, "EDI; BWIPP uses different encodation, switching to ASC for last 2 chars, not last 3 like Zint", 0,
                    "10101010101010101010101010"
                    "10000000001001100100101011"
                    "10000000000010000000111000"
                    "10000000000100011100111111"
                    "10000000110100111100011110"
                    "10000001110100100011100001"
                    "10000000111110000011001100"
                    "10000000101110100110101101"
                    "10000000101010000110010000"
                    "10000010101001111100101001"
                    "10000001000001101011010000"
                    "11111111111111111111111111"
                },
        /*173*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "@@@@@@@@@@@@@_", -1, 0, 16, 16, 0, "AEEEEEEEEEEEEA; BWIPP uses different encodation, see above", 1,
                    "1010101010101010"
                    "1110000001000001"
                    "1110000001000000"
                    "1000000011000111"
                    "1000000100011100"
                    "1100000000000011"
                    "1000000011100000"
                    "1000000011101001"
                    "1000001011000000"
                    "1000001110101001"
                    "1000101111000000"
                    "1000110100011011"
                    "1011100010001000"
                    "1000000000000101"
                    "1001100000111010"
                    "1111111111111111"
                },
        /*174*/ { BARCODE_DATAMATRIX, FAST_MODE, 26, -1, -1, -1, "abcdefghi1234FGHIJKLMNabc@@@@@@@@@é", -1, 0, 24, 24, 0, "Mix of modes TEX ASC C40 ASC EDI BAS; BWIPP uses different encodation", 0,
                    "101010101010101010101010"
                    "100111011110011101000101"
                    "101111001100101101101000"
                    "101110110110001110110011"
                    "100100110000101100000010"
                    "111011010011000111011111"
                    "101010011000101100110110"
                    "100111101000011111010001"
                    "100111010000001011100100"
                    "101110101000001101111011"
                    "101111100000100111011110"
                    "111111100000011100111011"
                    "100111000010101010001010"
                    "101000000010000111010101"
                    "110100001000111110001000"
                    "100000000100100010100001"
                    "111000101000011111101100"
                    "111000101111000100000011"
                    "111001010010011000011000"
                    "100001100010101101010101"
                    "101011000110000010000110"
                    "110111111101101101101101"
                    "111101010110111111111010"
                    "111111111111111111111111"
                },
        /*175*/ { BARCODE_DATAMATRIX, -1, 26, -1, -1, -1, "abcdefghi1234FGHIJKLMNabc@@@@@@@@@é", -1, 0, 24, 24, 0, "TTTTTTTTTAAAACCCCCCCCCAAAAAEEEEEEEAA; BWIPP uses different encodation", 0,
                    "101010101010101010101010"
                    "100111011110011100000101"
                    "101111001100101100111100"
                    "101110110110001001111011"
                    "100100110000101000010110"
                    "111011010011000000000111"
                    "101010011000100011100110"
                    "100111101000000010101001"
                    "100111010000000001110100"
                    "101110101000001010001111"
                    "101111100000000100010110"
                    "111111100000101111100001"
                    "100111000010010000000000"
                    "101001100010111000100101"
                    "110101101110101010101000"
                    "100010000111101110100001"
                    "111000111000111001101000"
                    "101001101100011000001011"
                    "100011000001001010000000"
                    "100111010111001001111001"
                    "101100001001101010011110"
                    "111101111010101100000101"
                    "111011010111011111010010"
                    "111111111111111111111111"
                },
        /*176*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^;<>@[]_`~!||()?{}'ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^;<>@[]_`~!||()?{}'\001\002\003\004\005\006...............\015\015\015\015\015\015\015\015abcdefghijklmnopqrstuvwxyz\015\015\015\015\015\015\015\015...............\001\002\003\004\005\006ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^...............", -1, 0, 132, 132, 0, "Mixed modes (except B256); BWIPP different encodation", 0,
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "101001101001010011000111100011111001011001011010011010010111111101110111100110110110000111110000111000011100111110111010111110010101"
                    "101100101010110110010011000010100000111010101111010100001111001010100011010100110101000010101011010001111011001010111111011000100110"
                    "100110010000100111011110100001010100101110111101000100110001000011101111111010010111110110001000111010011010111111011011000000110111"
                    "101010011110000110101010001010010010000101101101000011010001111110111011001010001100111011101001110100000111101100001000011111100100"
                    "100001101000001110110111000000101011010001011111111010111100100001111100100100110101001110111110100101000000011111100111011010010111"
                    "110000101111011110011010000001101111000100001110000111001100111010101010100011100111001011010111010100000100001100110101011111001100"
                    "111101011101000110000110111101011100101010111111000001010100011101100000011010110101000111010110110101101110111000001001011011101001"
                    "111100100111111111101010110000000110111100101100001001100110011110110100000111100001110010000010011001110010001110110000111110010010"
                    "110110011010110000100110010000000010111110111101101100111000110011100111000010011110011111111000101101010110111110010011111100000001"
                    "101111110100111101111010011100110111001000001001111000111011010010101110001111001110100011001010111010011001101001101101110000100110"
                    "101011101110100100000110101100100010010010111101100110111001010001110101101011110000101110000000101111000101111101101011110110101101"
                    "101011000011010111011011110110000100101101101011110111101000011000111001111110101011000011111100101110001011101100011101100100101010"
                    "110110000001101101000111110100001000100010111110111110111011100111101101111100001001101111001010101001000010111100001100110111111011"
                    "100010110001001100001011011111001110100010101111110111010011100100111001110111101000011010000100111101110000101000100001101111000110"
                    "101110110111001001001110011110111011101010111111111101011011100011101101110010111001010111110000101110101011011110011010110000011111"
                    "111101111011011010001011011100111101010001101101001100011111110010101001110011010010000011001111111010101110001111010010010001110110"
                    "111111101001100101100111111011000101000001111101100111111001001011100011010001010000000110011111110101101011011100101010111100011001"
                    "101101101101010111011011010000110000111001101110101011011010110010101100011100011110011010100111010001110110101011010011001100111100"
                    "101101111111010111111110000001011001110011111100011111111111000011100111100111101010111110000111011001101011111001100001000010111111"
                    "101100010101001011001011011110100010101100101010001100000100111010100011101000100010110010100110111001001010001111111001100110100000"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "111111000101011100010110011100111000100010111110110110010001111111111100001101011110111110001100101110101011011001111100110001001101"
                    "111101101010101010000011111011010010100111101111111011000010110100101010110100111101111010110010011011001111001000111001100010000110"
                    "110101100101000001100111011000111011111001011001110000111000010101100010010110000110101110001011111000001011111000100111001011001111"
                    "100100011111110111111011101001111101000011101010100110110011000110111010001101000111101010001100000110000011001011110011110111101100"
                    "111011100100101000110110111110100100000001111010110010111001110011101111001011110110111111011101000111101001111010101101000001001001"
                    "100110001100010000111011010111110001111001101111110111111110010010110100101100100011010010001000000110111000101111100001001011010010"
                    "111001010100010110110111010110011101100001111001100110101011010001110010110101111001000110000001101010100110111111011101110110111101"
                    "111011000101010111110010000011100101100100001010000111011110011100100011100111001100010010000111010111000001101101101100000000000010"
                    "100111111011011110011111111010100101000000011000010110011111100111110111110010000110110110010001110110010101111000001011111101000011"
                    "101110111001110110100011000111100010010111101111000011001011101000100011100001111001010011000111100100001001101011101101011000010110"
                    "101111000011111100101110000111001100110110111111111010111100001001100010011011101111010111010110111001000011011010000011111000101001"
                    "110010101011001011000011110010111011010101101101000100101010110100101111000101111101100010101101101000101100001000110011001101111110"
                    "110000000101101001101111101110101000010111111010001011000010001101101010010101101001000110111111100011100110111101111001111110010011"
                    "111111110110101000010011001001101000001101001001001011111010010110111110101100101100111010111001100010100101101010100011101100111000"
                    "101000111100111001011110110000111100111001111011100100101111110111111100001011100110010110101111001011101011111100001001001010100101"
                    "101000001011010010001010100011010011001100001110100100110101010110110111010011011000010011001110111101001001001111010101100110001100"
                    "100110101101010000100111011001011010010001011100010100010010010111111010101111110100011110111111110101010100111101001011010011001101"
                    "100101111100011110100010011100000110000010001100000111000011100110101000101011100101010010110101110001111101001010000101101101011000"
                    "100110010111101000001111010111000001110000011111111001111001001101110110011100001111110110010011011001101001111101000001010011111101"
                    "111000111000100100011010111000000111111110101110111010010110110100101010001000011101100010111111100101010001101110010011101000111100"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "100000101101010110101111101101101001101001011111000011111110010111100001000011010001110111001010110000110011011101110111001001110011"
                    "111101111100101110100011111001010000101001101010101100111010001100110001111100011001111011010111101000111100001000000111001010101000"
                    "111001110110010010100111101101010101010111011000100111001111001001111100100101011001111111100111100100111111111011100010100111100101"
                    "111100101110011100110010111001010100100101101110100010110100101010111100100111000111110011110011111011101011101111101101001000000100"
                    "111011001001110000101110101111010110100100111011110000010010110101110100001110111001001111101010011000000000011010000000001001111101"
                    "110010101001001101001010101001101111110100101101011111000011100110100101111001010001110011001100001011110110101011101010011010100000"
                    "101010111000011110101110100011001001010100011101001001110111011011111000010110100100101110111011111100100111111010010011000010111111"
                    "101011111100100010110010101010101010100111001001001110000010010010110000101101001101001010100111100100111000001001000000101001000110"
                    "110111010110001001000110000010111111111001111101001010000010011011110101101011111110100110010001000011110000111101101001110001001101"
                    "100010101100101000111011110011000100111110001001011111001111011110101001100011110110100011100101000110010101101101111000010111010000"
                    "100011101110000001110110110111010111001010111110011011000101111001110010001001001100010111101010110100110101111001010111110000101001"
                    "100001011101000010100010000011000010011110101111001101101011010100111111010101110100001011000011100001001010101111000010000101101100"
                    "100101101001111000011110000000110000011110111110111101111111110101111011111100100110011111000111011001111001111001001110100011001101"
                    "110001111011110000010011000110111111001111001100110100010100010010111101101101101110011010001011100011111000101000000100111011011010"
                    "101101100100111000001111000011001010111110111010100101010110010111111011111110010010010111001110110011100110111111110100110100100001"
                    "101000100111111001110011110111000100110100101010111100011010101100110010110110011110001011100101111101011011101001100111101010001110"
                    "111110011110101011110111100101001011111110011110010011110001011111111011001101111001101111000000111000000011011101101111011110010111"
                    "110010010111001011011010000001001011001010001101011010100111110000100011111001001111110011101111001000001010001001101101001010110010"
                    "100010010110111010010110001110100101100001111111001010111001011011101110101000101000000110011000000101111001011100101010100010001011"
                    "111111000011110011111011001110100110010000101111011111100011100100110110111100000110001011011001111011110010101101100101110001011000"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "110101111000011001111111101100011100011111011010011001010111000001100000010101000111001111111100111111101111011011011011001111100011"
                    "101011000100010101110011110100001010111010101000000010110010010000100110101101110110000010011100000101110011001010110000010011000010"
                    "111010001111111111111111100110101101010000011000000001101011111001111011100101110111000111000100001111111110111000010110101101001111"
                    "110001110010111101010011101111111001111111101101111011111101100000110001001010001101100011100101111101000011001100100100010000010000"
                    "111011010111001001111111010110010111001011111101100011100111111111100100111100101100101110111011011010010101111111111110110111010001"
                    "100110011010001101001010011000100101000111001101101100000011011110110001110000101001001011100001100011001101101100101111011001111000"
                    "110111010000010101100111100000100101001110011101100000001010011001101111111001011000011111101110111111011010111000011100100011000001"
                    "101110011110000110100011011111110001010100001010100111000000000110101101100001111100001011010101001110010001001100101000001010100010"
                    "101001001001111100100111011001001110111100111111110110000000010011101000110001110100111111011101110010111110011001101000100101111001"
                    "111101000111101011110011011010101111011011101101010100010000110110101001110101011100010011111001111100110101001001010001000001100110"
                    "101101001000101101000111011111101111011010011100010100000000100101101000110001101100011110101111011101111110111110111100001010010011"
                    "111001010001111010100010000101111000000111001000000001100010000110111110101000101110100010101110110010101000101000001101011110001010"
                    "101110010110011111001111010111100111000001111100110000000100000011111001101000000010001111011011100111001010011100110111000000110001"
                    "101101010000000100111011000101110100001110001011001000001111010000110111111010101100101011101011111000000110101001010101000010011110"
                    "101111000000010110100110111111001101101011111010000110001111010111111010100111010001011110101010100010000000011111001111111101001111"
                    "111001111111000001011010110010110001011110001100101101011100010000110100000110001001011010110010101001101111101011000100001000011110"
                    "100011011000110000001111011010001101011100011000001110111111100101111100110111010101001111000010001111010000111001011101111011010111"
                    "101011010110100011011011101001111011001000001011011101010110100100100111010101000011110010011001001110010101001100001100010110010110"
                    "100001010011011010011111001100101110110110011001101111101110101001110100101101001001100110110001000100110110011110001100111101011001"
                    "110000110110011000001010111100100100100111101111111101110000000110100100001000101100100010111100011101011001001100001001110100010010"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "110011100011010000101111101010101010110111011111101101100011110111110111111101010001001111100100011010100010111111010101111111000001"
                    "101110000110111101110010001100001011101000001101100000011111110010111110000000100001010010111111101101100011001010100100010011110110"
                    "101000001011111110011110100000100010111111111111011000010100000011110000001001110000010110101110100110110001011011101001111010000111"
                    "111111001001110000101011111110100010110101001011010110011010011010101101001011110101010010010010001100011000001011000000001000011000"
                    "101110101011100000101110011100001010101101111000110011101010100011110010110100000000011110000010100111100111111101111010011101111001"
                    "111101001101011101110010000100010010011110101011001100100110001010101001110010001000000010100110111111111010001011000101010100001100"
                    "111100001001011001111111110110000010111100111000010000011010100111101100100111011110111110000010101100100101111100101100100001111111"
                    "110011001101011001010010101111101011001010001110001010001110100110111101010100100011111011011101100110010001001011011010000000101000"
                    "100111010001011110111110010001111110101111011011100110101111101011110110000100111101001110111110011010111111011001000010011111111001"
                    "101010000000011011111010011001110101111111001101000100000101010000110000010010000000110010011000110111000111001011111001000101110100"
                    "100001110011111011100111111010101110011011111110110101010101110111100011100001000101110110001000100000000110011011001100110101000111"
                    "100111101110011111100011010110011000000010101011001101110010011010100101010100101010101010000010111001000000101110010011010001111100"
                    "101001101011101100100110011110100000101011111100000110100001101001110110110000111100000111110000011010110001011111010110011110000111"
                    "110000010011100011110011111011011110111000001111101101111111001110110110100011010010000010100010000010011000001001001000001001100000"
                    "110100001110000001010111101111011010110001011111000011110000111001101000111110100001110110110000111001011010111101010001000110010001"
                    "110000001101011010101011110101010111010111101100000011010011110010111011001001011010111011110010010010101000001010001000010100000100"
                    "110110000000101110111111101001010110001011011110000100000101000111111101100000110110100111010100101101101001011011011110101110000001"
                    "100110001101000100111010011000100100101010001101101100010101100010110011001001001110010010100110111101100110101000110001101100010100"
                    "110111001001010100101110110011111011000010111001101010111111100011101111100011011011000110001001100011000110011000111011001111100011"
                    "100111111111000101101011001101101111001000101001100110111100011110101100000101010110000011111110010000101000101011111011011100010110"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "110110110100100110111111101011111100100100111100011011000110010011100001101011001011001110011001100111011011011100011001101001111011"
                    "111111110110110111010010011101010010011111101101101111111001100110111001101100111000100010001111101100100010101111001001101111110100"
                    "100001010011111110110110001010110111100100011110110001001011100011100000100001011001011110011101111011001010011001011110011010110111"
                    "100100110111011001100010100110010100110001001010000101110011010100100111100111000000010010100110101111001111101111000101000010011100"
                    "101010100100010111011110101101110101011011011101101110111100011101110100111111110000001111101110001111010011011011100011111111010001"
                    "111110000110111110011010110111110000010000001101010001011101111010111100100101000000101010011010111011010000101100100000000011101000"
                    "111100010110101011011111100000100111110010011111111011011101001111111001011011010001100110000001101101111100111001011100111000100011"
                    "101111010000110110110011010001110110100000101100001101010101111100101000100010101110011011101111111101100010001101100101100100111100"
                    "111111101100001011101111001011101111011100011110010010000000011101101011011000010011111110011011001010001111111101100101010101101001"
                    "111100010000001010011010010010110000110000001001101011010010011000100110111010111100010010000000011010110111101111001111001001001000"
                    "111101100000101010110110110101100111011010111010111011100111101101100010010111110110000111010100110010001110011001010110110001001111"
                    "110011000011001011000010101010011110110111101110111011010010001000111001011110011111100010101110101000000111101110001011111111000000"
                    "111010000000001110100111101000100010101011111101111100010111111111101100011111001110000111000010011000011111111010011111111101101011"
                    "111000000010110101110011011110010110101111101001100000100100001000110000100000110101011010000001101100001011001011010110110000000100"
                    "100000000111101101000110010100100101001100111010110011110101111001110011110100110111010111101110001001000111111111111100000111110011"
                    "111100100010011101110011001111010111010111001110000111000111110010111100000010100111010010011111010101110011101001001000011000100110"
                    "111111101010111110011110010111100101010001011011011100011100001001110010011011100000000110110001000110101010111011000001000001100011"
                    "101101011011010101000010001000010010011111001010101011100011100010100111011101100101110010011100001110011111101110011100011010110000"
                    "101111111111101110100111110101011100010000011000010101010001111111110011101101001001001110111101011001111010011010100011101010111001"
                    "110101010100010111111010100000000000110011101101010011010100111010111011101111011111110010101110001010101110001010111111010011111110"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                },
        /*177*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^;<>@[]_`~!||()?{}'ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^;<>@[]_`~!||()?{}'\001\002\003\004\005\006...............\015\015\015\015\015\015\015\015abcdefghijklmnopqrstuvwxyz\015\015\015\015\015\015\015\015...............\001\002\003\004\005\006ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^...............", -1, 0, 120, 120, 0, "Mixed modes (except B256); BWIPP uses different encodation", 13,
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "101001101001010011011001100011111001011110010010011011010111111101001110101111111010000001101101101110110111111011101011"
                    "101100101010110110001101000010100000111010101111010101000000110001110101001100101001111100111000101011100101101011001100"
                    "100110010000100111011110100001010100101111101101000101100101111110010011101101111011110110111011011111100100110111001001"
                    "101010011110000110101010001010010010000011011101000011110000101111000000101100001110011000111110001011110000100011011000"
                    "100001101000001110111101000000101011010110010111110010001111101101100000001001011101110101000000000111011110101001101111"
                    "110000101111011110001110000001101111000011000110001001101000101010101100101010101111010111101100101010110011111101011010"
                    "111101011101000110011000111101011100101110101110101100110101110111101111010000111111011101101100111110100000011111111111"
                    "111100100111111111101010110000000110111011001100111101110110100011011000111110101100111001110100010010011110110100000100"
                    "110110011010110000111000010000000010111111101101111001011111101111111001101001111100010001110101011110110111000101010111"
                    "101111110100111101101110011100110111001010000000100000111110111011010000101000101000100111011100000011110111101000010000"
                    "101011101110100100011000101100100010010110100000000110010001111011101101011011111011011010010010010111101101100001011001"
                    "101011000011010111001111110110000100101011010011110111111010100010100100111110101110001011011010100010111100000110111110"
                    "110110000001101101011001110100001000100110110011010110011101110111010110000110111110111001001100000111111011010100100101"
                    "100010110001001100001011011111001110100011011001101110011100111001101111101000101111110011011111010011100101000010101010"
                    "101110110111001001011010011111011111111111010111111000110111111001001001110101011001111010011010000111011001001011111101"
                    "111101111011011010001011011101101101010011111101101011001110111110101101110111101011111000001001000010111000101100010010"
                    "111111101001100101111001111001110100100111111100111001100101100010111011011101111011100001010110011110100001011101000011"
                    "101101101101010111001101010000110000111010110000101001111110110011001110111001101011111101001011011010001110001100000110"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "101101111111010111111111100001011110010111111110111101101001110111011100010001111000110101101111100110100000000010011011"
                    "101100010101001011101101111110100100100010110011010011110100100001011000100111001110011011010111111010000100011010011110"
                    "111111000101011100111001011000101110101110100001011110011001100000011011011001011101110010111011100111001100001001101101"
                    "111101101010101001101101111011011111111010011100011000010100100100001110001011101111001011001011100011010111011001100000"
                    "110101100101000001111011111000101100011110110111001101101101101001101110111101111110101111011000001111110110000110111111"
                    "100100011111111100101101101001111110100011101000001110110110101100001111101110101100111011010100011011011000000111010000"
                    "111011100100101011111011111110100110010111000111101011110111111101010001101001111101110010010111001110010000011010110101"
                    "100110001100010011101011010111110110101010010100011011010100111101111011001000101000001010111100001011111100011001110110"
                    "111001010100101010011001010110101100000110001000011010011111110111011110101010011000110111010101100111111001101011111011"
                    "111011000100101011101100000000111111100011100111000011110010101110011000111111001010001011100111010011110001101101011000"
                    "100111111010011010011111111001111010011111101111111001110101100100010110000011011010011010001101111111100110111100110001"
                    "101110111010011110101001000101101010001010011111010010110000101001110011101101001100010001111011010011000111010111110010"
                    "101111000001011000111010000001010110100111011001111000111011110111011101111101111011101000011111111110011010110010100101"
                    "110010010000001011001001010001001111101010011101110010101110100011101101110110101000000000101011011010011001110111010010"
                    "110000011111101001111011100101100001000110011011101101101111101010111010100111011101100101011111001111101011111011110011"
                    "111110111100101000001110110011111011111011001011100010001110111101001110111001101100101001011000011011101101010010011110"
                    "101001001000111001011111110010011110101110111000100101010001110100011110011001011111010110110110101111111100111111000001"
                    "101001100111010010001101101011011010010010110101000101111010110001011100111111101001011100110101111011010111000000010010"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "100111101101010000111110101110110101101110010110110111011001100000011110000010111101011101001110101110110010001001011111"
                    "100100011100011111001010110011110000101010111010001111011110100100011111110101001100111110110111100011110100101110011100"
                    "100001010111101011011101110111011010011111011101000110110001111000111010010100011101001010010110101111111010101000110101"
                    "110010111000101011001000010110111010001011100110000011100110101100111101010010101010101001100010110011011101101001100100"
                    "110000101101010010011000000100100001101110100110110111110111111110000110000001011010101101111011110110110100101011111101"
                    "111101111100001011001001000010010001001011111011010111100100111101000101100111101101111101001101011011101010011000100010"
                    "101001110110110011011010001011011110100110011011000011101111100101011100011000011110101010001011001110111010011100100011"
                    "101100101110001101101011001110111000111011000001000111110010101111101001100011101100001001100111011011100111011010001110"
                    "111011001010011101111111011101010110100111010110010001110111110110111101010110111110111011011111100111101000111000010111"
                    "110010100100010101001110001001110011010011011011011110110010110110111010110101101011100100111101000010011011110011010110"
                    "101010111010100111111000000111011101110111111110101100111011100110111001111010111010010100001011001110010010110000111111"
                    "101011101000111100101100100101100001001011010111000010110010110001111011111001101100100001011111101011110000100001000010"
                    "110111110110011100111100100010110111000111111011101101101011111100010010000001111100000001001001010110100111111100111101"
                    "100010011100101101001111110101111010001011001000100010001110110111101101011001001111000111110111110011110111011000101000"
                    "100010011110001111011101010100101110111111000000100101101101101011100001011111111010000011011000111110010001111000000111"
                    "101111111100101101101010110001101011101011001101000100100000110010100100011010101000100011100010100010111110111000110010"
                    "100000111011001001111110000001111011001111101110110111011011111011101111111101011100010010101101000111100101011110011101"
                    "111100111000101100001101101100000000101011011010011011010000101101011110011100001010110101010110101011101001101100100010"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "100110001001100101011100011111010111011111111100101100100001100100101111110110011000011011010001111111010110110010010001"
                    "110101010001001100001001111101011000101011100111011011001110110001110011101100101101011110110110011011011100000100000100"
                    "100101101111100011111101100101111001101111100101001001101101111110101111001000011111111011100110101110000010001010110101"
                    "101110100111011011101001110011010000001011111100100111011110100111001100101111101101100100101100010010111011010101100110"
                    "110111001110001011111001110001101101010110000000110111111011110110000010111001011001111000000000100111100010111100010011"
                    "111001101111011111101100100010100101111011011011110011011110100010001011011101101000101111111111010011100100001010000100"
                    "101001100010010001111011101111010111100111000110111011011111111011010111011101111001000110011001111111011110111011110011"
                    "111111011000011101001010111001101111010010010000100011001100100111100011001100101010100101110110000011100100001001000100"
                    "100001010000101110111110111001100101111111011011010110101001100110000010111011111111100000010111100111110001011110100111"
                    "110011111101000010101000111111110001011010101111010100100100111111000100100101101010000000100101101010100111111010110010"
                    "111101110011101001011101000101100110111110101101111011101111100011001011011001011100100001111101011111010011100000110011"
                    "101110011000101001001111110011111011110011110011001010010100100101001010111000001000010111000010111010110011000000111110"
                    "101001100000000101111110011110101100010111111001100001011001111101100110100011011000101011000110010111100011101110001101"
                    "101111001101000011101101011001101111100011011110100010010010110110100110100000101110101101010001011011000101000110100010"
                    "101111101101111001011111001101101011111111011000010000111011100100011000110100111010111111111000110110000010111001001001"
                    "101101011011111100001100001100010001101010110100011100111010110100011111101001001111100011001000101010111001000111011010"
                    "111111110101011001111111101110111011111111100111101010011001110110111000110100011100010111010111101111111110110110000111"
                    "101011100110111110101100111100001001111010101110100101010110101110110111111000101101101011000110100010000010011010010100"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "111011011111101010111000100000001111001111010101011110011101110110000110110110111101001111011001100111110101100111101011"
                    "100010010110011010001101110011001110001010011100011010011000111101111011000011101000111001111111011011000010001101110010"
                    "110110010011011110111101111011111111100111110110111001100111100101010110111100011100000011001101010111001100001000001001"
                    "111111000011000001101100110100110100100011001100101001100100101111111101101000001101001000101110001010011111111101110100"
                    "110101111011100101111011111100001000100111001010100110100111110101110000111010011011011010110110011110000001011000000011"
                    "101011001111001001001010111000100111110010100000100111110010111110010001110101101111011110011100100011011001001111000110"
                    "111010001001000010111110111101101101001110111010011001011011101010011100000001111010000101111010011110101100100011001101"
                    "110001011101110010101000011110011010101011010100011010110000101000001011100000001010000111101101001010101110010111001100"
                    "111011011110101001011101011001011111011111000001101111101011100010101011100001111001011010011100011111000011110010010111"
                    "100111001010101001001101111010000100000011000001000100001100100101111011101110101001101110110001011010101111000001100010"
                    "110110110101000101111110010000010000001111100110010101100111101001001101101001111000100011011101101110111000110011010011"
                    "100100110001000011001100011111000100011010000111000100011010110011111100011100101000100101111010110010100111001010111000"
                    "101011011001111001111001101000101100111110011000110111011011110101001001000010111110000111100101010111101111111010010111"
                    "101010111011111101001010100110110100110010010110111000110000110110010000100010101001000101001101101010011110000000101000"
                    "100100100101011011111101010011110110100111100010101100100101110110110010011100111100100000011110000110111000000011101001"
                    "100010100110110011101100110111010000000011110111010011101110111001100000010111001110101011000100000010101111001001100010"
                    "101011011111101001011110100010010001011110100101000011001001101001010100111110111000000110011010100111110000000100000001"
                    "101110010110010100101110000011000100111010000100010100001110111111011010111101001001001111100010001011010011101100010000"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "111110010011111100111000011001000111110110000000000001110101111001001011110101011111111110100100010111010000100001010011"
                    "101111000001101010101111010100011101111011011011111001111110111011101101010010101100000011100100110011011000010110101010"
                    "100101111011101011111111011100010011110111000111011100101011110100011100110110011110011100011101100110110100011100100111"
                    "101011000010101101101100110010011110011010010010100101111110111001101101000111101100110011100101001011000010110011110010"
                    "111010001011011111111111110001001001111111011000100010100001101111100111101011011000101110110100100110101010001110101101"
                    "110001000010000101001010001011011111011010001011000010100100110110111000000100101011010010110000100010000000010111110000"
                    "111011101000001111011111000111101110111110100000110110111011111001110010101111111010001011001100111111001100011011110001"
                    "100101010100000111001100110101101101110011000111111101001110100100000011011100001100011000100111111010001001111100111010"
                    "110100001100100110111100010011011100010111011010110000011111111000010011100111111101011001010000010111001011110101010111"
                    "100100010100111100101111111110001111110010101010100000101100110000101001000001101000100101101011110010001000010111101100"
                    "101111110110111101111101001010101011111110100011101000010001111011111101010101011100100101111010111110001100001010001001"
                    "111001011110111110001001011111110000011011101010011000010100111010101000001010101110001001101101000010100011011010011010"
                    "111010011101001011111111100000111000010111011100111100101001101100111100001110011010100100111101101111011110101000010001"
                    "110011110000101101001101111010001110110010000001111001111100110101011111011100101101011011000011010010001011010000000010"
                    "111001101010101011011011011000000000110111011110100100111011110111110111101000011110100011010010100111110011000110100001"
                    "110000010101100111101111011011001101110010001010011010000000111001011110011110001111010000010000101010010111110110100000"
                    "111011111110101110111010100011001101100111110010011001001011111110111101011101011010010111111011000110110100001011011011"
                    "100101110101110001001111111010101000000010110011011101000010101010011100100101101101011100100110001010000010100000111110"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                },
        /*178*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 0, 144, 144, 1, "144 x 144 default (de facto) format", 0,
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000111111011110111000111101111"
                    "101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011110110111011100011110111101110"
                    "100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000111"
                    "100111101110001111011110101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011111010"
                    "100111000111101111011101100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011111111"
                    "100011110111101110001110110111101110001111011110101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011011110"
                    "111011110111000111101111110111000111101111011101100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001011111"
                    "111011100011110111101110100011110111101110001110110111101110001111011110101110001111011110111000101111011110111000111100111110111000111110001100"
                    "110001111011110111000111111011110111000111101111110111000111101111011101100111101111011100011111101111011100011110111101111100011110111111110111"
                    "111101111011100011110110111011100011110111101110100011110111101110001110110111101110001111011110101110001111011110111000101111011110110111111110"
                    "111101110001111011110111110001111011110111000111111011110111000111101111110111000111101111011101100111101111011100011111101111011100010110111111"
                    "111000111101111011100010111101111011100011110110111011100011110111101110100011110111101110001110110111101110001111011110101110001111100010110110"
                    "111110111101110001111011111101110001111011110111110001111011110111000111111011110111000111101111110111000111101111011101100111101111111100010111"
                    "111110111000111101111010111000111101111011100010111101111011100011110110111011100011110111101110100011110111101110001110110111101101111111100010"
                    "111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000111111011110111000111101111110111000101101111111011"
                    "101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011110110111011100011110111101110100011111000101101111010"
                    "101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000111111011111111000101101001"
                    "101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011110110111011011111111000101100"
                    "100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011110111110001011011111111000101"
                    "110111101110001111011110101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011100010111110001011011111111000"
                    "110111000111101111011101100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001111011111111110001011011111111"
                    "100011110111101110001110110111101110001111011110101110001111011110111000101111011110111000111100111110111000111101111010110111111110001011011110"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "111011110111000111101111110111000111101111011101100111101111011100011111101111011100011110111101111100011110111101110001110110111111110001011101"
                    "111011100011110111101110100011110111101110001110110111101110001111011110101110001111011110111000101111011110111000111110100010110111111110001110"
                    "110001111011110111000111111011110111000111101111110111000111101111011101100111101111011100011111101111011100011110111111111100010110111111110111"
                    "111101111011100011110110111011100011110111101110100011110111101110001110110111101110001111011110101110001111011110110110111111100010110111111110"
                    "111101110001111011110111110001111011110111000111111011110111000111101111110111000111101111011101100111101111011100010111101111111100010110111111"
                    "111000111101111011100010111101111011100011110110111011100011110111101110100011110111101110001110110111101110001111100010101101111111100010110110"
                    "111110111101110001111011111101110001111011110111110001111011110111000111111011110111000111101111110111000111101111111101100101101111111100010111"
                    "111110111000111101111010111000111101111011100010111101111011100011110110111011100011110111101110100011110111101101111110111000101101111111100010"
                    "111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000111111011110111000101101111111111000101101111111011"
                    "101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011110110111011100011111000101100111111111000101101111010"
                    "101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011110111110001111011111111000101111011111111000101110001"
                    "101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011100010111101111011011111111000101011011111111000100000"
                    "100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001111011111101110001011011111111110001011011111111100001"
                    "110111101110001111011110101110001111011110111000101111011110111000111100111110111000111101111010111000111110001011011110111110001011011110011000"
                    "110111000111101111011101100111101111011100011111101111011100011110111101111100011110111101110001111110111111110001011011111111110001011100001101"
                    "100011110111101110001110110111101110001111011110101110001111011110111000101111011110111000111100111110110111111110001010110111111110001111111110"
                    "111011110111000111101111110111000111101111011101100111101111011100011111101111011100011110111101111100010110111111110001110110111111100011100101"
                    "111011100011110111101110100011110111101110001110110111101110001111011110101110001111011110111000101111100010110111111110100010110111111111100110"
                    "110001111011110111000111111011110111000111101111110111000111101111011101100111101111011100011111101111111100010110111111111100010110011001000111"
                    "111101111011100011110110111011100011110111101110100011110111101110001110110111101110001111011110101101111111100010110110111111100011101101101110"
                    "111101110001111011110111110001111011110111000111111011110111000111101111110111000111101111011101100101101111111100010111101111111001011000101011"
                    "111000111101111011100010111101111011100011110110111011100011110111101110100011110111101110001110111000101101111111100010101101111111111000110110"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "111110111101110001111011111101110001111011110111110001111011110111000111111011110111000111101111111111000101101111111101100101100110010111100111"
                    "111110111000111101111010111000111101111011100010111101111011100011110110111011100011110111101100111111111000101101111110111000111011011010001100"
                    "111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000101111011111111000101101111111110010110010001011011"
                    "101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011111000101011011111111000101100111111111110001001010010"
                    "101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011111111110001011011111111000101111001100101110101110001"
                    "101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011011110111110001011011111111000101110000110000011010100"
                    "100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001011011111111110001011011111111101101001111000111110001"
                    "110111101110001111011110101110001111011110111000101111011110111000111100111110111000111110001010110111111110001011011110110011101100110100101000"
                    "110111000111101111011101100111101111011100011111101111011100011110111101111100011110111111110001110110111111110001011011111100010010111011001001"
                    "100011110111101110001110110111101110001111011110101110001111011110111000101111011110110111111110100010110111111110001100111010010111011000110010"
                    "111011110111000111101111110111000111101111011101100111101111011100011111101111011100010110111111111100010110111111110101100101011111111010101101"
                    "111011100011110111101110100011110111101110001110110111101110001111011110101110001111100010110110111111100010110111100010101010010010001001111110"
                    "110001111011110111000111111011110111000111101111110111000111101111011101100111101111111100010111101111111100010111100101111101101110110011010111"
                    "111101111011100011110110111011100011110111101110100011110111101110001110110111101101111111100010101101111111100011011100101101001000001101101010"
                    "111101110001111011110111110001111011110111000111111011110111000111101111110111000101101111111101100101101111111001110111110111011110101101101011"
                    "111000111101111011100010111101111011100011110110111011100011110111101110100011111000101101111110111000101101111110111100100111001010010111101110"
                    "111110111101110001111011111101110001111011110111110001111011110111000111111011111111000101101111111111000101100011010101111000001100101101011101"
                    "111110111000111101111010111000111101111011100010111101111011100011110110111011011111111000101100111111111000101111010100111110000011111010101010"
                    "111100011110111101110001111110111101110001111011111101110001111011110111110001011011111111000101111011111110001001110111111001001011011010010011"
                    "101111011110111000111100111110111000111101111010111000111101111011100010111110001011011111111000101011011111101111000110110010010101111001010010"
                    "101111011100011110111101111100011110111101110001111110111101110001111011111111110001011011111111110001011000110100011001100000011010110101000001"
                    "101110001111011110111000101111011110111000111100111110111000111101111010110111111110001011011110111110001011110101101110100011111111110011011000"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "100111101111011100011111101111011100011110111101111100011110111101110001110110111111110001011011111111100010011001111001100111110000100000110001"
                    "110111101110001111011110101110001111011110111000101111011110111000111110100010110111111110001010110111111011110010110010110001011011000001000100"
                    "110111000111101111011101100111101111011100011111101111011100011110111111111100010110111111110001110110001101011110000001111010110101000001101001"
                    "100011110111101110001110110111101110001111011110101110001111011110110110111111100010110111111110100010111101010011100110110011101001000110001100"
                    "111011110111000111101111110111000111101111011101100111101111011100010111101111111100010110111111111010100011110010011111100001111011001100110101"
                    "111011100011110111101110100011110111101110001110110111101110001111100010101101111111100010110110111101101101010000101110100010010001001001100110"
                    "110001111011110111000111111011110111000111101111110111000111101111111101100101101111111100010111101000011000010101111111111110101110000110011111"
                    "111101111011100011110110111011100011110111101110100011110111101101111110111000101101111111100010100011111100110101100110111001011100000011010010"
                    "111101110001111011110111110001111011110111000111111011110111000101101111111111000101101111111001100101100111110110010111100111001110111000010011"
                    "111000111101111011100010111101111011100011110110111011100011111000101100111111111000101101111110110101010001011011100010110000100101010010100010"
                    "111110111101110001111011111101110001111011110111110001111011111111000101111011111111000101100101100100010110011111110111111011101000100111100101"
                    "111110111000111101111010111000111101111011100010111101111011011111111000101011011111111000110000111011001011111000010000110111001001101100111100"
                    "111100011110111101110001111110111101110001111011111101110001011011111111110001011011111110100001110001110001100100000111110011101111100100100011"
                    "101111011110111000111100111110111000111101111010111000111110001011011110111110001011011111001110111100001110111101110000101001100001001010100010"
                    "101111011100011110111101111100011110111101110001111110111111110001011011111111110001011110110111100101110111111011111011111000010010011001111001"
                    "101110001111011110111000101111011110111000111100111110110111111110001010110111111110001100010000110011110110000000010110110010100110110000110100"
                    "100111101111011100011111101111011100011110111101111100010110111111110001110110111111101110010001111101000001000001110011111011011111110000000101"
                    "110111101110001111011110101110001111011110111000101111100010110111111110100010110111110011111100100011000111011100011000111011111101111000011000"
                    "110111000111101111011101100111101111011100011111101111111100010110111111111100010111101101111101111100010010100110101001100110100101101100100011"
                    "100011110111101110001110110111101110001111011110101101111111100010110110111111100011000100010110111101000000011000110010101011001010000001100010"
                    "111011110111000111101111110111000111101111011101100101101111111100010111101111111011100101101001110100100001100101111011110110000110100001010101"
                    "111011100011110111101110100011110111101110001110111000101101111111100010101101111100111111010010110000110110110000011010111111001110100101001110"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "110001111011110111000111111011110111000111101111111111000101101111111101100101111011011000001101111010101000001010000111101011110111001010100111"
                    "111101111011100011110110111011100011110111101100111111111000101101111110111000110000000000111100100000000101100100101010110100100100010000010110"
                    "111101110001111011110111110001111011110111000101111011111111000101101111111110011011000100010101110001000111000010110111100110011000011001111011"
                    "111000111101111011100010111101111011100011111000101011011111111000101100111110000001011000110010111000000000001010111110111011111001110111001110"
                    "111110111101110001111011111101110001111011111111110001011011111111000101111101110100000111010101110111101001010001101011110001110010011010101001"
                    "111110111000111101111010111000111101111011011110111110001011011111111000101000001010111001010110111111001011111001110100101001000100010110000100"
                    "111100011110111101110001111110111101110001011011111111110001011011111111101010000101110011011001111011111110001100100111100110000110010100101011"
                    "101111011110111000111100111110111000111110001010110111111110001011011110111100101001001100110000110011001000011100011010111110001101111111110010"
                    "101111011100011110111101111100011110111111110001110110111111110001011011100000000100110101010011111000010110111010110001111100010110100010001001"
                    "101110001111011110111000101111011110110111111110100010110111111110001000100010101110111101100010110010111000010101001000110000000101110001001100"
                    "100111101111011100011111101111011100010110111111111100010110111111110011100001011111110110011011111111101011110001100111101010100100011011110101"
                    "110111101110001111011110101110001111100010110110111111100010110111111100100000011100101101000010110010000010111110111110111111011010011101101000"
                    "110111000111101111011101100111101111111100010111101111111100010110111101100001000101010100111001100100000110100100100111100001100100010000100011"
                    "100011110111101110001110110111101101111111100010101101111111100010110000100011010111011001110010101110100100000110101010110001001011011001101000"
                    "111011110111000111101111110111000101101111111101100101101111111100010001110111011001100110111111111010010000100101101001111111100101111010111101"
                    "111011100011110111101110100011111000101101111110111000101101111111000000100111101110010000110110100000101100110001011100111011011010100101101110"
                    "110001111011110111000111111011111111000101101111111111000101101111000101110001000010001010001111100001011000110011101101110001010001101010101111"
                    "111101111011100011110110111011011111111000101100111111111000101100000000100101100010011110110000101001001001000000011000111010101011101010010010"
                    "111101110001111011110111110001011011111111000101111011111111000100000111110110100101111010101011100100000111001011001101111001001111001111011011"
                    "111000111101111011100010111110001011011111111000101011011111110000001010111010110100000111101000101011001011101111100100110110101001011010010010"
                    "111110111101110001111011111111110001011011111111110001011011110000110001110110000010010101000001110110110000111011011001110101011010101011101101"
                    "111110111000111101111010110111111110001011011110111110001011000000100100111101000111001000101000110010001111000110000010101010011010100011011000"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "111100011110111101110001110110111111110001011011111111110001010001000111111101111100110000100101100000010010110011111001110111110011101011100011"
                    "101111011110111000111110100010110111111110001010110111111101001001101010111010111111000110011010110010110111111001110110101000010110100100100010"
                    "101111011100011110111111111100010110111111110001110110111101101110110101111100000111110001000111101001001110110101010101110110101010111101010001"
                    "101110001111011110110110111111100010110111111110100010110101101110101000110100101001100011010010100011110001100011011010100110111000110100100000"
                    "100111101111011100010111101111111100010110111111111100010010101110111111111011100010011100010001100100110100111010100111101100101010110001110101"
                    "110111101110001111100010101101111111100010110110111111101110010101011000101000100011000110000010101101111010011010101000101001011001000010111000"
                    "110111000111101111111101100101101111111100010111101111010011110111011101111001111101011010001001110010010101010110010011100000101111010001010011"
                    "100011110111101101111110111000101101111111100010101101010011011000010100110010011001011100000010111100001000100010100100110010001101001101001000"
                    "111011110111000101101111111111000101101111111101100110001011001111111101111000100101110100000101101100101111100001100111101110101100001111010101"
                    "111011100011111000101100111111111000101101111110111001100100110110001000101000110010111010101100111110100111011110011010111100010000101100000110"
                    "110001111011111111000101111011111111000101101111111110111110010111000001111111010000101000010011110000010110011101100111100001110101011101111111"
                    "111101111011011111111000101011011111111000101100111100110000000101010010100110000001000010100000100100001100001100100100101101010011000000110110"
                    "111101110001011011111111110001011011111111000111100010111101111111011001101001000101001000111011111111000101110101010001100111000011110101011111"
                    "111000111110001011011110111110001011011111111000111001000101010010001010101100100110100001111000100111010100011100101100111110001011000001001010"
                    "111110111111110001011011111111110001011011111111100111100101110001111111100100001010100001100001100111110001101110011011101110110111011011011001"
                    "111110110111111110001010110111111110001011011100111000000010010101110010110000010001000000110110111011001101000101001010110110110000001011111010"
                    "111100010110111111110001110110111111110001100001100011001111110000001101110001010001001100101101110001011101000100010001110010111101000101101011"
                    "101111100010110111111110100010110111111110011100100001011000100111101010101001101000000100000000111101011011001011001100101000110000001010111010"
                    "101111111100010110111111111100010110111111101101101001010000000111000101100010101001101000000111111100001011100111011111111101110111011110001001"
                    "101101111111100010110110111111100010110111011100100001000101001010110000100100010101011101000100110010001001010010111110110100000011111110110100"
                    "100101110111111010010111110111111010010101011111110011101101010001001001110101000000001011101101110110001001000100010001101111001101001010101101"
                    "110100111101111011100010111101111011100101110100110011101011110111101110101011110100001110100110110111001110110111011100101110100111011110111000"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                },
        /*179*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, -1, DM_ISO_144, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 0, 144, 144, 0, "144 x 144 ISO format; BWIPP not implemented yet TODO: implement", 0,
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000111111011110111000111101111"
                    "101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011110110111011100011110111101110"
                    "100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000111"
                    "100111101110001111011110101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011111010"
                    "100111000111101111011101100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011111111"
                    "100011110111101110001110110111101110001111011110101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011011110"
                    "111011110111000111101111110111000111101111011101100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001011111"
                    "111011100011110111101110100011110111101110001110110111101110001111011110101110001111011110111000101111011110111000111100111110111000111110001100"
                    "110001111011110111000111111011110111000111101111110111000111101111011101100111101111011100011111101111011100011110111101111100011110111111110111"
                    "111101111011100011110110111011100011110111101110100011110111101110001110110111101110001111011110101110001111011110111000101111011110110111111110"
                    "111101110001111011110111110001111011110111000111111011110111000111101111110111000111101111011101100111101111011100011111101111011100010110111111"
                    "111000111101111011100010111101111011100011110110111011100011110111101110100011110111101110001110110111101110001111011110101110001111100010110110"
                    "111110111101110001111011111101110001111011110111110001111011110111000111111011110111000111101111110111000111101111011101100111101111111100010111"
                    "111110111000111101111010111000111101111011100010111101111011100011110110111011100011110111101110100011110111101110001110110111101101111111100010"
                    "111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000111111011110111000111101111110111000101101111111011"
                    "101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011110110111011100011110111101110100011111000101101111010"
                    "101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000111111011111111000101101001"
                    "101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011110110111011011111111000101100"
                    "100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011110111110001011011111111000101"
                    "110111101110001111011110101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011100010111110001011011111111000"
                    "110111000111101111011101100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001111011111111110001011011111111"
                    "100011110111101110001110110111101110001111011110101110001111011110111000101111011110111000111100111110111000111101111010110111111110001011011110"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "111011110111000111101111110111000111101111011101100111101111011100011111101111011100011110111101111100011110111101110001110110111111110001011101"
                    "111011100011110111101110100011110111101110001110110111101110001111011110101110001111011110111000101111011110111000111110100010110111111110001110"
                    "110001111011110111000111111011110111000111101111110111000111101111011101100111101111011100011111101111011100011110111111111100010110111111110111"
                    "111101111011100011110110111011100011110111101110100011110111101110001110110111101110001111011110101110001111011110110110111111100010110111111110"
                    "111101110001111011110111110001111011110111000111111011110111000111101111110111000111101111011101100111101111011100010111101111111100010110111111"
                    "111000111101111011100010111101111011100011110110111011100011110111101110100011110111101110001110110111101110001111100010101101111111100010110110"
                    "111110111101110001111011111101110001111011110111110001111011110111000111111011110111000111101111110111000111101111111101100101101111111100010111"
                    "111110111000111101111010111000111101111011100010111101111011100011110110111011100011110111101110100011110111101101111110111000101101111111100010"
                    "111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000111111011110111000101101111111111000101101111111011"
                    "101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011110110111011100011111000101100111111111000101101111010"
                    "101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011110111110001111011111111000101111011111111000101111001"
                    "101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011100010111101111011011111111000101011011111111000110000"
                    "100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001111011111101110001011011111111110001011011111111010001"
                    "110111101110001111011110101110001111011110111000101111011110111000111100111110111000111101111010111000111110001011011110111110001011011111101100"
                    "110111000111101111011101100111101111011100011111101111011100011110111101111100011110111101110001111110111111110001011011111111110001011011110101"
                    "100011110111101110001110110111101110001111011110101110001111011110111000101111011110111000111100111110110111111110001010110111111110001101110110"
                    "111011110111000111101111110111000111101111011101100111101111011100011111101111011100011110111101111100010110111111110001110110111111110100100101"
                    "111011100011110111101110100011110111101110001110110111101110001111011110101110001111011110111000101111100010110111111110100010110111100011100110"
                    "110001111011110111000111111011110111000111101111110111000111101111011101100111101111011100011111101111111100010110111111111100010111000101000111"
                    "111101111011100011110110111011100011110111101110100011110111101110001110110111101110001111011110101101111111100010110110111111100011111101101110"
                    "111101110001111011110111110001111011110111000111111011110111000111101111110111000111101111011101100101101111111100010111101111111000111000101011"
                    "111000111101111011100010111101111011100011110110111011100011110111101110100011110111101110001110111000101101111111100010101101111111111000110110"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "111110111101110001111011111101110001111011110111110001111011110111000111111011110111000111101111111111000101101111111101100101100110010001100111"
                    "111110111000111101111010111000111101111011100010111101111011100011110110111011100011110111101100111111111000101101111110111000111000011011001100"
                    "111100011110111101110001111110111101110001111011111101110001111011110111110001111011110111000101111011111111000101101111111110010100101010011011"
                    "101111011110111000111100111110111000111101111010111000111101111011100010111101111011100011111000101011011111111000101100111111111110101111010010"
                    "101111011100011110111101111100011110111101110001111110111101110001111011111101110001111011111111110001011011111111000101111001100011111011100001"
                    "101110001111011110111000101111011110111000111100111110111000111101111010111000111101111011011110111110001011011111111000101110100000101101101100"
                    "100111101111011100011111101111011100011110111101111100011110111101110001111110111101110001011011111111110001011011111111100101000000010000011101"
                    "110111101110001111011110101110001111011110111000101111011110111000111100111110111000111110001010110111111110001011011110111110000010101001000100"
                    "110111000111101111011101100111101111011100011111101111011100011110111101111100011110111111110001110110111111110001011001111001111110010101101101"
                    "100011110111101110001110110111101110001111011110101110001111011110111000101111011110110111111110100010110111111110001110101100110000101101110110"
                    "111011110111000111101111110111000111101111011101100111101111011100011111101111011100010110111111111100010110111111101101110111110001111100110101"
                    "111011100011110111101110100011110111101110001110110111101110001111011110101110001111100010110110111111100010110111110010111101001100101001011110"
                    "110001111011110111000111111011110111000111101111110111000111101111011101100111101111111100010111101111111100010110111101110100100000110010110111"
                    "111101111011100011110110111011100011110111101110100011110111101110001110110111101101111111100010101101111111100010001100110101111000001111101010"
                    "111101110001111011110111110001111011110111000111111011110111000111101111110111000101101111111101100101101111111000010111110111100110101101101011"
                    "111000111101111011100010111101111011100011110110111011100011110111101110100011111000101101111110111000101101111111111100100111001010010111100110"
                    "111110111101110001111011111101110001111011110111110001111011110111000111111011111111000101101111111111000101111100010101111000001100101101010101"
                    "111110111000111101111010111000111101111011100010111101111011100011110110111011011111111000101100111111111000110111010100111110000011111010101100"
                    "111100011110111101110001111110111101110001111011111101110001111011110111110001011011111111000101111011111110011101110111111001101011011010010011"
                    "101111011110111000111100111110111000111101111010111000111101111011100010111110001011011111111000101011011111101111000110110010100101111000110010"
                    "101111011100011110111101111100011110111101110001111110111101110001111011111111110001011011111111110001011000110101011001100011001011010101100001"
                    "101110001111011110111000101111011110111000111100111110111000111101111010110111111110001011011110111110001011110101011110100000001110100110111000"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "100111101111011100011111101111011100011110111101111100011110111101110001110110111111110001011011111111100010001101111001110010000110101100110001"
                    "110111101110001111011110101110001111011110111000101111011110111000111110100010110111111110001010110111111011110001000010100110001110000101000100"
                    "110111000111101111011101100111101111011100011111101111011100011110111111111100010110111111110001110110001100000110010001100110001101111001101001"
                    "100011110111101110001110110111101110001111011110101110001111011110110110111111100010110111111110100010111010011011010100101100111101110110001100"
                    "111011110111000111101111110111000111101111011101100111101111011100010111101111111100010110111111111000100101111111110011101011101000101100100101"
                    "111011100011110111101110100011110111101110001110110111101110001111100010101101111111100010110110111110011011010010000010101110110010010011001110"
                    "110001111011110111000111111011110111000111101111110111000111101111111101100101101111111100010111100011010000001110001011110000001110011010111111"
                    "111101111011100011110110111011100011110111101110100011110111101101111110111000101101111111100010101101101000101001110000110101011101100000010010"
                    "111101110001111011110111110001111011110111000111111011110111000101101111111111000101101111111011101001011100000111100001111111001011000000010011"
                    "111000111101111011100010111101111011100011110110111011100011111000101100111111111000101101111100111111110100111011110110100100100010000110101010"
                    "111110111101110001111011111101110001111011110111110001111011111111000101111011111111000101110001100110000111011111101111101011110001001111100111"
                    "111110111000111101111010111000111101111011100010111101111011011111111000101011011111111000100000110001001111011000000000110111011010011100000010"
                    "111100011110111101110001111110111101110001111011111101110001011011111111110001011011111110101001110001110101100100000111110010001101100100010011"
                    "101111011110111000111100111110111000111101111010111000111110001011011110111110001011011111111110111100001100111101110000111000011111101011001010"
                    "101111011100011110111101111100011110111101110001111110111111110001011011111111110001011000000111100101110111111011111011101010111011111010110001"
                    "101110001111011110111000101111011110111000111100111110110111111110001010110111111110001100010000110011110110000000010010110010101101001010100000"
                    "100111101111011100011111101111011100011110111101111100010110111111110001110110111111101000010001111101011001000001110001111011011001001010101001"
                    "110111101110001111011110101110001111011110111000101111100010110111111110100010110111110011111100100011001111011100000000111011111010111011110100"
                    "110111000111101111011101100111101111011100011111101111111100010110111111111100010111101101100101111101111110111110000001100110100110011110001011"
                    "100011110111101110001110110111101110001111011110101101111111100010110110111111100011000000010010111101100000000110000010101011001110010101000000"
                    "111011110111000111101111110111000111101111011101100101101111111100010111101111111011101100011101110000010100011100011011110110111101100001100101"
                    "111011100011110111101110100011110111101110001110111000101101111111100010101101111100100101000010110001100001000000111010111110111111100111011110"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "110001111011110111000111111011110111000111101111111111000101101111111101100101111011011111011101100100111001100100000111101000110111001001100111"
                    "111101111011100011110110111011100011110111101100111111111000101101111110111000110011111101110100100001111110101110100110110001100100010001010110"
                    "111101110001111011110111110001111011110111000101111011111111000101101111111110111001100010110011100101111001111110110011100111011000011001111011"
                    "111000111101111011100010111101111011100011111000101011011111111000101100111111000001001001011010110000100100100110010000110011111001110111001110"
                    "111110111101110001111011111101110001111011111111110001011011111111000101111110110000110010011001100010010101000001100011111101110010011000101001"
                    "111110111000111101111010111000111101111011011110111110001011011111111000101100001010101011100010100011001011101001101100101001000100010101000100"
                    "111100011110111101110001111110111101110001011011111111110001011011111111100110000001001111010001111000111110101111100111100110000110011110101011"
                    "101111011110111000111100111110111000111110001010110111111110001011011110100000000110110010110100100000001000101001011010111110011101110001110010"
                    "101111011100011110111101111100011110111111110001110110111111110001011011111100000001010101010011111010010000011010110011100100100110101001010001"
                    "101110001111011110111000101111011110110111111110100010110111111110001000100000001101011101100110110010111011010001000010101001000101100101111100"
                    "100111101111011100011111101111011100010110111111111100010110111111110011100001011101110110011011111111111100000001100001100001111011001111010101"
                    "110111101110001111011110101110001111100010110110111111100010110111111100100000011110101101000010110110000110110110111000100010101010100010010100"
                    "110111000111101111011101100111101111111100010111101111111100010110111101100001000101010100111001101111111001101100101001100100110110111011001011"
                    "100011110111101110001110110111101101111111100010101101111111100010110000100011010111011001110000100000000100110000000000101000100010000101101000"
                    "111011110111000111101111110111000101101111111101100101101111111100010001110111011101100110111101111010110000011000110011100101000000100110111101"
                    "111011100011110111101110100011111000101101111110111000101101111111000000100111101011010000110000100000101101101110111010100010101010111001101110"
                    "110001111011110111000111111011111111000101101111111111000101101111000001110001010101001110001011100001011001000011100001111001000100000110101111"
                    "111101111011100011110110111011011111111000101100111111111000101100000010110101110100011100100100101001001001111100111000100010101000011010010010"
                    "111101110001111011110111110001011011111111000101111011111111000101010111110110011011101111000011100100001001001010011111111001011110101110010011"
                    "111000111101111011100010111110001011011111111000101011011111110000100110101011101011101100011000101011000011011111100110110110100001011011110010"
                    "111110111101110001111011111111110001011011111111110001011011110101010011110100100110100011000001110110100000111011010001110101011010101010110101"
                    "111110111000111101111010110111111110001011011110111110001011001010001010101010111111100110001000110011001111000110001010101010011011100011010010"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010"
                    "111100011110111101110001110110111111110001011011111111110001000001110011101111011000101000100101110001000010110011111001110111110010110001000011"
                    "101111011110111000111110100010110111111110001010110111111110101100001100111000010111001100001100100010110111111001110110101000011101100100001010"
                    "101111011100011110111111111100010110111111110001110110111101001100110111110001100111110000011011101100001110110100010101110110101100111001000001"
                    "101110001111011110110110111111100010110111111110100010110101011101111100110101001001100010010100100011110001100010101010100110010100110100101000"
                    "100111101111011100010111101111111100010110111111111100011000011100111101111101010010001001110001100100101100111110010111111100011010110001000001"
                    "110111101110001111100010101101111111100010110110111111100110010011011000101001110011000111111010101101111110011101111000100111101001000010010000"
                    "110111000111101111111101100101101111111100010111101111111011101001011101100001111101010010101001110011101101010101110111101010001111010001000111"
                    "100011110111101101111110111000101101111111100010101101110011000000010100110010011000000100000010100000010000110111110110100100101101001111000010"
                    "111011110111000101101111111111000101101111111101100110001011110111111101111000100001100100000101111101001101101001101101100000101100001110010101"
                    "111011100011111000101100111111111000101101111110111001100100010110001000101010110110011010100000110110011100101010101010101100011000100100000110"
                    "110001111011111111000101111011111111000101101111111110111110010111000001111110010101101010010011101101010010110000000001111011111010010001111111"
                    "111101111011011111111000101011011111111000101100111100110000000101010010101111010101000000000000101110011011111010110100101011011011010000110110"
                    "111101110001011011111111110001011011111111000111100010111101111111011001101011011101000001110101100111011101110000100001101011100011110101011111"
                    "111000111110001011011110111110001011011111111000111001000101100010001010111010100110101000010110110001010101101110101100100100110011000001001010"
                    "111110111111110001011011111111110001011011111111101111100101110000011101110100001010000110110111110111100100111011011001111111010111011111011001"
                    "111110110111111110001010110111111110001011011100101100000001010100101000101000010000100101100100100101001000101001001100111110010000001101111010"
                    "111100010110111111110001110110111111110001010011101111011111110110000101110001010110000000111111101001011001010100000111110000111100110111101011"
                    "101111100010110111111110100010110111111101010000111001010110101010101110101001101010000000001110100101011101001011011110101100111110010001111010"
                    "101111111100010110111111111100010110111101100111111000111100011111000101100010110101101000001001111100001011100110111011100101110110110110100001"
                    "101101111111100010110110111111100010110101111010101100110100111110001000100000010001011101001010110010001001100010010110111100101010111001101000"
                    "100101110111111010010111110111111010010011000011101011001011000001001001110001000000001011101101110110001001011101000111101111000101011110111101"
                    "110100111101111011100010111101111011101111110100110011101001110100001110100011110100001110100110110111001110111111011110101110101111011000111000"
                    "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[8192];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_encode", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci,
                                    -1 /*option_1*/, data[i].option_2, data[i].option_3, data[i].output_options,
                                    data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %s, %d, %s, %d, %s, \"%s\", %d, %s, %d, %d, %d, \"%s\", %d,\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].eci,
                    testUtilOutputOptionsName(data[i].output_options),
                    data[i].option_2, testUtilOption3Name(data[i].symbology, data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment,
                    data[i].expected_diff);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n", i, ret, width, row, data[i].data);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, cmp_buf, sizeof(cmp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, data[i].data, length, debug)) {
                    int cmp_len, ret_len;
                    char modules_dump[144 * 144 + 1];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilZXingCPP(i, symbol, data[i].data, length, modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilZXingCPPCmp(symbol, cmp_msg, cmp_buf, cmp_len, data[i].data, length, NULL /*primary*/, escaped, &ret_len);
                    assert_zero(ret, "i:%d %s testUtilZXingCPPCmp %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                }

#ifdef ZINT_TEST_ENCODING
                if (i && (data[i].input_mode & 0x07) == (data[i - 1].input_mode & 0x07) && !(data[i].input_mode & FAST_MODE) && (data[i - 1].input_mode & FAST_MODE)
                        && data[i].eci == data[i - 1].eci && data[i].option_2 == data[i - 1].option_2
                        && data[i].option_3 == data[i - 1].option_3 && data[i].output_options == data[i - 1].output_options
                        && strcmp(data[i].data, data[i - 1].data) == 0) {
                    unsigned char binary[2][2200];
                    int gs1;
                    int binlen;
                    int binlens[2] = {0};

                    assert_equal(data[i].expected_rows * data[i].expected_width <= data[i - 1].expected_rows * data[i - 1].expected_width, 1,
                        "i:%d data[i].expected_rows * data[i].expected_width %d > data[i - 1].expected_rows * data[i - 1].expected_width %d\n", i,
                        data[i].expected_rows * data[i].expected_width, data[i - 1].expected_rows * data[i - 1].expected_width);

                    binlen = 0;
                    symbol->input_mode = data[i - 1].input_mode;
                    gs1 = (symbol->input_mode & 0x07) != GS1_MODE ? 0 : (symbol->output_options & GS1_GS_SEPARATOR) ? 2 : 1;
                    ret = dm_encode_test(symbol, (unsigned char *) data[i].data, length, symbol->eci, gs1, binary[0], &binlen);
                    assert_zero(ret, "i:%d dm_encode() FAST_MODE ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

                    binlens[0] = binlen;

                    binlen = 0;
                    symbol->input_mode = data[i].input_mode;
                    gs1 = (symbol->input_mode & 0x07) != GS1_MODE ? 0 : (symbol->output_options & GS1_GS_SEPARATOR) ? 2 : 1;
                    ret = dm_encode_test(symbol, (unsigned char *) data[i].data, length, symbol->eci, gs1, binary[1], &binlen);
                    assert_zero(ret, "i:%d dm_encode() minimal ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

                    binlens[1] = binlen;

                    assert_equal(binlens[1] <= binlens[0], 1, "i:%d binlens[1] %d > binlens[0] %d\n", i, binlens[1], binlens[0]);
                    assert_equal(binlens[0], binlens[1] + data[i].expected_diff, "i:%d binlens[0] %d != %d binlens[1] (%d) + expected_diff (%d)\n",
                                i, binlens[0], binlens[1] + data[i].expected_diff, binlens[1], data[i].expected_diff);
                }
#endif
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int output_options;
        int option_2;
        int option_3;
        struct zint_structapp structapp;
        struct zint_seg segs[3];
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        char *expected;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 14, 14, 1, "ISO 16022:2006 11.6 example",
                    "10101010101010"
                    "10000100111111"
                    "11101100000000"
                    "11111010010001"
                    "11000110001000"
                    "11110110011111"
                    "10111101000000"
                    "10010010000111"
                    "10100110111100"
                    "11011111011011"
                    "10101001101110"
                    "10001001101001"
                    "10011111100000"
                    "11111111111111"
                },
        /*  1*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 14, 14, 1, "ISO 16022:2006 11.6 example auto-ECI",
                    "10101010101010"
                    "10000100111111"
                    "11101100000000"
                    "11111010010001"
                    "11000110001000"
                    "11110110011111"
                    "10111101000000"
                    "10010010000111"
                    "10100110111100"
                    "11011111011011"
                    "10101001101110"
                    "10001001101001"
                    "10011111100000"
                    "11111111111111"
                },
        /*  2*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 7 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, 0, 14, 14, 1, "ISO 16022:2006 11.6 example auto-ECI inverted",
                    "10101010101010"
                    "10001111001101"
                    "10011111110110"
                    "10001100000111"
                    "10000011111010"
                    "11000001100101"
                    "11100001111110"
                    "10101101000111"
                    "11101101001110"
                    "11100001000001"
                    "11110100111010"
                    "10010111100111"
                    "10011001010000"
                    "11111111111111"
                },
        /*  3*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("Ж"), -1, 0 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 14, 14, 1, "ISO 16022:2006 11.6 example inverted auto-ECI",
                    "10101010101010"
                    "10001111001101"
                    "10011111110110"
                    "10001100000111"
                    "10000011111010"
                    "11000001100101"
                    "11100001111110"
                    "10101101000111"
                    "11101101001110"
                    "11100001000001"
                    "11110100111010"
                    "10010111100111"
                    "10011001010000"
                    "11111111111111"
                },
        /*  4*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("product:Google Pixel 4a - 128 GB of Storage - Black;price:$439.97"), -1, 3 }, { TU("品名:Google 谷歌 Pixel 4a -128 GB的存储空间-黑色;零售价:￥3149.79"), -1, 29 }, { TU("Produkt:Google Pixel 4a - 128 GB Speicher - Schwarz;Preis:444,90 €"), -1, 17 } }, 0, 52, 52, 0, "AIM ITS/04-023:2022 Annex A example **NOT SAME** example corrupt??; BWIPP different encodation",
                    "1010101010101010101010101010101010101010101010101010"
                    "1001111110000011100010110111011001110111111001111011"
                    "1000000101110101100111011011101110011001111001000100"
                    "1100110100101001111110011111001110000110000111101111"
                    "1001101110101111110111110010111100011101001000010110"
                    "1110110010010101001110011111001001011001010001001001"
                    "1011111001110110111110111010011000101100110010000110"
                    "1000010111011001010111101110001100011011110011000011"
                    "1010010110001000100110010010011001000110001111101100"
                    "1011011010010000111001000111010000010101010000010111"
                    "1111001000010000010000010011101110011011010011011100"
                    "1000110001001000001100000111110000001111010011110111"
                    "1100100011001001011000011011010010110010100000101000"
                    "1011000000001101110101001110111010000100000100111111"
                    "1010001011100101001100110011110011100111001010100000"
                    "1010001110111001100001100111100110001111011001011101"
                    "1001110101011110011000011010110101111101000110000000"
                    "1001010111111011101111111111001000100011111111011001"
                    "1100000100000111010100111010110101110011010011100000"
                    "1000011000010011111011011110111111100101010110011001"
                    "1000001000101000011010100011000110110100000100011100"
                    "1000110110011010111111001111101001001001110000000101"
                    "1100011111101110000011100010100101111110100101100010"
                    "1110100000110111110110010111010101111110111110111001"
                    "1101100110011001001101000011110110101011001001101110"
                    "1111111111111111111111111111111111111111111111111111"
                    "1010101010101010101010101010101010101010101010101010"
                    "1010001100010001001111010111111001010101000110010101"
                    "1000110000010111111000011011110010101010101000011100"
                    "1001110110101100101110010111101010011011011011110111"
                    "1010110000000100101010010010111000010101100001001000"
                    "1111100001101000011110000111000100111111010100001101"
                    "1001110010010010000000010011010110010001101101111110"
                    "1011010010011010100000000111110110010101000100100001"
                    "1011000111011001111010110010101101100100010010100000"
                    "1110100011100110001001101111110010111100010010001101"
                    "1111111010100111111010111010000011100111111010111110"
                    "1101000000010110010000000110000001010011000001000011"
                    "1101100101110111000011111011100101000100110001101000"
                    "1011110111000000000110111111000110000011001101110111"
                    "1100011000010001101101011011010011100011000010001100"
                    "1011001111111111110011100111001000001000101001000111"
                    "1011000101011101100001111010011101011000000000100110"
                    "1000010011100100111000010110010110010111100010100011"
                    "1000010111101101110100000010001110000111110010111000"
                    "1010111010111001011110010111000010010001010111101011"
                    "1101011010010010011000011010110111000101111000011100"
                    "1111111000111101110111011111011110101111100100010011"
                    "1100110000000100110000111011100110101110101011011000"
                    "1101101101001110001100100111011110110000000010010101"
                    "1101011000011011100011100010001001010110110010101110"
                    "1111111111111111111111111111111111111111111111111111"
                },
        /*  5*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("price:$439.97"), -1, 3 }, { TU("零售价:￥3149.79"), -1, 29 }, { TU("Preis:444,90 €"), -1, 17 } }, 0, 16, 48, 1, "AIM ITS/04-023:2022 Annex A example price only",
                    "101010101010101010101010101010101010101010101010"
                    "100001110110010111001111101001001000110101101111"
                    "100001011100111101001110111101110011001101011100"
                    "110010100011100110010101100101101010001101110111"
                    "100101010111111010110010111000011000011000001110"
                    "111000011111100110110101110110010001000000001001"
                    "111001110100011100001100111010100000001100101100"
                    "100100100000111101000111100000111111000101110111"
                    "110101111100000101011100100010011101010101001110"
                    "100101111010010100101111110011010011011010100001"
                    "110001110001010011101100100000100001010111000000"
                    "110111110100110011000001110000000100010011100011"
                    "100001001001011101011100110101010101100000100010"
                    "101000111110011010011101111110111000001110011011"
                    "110011100111011000101110100011101100000110010010"
                    "111111111111111111111111111111111111111111111111"
                },
        /*  6*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, { 0, 0, "" }, { { TU("\266"), 1, 0 }, { TU("\266"), 1, 7 }, { TU("\266"), 1, 0 } }, 0, 8, 32, 1, "Standard example + extra seg, data mode",
                    "10101010101010101010101010101010"
                    "10000100100011111101000010000011"
                    "11101100000000101110100001010100"
                    "11111011110010011011000100011111"
                    "11000111100111101101010111010110"
                    "11110110010001111100110110010101"
                    "11111110000111001100101011000000"
                    "11111111111111111111111111111111"
                },
        /*  7*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, -1, -1, { 1, 2, "001001" }, { { TU("A"), -1, 3 }, { TU("B"), -1, 4 }, { TU("C"), -1, 5 } }, 0, 12, 26, 1, "",
                    "10101010101010101010101010"
                    "10000100011110000011000101"
                    "10011100111010100100011000"
                    "11111100100100001110011111"
                    "11000010000100110011010100"
                    "11000000110100010011001001"
                    "11100000111111010111000000"
                    "10011101101010000100011101"
                    "11011000101010010101011000"
                    "10000100000010000011010001"
                    "10000011000001110111011000"
                    "11111111111111111111111111"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[8192];
    char cmp_buf[32768];
    char cmp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */
    int do_zxingcpp = (debug & ZINT_DEBUG_TEST_ZXINGCPP) && testUtilHaveZXingCPPDecoder(); /* Only do ZXing-C++ test if asked, too slow otherwise */

    testStartSymbol("test_encode_segs", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/,
                                    -1 /*option_1*/, data[i].option_2, data[i].option_3, data[i].output_options,
                                    NULL, 0, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }
        for (j = 0, seg_count = 0; j < 3 && data[i].segs[j].length; j++, seg_count++);

        ret = ZBarcode_Encode_Segs(symbol, data[i].segs, seg_count);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_Segs ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            char escaped1[4096];
            char escaped2[4096];
            int length = data[i].segs[0].length == -1 ? (int) ustrlen(data[i].segs[0].source) : data[i].segs[0].length;
            int length1 = data[i].segs[1].length == -1 ? (int) ustrlen(data[i].segs[1].source) : data[i].segs[1].length;
            int length2 = data[i].segs[2].length == -1 ? (int) ustrlen(data[i].segs[2].source) : data[i].segs[2].length;
            printf("        /*%3d*/ { %s, %s, %s, %d, %s, { %d, %d, \"%s\" }, { { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d } }, %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode),
                    testUtilOutputOptionsName(data[i].output_options),
                    data[i].option_2, testUtilOption3Name(data[i].symbology, data[i].option_3),
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape((const char *) data[i].segs[0].source, length, escaped, sizeof(escaped)), data[i].segs[0].length, data[i].segs[0].eci,
                    testUtilEscape((const char *) data[i].segs[1].source, length1, escaped1, sizeof(escaped1)), data[i].segs[1].length, data[i].segs[1].eci,
                    testUtilEscape((const char *) data[i].segs[2].source, length2, escaped2, sizeof(escaped2)), data[i].segs[2].length, data[i].segs[2].eci,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            testUtilModulesPrint(symbol, "                    ", "\n");
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n", i, ret, width, row);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwippSegs(i, symbol, -1, data[i].option_2, -1, data[i].segs, seg_count, NULL, cmp_buf, sizeof(cmp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwippSegs ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, cmp_msg, cmp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_buf, data[i].expected);
                    }
                }
                if (do_zxingcpp && testUtilCanZXingCPP(i, symbol, (const char *) data[i].segs[0].source, data[i].segs[0].length, debug)) {
                    if (data[i].input_mode == DATA_MODE) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) {
                            printf("i:%d multiple segments in DATA_MODE not currently supported for ZXing-C++ testing (%s)\n",
                                    i, testUtilBarcodeName(symbol->symbology));
                        }
                    } else {
                        int cmp_len, ret_len;
                        char modules_dump[144 * 144 + 1];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilZXingCPP(i, symbol, (const char *) data[i].segs[0].source, data[i].segs[0].length,
                                modules_dump, cmp_buf, sizeof(cmp_buf), &cmp_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPP ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilZXingCPPCmpSegs(symbol, cmp_msg, cmp_buf, cmp_len, data[i].segs, seg_count,
                                NULL /*primary*/, escaped, &ret_len);
                        assert_zero(ret, "i:%d %s testUtilZXingCPPCmpSegs %d != 0 %s\n  actual: %.*s\nexpected: %.*s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, cmp_msg, cmp_len, cmp_buf, ret_len, escaped);
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

#ifdef ZINT_TEST_ENCODING
static void test_minimalenc(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int output_options;
        int option_2;
        char *data;
        int length;
        int ret;

        int expected_diff; /* Difference between default minimal encodation and ISO encodation (FAST_MODE) */
        char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, -1, -1, "A", -1, 0, 0, "ASC" },
        /*  1*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AA", -1, 0, 0, "ASC" },
        /*  2*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAA", -1, 0, 0, "ASC" },
        /*  3*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAA", -1, 0, 0, "ASC" },
        /*  4*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAA", -1, 0, 0, "ASC" },
        /*  5*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAA", -1, 0, 0, "" },
        /*  6*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAA", -1, 0, 0, "" },
        /*  7*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAA", -1, 0, 1, "C40; EEEEEEEE" },
        /*  8*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAA", -1, 0, 0, "" },
        /*  9*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAA", -1, 0, 0, "" },
        /* 10*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAA", -1, 0, 0, "" },
        /* 11*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAA", -1, 0, 0, "" },
        /* 12*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAA", -1, 0, 0, "" },
        /* 13*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAA", -1, 0, 0, "" },
        /* 14*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAAA", -1, 0, 0, "" },
        /* 15*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAAAA", -1, 0, 0, "" },
        /* 16*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAAAAAAA", -1, 0, 0, "" },
        /* 17*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001A", -1, 0, 0, "ASC" },
        /* 18*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AA", -1, 0, 0, "ASC" },
        /* 19*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AAA", -1, 0, 0, "ASC" },
        /* 20*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AAAA", -1, 0, 0, "ASC" },
        /* 21*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AAAAA", -1, 0, 0, "ASC" },
        /* 22*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AAAAAA", -1, 0, 0, "" },
        /* 23*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AAAAAAA", -1, 0, 0, "" },
        /* 24*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AAAAAAAA", -1, 0, 0, "" },
        /* 25*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AAAAAAAAA", -1, 0, 2, "C40; ACCCCCCCCC" },
        /* 26*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AAAAAAAAAA", -1, 0, 0, "" },
        /* 27*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AAAAAAAAAAA", -1, 0, 0, "" },
        /* 28*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AAAAAAAAAAAA", -1, 0, 2, "C40; ACCCCCCCCCCCC" },
        /* 29*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001AAAAAAAAAAAAA", -1, 0, 0, "" },
        /* 30*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001A", -1, 0, 0, "" },
        /* 31*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AA", -1, 0, 0, "" },
        /* 32*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AAA", -1, 0, 0, "" },
        /* 33*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AAAA", -1, 0, 0, "" },
        /* 34*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AAAAA", -1, 0, 0, "" },
        /* 35*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AAAAAA", -1, 0, 0, "" },
        /* 36*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AAAAAAA", -1, 0, 0, "" },
        /* 37*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AAAAAAAA", -1, 0, 1, "C40; AAEEEEEEEE" },
        /* 38*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AAAAAAAAA", -1, 0, 0, "" },
        /* 39*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AAAAAAAAAA", -1, 0, 0, "" },
        /* 40*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AAAAAAAAAAA", -1, 0, 0, "" },
        /* 41*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AAAAAAAAAAAA", -1, 0, 0, "" },
        /* 42*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001AAAAAAAAAAAAA", -1, 0, 0, "" },
        /* 43*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001A", -1, 0, 0, "" },
        /* 44*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AA", -1, 0, 0, "" },
        /* 45*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AAA", -1, 0, 0, "" },
        /* 46*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AAAA", -1, 0, 0, "" },
        /* 47*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AAAAA", -1, 0, 0, "" },
        /* 48*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AAAAAA", -1, 0, 0, "" },
        /* 49*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AAAAAAA", -1, 0, 0, "" },
        /* 50*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AAAAAAAA", -1, 0, 0, "" },
        /* 51*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AAAAAAAAA", -1, 0, 2, "C40; AAACCCCCCCCC" },
        /* 52*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AAAAAAAAAA", -1, 0, 0, "" },
        /* 53*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AAAAAAAAAAA", -1, 0, 0, "" },
        /* 54*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AAAAAAAAAAAA", -1, 0, 2, "C40; AAACCCCCCCCCCCC" },
        /* 55*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001AAAAAAAAAAAAA", -1, 0, 0, "" },
        /* 56*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001A", -1, 0, 0, "" },
        /* 57*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AA", -1, 0, 0, "" },
        /* 58*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AAA", -1, 0, 0, "" },
        /* 59*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AAAA", -1, 0, 0, "" },
        /* 60*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AAAAA", -1, 0, 0, "" },
        /* 61*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AAAAAA", -1, 0, 0, "" },
        /* 62*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AAAAAAA", -1, 0, 0, "" },
        /* 63*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AAAAAAAA", -1, 0, 1, "C40; AAAAEEEEEEEE" },
        /* 64*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AAAAAAAAA", -1, 0, 0, "" },
        /* 65*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AAAAAAAAAA", -1, 0, 0, "" },
        /* 66*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AAAAAAAAAAA", -1, 0, 0, "" },
        /* 67*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AAAAAAAAAAAA", -1, 0, 1, "C40; AAAACCCCCCCCCCCC" },
        /* 68*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001AAAAAAAAAAAAA", -1, 0, 0, "" },
        /* 69*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001A", -1, 0, 0, "" },
        /* 70*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AA", -1, 0, 0, "" },
        /* 71*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AAA", -1, 0, 0, "" },
        /* 72*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AAAA", -1, 0, 0, "" },
        /* 73*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AAAAA", -1, 0, 0, "" },
        /* 74*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AAAAAA", -1, 0, 0, "" },
        /* 75*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AAAAAAA", -1, 0, 0, "" },
        /* 76*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AAAAAAAA", -1, 0, 0, "" },
        /* 77*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AAAAAAAAA", -1, 0, 2, "C40; AAAAACCCCCCCCC" },
        /* 78*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AAAAAAAAAA", -1, 0, 0, "" },
        /* 79*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AAAAAAAAAAA", -1, 0, 0, "" },
        /* 80*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AAAAAAAAAAAA", -1, 0, 1, "C40; AAAAACCCCCCCCCCCC" },
        /* 81*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001AAAAAAAAAAAAA", -1, 0, 0, "" },
        /* 82*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200A", -1, 0, 0, "" },
        /* 83*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AA", -1, 0, 0, "" },
        /* 84*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AAA", -1, 0, 0, "" },
        /* 85*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AAAA", -1, 0, 0, "" },
        /* 86*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AAAAA", -1, 0, 0, "" },
        /* 87*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AAAAAA", -1, 0, 0, "" },
        /* 88*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AAAAAAA", -1, 0, 0, "" },
        /* 89*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AAAAAAAA", -1, 0, 1, "" },
        /* 90*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AAAAAAAAA", -1, 0, 0, "" },
        /* 91*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AAAAAAAAAA", -1, 0, 2, "" },
        /* 92*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AAAAAAAAAAA", -1, 0, 0, "" },
        /* 93*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AAAAAAAAAAAA", -1, 0, 0, "" },
        /* 94*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200AAAAAAAAAAAAA", -1, 0, 2, "" },
        /* 95*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200A", -1, 0, 0, "" },
        /* 96*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AA", -1, 0, 0, "" },
        /* 97*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AAA", -1, 0, 0, "" },
        /* 98*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AAAA", -1, 0, 0, "" },
        /* 99*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AAAAA", -1, 0, 0, "" },
        /*100*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AAAAAA", -1, 0, 0, "" },
        /*101*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AAAAAAA", -1, 0, 0, "" },
        /*102*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AAAAAAAA", -1, 0, 1, "" },
        /*103*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AAAAAAAAA", -1, 0, 0, "" },
        /*104*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AAAAAAAAAA", -1, 0, 0, "" },
        /*105*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AAAAAAAAAAA", -1, 0, 0, "" },
        /*106*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AAAAAAAAAAAA", -1, 0, 0, "" },
        /*107*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200AAAAAAAAAAAAA", -1, 0, 0, "" },
        /*108*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200A", -1, 0, 0, "ASC last 1" },
        /*109*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AA", -1, 0, 0, "ASC 2" },
        /*110*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAA", -1, 0, 0, "ASC 3" },
        /*111*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAA", -1, 0, 0, "ASC 4" },
        /*112*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAA", -1, 0, 0, "ASC 5" },
        /*113*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAA", -1, 0, 0, "C40 6" },
        /*114*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAA", -1, 0, 0, "C40" },
        /*115*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAA", -1, 0, 0, "C40" },
        /*116*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAA", -1, 0, 0, "C40" },
        /*117*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAA", -1, 0, 0, "C40 except last ASC" },
        /*118*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAA", -1, 0, 0, "C40 except last 2 ASC" },
        /*119*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAAA", -1, 0, 0, "C40 except last ASC" },
        /*120*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAAAA", -1, 0, 0, "C40 except last 2 ASC" },
        /*121*/ { BARCODE_DATAMATRIX, -1, -1, -1, "9", -1, 0, 0, "" },
        /*122*/ { BARCODE_DATAMATRIX, -1, -1, -1, "99", -1, 0, 0, "" },
        /*123*/ { BARCODE_DATAMATRIX, -1, -1, -1, "999", -1, 0, 0, "" },
        /*124*/ { BARCODE_DATAMATRIX, -1, -1, -1, "9999", -1, 0, 0, "" },
        /*125*/ { BARCODE_DATAMATRIX, -1, -1, -1, "99999", -1, 0, 0, "" },
        /*126*/ { BARCODE_DATAMATRIX, -1, -1, -1, "999999", -1, 0, 0, "" },
        /*127*/ { BARCODE_DATAMATRIX, -1, -1, -1, "9999999", -1, 0, 0, "" },
        /*128*/ { BARCODE_DATAMATRIX, -1, -1, -1, "99999999", -1, 0, 0, "" },
        /*129*/ { BARCODE_DATAMATRIX, -1, -1, -1, "999999999", -1, 0, 0, "" },
        /*130*/ { BARCODE_DATAMATRIX, -1, -1, -1, "9999999999", -1, 0, 0, "" },
        /*131*/ { BARCODE_DATAMATRIX, -1, -1, -1, "99999999999", -1, 0, 0, "" },
        /*132*/ { BARCODE_DATAMATRIX, -1, -1, -1, "999999999999", -1, 0, 0, "" },
        /*133*/ { BARCODE_DATAMATRIX, -1, -1, -1, "9999999999999", -1, 0, 0, "" },
        /*134*/ { BARCODE_DATAMATRIX, -1, -1, -1, "99999999999999", -1, 0, 0, "" },
        /*135*/ { BARCODE_DATAMATRIX, -1, -1, -1, "999999999999999", -1, 0, 0, "" },
        /*136*/ { BARCODE_DATAMATRIX, -1, -1, -1, "9999999999999999", -1, 0, 0, "" },
        /*137*/ { BARCODE_DATAMATRIX, -1, -1, -1, "99999999999999999", -1, 0, 0, "" },
        /*138*/ { BARCODE_DATAMATRIX, -1, -1, -1, "999999999999999999", -1, 0, 0, "" },
        /*139*/ { BARCODE_DATAMATRIX, -1, -1, -1, "9999999999999999999", -1, 0, 0, "" },
        /*140*/ { BARCODE_DATAMATRIX, -1, -1, -1, "99999999999999999999", -1, 0, 0, "" },
        /*141*/ { BARCODE_DATAMATRIX, -1, -1, -1, "999999999999999999999", -1, 0, 0, "" },
        /*142*/ { BARCODE_DATAMATRIX, -1, -1, -1, "9999999999999999999999", -1, 0, 0, "" },
        /*143*/ { BARCODE_DATAMATRIX, -1, -1, -1, "99999999999999999999999", -1, 0, 0, "" },
        /*144*/ { BARCODE_DATAMATRIX, -1, -1, -1, "999999999999999999999999", -1, 0, 0, "" },
        /*145*/ { BARCODE_DATAMATRIX, -1, -1, -1, "9999999999999999999999999", -1, 0, 0, "" },
        /*146*/ { BARCODE_DATAMATRIX, -1, -1, -1, "99999999999999999999999999", -1, 0, 0, "" },
        /*147*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\0019", -1, 0, 0, "" },
        /*148*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\00199", -1, 0, 0, "" },
        /*149*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001999", -1, 0, 0, "" },
        /*150*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\0019999", -1, 0, 0, "" },
        /*151*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\00199999", -1, 0, 0, "" },
        /*152*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001999999", -1, 0, 0, "" },
        /*153*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\0019999999", -1, 0, 0, "" },
        /*154*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\00199999999", -1, 0, 0, "" },
        /*155*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001999999999", -1, 0, 0, "" },
        /*156*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\0019999999999", -1, 0, 0, "" },
        /*157*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\00199999999999", -1, 0, 0, "" },
        /*158*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001999999999999", -1, 0, 0, "" },
        /*159*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\0019999999999999", -1, 0, 0, "" },
        /*160*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\00199999999999999", -1, 0, 0, "" },
        /*161*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001999999999999999", -1, 0, 0, "" },
        /*162*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\0019999999999999999", -1, 0, 0, "" },
        /*163*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\00199999999999999999", -1, 0, 0, "" },
        /*164*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001999999999999999999", -1, 0, 0, "" },
        /*165*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\0019999999999999999999", -1, 0, 0, "" },
        /*166*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\00199999999999999999999", -1, 0, 0, "" },
        /*167*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001999999999999999999999", -1, 0, 0, "" },
        /*168*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\0019999999999999999999999", -1, 0, 0, "" },
        /*169*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\00199999999999999999999999", -1, 0, 0, "" },
        /*170*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001999999999999999999999999", -1, 0, 0, "" },
        /*171*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\0019999999999999999999999999", -1, 0, 0, "" },
        /*172*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\00199999999999999999999999999", -1, 0, 0, "" },
        /*173*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\0019", -1, 0, 0, "" },
        /*174*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\00199", -1, 0, 0, "" },
        /*175*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001999", -1, 0, 0, "" },
        /*176*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\0019999", -1, 0, 0, "" },
        /*177*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\00199999", -1, 0, 0, "" },
        /*178*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001999999", -1, 0, 0, "" },
        /*179*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\0019999999", -1, 0, 0, "" },
        /*180*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\00199999999", -1, 0, 0, "" },
        /*181*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001999999999", -1, 0, 0, "" },
        /*182*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\0019999999999", -1, 0, 0, "" },
        /*183*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\00199999999999", -1, 0, 0, "" },
        /*184*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001999999999999", -1, 0, 0, "" },
        /*185*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\0019999999999999", -1, 0, 0, "" },
        /*186*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\00199999999999999", -1, 0, 0, "" },
        /*187*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001999999999999999", -1, 0, 0, "" },
        /*188*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\0019999999999999999", -1, 0, 0, "" },
        /*189*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\00199999999999999999", -1, 0, 0, "" },
        /*190*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001999999999999999999", -1, 0, 0, "" },
        /*191*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\0019999999999999999999", -1, 0, 0, "" },
        /*192*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\00199999999999999999999", -1, 0, 0, "" },
        /*193*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001999999999999999999999", -1, 0, 0, "" },
        /*194*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\0019999999999999999999999", -1, 0, 0, "" },
        /*195*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\00199999999999999999999999", -1, 0, 0, "" },
        /*196*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001999999999999999999999999", -1, 0, 0, "" },
        /*197*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\0019999999999999999999999999", -1, 0, 0, "" },
        /*198*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\00199999999999999999999999999", -1, 0, 0, "" },
        /*199*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\0019", -1, 0, 0, "" },
        /*200*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\00199", -1, 0, 0, "" },
        /*201*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001999", -1, 0, 0, "" },
        /*202*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\0019999", -1, 0, 0, "" },
        /*203*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\00199999", -1, 0, 0, "" },
        /*204*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001999999", -1, 0, 0, "" },
        /*205*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\0019999999", -1, 0, 0, "" },
        /*206*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\00199999999", -1, 0, 0, "" },
        /*207*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001999999999", -1, 0, 0, "" },
        /*208*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\0019999999999", -1, 0, 0, "" },
        /*209*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\00199999999999", -1, 0, 0, "" },
        /*210*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001999999999999", -1, 0, 0, "" },
        /*211*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\0019999999999999", -1, 0, 0, "" },
        /*212*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\00199999999999999", -1, 0, 0, "" },
        /*213*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001999999999999999", -1, 0, 0, "" },
        /*214*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\0019999999999999999", -1, 0, 0, "" },
        /*215*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\00199999999999999999", -1, 0, 0, "" },
        /*216*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001999999999999999999", -1, 0, 0, "" },
        /*217*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\0019999999999999999999", -1, 0, 0, "" },
        /*218*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\00199999999999999999999", -1, 0, 0, "" },
        /*219*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001999999999999999999999", -1, 0, 0, "" },
        /*220*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\0019999999999999999999999", -1, 0, 0, "" },
        /*221*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\00199999999999999999999999", -1, 0, 0, "" },
        /*222*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001999999999999999999999999", -1, 0, 0, "" },
        /*223*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\0019999999999999999999999999", -1, 0, 0, "" },
        /*224*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\00199999999999999999999999999", -1, 0, 0, "" },
        /*225*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\0019", -1, 0, 0, "" },
        /*226*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\00199", -1, 0, 0, "" },
        /*227*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001999", -1, 0, 0, "" },
        /*228*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\0019999", -1, 0, 0, "" },
        /*229*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\00199999", -1, 0, 0, "" },
        /*230*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001999999", -1, 0, 0, "" },
        /*231*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\0019999999", -1, 0, 0, "" },
        /*232*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\00199999999", -1, 0, 0, "" },
        /*233*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001999999999", -1, 0, 0, "" },
        /*234*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\0019999999999", -1, 0, 0, "" },
        /*235*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\00199999999999", -1, 0, 0, "" },
        /*236*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001999999999999", -1, 0, 0, "" },
        /*237*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\0019999999999999", -1, 0, 0, "" },
        /*238*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\00199999999999999", -1, 0, 0, "" },
        /*239*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001999999999999999", -1, 0, 0, "" },
        /*240*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\0019999999999999999", -1, 0, 0, "" },
        /*241*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\00199999999999999999", -1, 0, 0, "" },
        /*242*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001999999999999999999", -1, 0, 0, "" },
        /*243*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\0019999999999999999999", -1, 0, 0, "" },
        /*244*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\00199999999999999999999", -1, 0, 0, "" },
        /*245*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001999999999999999999999", -1, 0, 0, "" },
        /*246*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\0019999999999999999999999", -1, 0, 0, "" },
        /*247*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\00199999999999999999999999", -1, 0, 0, "" },
        /*248*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001999999999999999999999999", -1, 0, 0, "" },
        /*249*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\0019999999999999999999999999", -1, 0, 0, "" },
        /*250*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\00199999999999999999999999999", -1, 0, 0, "" },
        /*251*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\0019", -1, 0, 0, "" },
        /*252*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\00199", -1, 0, 0, "" },
        /*253*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001999", -1, 0, 0, "" },
        /*254*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\0019999", -1, 0, 0, "" },
        /*255*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\00199999", -1, 0, 0, "" },
        /*256*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001999999", -1, 0, 0, "" },
        /*257*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\0019999999", -1, 0, 0, "" },
        /*258*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\00199999999", -1, 0, 0, "" },
        /*259*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001999999999", -1, 0, 0, "" },
        /*260*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\0019999999999", -1, 0, 0, "" },
        /*261*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\00199999999999", -1, 0, 0, "" },
        /*262*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001999999999999", -1, 0, 0, "" },
        /*263*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\0019999999999999", -1, 0, 0, "" },
        /*264*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\00199999999999999", -1, 0, 0, "" },
        /*265*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001999999999999999", -1, 0, 0, "" },
        /*266*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\0019999999999999999", -1, 0, 0, "" },
        /*267*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\00199999999999999999", -1, 0, 0, "" },
        /*268*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001999999999999999999", -1, 0, 0, "" },
        /*269*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\0019999999999999999999", -1, 0, 0, "" },
        /*270*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\00199999999999999999999", -1, 0, 0, "" },
        /*271*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001999999999999999999999", -1, 0, 0, "" },
        /*272*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\0019999999999999999999999", -1, 0, 0, "" },
        /*273*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\00199999999999999999999999", -1, 0, 0, "" },
        /*274*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001999999999999999999999999", -1, 0, 0, "" },
        /*275*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\0019999999999999999999999999", -1, 0, 0, "" },
        /*276*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\00199999999999999999999999999", -1, 0, 0, "" },
        /*277*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\2009", -1, 0, 0, "" },
        /*278*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\20099", -1, 0, 0, "" },
        /*279*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200999", -1, 0, 0, "" },
        /*280*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\2009999", -1, 0, 0, "" },
        /*281*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\20099999", -1, 0, 0, "" },
        /*282*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200999999", -1, 0, 0, "" },
        /*283*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\2009999999", -1, 0, 0, "" },
        /*284*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\20099999999", -1, 0, 0, "" },
        /*285*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200999999999", -1, 0, 0, "" },
        /*286*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\2009999999999", -1, 0, 0, "" },
        /*287*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\20099999999999", -1, 0, 0, "" },
        /*288*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200999999999999", -1, 0, 0, "" },
        /*289*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\2009999999999999", -1, 0, 0, "" },
        /*290*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\20099999999999999", -1, 0, 0, "" },
        /*291*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200999999999999999", -1, 0, 0, "" },
        /*292*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\2009999999999999999", -1, 0, 0, "" },
        /*293*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\20099999999999999999", -1, 0, 0, "" },
        /*294*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200999999999999999999", -1, 0, 0, "" },
        /*295*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\2009999999999999999999", -1, 0, 0, "" },
        /*296*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\20099999999999999999999", -1, 0, 0, "" },
        /*297*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200999999999999999999999", -1, 0, 0, "" },
        /*298*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\2009999999999999999999999", -1, 0, 0, "" },
        /*299*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\20099999999999999999999999", -1, 0, 0, "" },
        /*300*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200999999999999999999999999", -1, 0, 0, "" },
        /*301*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\2009999999999999999999999999", -1, 0, 0, "" },
        /*302*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\2009", -1, 0, 0, "" },
        /*303*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\20099", -1, 0, 1, "BAS FN4 ASC; AAAA" },
        /*304*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200999", -1, 0, 0, "" },
        /*305*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\2009999", -1, 0, 0, "" },
        /*306*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\20099999", -1, 0, 0, "" },
        /*307*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200999999", -1, 0, 0, "" },
        /*308*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\2009999999", -1, 0, 0, "" },
        /*309*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\20099999999", -1, 0, 0, "" },
        /*310*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200999999999", -1, 0, 0, "" },
        /*311*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\2009999999999", -1, 0, 0, "" },
        /*312*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\20099999999999", -1, 0, 0, "" },
        /*313*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200999999999999", -1, 0, 0, "" },
        /*314*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\2009999999999999", -1, 0, 0, "" },
        /*315*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\20099999999999999", -1, 0, 0, "" },
        /*316*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200999999999999999", -1, 0, 0, "" },
        /*317*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\2009999999999999999", -1, 0, 0, "" },
        /*318*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\20099999999999999999", -1, 0, 0, "" },
        /*319*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200999999999999999999", -1, 0, 0, "" },
        /*320*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\2009999999999999999999", -1, 0, 0, "" },
        /*321*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\20099999999999999999999", -1, 0, 0, "" },
        /*322*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200999999999999999999999", -1, 0, 0, "" },
        /*323*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\2009999999999999999999999", -1, 0, 0, "" },
        /*324*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\20099999999999999999999999", -1, 0, 0, "" },
        /*325*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200999999999999999999999999", -1, 0, 0, "" },
        /*326*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\2009999999999999999999999999", -1, 0, 0, "" },
        /*327*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009", -1, 0, 0, "ASC last 1" },
        /*328*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099", -1, 0, 1, "BBBAA" },
        /*329*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999", -1, 0, 0, "" },
        /*330*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999", -1, 0, 0, "" },
        /*331*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999", -1, 0, 0, "" },
        /*332*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999", -1, 0, 0, "" },
        /*333*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999", -1, 0, 0, "" },
        /*334*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999", -1, 0, 0, "" },
        /*335*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999", -1, 0, 0, "" },
        /*336*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999", -1, 0, 0, "" },
        /*337*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999999", -1, 0, 0, "" },
        /*338*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999999", -1, 0, 0, "" },
        /*339*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999999", -1, 0, 0, "" },
        /*340*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999999999", -1, 0, 0, "" },
        /*341*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999999999", -1, 0, 0, "" },
        /*342*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999999999", -1, 0, 0, "" },
        /*343*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999999999999", -1, 0, 0, "" },
        /*344*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999999999999", -1, 0, 0, "" },
        /*345*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999999999999", -1, 0, 0, "" },
        /*346*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999999999999999", -1, 0, 0, "" },
        /*347*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999999999999999", -1, 0, 0, "" },
        /*348*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999999999999999", -1, 0, 0, "" },
        /*349*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999999999999999999", -1, 0, 0, "" },
        /*350*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999999999999999999", -1, 0, 0, "" },
        /*351*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999999999999999999", -1, 0, 0, "" },
        /*352*/ { BARCODE_DATAMATRIX, -1, -1, -1, "a", -1, 0, 0, "ASC" },
        /*353*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aa", -1, 0, 0, "ASC" },
        /*354*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aaa", -1, 0, 0, "ASC" },
        /*355*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aaaa", -1, 0, 0, "ASC" },
        /*356*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aaaaa", -1, 0, 0, "ASC" },
        /*357*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aaaaaa", -1, 0, 0, "" },
        /*358*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aaaaaaa", -1, 0, 0, "" },
        /*359*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aaaaaaaa", -1, 0, 0, "ASC" },
        /*360*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aaaaaaaaa", -1, 0, 0, "TEX" },
        /*361*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aaaaaaaaaa", -1, 0, 0, "" },
        /*362*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aaaaaaaaaaa", -1, 0, 0, "" },
        /*363*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aaaaaaaaaaaa", -1, 0, 0, "" },
        /*364*/ { BARCODE_DATAMATRIX, -1, -1, -1, "aaaaaaaaaaaaa", -1, 0, 0, "" },
        /*365*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001a", -1, 0, 0, "ASC" },
        /*366*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aa", -1, 0, 0, "ASC" },
        /*367*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aaa", -1, 0, 0, "ASC" },
        /*368*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aaaa", -1, 0, 0, "ASC" },
        /*369*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aaaaa", -1, 0, 0, "ASC" },
        /*370*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aaaaaa", -1, 0, 0, "" },
        /*371*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aaaaaaa", -1, 0, 0, "" },
        /*372*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aaaaaaaa", -1, 0, 0, "" },
        /*373*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aaaaaaaaa", -1, 0, 2, "TEX; ATTTTTTTTT" },
        /*374*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aaaaaaaaaa", -1, 0, 0, "" },
        /*375*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aaaaaaaaaaa", -1, 0, 0, "" },
        /*376*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aaaaaaaaaaaa", -1, 0, 2, "TEX; ATTTTTTTTTTTT" },
        /*377*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001aaaaaaaaaaaaa", -1, 0, 0, "" },
        /*378*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001a", -1, 0, 0, "" },
        /*379*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aa", -1, 0, 0, "" },
        /*380*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aaa", -1, 0, 0, "" },
        /*381*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aaaa", -1, 0, 0, "" },
        /*382*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aaaaa", -1, 0, 0, "" },
        /*383*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aaaaaa", -1, 0, 0, "" },
        /*384*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aaaaaaa", -1, 0, 0, "" },
        /*385*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aaaaaaaa", -1, 0, 0, "" },
        /*386*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aaaaaaaaa", -1, 0, 0, "" },
        /*387*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aaaaaaaaaa", -1, 0, 0, "" },
        /*388*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aaaaaaaaaaa", -1, 0, 0, "" },
        /*389*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aaaaaaaaaaaa", -1, 0, 0, "" },
        /*390*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001aaaaaaaaaaaaa", -1, 0, 0, "" },
        /*391*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200a", -1, 0, 0, "ASC last 1" },
        /*392*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aa", -1, 0, 0, "ASC 2" },
        /*393*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaa", -1, 0, 0, "ASC 3" },
        /*394*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaa", -1, 0, 0, "ASC 4" },
        /*395*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaa", -1, 0, 0, "ASC 5" },
        /*396*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaa", -1, 0, 0, "TEX 6" },
        /*397*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa", -1, 0, 0, "TEX" },
        /*398*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaa", -1, 0, 0, "TEX" },
        /*399*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaa", -1, 0, 0, "TEX" },
        /*400*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaa", -1, 0, 0, "TEX except last ASC" },
        /*401*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaa", -1, 0, 0, "TEX except last 2 ASC" },
        /*402*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaa", -1, 0, 0, "TEX" },
        /*403*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaa", -1, 0, 0, "TEX except last ASC" },
        /*404*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">", -1, 0, 0, "" },
        /*405*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>", -1, 0, 0, "" },
        /*406*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>", -1, 0, 0, "" },
        /*407*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>", -1, 0, 0, "" },
        /*408*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>", -1, 0, 0, "" },
        /*409*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>", -1, 0, 0, "" },
        /*410*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>", -1, 0, 0, "" },
        /*411*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>", -1, 0, 0, "" },
        /*412*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>", -1, 0, 0, "" },
        /*413*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>", -1, 0, 0, "" },
        /*414*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>", -1, 0, 0, "" },
        /*415*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>", -1, 0, 0, "" },
        /*416*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>", -1, 0, 0, "" },
        /*417*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>>>>>", -1, 0, 0, "" },
        /*418*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>", -1, 0, 0, "" },
        /*419*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>", -1, 0, 0, "" },
        /*420*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>>", -1, 0, 0, "" },
        /*421*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>>>", -1, 0, 0, "" },
        /*422*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>>>>", -1, 0, 0, "" },
        /*423*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>>>>>", -1, 0, 0, "" },
        /*424*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>>>>>>", -1, 0, 0, "" },
        /*425*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>>>>>>>", -1, 0, 0, "" },
        /*426*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>>>>>>>>", -1, 0, 0, "" },
        /*427*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>>>>>>>>>", -1, 0, 0, "" },
        /*428*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>>>>>>>>>>", -1, 0, 1, "X12; AAAXXXXXXXXX" },
        /*429*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>>>>>>>>>>>", -1, 0, 0, "" },
        /*430*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001>>>>>>>>>>>>>", -1, 0, 0, "" },
        /*431*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>", -1, 0, 0, "" },
        /*432*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>", -1, 0, 0, "" },
        /*433*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>>", -1, 0, 0, "" },
        /*434*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>>>", -1, 0, 0, "" },
        /*435*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>>>>", -1, 0, 0, "" },
        /*436*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>>>>>", -1, 0, 0, "" },
        /*437*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>>>>>>", -1, 0, 0, "" },
        /*438*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>>>>>>>", -1, 0, 0, "" },
        /*439*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>>>>>>>>", -1, 0, 0, "" },
        /*440*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>>>>>>>>>", -1, 0, 0, "" },
        /*441*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>>>>>>>>>>", -1, 0, 0, "" },
        /*442*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>>>>>>>>>>>", -1, 0, 0, "" },
        /*443*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001>>>>>>>>>>>>>", -1, 0, 0, "" },
        /*444*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>", -1, 0, 0, "" },
        /*445*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>", -1, 0, 0, "" },
        /*446*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>>", -1, 0, 0, "" },
        /*447*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>>>", -1, 0, 0, "" },
        /*448*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>>>>", -1, 0, 0, "" },
        /*449*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>>>>>", -1, 0, 0, "" },
        /*450*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>>>>>>", -1, 0, 0, "" },
        /*451*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>>>>>>>", -1, 0, 0, "" },
        /*452*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>>>>>>>>", -1, 0, 0, "" },
        /*453*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>>>>>>>>>", -1, 0, 0, "" },
        /*454*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>>>>>>>>>>", -1, 0, 1, "X12; AAAAAXXXXXXXXX" },
        /*455*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>>>>>>>>>>>", -1, 0, 0, "" },
        /*456*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001>>>>>>>>>>>>>", -1, 0, 0, "" },
        /*457*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>", -1, 0, 0, "" },
        /*458*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>", -1, 0, 0, "" },
        /*459*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>>", -1, 0, 0, "" },
        /*460*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>>>", -1, 0, 0, "" },
        /*461*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>>>>", -1, 0, 0, "" },
        /*462*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>>>>>", -1, 0, 0, "" },
        /*463*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>>>>>>", -1, 0, 0, "" },
        /*464*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>>>>>>>", -1, 0, 0, "" },
        /*465*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>>>>>>>>", -1, 0, 0, "" },
        /*466*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>>>>>>>>>", -1, 0, 0, "" },
        /*467*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>>>>>>>>>>", -1, 0, 0, "" },
        /*468*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>>>>>>>>>>>", -1, 0, 0, "" },
        /*469*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001>>>>>>>>>>>>>", -1, 0, 0, "" },
        /*470*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>", -1, 0, 0, "" },
        /*471*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>", -1, 0, 0, "" },
        /*472*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>>", -1, 0, 0, "" },
        /*473*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>>>", -1, 0, 0, "" },
        /*474*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>>>>", -1, 0, 0, "" },
        /*475*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>>>>>", -1, 0, 0, "" },
        /*476*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>>>>>>", -1, 0, 0, "" },
        /*477*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>>>>>>>", -1, 0, 0, "" },
        /*478*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>>>>>>>>", -1, 0, 0, "" },
        /*479*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>>>>>>>>>", -1, 0, 0, "" },
        /*480*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>>>>>>>>>>", -1, 0, 0, "" },
        /*481*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>>>>>>>>>>>", -1, 0, 0, "" },
        /*482*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001>>>>>>>>>>>>>", -1, 0, 0, "" },
        /*483*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>", -1, 0, 0, "" },
        /*484*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>", -1, 0, 0, "" },
        /*485*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>>", -1, 0, 1, "BAS ASC; AAAA" },
        /*486*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>>>", -1, 0, 1, "BAS ASC; AAAAA" },
        /*487*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>>>>", -1, 0, 1, "BAS ASC; AAEEEE" },
        /*488*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>>>>>", -1, 0, 0, "BAS X12; AAEEEEA" },
        /*489*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>>>>>>", -1, 0, 2, "BAS X12; AAXXXXXX" },
        /*490*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>>>>>>>", -1, 0, 1, "BAS EDI; AEEEEEEEE" },
        /*491*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>>>>>>>>", -1, 0, 0, "" },
        /*492*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>>>>>>>>>", -1, 0, 2, "BAS X12; AAXXXXXXXXX" },
        /*493*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>>>>>>>>>>", -1, 0, 1, "BAS X12;2; AEEEEEEEEEEE" },
        /*494*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>>>>>>>>>>>", -1, 0, 0, "" },
        /*495*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200>>>>>>>>>>>>>", -1, 0, 2, "BAS; AAXXXXXXXXXXXX" },
        /*496*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>", -1, 0, 0, "" },
        /*497*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>", -1, 0, 0, "" },
        /*498*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>>", -1, 0, 0, "" },
        /*499*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>>>", -1, 0, 0, "" },
        /*500*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>>>>", -1, 0, 0, "" },
        /*501*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>>>>>", -1, 0, 0, "" },
        /*502*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>>>>>>", -1, 0, 0, "" },
        /*503*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>>>>>>>", -1, 0, 0, "" },
        /*504*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>>>>>>>>", -1, 0, 0, "" },
        /*505*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>>>>>>>>>", -1, 0, 0, "" },
        /*506*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>>>>>>>>>>", -1, 0, 0, "" },
        /*507*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>>>>>>>>>>>", -1, 0, 0, "" },
        /*508*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200>>>>>>>>>>>>>", -1, 0, 0, "" },
        /*509*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>", -1, 0, 0, "ASC last 1" },
        /*510*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>", -1, 0, 0, "ASC 2" },
        /*511*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>", -1, 0, 0, "ASC 3" },
        /*512*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>", -1, 0, 0, "ASC 4" },
        /*513*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>", -1, 0, 0, "ASC 5" },
        /*514*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>", -1, 0, 0, "X12 6" },
        /*515*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>", -1, 0, 0, "X12 except last 2 ASC" },
        /*516*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>", -1, 0, 0, "X12" },
        /*517*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>", -1, 0, 0, "X12" },
        /*518*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>", -1, 0, 0, "X12 except last 2 ASC" },
        /*519*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>", -1, 0, 0, "X12 except last 3 ASC" },
        /*520*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>", -1, 0, 0, "X12" },
        /*521*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>", -1, 0, 0, "X12 except last ASC" },
        /*522*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".", -1, 0, 0, "" },
        /*523*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..", -1, 0, 0, "" },
        /*524*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...", -1, 0, 0, "" },
        /*525*/ { BARCODE_DATAMATRIX, -1, -1, -1, "....", -1, 0, 0, "" },
        /*526*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".....", -1, 0, 0, "" },
        /*527*/ { BARCODE_DATAMATRIX, -1, -1, -1, "......", -1, 0, 0, "" },
        /*528*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".......", -1, 0, 0, "" },
        /*529*/ { BARCODE_DATAMATRIX, -1, -1, -1, "........", -1, 0, 0, "" },
        /*530*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".........", -1, 0, 0, "" },
        /*531*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..........", -1, 0, 1, "EDI; AAEEEEEEEE" },
        /*532*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...........", -1, 0, 0, "" },
        /*533*/ { BARCODE_DATAMATRIX, -1, -1, -1, "............", -1, 0, 0, "" },
        /*534*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".............", -1, 0, 0, "" },
        /*535*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001.", -1, 0, 0, "" },
        /*536*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001..", -1, 0, 0, "" },
        /*537*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001...", -1, 0, 0, "" },
        /*538*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001....", -1, 0, 0, "" },
        /*539*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001.....", -1, 0, 0, "" },
        /*540*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001......", -1, 0, 0, "" },
        /*541*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001.......", -1, 0, 0, "" },
        /*542*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001........", -1, 0, 0, "" },
        /*543*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001.........", -1, 0, 0, "" },
        /*544*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001..........", -1, 0, 0, "" },
        /*545*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001...........", -1, 0, 0, "" },
        /*546*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001............", -1, 0, 0, "" },
        /*547*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001.............", -1, 0, 0, "" },
        /*548*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001.", -1, 0, 0, "" },
        /*549*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001..", -1, 0, 0, "" },
        /*550*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001...", -1, 0, 0, "" },
        /*551*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001....", -1, 0, 0, "" },
        /*552*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001.....", -1, 0, 0, "" },
        /*553*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001......", -1, 0, 0, "" },
        /*554*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001.......", -1, 0, 0, "" },
        /*555*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001........", -1, 0, 0, "" },
        /*556*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001.........", -1, 0, 0, "" },
        /*557*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001..........", -1, 0, 1, "EDI; AAAAEEEEEEEE" },
        /*558*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001...........", -1, 0, 0, "" },
        /*559*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001............", -1, 0, 0, "" },
        /*560*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001.............", -1, 0, 0, "" },
        /*561*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001.", -1, 0, 0, "" },
        /*562*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001..", -1, 0, 0, "" },
        /*563*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001...", -1, 0, 0, "" },
        /*564*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001....", -1, 0, 0, "" },
        /*565*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001.....", -1, 0, 0, "" },
        /*566*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001......", -1, 0, 0, "" },
        /*567*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001.......", -1, 0, 0, "" },
        /*568*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001........", -1, 0, 0, "" },
        /*569*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001.........", -1, 0, 0, "" },
        /*570*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001..........", -1, 0, 0, "" },
        /*571*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001...........", -1, 0, 0, "" },
        /*572*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001............", -1, 0, 0, "" },
        /*573*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001.............", -1, 0, 1, "EDI; AAAAEEEEEEEEEEEE" },
        /*574*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001.", -1, 0, 0, "" },
        /*575*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001..", -1, 0, 0, "" },
        /*576*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001...", -1, 0, 0, "" },
        /*577*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001....", -1, 0, 0, "" },
        /*578*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001.....", -1, 0, 0, "" },
        /*579*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001......", -1, 0, 0, "" },
        /*580*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001.......", -1, 0, 0, "" },
        /*581*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001........", -1, 0, 0, "" },
        /*582*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001.........", -1, 0, 0, "" },
        /*583*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001..........", -1, 0, 0, "" },
        /*584*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001...........", -1, 0, 0, "" },
        /*585*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001............", -1, 0, 0, "" },
        /*586*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001.............", -1, 0, 0, "" },
        /*587*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001.", -1, 0, 0, "" },
        /*588*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001..", -1, 0, 0, "" },
        /*589*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001...", -1, 0, 0, "" },
        /*590*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001....", -1, 0, 0, "" },
        /*591*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001.....", -1, 0, 0, "" },
        /*592*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001......", -1, 0, 0, "" },
        /*593*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001.......", -1, 0, 0, "" },
        /*594*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001........", -1, 0, 0, "" },
        /*595*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001.........", -1, 0, 0, "" },
        /*596*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001..........", -1, 0, 1, "" },
        /*597*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001...........", -1, 0, 0, "" },
        /*598*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001............", -1, 0, 0, "" },
        /*599*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\001\001\001\001\001.............", -1, 0, 0, "" },
        /*600*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200.", -1, 0, 0, "" },
        /*601*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200..", -1, 0, 0, "" },
        /*602*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200...", -1, 0, 1, "" },
        /*603*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200....", -1, 0, 1, "" },
        /*604*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200.....", -1, 0, 1, "" },
        /*605*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200......", -1, 0, 1, "" },
        /*606*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200.......", -1, 0, 1, "" },
        /*607*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200........", -1, 0, 1, "" },
        /*608*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200.........", -1, 0, 1, "" },
        /*609*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200..........", -1, 0, 1, "" },
        /*610*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200...........", -1, 0, 1, "" },
        /*611*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200............", -1, 0, 2, "" },
        /*612*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200.............", -1, 0, 1, "" },
        /*613*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200.", -1, 0, 0, "" },
        /*614*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200..", -1, 0, 0, "" },
        /*615*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200...", -1, 0, 0, "" },
        /*616*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200....", -1, 0, 0, "" },
        /*617*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200.....", -1, 0, 0, "" },
        /*618*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200......", -1, 0, 0, "" },
        /*619*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200.......", -1, 0, 0, "" },
        /*620*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200........", -1, 0, 0, "" },
        /*621*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200.........", -1, 0, 0, "" },
        /*622*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200..........", -1, 0, 0, "" },
        /*623*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200...........", -1, 0, 0, "" },
        /*624*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200............", -1, 0, 0, "" },
        /*625*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200.............", -1, 0, 0, "" },
        /*626*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200.", -1, 0, 0, "ASC last 1" },
        /*627*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200..", -1, 0, 0, "ASC 2" },
        /*628*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200...", -1, 0, 0, "ASC 3" },
        /*629*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200....", -1, 0, 0, "ASC 4" },
        /*630*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200.....", -1, 0, 0, "ASC 5" },
        /*631*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200......", -1, 0, 0, "ASC 6" },
        /*632*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200.......", -1, 0, 0, "ASC 7" },
        /*633*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200........", -1, 0, 0, "EDI 8" },
        /*634*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200.........", -1, 0, 0, "EDI" },
        /*635*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200..........", -1, 0, 1, "BBBAAEEEEEEEE" },
        /*636*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200...........", -1, 0, 0, "EDI" },
        /*637*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200............", -1, 0, 0, "EDI" },
        /*638*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200.............", -1, 0, 0, "EDI except last ASC" },
        /*639*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200A\200\200\200", -1, 0, 0, "BAS" },
        /*640*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AA\200\200\200", -1, 0, 0, "BAS" },
        /*641*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAA\200\200\200", -1, 0, 0, "BAS" },
        /*642*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAA\200\200\200", -1, 0, 0, "BAS" },
        /*643*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAA\200\200\200", -1, 0, 0, "BAS" },
        /*644*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAA\200\200\200", -1, 0, 0, "BAS" },
        /*645*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAA\200\200\200", -1, 0, 0, "BAS 7 As, no switch as C40 & X12 cancel each other out" },
        /*646*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAA\200\200\200", -1, 0, 0, "BAS 8 As, no switch as C40 & X12 cancel each other out" },
        /*647*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAA\200\200\200", -1, 0, 0, "BAS 9 As, no switch as C40 & X12 cancel each other out" },
        /*648*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAA\200\200\200", -1, 0, 0, "BAS 10 As, no switch as C40 & X12 cancel each other out" },
        /*649*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAA\200\200\200", -1, 0, 0, "BAS 11 As, no switch as C40 & X12 cancel each other out" },
        /*650*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAAA\200\200\200", -1, 0, 0, "BAS 12 As, no switch as C40 & X12 cancel each other out" },
        /*651*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAAAA\200\200\200", -1, 0, 0, "BAS 13 As, no switch as C40 & X12 cancel each other out" },
        /*652*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009\200\200\200", -1, 0, 0, "BAS" },
        /*653*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099\200\200\200", -1, 0, 0, "BAS" },
        /*654*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999\200\200\200", -1, 0, 0, "BAS" },
        /*655*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999\200\200\200", -1, 0, 0, "ASC 4" },
        /*656*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999\200\200\200", -1, 0, 0, "ASC 4" },
        /*657*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999\200\200\200", -1, 0, 0, "ASC 6" },
        /*658*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999\200\200\200", -1, 0, 0, "ASC 6" },
        /*659*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999\200\200\200", -1, 0, 0, "ASC 8" },
        /*660*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999\200\200\200", -1, 0, 0, "ASC 8" },
        /*661*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999\200\200\200", -1, 0, 0, "ASC 10" },
        /*662*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999999\200\200\200", -1, 0, 0, "ASC 10" },
        /*663*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999999\200\200\200", -1, 0, 0, "ASC 12" },
        /*664*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999999\200\200\200", -1, 0, 0, "ASC 12" },
        /*665*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200a\200\200\200", -1, 0, 0, "BAS" },
        /*666*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aa\200\200\200", -1, 0, 0, "BAS" },
        /*667*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaa\200\200\200", -1, 0, 0, "BAS" },
        /*668*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaa\200\200\200", -1, 0, 0, "BAS" },
        /*669*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaa\200\200\200", -1, 0, 0, "BAS" },
        /*670*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaa\200\200\200", -1, 0, 0, "BAS" },
        /*671*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa\200\200\200", -1, 0, 0, "7 a's, was worse before dm_text_sp_cnt()" },
        /*672*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa\200\200", -1, 0, 0, "7 a's end 2 extended, was worse before dm_text_sp_cnt()" },
        /*673*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa\200", -1, 0, 0, "7 a's end 1 extended, was worse before dm_text_sp_cnt()" },
        /*674*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa", -1, 0, 0, "7 a's end 0 extended, switches to TEX but same codeword count" },
        /*675*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200\200aaaaaaa\200\200", -1, 0, 0, "7 a's, was worse before dm_text_sp_cnt()" },
        /*676*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200\200\200aaaaaaa\200\200", -1, 0, 0, "7 a's, was worse before dm_text_sp_cnt()" },
        /*677*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa\200\200\200\200", -1, 0, 0, "7 a's end 4 extended, was worse before dm_text_sp_cnt()" },
        /*678*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa\200\200\200\200\200", -1, 0, 0, "7 a's end 5 extended, was worse before dm_text_sp_cnt()" },
        /*679*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaa\200\200", -1, 0, 0, "8 a's end 2 extended, was worse before dm_text_sp_cnt()" },
        /*680*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaa\200\200\200", -1, 0, 0, "8 a's, was worse before dm_text_sp_cnt()" },
        /*681*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaa\200\200\200", -1, 0, 0, "9 a's, was worse before dm_text_sp_cnt()" },
        /*682*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaa\200\200\200", -1, 0, 0, "10 a's, was worse before dm_text_sp_cnt()" },
        /*683*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaa\200\200\200", -1, 0, 0, "11 a's, was worse before dm_text_sp_cnt()" },
        /*684*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaa\200\200\200", -1, 0, 0, "12 a's, switches to TEX but same codeword count" },
        /*685*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaa\200\200\200", -1, 0, 0, "13 a's, switches to TEX but same codeword count" },
        /*686*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaa\200\200\200", -1, 0, 0, "14 a's, switches to TEX but same codeword count" },
        /*687*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaa\200\200\200", -1, 0, 0, "15 a's, switches to TEX, better" },
        /*688*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaa\200\200\200", -1, 0, 0, "16 a's, switches to TEX, better" },
        /*689*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaaa\200\200\200", -1, 0, 0, "17 a's, switches to TEX, better" },
        /*690*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaaaa\200\200\200", -1, 0, 0, "18 a's, switches to TEX, better" },
        /*691*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaaaaa\200\200\200", -1, 0, 0, "19 a's, switches to TEX, better" },
        /*692*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaaaaaa\200\200\200", -1, 0, 0, "20 a's, switches to TEX, better" },
        /*693*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaaaaaaa\200\200\200", -1, 0, 0, "21 a's, switches to TEX, better" },
        /*694*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200Aaaaaaa\200\200\200", -1, 0, 0, "" },
        /*695*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaAaaa\200\200\200", -1, 0, 0, "" },
        /*696*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaA\200\200\200", -1, 0, 0, "" },
        /*697*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200Aaaaaaaa\200\200\200", -1, 0, 0, "" },
        /*698*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaAaaa\200\200\200", -1, 0, 0, "" },
        /*699*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaAa\200\200\200", -1, 0, 0, "" },
        /*700*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaA\200\200\200", -1, 0, 0, "Was worse before dm_text_sp_cnt()" },
        /*701*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaAa\200\200\200", -1, 0, 0, "Was worse before dm_text_sp_cnt()" },
        /*702*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>\200\200\200", -1, 0, 0, "BAS" },
        /*703*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>\200\200\200", -1, 0, 0, "BAS" },
        /*704*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>\200\200\200", -1, 0, 0, "BAS" },
        /*705*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*706*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*707*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*708*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*709*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*710*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*711*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*712*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*713*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*714*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>\200\200\200", -1, 0, 0, "X12 12" },
        /*715*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>\200\200\200", -1, 0, 0, "X12 12" },
        /*716*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>\200\200\200", -1, 0, 1, "X12 12" },
        /*717*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 0, "X12 15" },
        /*718*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 0, "X12 15" },
        /*719*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 1, "X12 15" },
        /*720*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 0, "X12 18" },
        /*721*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 0, "X12 18" },
        /*722*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 1, "X12 18" },
        /*723*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 0, "X12 21" },
        /*724*/ { BARCODE_DATAMATRIX, -1, -1, -1, "Aa", -1, 0, 0, "" },
        /*725*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAa", -1, 0, 0, "" },
        /*726*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAa", -1, 0, 0, "" },
        /*727*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAa", -1, 0, 0, "" },
        /*728*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAa", -1, 0, 0, "" },
        /*729*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAa", -1, 0, 0, "" },
        /*730*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAa", -1, 0, 0, "" },
        /*731*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAa", -1, 0, 1, "" },
        /*732*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAa", -1, 0, 0, "" },
        /*733*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAa", -1, 0, 0, "" },
        /*734*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAa", -1, 0, 1, "" },
        /*735*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAa", -1, 0, 0, "" },
        /*736*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAa", -1, 0, 0, "" },
        /*737*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAAa", -1, 0, 1, "" },
        /*738*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAAAa", -1, 0, 0, "" },
        /*739*/ { BARCODE_DATAMATRIX, -1, -1, -1, "A12", -1, 0, 0, "" },
        /*740*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AA12", -1, 0, 0, "" },
        /*741*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAA12", -1, 0, 0, "" },
        /*742*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAA12", -1, 0, 0, "" },
        /*743*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAA12", -1, 0, 0, "" },
        /*744*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAA12", -1, 0, 0, "" },
        /*745*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAA12", -1, 0, 0, "" },
        /*746*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAA12", -1, 0, 0, "" },
        /*747*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAA12", -1, 0, 0, "" },
        /*748*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAA12", -1, 0, 0, "" },
        /*749*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAA12", -1, 0, 0, "" },
        /*750*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAA12", -1, 0, 0, "" },
        /*751*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAA12", -1, 0, 0, "" },
        /*752*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAA12", -1, 0, 0, "" },
        /*753*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAAA12", -1, 0, 0, "" },
        /*754*/ { BARCODE_DATAMATRIX, -1, -1, -1, "A12a", -1, 0, 0, "" },
        /*755*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AA12a", -1, 0, 0, "" },
        /*756*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAA12a", -1, 0, 0, "" },
        /*757*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAA12a", -1, 0, 0, "" },
        /*758*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAA12a", -1, 0, 0, "" },
        /*759*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAA12a", -1, 0, 0, "" },
        /*760*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAA12a", -1, 0, 0, "" },
        /*761*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAA12a", -1, 0, 0, "" },
        /*762*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAA12a", -1, 0, 0, "" },
        /*763*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAA12a", -1, 0, 0, "" },
        /*764*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAA12a", -1, 0, 0, "" },
        /*765*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAA12a", -1, 0, 0, "" },
        /*766*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAA12a", -1, 0, 0, "" },
        /*767*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAA12a", -1, 0, 0, "" },
        /*768*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAAA12a", -1, 0, 0, "" },
        /*769*/ { BARCODE_DATAMATRIX, -1, -1, -1, "A1212", -1, 0, 0, "" },
        /*770*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AA1212", -1, 0, 0, "" },
        /*771*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAA1212", -1, 0, 0, "" },
        /*772*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAA1212", -1, 0, 0, "" },
        /*773*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAA1212", -1, 0, 0, "" },
        /*774*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAA1212", -1, 0, 0, "" },
        /*775*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAA1212", -1, 0, 0, "" },
        /*776*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAA1212", -1, 0, 0, "" },
        /*777*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAA1212", -1, 0, 0, "" },
        /*778*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAA1212", -1, 0, 0, "" },
        /*779*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAA1212", -1, 0, 0, "" },
        /*780*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAA1212", -1, 0, 0, "" },
        /*781*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAA1212", -1, 0, 0, "" },
        /*782*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAA1212", -1, 0, 0, "" },
        /*783*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAAA1212", -1, 0, 0, "" },
        /*784*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".a", -1, 0, 0, "" },
        /*785*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..a", -1, 0, 0, "" },
        /*786*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...a", -1, 0, 0, "" },
        /*787*/ { BARCODE_DATAMATRIX, -1, -1, -1, "....a", -1, 0, 0, "" },
        /*788*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".....a", -1, 0, 0, "" },
        /*789*/ { BARCODE_DATAMATRIX, -1, -1, -1, "......a", -1, 0, 0, "" },
        /*790*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".......a", -1, 0, 0, "" },
        /*791*/ { BARCODE_DATAMATRIX, -1, -1, -1, "........a", -1, 0, 1, "" },
        /*792*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".........a", -1, 0, 1, "" },
        /*793*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..........a", -1, 0, 1, "" },
        /*794*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...........a", -1, 0, 0, "" },
        /*795*/ { BARCODE_DATAMATRIX, -1, -1, -1, "............a", -1, 0, 1, "" },
        /*796*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".............a", -1, 0, 1, "" },
        /*797*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..............a", -1, 0, 0, "" },
        /*798*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...............a", -1, 0, 0, "" },
        /*799*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".12", -1, 0, 0, "" },
        /*800*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..12", -1, 0, 0, "" },
        /*801*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...12", -1, 0, 0, "" },
        /*802*/ { BARCODE_DATAMATRIX, -1, -1, -1, "....12", -1, 0, 0, "" },
        /*803*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".....12", -1, 0, 0, "" },
        /*804*/ { BARCODE_DATAMATRIX, -1, -1, -1, "......12", -1, 0, 0, "" },
        /*805*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".......12", -1, 0, 0, "" },
        /*806*/ { BARCODE_DATAMATRIX, -1, -1, -1, "........12", -1, 0, 1, "" },
        /*807*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".........12", -1, 0, 1, "" },
        /*808*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..........12", -1, 0, 0, "" },
        /*809*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...........12", -1, 0, 0, "" },
        /*810*/ { BARCODE_DATAMATRIX, -1, -1, -1, "............12", -1, 0, 1, "" },
        /*811*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".............12", -1, 0, 1, "" },
        /*812*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..............12", -1, 0, 0, "" },
        /*813*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...............12", -1, 0, 0, "" },
        /*814*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".\200", -1, 0, 0, "" },
        /*815*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..\200", -1, 0, 0, "" },
        /*816*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...\200", -1, 0, 1, "" },
        /*817*/ { BARCODE_DATAMATRIX, -1, -1, -1, "....\200", -1, 0, 1, "" },
        /*818*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".....\200", -1, 0, 1, "" },
        /*819*/ { BARCODE_DATAMATRIX, -1, -1, -1, "......\200", -1, 0, 1, "" },
        /*820*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".......\200", -1, 0, 1, "" },
        /*821*/ { BARCODE_DATAMATRIX, -1, -1, -1, "........\200", -1, 0, 1, "" },
        /*822*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".........\200", -1, 0, 0, "" },
        /*823*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..........\200", -1, 0, 1, "" },
        /*824*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...........\200", -1, 0, 0, "" },
        /*825*/ { BARCODE_DATAMATRIX, -1, -1, -1, "............\200", -1, 0, 0, "" },
        /*826*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".............\200", -1, 0, 0, "" },
        /*827*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..............\200", -1, 0, 1, "" },
        /*828*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...............\200", -1, 0, 0, "" },
        /*829*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".aa", -1, 0, 0, "" },
        /*830*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..aa", -1, 0, 0, "" },
        /*831*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...aa", -1, 0, 0, "" },
        /*832*/ { BARCODE_DATAMATRIX, -1, -1, -1, "....aa", -1, 0, 0, "" },
        /*833*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".....aa", -1, 0, 0, "" },
        /*834*/ { BARCODE_DATAMATRIX, -1, -1, -1, "......aa", -1, 0, 0, "" },
        /*835*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".......aa", -1, 0, 0, "" },
        /*836*/ { BARCODE_DATAMATRIX, -1, -1, -1, "........aa", -1, 0, 0, "" },
        /*837*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".........aa", -1, 0, 1, "" },
        /*838*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..........aa", -1, 0, 0, "" },
        /*839*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...........aa", -1, 0, 0, "" },
        /*840*/ { BARCODE_DATAMATRIX, -1, -1, -1, "............aa", -1, 0, 1, "" },
        /*841*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".............aa", -1, 0, 0, "" },
        /*842*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..............aa", -1, 0, 0, "" },
        /*843*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...............aa", -1, 0, 0, "" },
        /*844*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".12a", -1, 0, 0, "" },
        /*845*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..12a", -1, 0, 0, "" },
        /*846*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...12a", -1, 0, 0, "" },
        /*847*/ { BARCODE_DATAMATRIX, -1, -1, -1, "....12a", -1, 0, 0, "" },
        /*848*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".....12a", -1, 0, 0, "" },
        /*849*/ { BARCODE_DATAMATRIX, -1, -1, -1, "......12a", -1, 0, 0, "" },
        /*850*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".......12a", -1, 0, 0, "" },
        /*851*/ { BARCODE_DATAMATRIX, -1, -1, -1, "........12a", -1, 0, 0, "" },
        /*852*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".........12a", -1, 0, 1, "" },
        /*853*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..........12a", -1, 0, 1, "" },
        /*854*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...........12a", -1, 0, 0, "" },
        /*855*/ { BARCODE_DATAMATRIX, -1, -1, -1, "............12a", -1, 0, 1, "" },
        /*856*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".............12a", -1, 0, 0, "" },
        /*857*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..............12a", -1, 0, 0, "" },
        /*858*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...............12a", -1, 0, 0, "" },
        /*859*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".a12", -1, 0, 0, "" },
        /*860*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..a12", -1, 0, 0, "" },
        /*861*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...a12", -1, 0, 0, "" },
        /*862*/ { BARCODE_DATAMATRIX, -1, -1, -1, "....a12", -1, 0, 0, "" },
        /*863*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".....a12", -1, 0, 0, "" },
        /*864*/ { BARCODE_DATAMATRIX, -1, -1, -1, "......a12", -1, 0, 0, "" },
        /*865*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".......a12", -1, 0, 0, "" },
        /*866*/ { BARCODE_DATAMATRIX, -1, -1, -1, "........a12", -1, 0, 0, "" },
        /*867*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".........a12", -1, 0, 1, "" },
        /*868*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..........a12", -1, 0, 0, "" },
        /*869*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...........a12", -1, 0, 0, "" },
        /*870*/ { BARCODE_DATAMATRIX, -1, -1, -1, "............a12", -1, 0, 1, "" },
        /*871*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".............a12", -1, 0, 0, "" },
        /*872*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..............a12", -1, 0, 0, "" },
        /*873*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...............a12", -1, 0, 0, "" },
        /*874*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".1212", -1, 0, 0, "" },
        /*875*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..1212", -1, 0, 0, "" },
        /*876*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...1212", -1, 0, 0, "" },
        /*877*/ { BARCODE_DATAMATRIX, -1, -1, -1, "....1212", -1, 0, 0, "" },
        /*878*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".....1212", -1, 0, 0, "" },
        /*879*/ { BARCODE_DATAMATRIX, -1, -1, -1, "......1212", -1, 0, 0, "" },
        /*880*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".......1212", -1, 0, 0, "" },
        /*881*/ { BARCODE_DATAMATRIX, -1, -1, -1, "........1212", -1, 0, 0, "" },
        /*882*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".........1212", -1, 0, 1, "" },
        /*883*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..........1212", -1, 0, 1, "" },
        /*884*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...........1212", -1, 0, 0, "" },
        /*885*/ { BARCODE_DATAMATRIX, -1, -1, -1, "............1212", -1, 0, 1, "" },
        /*886*/ { BARCODE_DATAMATRIX, -1, -1, -1, ".............1212", -1, 0, 0, "" },
        /*887*/ { BARCODE_DATAMATRIX, -1, -1, -1, "..............1212", -1, 0, 0, "" },
        /*888*/ { BARCODE_DATAMATRIX, -1, -1, -1, "...............1212", -1, 0, 0, "" },
        /*889*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">a", -1, 0, 0, "" },
        /*890*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>a", -1, 0, 0, "" },
        /*891*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>a", -1, 0, 0, "" },
        /*892*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>a", -1, 0, 0, "" },
        /*893*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>a", -1, 0, 0, "" },
        /*894*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>a", -1, 0, 0, "" },
        /*895*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>a", -1, 0, 0, "" },
        /*896*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>a", -1, 0, 1, "" },
        /*897*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>a", -1, 0, 1, "" },
        /*898*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>a", -1, 0, 1, "" },
        /*899*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>a", -1, 0, 1, "" },
        /*900*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>a", -1, 0, 2, "" },
        /*901*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>a", -1, 0, 0, "" },
        /*902*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>a", -1, 0, 0, "" },
        /*903*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>>a", -1, 0, 1, "" },
        /*904*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">12", -1, 0, 0, "" },
        /*905*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>12", -1, 0, 0, "" },
        /*906*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>12", -1, 0, 0, "" },
        /*907*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>12", -1, 0, 0, "" },
        /*908*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>12", -1, 0, 0, "" },
        /*909*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>12", -1, 0, 0, "" },
        /*910*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>12", -1, 0, 0, "" },
        /*911*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>12", -1, 0, 0, "" },
        /*912*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>12", -1, 0, 0, "" },
        /*913*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>12", -1, 0, 0, "" },
        /*914*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>12", -1, 0, 0, "" },
        /*915*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>12", -1, 0, 0, "" },
        /*916*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>12", -1, 0, 0, "" },
        /*917*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>12", -1, 0, 0, "" },
        /*918*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>>12", -1, 0, 0, "" },
        /*919*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">\200", -1, 0, 0, "" },
        /*920*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>\200", -1, 0, 0, "" },
        /*921*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>\200", -1, 0, 1, "" },
        /*922*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>\200", -1, 0, 1, "" },
        /*923*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>\200", -1, 0, 1, "" },
        /*924*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>\200", -1, 0, 1, "" },
        /*925*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>\200", -1, 0, 1, "" },
        /*926*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>\200", -1, 0, 1, "" },
        /*927*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>\200", -1, 0, 1, "" },
        /*928*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>\200", -1, 0, 2, "" },
        /*929*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>\200", -1, 0, 2, "" },
        /*930*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>\200", -1, 0, 2, "" },
        /*931*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>\200", -1, 0, 0, "" },
        /*932*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>\200", -1, 0, 0, "" },
        /*933*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>>\200", -1, 0, 1, "" },
        /*934*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">aa", -1, 0, 0, "" },
        /*935*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>aa", -1, 0, 0, "" },
        /*936*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>aa", -1, 0, 0, "" },
        /*937*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>aa", -1, 0, 0, "" },
        /*938*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>aa", -1, 0, 0, "" },
        /*939*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>aa", -1, 0, 0, "" },
        /*940*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>aa", -1, 0, 0, "" },
        /*941*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>aa", -1, 0, 0, "" },
        /*942*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>aa", -1, 0, 1, "" },
        /*943*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>aa", -1, 0, 1, "" },
        /*944*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>aa", -1, 0, 1, "" },
        /*945*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>aa", -1, 0, 2, "" },
        /*946*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>aa", -1, 0, 0, "" },
        /*947*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>aa", -1, 0, 0, "XXXXXXXXXXXXAAAA" },
        /*948*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>>aa", -1, 0, 0, "EEEEEEEEEEEEEEEAA" },
        /*949*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">12a", -1, 0, 0, "" },
        /*950*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>12a", -1, 0, 0, "" },
        /*951*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>12a", -1, 0, 0, "" },
        /*952*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>12a", -1, 0, 0, "" },
        /*953*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>12a", -1, 0, 0, "" },
        /*954*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>12a", -1, 0, 0, "" },
        /*955*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>12a", -1, 0, 0, "" },
        /*956*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>12a", -1, 0, 0, "" },
        /*957*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>12a", -1, 0, 1, "" },
        /*958*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>12a", -1, 0, 1, "" },
        /*959*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>12a", -1, 0, 0, "" },
        /*960*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>12a", -1, 0, 0, "" },
        /*961*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>12a", -1, 0, 0, "" },
        /*962*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>12a", -1, 0, 0, "" },
        /*963*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>>12a", -1, 0, 0, "" },
        /*964*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">a12", -1, 0, 0, "" },
        /*965*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>a12", -1, 0, 0, "" },
        /*966*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>a12", -1, 0, 0, "" },
        /*967*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>a12", -1, 0, 0, "" },
        /*968*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>a12", -1, 0, 0, "" },
        /*969*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>a12", -1, 0, 0, "" },
        /*970*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>a12", -1, 0, 0, "" },
        /*971*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>a12", -1, 0, 0, "" },
        /*972*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>a12", -1, 0, 1, "" },
        /*973*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>a12", -1, 0, 1, "" },
        /*974*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>a12", -1, 0, 1, "" },
        /*975*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>a12", -1, 0, 2, "" },
        /*976*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>a12", -1, 0, 0, "" },
        /*977*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>a12", -1, 0, 0, "" },
        /*978*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>>a12", -1, 0, 1, "" },
        /*979*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">1212", -1, 0, 0, "" },
        /*980*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>1212", -1, 0, 0, "" },
        /*981*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>1212", -1, 0, 0, "" },
        /*982*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>1212", -1, 0, 0, "" },
        /*983*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>1212", -1, 0, 0, "" },
        /*984*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>1212", -1, 0, 0, "" },
        /*985*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>1212", -1, 0, 0, "" },
        /*986*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>1212", -1, 0, 0, "" },
        /*987*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>1212", -1, 0, 0, "" },
        /*988*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>1212", -1, 0, 0, "" },
        /*989*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>1212", -1, 0, 0, "" },
        /*990*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>1212", -1, 0, 0, "EEEEEEEEEEEEAAAA" },
        /*991*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>1212", -1, 0, 0, "XXXXXXXXXXXXAAAAA" },
        /*992*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>1212", -1, 0, 0, "XXXXXXXXXXXXAAAAAA" },
        /*993*/ { BARCODE_DATAMATRIX, -1, -1, -1, ">>>>>>>>>>>>>>>1212", -1, 0, 0, "" },
        /*994*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200|||||||||||||", -1, 0, 0, "Variant 1 last 3 ASC" },
        /*995*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAA|||||||||||AAAAAAAAAAAAAAA", -1, 0, 1, "" },
        /*996*/ { BARCODE_DATAMATRIX, -1, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM" "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK" "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG" "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567" "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde" "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO", -1, 0, 10, "960 chars, text/numeric" },
        /*997*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\000\004\a\b\t\n\v\f\r\033\035\036\201\\", 14, 0, 1, "From test_library escape_char_process 0, variant 1 later switch to B256 and changes to ASC for last 2 chars" },
        /*998*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAA*+*0**AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 0, 2, "" },
        /*999*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 0, 0, "" },
        /*1000*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037 !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\177", 128, 0, 0, "" },
        /*1001*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, "\200\200\200\035\200\200\035\200\200", -1, 0, 1, "Can't have extended ASCII in GS1 mode so these will never happen" },
        /*1002*/ { BARCODE_DATAMATRIX, GS1_MODE, GS1_GS_SEPARATOR, -1, "\200\200\200\035\200\200\035\200\200", -1, 0, 0, "" },
        /*1003*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, "\200\200\200\200\035\200\200\200\035\200\200", -1, 0, 1, "Stays in ASC after 1st FNC1" },
        /*1004*/ { BARCODE_DATAMATRIX, GS1_MODE, GS1_GS_SEPARATOR, -1, "\200\200\200\200\035\200\200\200\035\200\200", -1, 0, 0, "" },
        /*1005*/ { BARCODE_DATAMATRIX, -1, -1, -1, "https://example.com/01/09506000134369", -1, 0, 0, "" },
        /*1006*/ { BARCODE_DATAMATRIX, -1, -1, -1, "abcdefghi1234FGHIJKLMNabc@@@@@@@@@é", -1, 0, 0, "" },
        /*1007*/ { BARCODE_DATAMATRIX, -1, -1, -1, "ABCDEF", -1, 0, 0, "" },
        /*1008*/ { BARCODE_DATAMATRIX, -1, -1, -1, "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111" "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111", -1, 0, 0, "" },
        /*1009*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200@A1^B2?C", -1, 0, 0, "" },
        /*1010*/ { BARCODE_DATAMATRIX, -1, -1, -1, "@A1^@B2^@C3^\200\200\200", -1, 0, 0, "" },
        /*1011*/ { BARCODE_DATAMATRIX, -1, -1, -1, "@A1^@B2^@C3^@\200\200\200", -1, 0, 0, "" },
        /*1012*/ { BARCODE_DATAMATRIX, -1, -1, -1, "@A1^@B2^@C3^@D\200\200\200", -1, 0, 0, "" },
        /*1013*/ { BARCODE_DATAMATRIX, -1, -1, -1, "@A1^@B2^@C3^@D4\200\200\200", -1, 0, 0, "" },
        /*1014*/ { BARCODE_DATAMATRIX, -1, -1, -1, "@A1^@B2^@C3^@D4@\200\200\200", -1, 0, 0, "" },
        /*1015*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200@A1^B2?C\200\200\200", -1, 0, 0, "" },
        /*1016*/ { BARCODE_DATAMATRIX, -1, -1, -1, "@@@@@@@@@_", -1, 0, 1, "" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    unsigned char binary[2][2200];
    int gs1;
    int binlen;
    int binlens[2] = {0};

    testStartSymbol("test_minimalenc", &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, data[i].output_options, data[i].data, data[i].length, debug);

        binlen = 0;
        symbol->input_mode |= FAST_MODE;
        gs1 = (symbol->input_mode & 0x07) != GS1_MODE ? 0 : (symbol->output_options & GS1_GS_SEPARATOR) ? 2 : 1;
        ret = dm_encode_test(symbol, (unsigned char *) data[i].data, length, symbol->eci, gs1, binary[0], &binlen);
        assert_equal(ret, data[i].ret, "i:%d dm_encode() FAST_MODE ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        binlens[0] = binlen;

        binlen = 0;
        symbol->input_mode &= ~FAST_MODE;
        gs1 = (symbol->input_mode & 0x07) != GS1_MODE ? 0 : (symbol->output_options & GS1_GS_SEPARATOR) ? 2 : 1;
        ret = dm_encode_test(symbol, (unsigned char *) data[i].data, length, symbol->eci, gs1, binary[1], &binlen);
        assert_equal(ret, data[i].ret, "i:%d dm_encode() minimal ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        binlens[1] = binlen;

        fflush(stdout);
        assert_equal(binlens[0], binlens[1] + data[i].expected_diff, "i:%d binlens[0] %d != %d binlens[1] (%d) + expected_diff (%d)\n",
                    i, binlens[0], binlens[1] + data[i].expected_diff, binlens[1], data[i].expected_diff);

        if (debug & ZINT_DEBUG_TEST_PRINT) {
            if (data[i].expected_diff == 0) {
                if (memcmp(binary[0], binary[1], binlen) != 0) {
                    printf("i:%d binaries differ\n", i);
                }
            } else {
                printf("i:%d diff %d%s\n", i, data[i].expected_diff, data[i].expected_diff < 0 ? " ***worse***" : "");
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}
#endif

#include <time.h>

#if 1
#define TEST_PERF_ITER_MILLES   5
#define TEST_PERF_ITERATIONS    (TEST_PERF_ITER_MILLES * 1000)
#else
#define TEST_PERF_ITERATIONS    100 /* For valgrind */
#endif
#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)

/* Not a real test, just performance indicator */
static void test_perf(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int symbology;
        int input_mode;
        int option_2;
        int option_3;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
    };
    static const struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                    "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK"
                    "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG"
                    "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567"
                    "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde"
                    "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO",
                    0, 96, 96, "960 chars, text/numeric" },
        /*  1*/ { BARCODE_DATAMATRIX, -1, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                    "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK"
                    "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG"
                    "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567"
                    "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde"
                    "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO",
                    0, 96, 96, "960 chars, text/numeric" },
        /*  2*/ { BARCODE_DATAMATRIX, DATA_MODE | FAST_MODE, -1, -1,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    0, 120, 120, "960 chars, byte" },
        /*  3*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1,
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240"
                    "\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240\240",
                    0, 120, 120, "960 chars, byte" },
        /*  4*/ { BARCODE_DATAMATRIX, FAST_MODE, -1, -1, "https://example.com/01/09506000134369", 0, 22, 22, "37 chars, text/numeric" },
        /*  5*/ { BARCODE_DATAMATRIX, -1, -1, -1, "https://example.com/01/09506000134369", 0, 22, 22, "37 chars, text/numeric" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    clock_t start;
    clock_t total_create = 0, total_encode = 0, total_buffer = 0, total_buf_inter = 0, total_print = 0;
    clock_t diff_create, diff_encode, diff_buffer, diff_buf_inter, diff_print;
    int comment_max = 0;

    if (!(debug & ZINT_DEBUG_TEST_PERFORMANCE)) { /* -d 256 */
        return;
    }

    for (i = 0; i < data_size; i++) if ((int) strlen(data[i].comment) > comment_max) comment_max = (int) strlen(data[i].comment);

    printf("Iterations %d\n", TEST_PERF_ITERATIONS);

    for (i = 0; i < data_size; i++) {
        int j;

        if (testContinue(p_ctx, i)) continue;

        diff_create = diff_encode = diff_buffer = diff_buf_inter = diff_print = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            start = clock();
            symbol = ZBarcode_Create();
            diff_create += clock() - start;
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, data[i].option_3, -1 /*output_options*/, data[i].data, -1, debug);

            start = clock();
            ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
            diff_encode += clock() - start;
            assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n", i, symbol->rows, data[i].expected_rows, data[i].data);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n", i, symbol->width, data[i].expected_width, data[i].data);

            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buffer += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer ret %d != 0 (%s)\n", i, ret, symbol->errtxt);

            symbol->output_options |= OUT_BUFFER_INTERMEDIATE;
            start = clock();
            ret = ZBarcode_Buffer(symbol, 0 /*rotate_angle*/);
            diff_buf_inter += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Buffer OUT_BUFFER_INTERMEDIATE ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            symbol->output_options &= ~OUT_BUFFER_INTERMEDIATE; /* Undo */

            start = clock();
            ret = ZBarcode_Print(symbol, 0 /*rotate_angle*/);
            diff_print += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Print ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            assert_zero(testUtilRemove(symbol->outfile), "i:%d testUtilRemove(%s) != 0\n", i, symbol->outfile);

            ZBarcode_Delete(symbol);
        }

        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, data[i].comment,
                TEST_PERF_TIME(diff_encode), TEST_PERF_TIME(diff_buffer), TEST_PERF_TIME(diff_buf_inter), TEST_PERF_TIME(diff_print), TEST_PERF_TIME(diff_create));

        total_create += diff_create;
        total_encode += diff_encode;
        total_buffer += diff_buffer;
        total_buf_inter += diff_buf_inter;
        total_print += diff_print;
    }
    if (p_ctx->index == -1) {
        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_buf_inter), TEST_PERF_TIME(total_print), TEST_PERF_TIME(total_create));
    }
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_buffer", test_buffer },
        { "test_options", test_options },
        { "test_reader_init", test_reader_init },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_encode_segs", test_encode_segs },
#ifdef ZINT_TEST_ENCODING
        { "test_minimalenc", test_minimalenc },
#endif
        { "test_perf", test_perf },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
