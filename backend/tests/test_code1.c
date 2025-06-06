/*
    libzint - the open source barcode library
    Copyright (C) 2020-2025 Robin Stuart <rstuart114@gmail.com>

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

static void test_large(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int eci;
        int option_2;
        struct zint_structapp structapp;
        const char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
    };
    /* Reference AIM USS Code One Table 2 */
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { -1, -1, { 0, 0, "" }, "1", 3550, 0, 148, 134, "" }, /* Auto Version H */
        /*  1*/ { -1, -1, { 0, 0, "" }, "1", 3551, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /*  2*/ { 3, -1, { 0, 0, "" }, "1", 3535, 0, 148, 134, "" }, /* With ECI */
        /*  3*/ { 3, -1, { 0, 0, "" }, "1", 3536, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /*  4*/ { -1, -1, { 1, 2, "" }, "1", 3546, 0, 148, 134, "" }, /* With Structured Append (Group mode, count < 2) */
        /*  5*/ { -1, -1, { 1, 2, "" }, "1", 3547, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /*  6*/ { -1, -1, { 1, 16, "" }, "1", 3541, 0, 148, 134, "" }, /* With Structured Append (Extended Group mode, count >= 16) */
        /*  7*/ { -1, -1, { 1, 16, "" }, "1", 3542, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /*  8*/ { 3, -1, { 1, 2, "" }, "1", 3532, 0, 148, 134, "" }, /* With ECI and Structured Append (Group mode) 1st symbol */
        /*  9*/ { 3, -1, { 1, 2, "" }, "1", 3533, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /* 10*/ { 3, -1, { 2, 2, "" }, "1", 3537, 0, 148, 134, "" }, /* With ECI and Structured Append (Group mode) subsequent symbol */
        /* 11*/ { 3, -1, { 2, 2, "" }, "1", 3538, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /* 12*/ { 3, -1, { 1, 16, "" }, "1", 3530, 0, 148, 134, "" }, /* With ECI and Structured Append (Extended Group mode) 1st symbol */
        /* 13*/ { 3, -1, { 1, 16, "" }, "1", 3531, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /* 14*/ { 3, -1, { 2, 16, "" }, "1", 3535, 0, 148, 134, "" }, /* With ECI and Structured Append (Extended Group mode) subsequent symbol */
        /* 15*/ { 3, -1, { 2, 16, "" }, "1", 3536, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /* 16*/ { -1, -1, { 0, 0, "" }, "A", 2218, 0, 148, 134, "" },
        /* 17*/ { -1, -1, { 0, 0, "" }, "A", 2219, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /* 18*/ { -1, -1, { 0, 0, "" }, "\001", 1480, 0, 148, 134, "" }, /* Full ASCII */
        /* 19*/ { -1, -1, { 0, 0, "" }, "\001", 1481, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /* 20*/ { -1, -1, { 0, 0, "" }, "\200", 1478, 0, 148, 134, "" }, /* BYTE */
        /* 21*/ { -1, -1, { 0, 0, "" }, "\200", 1479, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /* 22*/ { -1, 1, { 0, 0, "" }, "1", 22, 0, 16, 18, "" }, /* Version A */
        /* 23*/ { -1, 1, { 0, 0, "" }, "1", 23, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version A, requires 11 codewords (maximum 10)" },
        /* 24*/ { -1, 1, { 0, 0, "" }, "A", 13, 0, 16, 18, "" },
        /* 25*/ { -1, 1, { 0, 0, "" }, "A", 14, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version A, requires 12 codewords (maximum 10)" },
        /* 26*/ { -1, 1, { 0, 0, "" }, "A", 2218, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version A, requires 1480 codewords (maximum 10)" },
        /* 27*/ { 3, 1, { 0, 0, "" }, "A", 4, 0, 16, 18, "" }, /* With ECI */
        /* 28*/ { 3, 1, { 0, 0, "" }, "A", 5, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version A, requires 11 codewords (maximum 10)" },
        /* 29*/ { -1, 1, { 1, 2, "" }, "A", 10, 0, 16, 18, "" }, /* With Structured Append */
        /* 30*/ { -1, 1, { 1, 2, "" }, "A", 11, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version A, requires 12 codewords (maximum 10)" },
        /* 31*/ { 3, 1, { 1, 2, "" }, "A", 2, 0, 16, 18, "" }, /* With ECI and Structured Append 1st symbol */
        /* 32*/ { 3, 1, { 1, 2, "" }, "A", 3, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version A, requires 11 codewords (maximum 10)" },
        /* 33*/ { 3, 1, { 2, 2, "" }, "A", 4, 0, 16, 18, "" }, /* With ECI and Structured Append subsequent symbol */
        /* 34*/ { 3, 1, { 2, 2, "" }, "A", 5, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version A, requires 11 codewords (maximum 10)" },
        /* 35*/ { -1, 1, { 0, 0, "" }, "\001", 10, 0, 16, 18, "" },
        /* 36*/ { -1, 1, { 0, 0, "" }, "\001", 11, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version A, requires 11 codewords (maximum 10)" },
        /* 37*/ { -1, 1, { 0, 0, "" }, "\200", 8, 0, 16, 18, "" },
        /* 38*/ { -1, 1, { 0, 0, "" }, "\200", 9, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version A, requires 11 codewords (maximum 10)" },
        /* 39*/ { -1, 2, { 0, 0, "" }, "1", 44, 0, 22, 22, "" }, /* Version B */
        /* 40*/ { -1, 2, { 0, 0, "" }, "1", 45, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version B, requires 20 codewords (maximum 19)" },
        /* 41*/ { -1, 2, { 0, 0, "" }, "A", 27, 0, 22, 22, "" },
        /* 42*/ { -1, 2, { 0, 0, "" }, "A", 28, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version B, requires 21 codewords (maximum 19)" },
        /* 43*/ { -1, 2, { 0, 0, "" }, "A", 26, 0, 22, 22, "" },
        /* 44*/ { -1, 2, { 0, 0, "" }, "\001", 19, 0, 22, 22, "" },
        /* 45*/ { -1, 2, { 0, 0, "" }, "\001", 20, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version B, requires 20 codewords (maximum 19)" },
        /* 46*/ { -1, 2, { 0, 0, "" }, "\200", 17, 0, 22, 22, "" },
        /* 47*/ { -1, 2, { 0, 0, "" }, "\200", 18, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version B, requires 20 codewords (maximum 19)" },
        /* 48*/ { -1, 3, { 0, 0, "" }, "1", 104, 0, 28, 32, "" }, /* Version C */
        /* 49*/ { -1, 3, { 0, 0, "" }, "1", 105, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version C, requires 45 codewords (maximum 44)" },
        /* 50*/ { -1, 3, { 0, 0, "" }, "A", 64, 0, 28, 32, "" },
        /* 51*/ { -1, 3, { 0, 0, "" }, "A", 65, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version C, requires 46 codewords (maximum 44)" },
        /* 52*/ { -1, 3, { 0, 0, "" }, "\001", 44, 0, 28, 32, "" },
        /* 53*/ { -1, 3, { 0, 0, "" }, "\001", 45, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version C, requires 45 codewords (maximum 44)" },
        /* 54*/ { -1, 3, { 0, 0, "" }, "\200", 42, 0, 28, 32, "" },
        /* 55*/ { -1, 3, { 0, 0, "" }, "\200", 43, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version C, requires 45 codewords (maximum 44)" },
        /* 56*/ { -1, 4, { 0, 0, "" }, "1", 217, 0, 40, 42, "" }, /* Version D */
        /* 57*/ { -1, 4, { 0, 0, "" }, "1", 218, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version D, requires 93 codewords (maximum 91)" },
        /* 58*/ { -1, 4, { 0, 0, "" }, "A", 135, 0, 40, 42, "" },
        /* 59*/ { -1, 4, { 0, 0, "" }, "A", 136, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version D, requires 93 codewords (maximum 91)" },
        /* 60*/ { -1, 4, { 0, 0, "" }, "\001", 91, 0, 40, 42, "" },
        /* 61*/ { -1, 4, { 0, 0, "" }, "\001", 92, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version D, requires 92 codewords (maximum 91)" },
        /* 62*/ { -1, 4, { 0, 0, "" }, "\200", 89, 0, 40, 42, "" },
        /* 63*/ { -1, 4, { 0, 0, "" }, "\200", 90, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version D, requires 92 codewords (maximum 91)" },
        /* 64*/ { -1, 5, { 0, 0, "" }, "1", 435, 0, 52, 54, "" }, /* Version E (note 435 multiple of 3) */
        /* 65*/ { -1, 5, { 0, 0, "" }, "1", 436, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version E, requires 183 codewords (maximum 182)" },
        /* 66*/ { -1, 5, { 0, 0, "" }, "1", 434, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version E, requires 183 codewords (maximum 182)" }, /* NOTE: a quirk of decimal end-of-data processing is existence of "lower maxs" if digits are not a multiple of 3 */
        /* 67*/ { -1, 5, { 0, 0, "" }, "1", 433, 0, 52, 54, "" },
        /* 68*/ { -1, 5, { 0, 0, "" }, "A", 271, 0, 52, 54, "" },
        /* 69*/ { -1, 5, { 0, 0, "" }, "A", 272, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version E, requires 184 codewords (maximum 182)" },
        /* 70*/ { -1, 5, { 0, 0, "" }, "\001", 182, 0, 52, 54, "" },
        /* 71*/ { -1, 5, { 0, 0, "" }, "\001", 183, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version E, requires 183 codewords (maximum 182)" },
        /* 72*/ { -1, 5, { 0, 0, "" }, "\200", 180, 0, 52, 54, "" },
        /* 73*/ { -1, 5, { 0, 0, "" }, "\200", 181, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version E, requires 183 codewords (maximum 182)" },
        /* 74*/ { -1, 6, { 0, 0, "" }, "1", 886, 0, 70, 76, "" }, /* Version F */
        /* 75*/ { -1, 6, { 0, 0, "" }, "1", 887, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version F, requires 371 codewords (maximum 370)" },
        /* 76*/ { -1, 6, { 0, 0, "" }, "A", 553, 0, 70, 76, "" },
        /* 77*/ { -1, 6, { 0, 0, "" }, "A", 554, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version F, requires 372 codewords (maximum 370)" },
        /* 78*/ { -1, 6, { 0, 0, "" }, "\001", 370, 0, 70, 76, "" },
        /* 79*/ { -1, 6, { 0, 0, "" }, "\001", 371, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version F, requires 371 codewords (maximum 370)" },
        /* 80*/ { -1, 6, { 0, 0, "" }, "\200", 368, 0, 70, 76, "" },
        /* 81*/ { -1, 6, { 0, 0, "" }, "\200", 369, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version F, requires 372 codewords (maximum 370)" },
        /* 82*/ { -1, 7, { 0, 0, "" }, "1", 1755, 0, 104, 98, "" }, /* Version G (note 1755 multiple of 3) */
        /* 83*/ { -1, 7, { 0, 0, "" }, "1", 1756, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version G, requires 733 codewords (maximum 732)" },
        /* 84*/ { -1, 7, { 0, 0, "" }, "1", 1754, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version G, requires 733 codewords (maximum 732)" }, /* NOTE: a quirk of decimal end-of-data processing is existence of "lower maxs" if digits are not a multiple of 3 */
        /* 85*/ { -1, 7, { 0, 0, "" }, "1", 1753, 0, 104, 98, "" },
        /* 86*/ { -1, 7, { 0, 0, "" }, "A", 1096, 0, 104, 98, "" },
        /* 87*/ { -1, 7, { 0, 0, "" }, "A", 1097, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version G, requires 734 codewords (maximum 732)" },
        /* 88*/ { -1, 7, { 0, 0, "" }, "\001", 732, 0, 104, 98, "" },
        /* 89*/ { -1, 7, { 0, 0, "" }, "\001", 733, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version G, requires 733 codewords (maximum 732)" },
        /* 90*/ { -1, 7, { 0, 0, "" }, "\200", 730, 0, 104, 98, "" },
        /* 91*/ { -1, 7, { 0, 0, "" }, "\200", 731, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version G, requires 734 codewords (maximum 732)" },
        /* 92*/ { -1, 7, { 0, 0, "" }, "\200", 732, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version G, requires 735 codewords (maximum 732)" },
        /* 93*/ { -1, 7, { 0, 0, "" }, "\200", 1478, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version G, requires 1480 codewords (maximum 732)" },
        /* 94*/ { -1, 8, { 0, 0, "" }, "1", 3550, 0, 148, 134, "" }, /* Version H */
        /* 95*/ { -1, 8, { 0, 0, "" }, "1", 3551, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /* 96*/ { -1, 8, { 0, 0, "" }, "A", 2218, 0, 148, 134, "" },
        /* 97*/ { -1, 8, { 0, 0, "" }, "A", 2219, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /* 98*/ { -1, 8, { 0, 0, "" }, "\001", 1480, 0, 148, 134, "" },
        /* 99*/ { -1, 8, { 0, 0, "" }, "\001", 1481, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /*100*/ { -1, 8, { 0, 0, "" }, "\200", 1478, 0, 148, 134, "" },
        /*101*/ { -1, 8, { 0, 0, "" }, "\200", 1479, ZINT_ERROR_TOO_LONG, -1, -1, "Error 517: Input too long, requires too many codewords (maximum 1480)" },
        /*102*/ { -1, 9, { 0, 0, "" }, "1", 6, 0, 8, 11, "" }, /* Version S-10 */
        /*103*/ { -1, 9, { 0, 0, "" }, "1", 7, 0, 8, 21, "" }, /* -> S-20 */
        /*104*/ { -1, 9, { 0, 0, "" }, "1", 12, 0, 8, 21, "" }, /* Version S-20 */
        /*105*/ { -1, 9, { 0, 0, "" }, "1", 13, 0, 8, 31, "" }, /* -> S-30 */
        /*106*/ { -1, 9, { 0, 0, "" }, "1", 18, 0, 8, 31, "" }, /* Version S-30 */
        /*107*/ { -1, 9, { 0, 0, "" }, "1", 19, ZINT_ERROR_TOO_LONG, -1, -1, "Error 514: Input length 19 too long for Version S (maximum 18)" },
        /*108*/ { -1, 9, { 0, 0, "" }, "1", 17, 0, 8, 31, "" },
        /*109*/ { -1, 10, { 0, 0, "" }, "1", 22, 0, 16, 17, "" }, /* Version T-16 */
        /*110*/ { -1, 10, { 0, 0, "" }, "1", 23, 0, 16, 33, "" }, /* -> T-32 */
        /*111*/ { -1, 10, { 0, 0, "" }, "A", 13, 0, 16, 17, "" },
        /*112*/ { -1, 10, { 0, 0, "" }, "A", 14, 0, 16, 33, "" }, /* -> T-32 */
        /*113*/ { -1, 10, { 0, 0, "" }, "\001", 10, 0, 16, 17, "" },
        /*114*/ { -1, 10, { 0, 0, "" }, "\001", 11, 0, 16, 33, "" }, /* -> T-32 */
        /*115*/ { -1, 10, { 0, 0, "" }, "\200", 8, 0, 16, 17, "" },
        /*116*/ { -1, 10, { 0, 0, "" }, "\200", 9, 0, 16, 33, "" }, /* -> T-32 */
        /*117*/ { -1, 10, { 0, 0, "" }, "1", 56, 0, 16, 33, "" }, /* Version T-32 */
        /*118*/ { -1, 10, { 0, 0, "" }, "1", 57, 0, 16, 49, "" }, /* -> T-48 */
        /*119*/ { -1, 10, { 0, 0, "" }, "A", 34, 0, 16, 33, "" },
        /*120*/ { -1, 10, { 0, 0, "" }, "A", 35, 0, 16, 49, "" }, /* -> T-48 */
        /*121*/ { -1, 10, { 0, 0, "" }, "\001", 24, 0, 16, 33, "" },
        /*122*/ { -1, 10, { 0, 0, "" }, "\001", 25, 0, 16, 49, "" }, /* -> T-48 */
        /*123*/ { -1, 10, { 0, 0, "" }, "\200", 22, 0, 16, 33, "" },
        /*124*/ { -1, 10, { 0, 0, "" }, "\200", 23, 0, 16, 49, "" }, /* -> T-48 */
        /*125*/ { -1, 10, { 0, 0, "" }, "1", 90, 0, 16, 49, "" }, /* Version T-48 (note 90 multiple of 3) */
        /*126*/ { -1, 10, { 0, 0, "" }, "1", 91, ZINT_ERROR_TOO_LONG, -1, -1, "Error 519: Input length 91 too long for Version T (maximum 90)" },
        /*127*/ { -1, 10, { 0, 0, "" }, "1", 89, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 39 codewords (maximum 38)" }, /* NOTE: a quirk of decimal end-of-data processing is existence of "lower maxs" if digits are not a multiple of 3 */
        /*128*/ { -1, 10, { 0, 0, "" }, "1", 88, 0, 16, 49, "" },
        /*129*/ { -1, 10, { 0, 0, "" }, "A", 55, 0, 16, 49, "" },
        /*130*/ { -1, 10, { 0, 0, "" }, "A", 56, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 40 codewords (maximum 38)" },
        /*131*/ { -1, 10, { 0, 0, "" }, "A", 90, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 61 codewords (maximum 38)" },
        /*132*/ { -1, 10, { 0, 0, "" }, "\001", 38, 0, 16, 49, "" },
        /*133*/ { -1, 10, { 0, 0, "" }, "\001", 39, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 39 codewords (maximum 38)" },
        /*134*/ { -1, 10, { 0, 0, "" }, "\001", 90, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 90 codewords (maximum 38)" },
        /*135*/ { -1, 10, { 0, 0, "" }, "\\", 38, 0, 16, 49, "" },
        /*136*/ { -1, 10, { 0, 0, "" }, "\\", 39, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 39 codewords (maximum 38)" },
        /*137*/ { -1, 10, { 0, 0, "" }, "\200", 36, 0, 16, 49, "" },
        /*138*/ { -1, 10, { 0, 0, "" }, "\200", 37, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 39 codewords (maximum 38)" },
        /*139*/ { -1, 10, { 0, 0, "" }, "AAA\200", 31, 0, 16, 49, "" }, /* ASCII + BYTE (ASCII UpSh - worse than BYTE) */
        /*140*/ { -1, 10, { 0, 0, "" }, "AAA\200", 32, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 40 codewords (maximum 38)" },
        /*141*/ { 3, 10, { 0, 0, "" }, "A", 46, 0, 16, 49, "" }, /* Version T-48 with ECI (9 less as PAD escape char + "\123456") */
        /*142*/ { 3, 10, { 0, 0, "" }, "A", 47, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 40 codewords (maximum 38)" },
        /*143*/ { 3, 10, { 0, 0, "" }, "\001", 32, 0, 16, 49, "" },
        /*144*/ { 3, 10, { 0, 0, "" }, "\001", 33, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 39 codewords (maximum 38)" },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char data_buf[4096];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, BARCODE_CODEONE, -1 /*input_mode*/, data[i].eci, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, TCU(data_buf), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);
        assert_equal(symbol->errtxt[0] == '\0', ret == 0, "i:%d symbol->errtxt not %s (%s)\n", i, ret ? "set" : "empty", symbol->errtxt);
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d strcmp(%s, %s) != 0\n", i, symbol->errtxt, data[i].expected_errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_input(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_2;
        struct zint_structapp structapp;
        const char *data;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
        const char *expected_errtxt;
        int expected_option_2;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { -1, -1, -1, { 0, 0, "" }, "123456789012ABCDEFGHI", -1, 0, 22, 22, "", 2 },
        /*  1*/ { -1, -1, -1, { 0, 0, "" }, "123456789012ABCDEFGHIJ", -1, 0, 22, 22, "", 2 },
        /*  2*/ { -1, -1, -1, { 0, 0, "" }, "1", -1, 0, 16, 18, "", 1 },
        /*  3*/ { -1, -1, 0, { 0, 0, "" }, "1", -1, 0, 16, 18, "", 1 },
        /*  4*/ { -1, -1, 1, { 0, 0, "" }, "1", -1, 0, 16, 18, "", 1 },
        /*  5*/ { -1, -1, 1, { 0, 0, "" }, "ABCDEFGHIJKLMN", -1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 518: Input too long for Version A, requires 12 codewords (maximum 10)", 1 },
        /*  6*/ { GS1_MODE, -1, 1, { 0, 0, "" }, "[01]12345678901231", -1, 0, 16, 18, "", 1 },
        /*  7*/ { GS1_MODE | GS1PARENS_MODE, -1, 1, { 0, 0, "" }, "(01)12345678901231", -1, 0, 16, 18, "", 1 },
        /*  8*/ { GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, 1, { 0, 0, "" }, "(21)\\(12\\)4567890123", -1, 0, 16, 18, "", 1 },
        /*  9*/ { -1, 3, 1, { 0, 0, "" }, "1", -1, 0, 16, 18, "", 1 },
        /* 10*/ { UNICODE_MODE, 3, 1, { 0, 0, "" }, "é", -1, 0, 16, 18, "", 1 },
        /* 11*/ { GS1_MODE, 3, 1, { 0, 0, "" }, "[01]12345678901231", -1, ZINT_WARN_INVALID_OPTION, 16, 18, "Warning 512: ECI ignored for GS1 mode", 1 },
        /* 12*/ { -1, -1, 9, { 0, 0, "" }, "123456789012345678", -1, 0, 8, 31, "", 9 },
        /* 13*/ { -1, -1, 9, { 0, 0, "" }, "12345678901234567A", -1, ZINT_ERROR_INVALID_DATA, -1, -1, "Error 515: Invalid character at position 18 in input (Version S encodes digits only)", 9 },
        /* 14*/ { -1, -1, 9, { 0, 0, "" }, "1234567890123456789", -1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 514: Input length 19 too long for Version S (maximum 18)", 9 },
        /* 15*/ { GS1_MODE, -1, 9, { 0, 0, "" }, "[01]12345678901231", -1, ZINT_WARN_INVALID_OPTION, 8, 31, "Warning 511: GS1 mode ignored for Version S", 9 },
        /* 16*/ { -1, 3, 9, { 0, 0, "" }, "1", -1, ZINT_WARN_INVALID_OPTION, 8, 11, "Warning 511: ECI ignored for Version S", 9 },
        /* 17*/ { GS1_MODE, 3, 9, { 0, 0, "" }, "[01]12345678901231", -1, ZINT_WARN_INVALID_OPTION, 8, 31, "Warning 511: ECI and GS1 mode ignored for Version S", 9 },
        /* 18*/ { -1, -1, 10, { 0, 0, "" }, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", -1, 0, 16, 49, "", 10 },
        /* 19*/ { -1, -1, 10, { 0, 0, "" }, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901", -1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 519: Input length 91 too long for Version T (maximum 90)", 10 },
        /* 20*/ { -1, -1, 10, { 0, 0, "" }, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 0, 16, 49, "", 10 },
        /* 21*/ { -1, -1, 10, { 0, 0, "" }, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 55 codewords (maximum 38)", 10 },
        /* 22*/ { -1, -1, 10, { 0, 0, "" }, "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 38, 0, 16, 49, "", 10 },
        /* 23*/ { -1, 3, 10, { 0, 0, "" }, "1234567890123456789012345678901234567890123456789012345678901234567890123456", -1, 0, 16, 49, "", 10 },
        /* 24*/ { -1, 3, 10, { 0, 0, "" }, "12345678901234567890123456789012345678901234567890123456789012345678901234567", -1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 516: Input too long for Version T, requires 39 codewords (maximum 38)", 10 },
        /* 25*/ { -1, 3, 10, { 0, 0, "" }, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234", -1, ZINT_ERROR_TOO_LONG, -1, -1, "Error 519: Input length 91 too long for Version T (maximum 90)", 10 },
        /* 26*/ { GS1_MODE, -1, 10, { 0, 0, "" }, "[01]12345678901231", -1, 0, 16, 17, "", 10 },
        /* 27*/ { GS1_MODE, 3, 10, { 0, 0, "" }, "[01]12345678901231", -1, ZINT_WARN_INVALID_OPTION, 16, 17, "Warning 512: ECI ignored for GS1 mode", 10 },
        /* 28*/ { -1, -1, 11, { 0, 0, "" }, "1", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 513: Version '11' out of range (1 to 10)", 11 },
        /* 29*/ { -1, -1, -2, { 0, 0, "" }, "1", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 513: Version '-2' out of range (1 to 10)", -2 },
        /* 30*/ { GS1_MODE, -1, -1, { 1, 2, "" }, "[01]12345678901231", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 710: Cannot have Structured Append and GS1 mode at the same time", 0 },
        /* 31*/ { -1, -1, -1, { 1, 1, "" }, "123456789012ABCDEFGHI", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 711: Structured Append count '1' out of range (2 to 128)", 0 },
        /* 32*/ { -1, -1, -1, { 1, -1, "" }, "123456789012ABCDEFGHI", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 711: Structured Append count '-1' out of range (2 to 128)", 0 },
        /* 33*/ { -1, -1, -1, { 1, 129, "" }, "123456789012ABCDEFGHI", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 711: Structured Append count '129' out of range (2 to 128)", 0 },
        /* 34*/ { -1, -1, -1, { 0, 2, "" }, "123456789012ABCDEFGHI", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 712: Structured Append index '0' out of range (1 to count 2)", 0 },
        /* 35*/ { -1, -1, -1, { 3, 2, "" }, "123456789012ABCDEFGHI", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 712: Structured Append index '3' out of range (1 to count 2)", 0 },
        /* 36*/ { -1, -1, -1, { 1, 2, "1" }, "123456789012ABCDEFGHI", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 713: Structured Append ID not available for Code One", 0 },
        /* 37*/ { -1, -1, 9, { 1, 2, "" }, "123456789012ABCDEFGHI", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 714: Structured Append not available for Version S", 9 },
        /* 38*/ { -1, -1, 9, { 3, 2, "" }, "123456789012ABCDEFGHI", -1, ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 714: Structured Append not available for Version S", 9 }, /* Trumps other checking */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODEONE, data[i].input_mode, data[i].eci,
                                    -1 /*option_1*/, data[i].option_2, -1 /*option_3*/, -1 /*output_options*/,
                                    data[i].data, data[i].length, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }
        assert_zero(strcmp(symbol->errtxt, data[i].expected_errtxt), "i:%d symbol->errtxt %s != %s\n", i, symbol->errtxt, data[i].expected_errtxt);
        assert_equal(symbol->option_2, data[i].expected_option_2, "i:%d symbol->option_2 %d != %d\n",
                    i, symbol->option_2, data[i].expected_option_2);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_2;
        struct zint_structapp structapp;
        const char *data;
        int length;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        const char *comment;
        const char *expected;
    };
    /* Figure examples AIM USS Code One (USSCO) Revision March 3, 2000 */
    static const struct item data[] = {
        /*  0*/ { -1, -1, -1, { 0, 0, "" }, "1234567890123456789012", -1, 0, 16, 18, 1, "USSCO Figure 1 (Version A, no padding), same",
                    "111111111111001100"
                    "000110000110010101"
                    "100010110101101010"
                    "000010010110100111"
                    "111010100010101100"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000111011001101110"
                    "010110011110101111"
                    "101000010001101000"
                    "100111000001100000"
                    "101000001010111101"
                },
        /*  1*/ { -1, -1, -1, { 0, 0, "" }, "???????????????????????????????????????????????????????????????????????????????????????????", -1, 0, 40, 42, 1, "USSCO Figure 7 (Version D, no padding), same",
                    "010011010001000100011100010001000100110100"
                    "000010000000000000001000000000000000100000"
                    "010010010001000100011000010001000100100100"
                    "000010000000000000001000000000000000100000"
                    "010010010001000100011000010001000100100100"
                    "000010000000000000001000000000000000100000"
                    "010010010001000100011000010001000100100100"
                    "000010000000000000001000000000000000100000"
                    "010010010001000100011000010001000100100100"
                    "000010000000000000001000000000000000100000"
                    "010010010001000100011000010001000100100100"
                    "000010000000000000001000000000000000100000"
                    "010011010001000100011100010001000100110100"
                    "000010000000000000001000000000000000100000"
                    "010010010001000100011000010001000100100100"
                    "000010000000000000001000000000000000100000"
                    "111111111111111111111111111111111111111111"
                    "000000000000000000000000000000000000000000"
                    "011111111111111111111111111111111111111110"
                    "010000000000000000000000000000000000000010"
                    "011111111111111111111111111111111111111110"
                    "010000000000000000000000000000000000000010"
                    "011111111111111111111111111111111111111110"
                    "010000000000000000000000000000000000000010"
                    "011111111111111111111111111111111111111110"
                    "000010000000000000001000000000000000100000"
                    "010010010001000100011000010001000100100100"
                    "000011000000000000001100000000000000110000"
                    "010010010001000100011000010001000100100100"
                    "000010000000000000001000000000000000100000"
                    "010010111100010001111000011011111001100011"
                    "000010001000000000011010101011101000101101"
                    "001110101011100111011000101110010010101011"
                    "101010110000000011011001000100101100100000"
                    "100110011111100011001010001111001100101100"
                    "001110111010110001101011100101111101101100"
                    "101010111111011110111001000100010110100111"
                    "001110010111101000111000010011001011101011"
                    "001010001001110101011000110000111010101100"
                    "101111001111111001001101111000011111110010"
                },
        /*  2*/ { -1, -1, 9, { 0, 0, "" }, "1234567890", -1, 0, 8, 21, 1, "USSCO Figure 9 (left) NOTE: S-20 not S-10 as stated, same",
                    "000000011110110010110"
                    "000010000100000010011"
                    "101111110100010100100"
                    "010010001100110001010"
                    "000000000010000000000"
                    "111111111111111111111"
                    "100000000000000000001"
                    "101111111111111111101"
                },
        /*  3*/ { -1, -1, 10, { 0, 0, "" }, "12345678901234567890", -1, 0, 16, 17, 1, "USSCO Figure 9 (right) **NOT SAME** different encodation, figure uses ASCII double digits, Zint DECIMAL (same no. of codewords)",
                    "11111111111001100"
                    "00010001010010101"
                    "10001101001101010"
                    "00000101010100111"
                    "11111101010000111"
                    "11011100000100101"
                    "11000001000000111"
                    "01111010010010011"
                    "11111010001101101"
                    "00100011010111111"
                    "00000000100000000"
                    "11111111111111111"
                    "10000000000000001"
                    "10111111111111101"
                    "10000000000000001"
                    "10111111111111101"
                },
        /*  4*/ { GS1_MODE, -1, 2, { 0, 0, "" }, "[01]00312341234014[15]950915[10]ABC123456", -1, 0, 22, 22, 1, "USSCO Figure B1 **NOT SAME** using (10) not (30) as (30) no longer compliant",
                    "1110110000110000001010"
                    "1100100010001000111100"
                    "0111100110100100001101"
                    "1100100100101011101111"
                    "1001101111100001000100"
                    "0100101101110000100011"
                    "0100101000101010110101"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100111001001010100111"
                    "1001110100011011100001"
                    "0011101101100110100101"
                    "1101000110001101100000"
                    "1010001101011111101100"
                    "1111101011001110101010"
                    "1010010111000001110111"
                },
        /*  5*/ { GS1_MODE | GS1NOCHECK_MODE, -1, 2, { 0, 0, "" }, "[01]00312341234014[15]950915[30]ABC123456", -1, 0, 22, 22, 1, "USSCO Figure B1 same",
                    "1110110000110000001010"
                    "1100100010001000111100"
                    "0111100110100100001101"
                    "1100100100101011101111"
                    "1001101111101001000100"
                    "0100101101000000100011"
                    "0100101000101010110111"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100111001001010101110"
                    "1100100111000101101010"
                    "1011001110001010100000"
                    "0110011100010001101001"
                    "1010101110011010101001"
                    "1101111110110010101110"
                    "1010011100101001110101"
                },
        /*  6*/ { -1, -1, -1, { 0, 0, "" }, "Code One", -1, 0, 16, 18, 1, "BWIPP example",
                    "010011011101100110"
                    "010010000001010110"
                    "001010010101100110"
                    "000110000011110110"
                    "100010100000111001"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011110101101"
                    "101101000111101011"
                    "010100001110101100"
                    "100001100111100100"
                    "100000111000111000"
                },
        /*  7*/ { -1, -1, 9, { 0, 0, "" }, "406990", -1, 0, 8, 11, 1, "BWIPP Version S-10 example",
                    "01101101101"
                    "00101010111"
                    "01101001101"
                    "00100000100"
                    "00000000000"
                    "11111111111"
                    "10000000001"
                    "10111111101"
                },
        /*  8*/ { -1, -1, 3, { 0, 0, "" }, "1234567890ABCDEF", -1, 0, 28, 32, 1, "https://fr.wikipedia.org/wiki/Liste_des_symbologies **NOT SAME** as Zint has unlatch to ASCII at end (before padding)",
                    "10001110101011110111011110110101"
                    "11101001001010000011000110101001"
                    "11101001100010111110001000101000"
                    "10011011010100111100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "00001000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000000000"
                    "01111111111111111111111111111110"
                    "01000000000000000000000000000010"
                    "01111111111111111111111111111110"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010001010101001001100101101"
                    "00011000011001100111111110100001"
                    "01101011001000111101000001100000"
                    "11101010110111100100001100100100"
                    "10111001101111010101010011100101"
                    "11001001100111010000000111101100"
                    "01101001101100000101111100100000"
                    "00111100100000110101111110111001"
                },
        /*  9*/ { -1, -1, 1, { 0, 0, "" }, "1", -1, 0, 16, 18, 1, "Version A",
                    "001111100010001000"
                    "001010000100010001"
                    "100010100010001000"
                    "000110000100010001"
                    "100010100010111010"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011111101101"
                    "110101011101100011"
                    "001011010100100000"
                    "101000000101100011"
                    "001110100110110101"
                },
        /* 10*/ { -1, -1, 2, { 0, 0, "" }, "1", -1, 0, 22, 22, 1, "Version B",
                    "0011111000100010001000"
                    "0010100001000100010001"
                    "1000101000100010001000"
                    "0001100001000100010001"
                    "1000101000100010001000"
                    "0001100001000100010001"
                    "1000101000100010000101"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001100011"
                    "1011100100100100101111"
                    "1000001000100010101000"
                    "1000101010001101101011"
                    "0011100111110001101010"
                    "0000001101010001100110"
                    "0001001111111101111101"
                },
        /* 11*/ { -1, -1, 3, { 0, 0, "" }, "1", -1, 0, 28, 32, 1, "Version C",
                    "00111110001000100010001000111000"
                    "00101000010001000100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "00001000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000000000"
                    "01111111111111111111111111111110"
                    "01000000000000000000000000000010"
                    "01111111111111111111111111111110"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010000011010001010010101000"
                    "00011000010011100110010111100110"
                    "10101001001100010000111010101011"
                    "11111001011001101001001101100100"
                    "11001011100101010101000111100000"
                    "01001000010101111111101101101001"
                    "00001000011000110111001001100111"
                    "10001110101100110010000011111001"
                },
        /* 12*/ { -1, -1, 4, { 0, 0, "" }, "1", -1, 0, 40, 42, 1, "Version D",
                    "001111100010001000101100100010001000111000"
                    "001010000100010001001001000100010001100001"
                    "100010100010001000101000100010001000101000"
                    "000110000100010001001001000100010001100001"
                    "100010100010001000101000100010001000101000"
                    "000110000100010001001001000100010001100001"
                    "100010100010001000101000100010001000101000"
                    "000110000100010001001001000100010001100001"
                    "100010100010001000101000100010001000101000"
                    "000110000100010001001001000100010001100001"
                    "100010100010001000101000100010001000101000"
                    "000110000100010001001001000100010001100001"
                    "100011100010001000101100100010001000111000"
                    "000110000100010001001001000100010001100001"
                    "100010100010001000101000100010001000101000"
                    "000010000000000000001000000000000000100000"
                    "111111111111111111111111111111111111111111"
                    "000000000000000000000000000000000000000000"
                    "011111111111111111111111111111111111111110"
                    "010000000000000000000000000000000000000010"
                    "011111111111111111111111111111111111111110"
                    "010000000000000000000000000000000000000010"
                    "011111111111111111111111111111111111111110"
                    "010000000000000000000000000000000000000010"
                    "011111111111111111111111111111111111111110"
                    "000110000100010001001001000100010001100001"
                    "100010100010001000101000100010001000101000"
                    "000111000100010001001101000100010001110001"
                    "100010100010001000101000100010001000101000"
                    "000110000100010001001001000100010001100001"
                    "100010101001100100001011011010110110100000"
                    "000110100110101111011011110010011011101001"
                    "000010010111010100101010110101001011101101"
                    "000110100010000101111000100111011100101011"
                    "001110001100000101111001010011111100101011"
                    "001110011010000011011001001111011001100010"
                    "110110110000111010011001111100001111101001"
                    "001110010011111000101011111101110101100100"
                    "111110111100011110001010001011010010101010"
                    "001111110100111100111110111101110110111000"
                },
        /* 13*/ { -1, -1, 5, { 0, 0, "" }, "1", -1, 0, 52, 54, 1, "Version E",
                    "001111100010001000100010001110001000100010001000111000"
                    "001010000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100011100010001000100010001110001000100010001000111000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000010000000000000000000001000000000000000000000100000"
                    "111111111111111111111111111111111111111111111111111111"
                    "000000000000000000000000001000000000000000000000000000"
                    "111111111111111111111111111111111111111111111111111111"
                    "000000000000000000000000000000000000000000000000000000"
                    "011111111111111111111111111111111111111111111111111110"
                    "010000000000000000000000000000000000000000000000000010"
                    "011111111111111111111111111111111111111111111111111110"
                    "010000000000000000000000000000000000000000000000000010"
                    "011111111111111111111111111111111111111111111111111110"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000110000100010001000100011000010001000100010001100001"
                    "100010100010001000100010001010001000100010001000101000"
                    "000111000100010001000100011100010001000100010001110001"
                    "100010100011001110101101011001001110100110111010100011"
                    "000110000110110111000001011000110010010101010110100000"
                    "011110101100010100001110011000100001000000000011101111"
                    "101010010111001011101001101001011011111100100001100110"
                    "110010110111001110010010111010101010101111101100101100"
                    "010010111100101001101011101010101101010000110100100000"
                    "100010001111111011100100001010111111011111101010101011"
                    "001110101110001110100000101011001010100011010000101010"
                    "001010111001011111011100011011011000010001110111100110"
                    "101110011000100101111110011000100011001011110111101111"
                    "111110101100111001100001101011110110001010011110100010"
                    "111111001000100010110011011100010011010001110011110110"
                },
        /* 14*/ { -1, -1, 6, { 0, 0, "" }, "1", -1, 0, 70, 76, 1, "Version F",
                    "0011111000100010001000100011100010001000100010001110001000100010001000111000"
                    "0010100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000111000100010001000100011100010001000100010001110001000100010001000111000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000111000100010001000100011100010001000100010001110001000100010001000111000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000010000000000000000000000000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000010000000000000000000000000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000000000000000000000000000000000000000000000000000000"
                    "0111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "0100000000000000000000000000000000000000000000000000000000000000000000000010"
                    "0111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001110001000100010001000111000100010001000100011100010001000100010001110001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000011011111010101001"
                    "0001100001000100010001000110000100010001000100011000010001111011011111101101"
                    "0110101100001000110011100010010001101110011110001010011001110110100001100110"
                    "1111101111011010000010100010111111101100011011101001001010011000000111100001"
                    "1101100110100110101000011010111110101010011000011010001011101010101100100001"
                    "0110111110001001101100001111000011101011100101011101110010001101111001110000"
                    "0100100100010000000011110010110000101001011011011010110100000101010001101110"
                    "1110101011010110011110010010101111000100100011011011011011110111000001101010"
                    "0010101100100000101011100110011110000001100010111000101110111101110110101001"
                    "0000101100111111111101000010111100010110010000011001110100001110101111101101"
                    "1101101011000000110000110110010101111111001000111000010110000010100010100100"
                    "1011100000000011011101000110100110010101001101011001110010010111101101101111"
                    "1010101001010010110010111110000010100100101111001000101111010101011111100100"
                    "0001100001110100110100000110010001101111111011101011110000110100110100100000"
                    "0110101111010001100101000010110001111001111001101011010001001111100001100100"
                    "1111101011000011001010101110000111001100100011011000010110011100100111101001"
                    "0100101101010010100110011010000001101000011011111001011101001000011110100101"
                    "1100111111000110110101110011011001001010010101101111011110000111010100110101"
                },
        /* 15*/ { -1, -1, 7, { 0, 0, "" }, "1", -1, 0, 104, 98, 1, "Version G",
                    "00111011001000100010001000111000100010001000100011100010001000100010011010001000100010001011001000"
                    "00100010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001011001000100010001000111000100010001000100011100010001000100010011010001000100010001011001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001011001000100010001000111000100010001000100011100010001000100010011010001000100010001011001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001011001000100010001000111000100010001000100011100010001000100010011010001000100010001011001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "00000000000000000000000000010000000000000000000000000000000000000000010000000000000000000000000000"
                    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010011010001000100010001011001000100010001000111000100010001000100011100010001000100010011010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000010011110011001100000110110111011101100001110001100110110000100010100010"
                    "00010011010001000100010001111101100110011001111111101110111011010110011110011001000000010011010001"
                    "11110010000000000000111011110011101110011001100110100111000010001000110010100010001000101110101001"
                    "11101010001000100001011001110001100110010110011010011010010101010101010101010110011001101110110001"
                    "10011010011111000100010001110001111111111111000010011101110111111100110000100010010110101010101010"
                    "00010010010010011101110111010100100110011001000110001100110011100100010100010001110111101110101110"
                    "01001110101110111011100110010110011010100011001110001111001101110111010101011101110111010010110011"
                    "00101010101010101001111111110111111110000101010110010100010110011001110011101101110111010010110001"
                    "00110010111001100110011001110001010101010101101010010001000100101000010100010001000101110110110111"
                    "00010010010000111011101110110000100110011001011110111011101110101010110010101010101010011010011001"
                    "11010110110111011110100000010000000011011011101110101100110010001000110001010101010101011010101000"
                    "11111110111111111111010001010001000110000110011010011001011101110111010101010110011001100010110010"
                    "10001010000100110111011101110110100110011001001010011101110111000001110001100110000011111110111111"
                    "00100011101011100110011001011101100110011001101111011101110111100111111011101110110010011011011001"
                    "11110010000000000000101010110010101001111101110110110110100101010101010110100101010101010010101011"
                    "11000010100010001000010100010100010000110101010110010101110001000100010100001110111011100010001110"
                    "10111010110001100010001000110101001100110011100110110011001100010001110001100110010100000010000000"
                    "11101110101110101010101010110110010001000100110110111111111111101100110000100010000101110110110111"
                    "00010010100010001001010010010010001001000110011010011000000101010101010101110001000100010110001000"
                    "01110110010101010110101011110011101111101110111010111001100010001000110001000111011101111010001001"
                    "10001010001000111011101110010100100010001000011110111111111111111101010101010101101001110110110111"
                    "10011010010001101010101010110001011101110111000110110111011101111001010101010101111011011110011101"
                    "10010010010001000101011000110000100011011100110010110011110111011101110100110000000000001110010101"
                    "11111110111111111100010011010011001100000000000010000000001101110111010111111100110011001110010101"
                    "01010110010110111111111111010101011001100110011010000000000000001110010010001000011000100010100010"
                    "01010111011110111111111111111010000100010001111011010001000100000111011011001100100100010011010001"
                },
        /* 16*/ { -1, -1, 8, { 0, 0, "" }, "1", -1, 0, 148, 134, 1, "Version H",
                    "00111011001000100010001000111000100010001000101100100010001000100011100010001000100010110010001000100010001110001000100010001011001000"
                    "00100010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001011001000100010001000111000100010001000101100100010001000100011100010001000100010110010001000100010001110001000100010001011001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001011001000100010001000111000100010001000101100100010001000100011100010001000100010110010001000100010001110001000100010001011001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001011001000100010001000111000100010001000101100100010001000100011100010001000100010110010001000100010001110001000100010001011001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001011001000100010001000111000100010001000101100100010001000100011100010001000100010110010001000100010001110001000100010001011001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001011001000100010001000111000100010001000101100100010001000100011100010001000100010110010001000100010001110001000100010001011001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "00000010000000000000000000100000000000000000001000000000000000000010000000000000000000100000000000000000001000000000000000000010000000"
                    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "00000000000000000000000000100000000000000000000000000000000000000010000000000000000000000000000000000000001000000000000000000000000000"
                    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "00000000000000000000000000100000000000000000000000000000000000000010000000000000000000000000000000000000001000000000000000000000000000"
                    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010011010001000100010001110001000100010001001101000100010001000111000100010001000100110100010001000100011100010001000100010011010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010011010001000100010001110001000100010001001101000100010001000111000100010001000100110100010001000100011100010001000100010011010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000101000100010001000100010100010001000100010100010001000100010001010001000100010001010001000"
                    "00010010010001000100010001100001000100010001001001000100010001000110000100010001000100100100010001000100011000010001000100010010010001"
                    "10001010001000100010001000101000100010001000011010111111111111111110111111111111101010100110011001100110011010011001011011001110001100"
                    "00010011010001000100010001110001000100010001111110110111011101110111110111011101111111110111011101110111011111011101111111111111111111"
                    "11001110001100110000100011100011001100110011001011001100111000100010100010001000100010100001000110011001101001100110011001101110100111"
                    "11111110111111111101101000101000100010001000101000100000101000100010100010001000100010100011111110111011101011101110111011101110000110"
                    "01110110110111011101110111101000100110011001101001100110011001100110110111011101110111100111011101101000011000010001000100010010010001"
                    "01100110100110011001100110101110010001000100011000010001000100101010011001100110011001101001100110000001111001110111011101110110110111"
                    "01101110011101110111011101101101110101101001101001100110011001100110100110101000100010100010001000100010001001101111111111111110111111"
                    "00010110100110011001100110100110011011100010001010001000100010001010001001101100110011100011001100110011001000110000000000000010000000"
                    "11111110111010101010101010101010101010101010011010000100010001000110000100010001100010101110111011101110111010111011101010101010101010"
                    "00000010001110101110111011101011101110111011101001010101010101010110010101010101011100101000100010001000101000100010001101110110110111"
                    "10101010101010101011000001100001000100010001001001000101000100010010010001000100010001100001000000000000001000000000000000000010111100"
                    "01110110110111011110010000100000000000000000001000000011101011101110101110111011101110101101011001100110011010011001100110011110100100"
                    "11001110001100110011001100101100001000100010001010001000100010100110011001100110011001101001100110110110111010111011101110111010111011"
                    "01000111000100010001000100111011110011001100111100110011001100010011100010001000100010110010001000110111011111011101110111011111011101"
                    "01000010010001000100010001100001000101010001001001000100010001000110000111100010001000101000100010001000101011001000100010001010001000"
                    "10111110011101110111011101101101110111000001001001000100010001000110000110100000000000100000000000000000001011001111111111111110111111"
                    "10001010000010101110111011101011101110111011101001011101110111011110011101110111100000100100010001000100011000010001100010111010111011"
                    "11111110111111110111011101101101110111011101001000111111111111111110111111111111000101101001100110011001101001100110010000010010010001"
                    "10111010111011101100101110101110111011101110111010111011110111011110011101110111011101101101101001100110011010011001100110011110011011"
                    "00010010010001000101110100100100010001000100011000010000111010101010101010101010101010101011111101110111011011011101110111011010110000"
                    "10111010111011101110111011100000001100110011001011001100110011000010110111011101110111100111011101010101111001110111011101110110110111"
                    "00000010000000000000000000100000110111011101111001110111011101110110000100010001000100100100010001100110011010011001100110011010011001"
                    "11110010100010001000100010100010001000110001001001000100010001000110000101100000000000100000000000000000001010100010001000100010100010"
                    "01010010110011001100110011100011001101010111011011011101110111011110011100100101010101100101010101010101011011101101110111011110011101"
                    "00100010100100101010101010101010101010101010101001010001000100010010010001000100101100101000100010001000101000100010111100000010000000"
                    "11011111011111000100010001110001000100010001001101110111011101110111110111011101001101110001000100010001001101000100000101000111000100"
                    "00000010000000000010100100100100010001000100011000010010111100110010110011001100110011100000101111111111111011111111111111110110010101"
                    "01000110000100010001101111101111111111111111111011111111101110111010111011101110111011101011110000000000001000000000000000001110010011"
                    "01010110010101010101010101101111010001000100011000010001000100011010111111111111111111101111111111010001101001100110011001100110100110"
                    "00110010110011001100110011100100000000000000001000000000000000111110101110111011101110101110111011000011001011001100110011001110001100"
                    "01010010000000000000000000100000000011000111011011011101110111011110011110011110111011101011101110111011101001101101110111011110011101"
                    "10101010111011101110111011101011101100011100111000110011001100110010110011001000100010100010001000100010001011010001000100010010010001"
                    "11011110010001001100110011100011001100110011111010000100010001000110000100010001010001100001000100010001001001000100110101000110000100"
                    "00010010010110011101110111100111011101110111001010011101110111011110011101110111100111100011001100110011001011001100111100010010010001"
                    "01000110000100010010100110100110011001100110011010011001100000000010000000000000000000100001101000100010001010001000100010000110101111"
                    "00010010010001000110110101100101010101010101011001010111000110011010011001100110011001101001001010101010101010101010101010100110010110"
                    "11111110111111111111111111100101110111011101111001110111011101001010000100010001000100100100010001111001011001010101010101010110010101"
                    "01100111100110011001100110111101111011101110111110111011101110000111110111011101110111110111011101010001011101010101010101010111010101"
                },
        /* 17*/ { -1, -1, 9, { 0, 0, "" }, "123456", -1, 0, 8, 11, 1, "Version S-10",
                    "00011110000"
                    "11000010001"
                    "10100011100"
                    "11000011000"
                    "00000000000"
                    "11111111111"
                    "10000000001"
                    "10111111101"
                },
        /* 18*/ { -1, -1, 9, { 0, 0, "" }, "123456789012", -1, 0, 8, 21, 1, "Version S-20",
                    "000101110111000010010"
                    "110101100101011000101"
                    "100000101001110101100"
                    "010110101001010001000"
                    "000000000010000000000"
                    "111111111111111111111"
                    "100000000000000000001"
                    "101111111111111111101"
                },
        /* 19*/ { -1, -1, 9, { 0, 0, "" }, "123456789012345678", -1, 0, 8, 31, 1, "Version S-30",
                    "0000110111010011100000001111001"
                    "1110100011011100110110110010111"
                    "1111100010010100111000011001010"
                    "1101100000010010010111110001110"
                    "0000000000000001000000000000000"
                    "1111111111111111111111111111111"
                    "1000000000000001000000000000001"
                    "1011111111111111111111111111101"
                },
        /* 20*/ { -1, -1, 10, { 0, 0, "" }, "ABCDEFGHIJKLM", -1, 0, 16, 17, 1, "Version T-16",
                    "11100101111100110"
                    "01101001010011101"
                    "00101000001011001"
                    "01000000011110011"
                    "10010100010001011"
                    "10101110001001010"
                    "00101001001001110"
                    "01101110011001011"
                    "01111001001110011"
                    "10000001001101001"
                    "00000000100000000"
                    "11111111111111111"
                    "10000000000000001"
                    "10111111111111101"
                    "10000000000000001"
                    "10111111111111101"
                },
        /* 21*/ { -1, -1, 10, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGH", -1, 0, 16, 33, 1, "Version T-32",
                    "111001011110011010010100001011001"
                    "011010011001110100100000011110011"
                    "100110101101101100001110001001110"
                    "101001100101101000000110110110000"
                    "100011111010011000101011110000100"
                    "011000110111000000010001111011001"
                    "010000111111100101010011010011101"
                    "010110011111011000010111100011110"
                    "000110111101100001011011000011000"
                    "101111100010010000100011100000110"
                    "000000000000000010000000000000000"
                    "111111111111111111111111111111111"
                    "100000000000000010000000000000001"
                    "101111111111111111111111111111101"
                    "100000000000000000000000000000001"
                    "101111111111111111111111111111101"
                },
        /* 22*/ { -1, -1, 10, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABC", -1, 0, 16, 49, 1, "Version T-48",
                    "1110010111100110001010001010110011001101011011011"
                    "0110100110011101010000000111100111010011001011010"
                    "0001110001001110100011110101001100101011110001000"
                    "0000110110110000011000110011100000010001111010110"
                    "1100100100001010001111000011111011011111011101111"
                    "1000101000111101111000000100100110100011011110110"
                    "0000010001110011111001000001000010101000001001110"
                    "0000010010111000001000100100111111001101000110011"
                    "0101010011001000001111000100101010001011010001111"
                    "1100010001111110010001100010011010010011101100110"
                    "0000000000000000000000001000000000000000000000000"
                    "1111111111111111111111111111111111111111111111111"
                    "1000000000000000000000001000000000000000000000001"
                    "1011111111111111111111111111111111111111111111101"
                    "1000000000000000000000001000000000000000000000001"
                    "1011111111111111111111111111111111111111111111101"
                },
        /* 23*/ { -1, -1, -1, { 0, 0, "" }, "123456789012", -1, 0, 16, 18, 1, "ASCII double digits",
                    "100011101010111101"
                    "111010010010100000"
                    "110110100010001000"
                    "110010111000010001"
                    "100010100011010100"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010100100101"
                    "011001001110101101"
                    "011010010010101111"
                    "010111110100100111"
                    "100010001101111100"
                },
        /* 24*/ { -1, -1, -1, { 0, 0, "" }, "1234567890123", -1, 0, 16, 18, 1, "DECIMAL (numeric ending >= 13)",
                    "111111111111001100"
                    "000110000110010101"
                    "100010110111011000"
                    "000010111100010001"
                    "100010100011100101"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011011101000"
                    "110111100101101000"
                    "011011111110101010"
                    "011011110100101000"
                    "010000100100111111"
                },
        /* 25*/ { -1, -1, -1, { 0, 0, "" }, "1234567890123A", -1, 0, 16, 18, 1, "ASCII (ending not fully numeric)",
                    "100011101010111101"
                    "111010010010100000"
                    "110110100000110100"
                    "110010111001000010"
                    "100010100011101100"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010110100100"
                    "010101000111101010"
                    "001001000010100110"
                    "001101100111101010"
                    "010000000011110011"
                },
        /* 26*/ { -1, -1, -1, { 0, 0, "" }, "GOSGOS", -1, 0, 16, 18, 1, "2 C40 triplets, C40 at end, buffer empty, switch to ASCII before padding (5)",
                    "111011100010001000"
                    "011010000100010001"
                    "100010111110001000"
                    "000110111100010001"
                    "100010100000100111"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010010100000"
                    "011011101100100111"
                    "111111101001100010"
                    "110101010100101011"
                    "100110001001111001"
                },
        /* 27*/ { -1, -1, -1, { 0, 0, "" }, "GOSGOSG", -1, 0, 16, 18, 1, "2 C40 triplets + C40 singlet, C40 at end, singlet in buffer, switch to ASCII before padding (3)",
                    "111011100010001000"
                    "011010000100010001"
                    "100010111101001000"
                    "000110111110000001"
                    "100010100011000101"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011111101011"
                    "010101100101101111"
                    "110010101010101010"
                    "001000000101101010"
                    "100101111101110001"
                },
        /* 28*/ { -1, -1, -1, { 0, 0, "" }, "GOSGOSGO", -1, 0, 16, 18, 1, "2 C40 triplets + C40 doublet, C40 at end, doublet in buffer, switch to ASCII for doublet (2 pad)",
                    "111011100010001000"
                    "011010000100010001"
                    "100010111101000101"
                    "000110111110000000"
                    "100010100010100100"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010000100010"
                    "010011110001101000"
                    "101110101100101111"
                    "110011010011101110"
                    "010001011100111100"
                },
        /* 29*/ { -1, -1, -1, { 0, 0, "" }, "GOSGOSGOS", -1, 0, 16, 18, 1, "3 C40 triplets, C40 at end, switch to ASCII before padding (3)",
                    "111011100010001000"
                    "011010000100010001"
                    "100010100010001111"
                    "000110000100011111"
                    "100010100001110000"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010100100101"
                    "011101101001100111"
                    "110000000100101000"
                    "000011011111101011"
                    "000011110110110001"
                },
        /* 30*/ { -1, -1, -1, { 0, 0, "" }, "GOSGOSGOSG", -1, 0, 16, 18, 1, "3 C40 triplets + C40 singlet, C40 at end, singlet in buffer, switch to ASCII for singlet (1 pad)",
                    "111011100010001000"
                    "011010000100010001"
                    "100010100010001111"
                    "000110000100011111"
                    "010010100000100001"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "100000011011100000"
                    "001111100101101101"
                    "000001100100100010"
                    "101001010011100111"
                    "101111111001110110"
                },
        /* 31*/ { -1, -1, -1, { 0, 0, "" }, "GOSGOSGOSGO", -1, 0, 16, 18, 1, "3 C40 triplets + C40 doublet, C40 at end, doublet in buffer, switch to ASCII for doublet (0 pad)",
                    "111011100010001000"
                    "011010000100010001"
                    "100010100010001111"
                    "000110000100011111"
                    "010010010111000001"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "100000000011101010"
                    "110111111011100101"
                    "111101001000101111"
                    "110000011111100011"
                    "001001001110111100"
                },
        /* 32*/ { -1, -1, -1, { 0, 0, "" }, "GOSGOSGOSGOS", -1, 0, 16, 18, 1, "4 C40 triplets, C40 at end, empty buffer, switch to ASCII before padding (1)",
                    "111011100010001000"
                    "011010000100010001"
                    "100010100010001000"
                    "000110000100010001"
                    "100010111100100001"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000111111110100010"
                    "101010100011101010"
                    "111100000000101011"
                    "001010101010101001"
                    "110011100111110100"
                },
        /* 33*/ { -1, -1, -1, { 0, 0, "" }, ".GOSGOSGOSGOS", -1, 0, 16, 18, 1, "ASCII + 4 C40 triplets, C40 at end, empty buffer, no switch to ASCII as no padding",
                    "001011111010001000"
                    "111110011000010001"
                    "100010100010001000"
                    "000110000100010001"
                    "100010100010110100"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010111101010"
                    "011010010011100110"
                    "000110111100100111"
                    "111011101001100000"
                    "010001000011110110"
                },
        /* 34*/ { -1, -1, -1, { 0, 0, "" }, "ABCDEFGHIJ\001K", -1, 0, 16, 18, 1, "4 C40 triplets (last shifted) + singlet, no unlatch, singlet as ASCII, no padding",
                    "111011010111100110"
                    "011010100110011101"
                    "001010100001011000"
                    "010010000011111111"
                    "110010010000111101"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "001011001010100001"
                    "101001001111101001"
                    "101110101101100110"
                    "001100001010100011"
                    "001001000000110110"
                },
        /* 35*/ { -1, -1, -1, { 0, 0, "" }, "ABCDEFGHIJK\001", -1, 0, 22, 22, 1, "4 C40 triplets + singlet (shifted), backtrack to 3rd triplet and ASCII encode",
                    "1110110101111001100010"
                    "0110101001100111010100"
                    "1000100101111101000100"
                    "0000101111111110111100"
                    "0000101000100010001000"
                    "0010100001000100010001"
                    "1000101000100010000000"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101001"
                    "1010111010001011101100"
                    "0011100100111011101111"
                    "1001100110000000101001"
                    "1100001010011100100111"
                    "0011000011111010101111"
                    "1111110001000011110110"
                },
        /* 36*/ { -1, -1, -1, { 0, 0, "" }, "ABCDEFGH\001I\001", -1, 0, 22, 22, 1, "4 C40 triplets + singlet (shifted), backtrack to 2nd triplet and ASCII encode",
                    "1110110101111001100010"
                    "0110101001100111010100"
                    "1111100100010000000100"
                    "1111101000100100101010"
                    "0000101000100010001000"
                    "0010100001000100010001"
                    "1000101000100010001010"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101011"
                    "0110001001011110100100"
                    "0000011100110101101011"
                    "0100111101001101101110"
                    "0010001000101001101100"
                    "1110110010000110101011"
                    "1001100010111000111111"
                },
        /* 37*/ { -1, -1, -1, { 0, 0, "" }, "\101\102\103\104\105\106\107\110\200\101\102", -1, 0, 22, 22, 1, "'ABCDEFGH<80>AB' - cte_buffer_transfer with cte_p > 3",
                    "1110110101111001100010"
                    "0110101001100111010100"
                    "1000100100101110001111"
                    "0000101010101100011111"
                    "0100100100100010001000"
                    "0010100011000100010001"
                    "1000101000100010000110"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001100101"
                    "1010111100010110100110"
                    "1001111110011001100011"
                    "1101111110111000100101"
                    "1010110011101001101111"
                    "0111100000001001101001"
                    "0101000100001100110110"
                },
        /* 38*/ { -1, -1, -1, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", -1, 0, 22, 22, 1, "7 EDI triplets + doublet, doublet encoded as triplet with Shift 0 pad, no switch to ASCII, no padding",
                    "1110110101111001100010"
                    "0110101001100111010100"
                    "1000100101100110011010"
                    "0000101111001110100110"
                    "1101101011000111000100"
                    "0101101010000011011011"
                    "1110101000111110011010"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0000011000111001101001"
                    "0110011000110111101101"
                    "0011100111101100101101"
                    "1110111011011111101110"
                    "0011000011001101101000"
                    "0101100011001011101100"
                    "0100011100011000110110"
                },
        /* 39*/ { -1, -1, -1, { 0, 0, "" }, "gosgos", -1, 0, 16, 18, 1, "2 TEXT triplets, TEXT at end, buffer empty, switch to ASCII before padding",
                    "111011100010001000"
                    "111110000100010001"
                    "100010111110001000"
                    "000110111100010001"
                    "100010100010011101"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011000101000"
                    "110001011110101111"
                    "000001000110100100"
                    "000011110000100010"
                    "101100010110111010"
                },
        /* 40*/ { -1, -1, -1, { 0, 0, "" }, "gosgosg", -1, 0, 16, 18, 1, "2 TEXT triplets + TEXT singlet, TEXT at end, singlet in buffer, switch to ASCII for singlet (3 pads)",
                    "111011100010001000"
                    "111110000100010001"
                    "100010111101101000"
                    "000110111110000001"
                    "100010100010000100"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011010100100"
                    "000110100011100010"
                    "011000001111100101"
                    "100100000110100110"
                    "010101111001110000"
                },
        /* 41*/ { -1, -1, -1, { 0, 0, "" }, "gosgosgo", -1, 0, 16, 18, 1, "2 TEXT triplets + TEXT doublet, TEXT at end, doublet in buffer, switch to ASCII for doublet, (2 pads)",
                    "111011100010001000"
                    "111110000100010001"
                    "100010111101100111"
                    "000110111110000000"
                    "100010100010110111"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011100101000"
                    "011110111111100101"
                    "110000110110101111"
                    "010010000000100110"
                    "101001101001111111"
                },
        /* 42*/ { -1, -1, -1, { 0, 0, "" }, ".gosgosgosgos", -1, 0, 16, 18, 1, "ASCII + 4 TEXT triplets, TEXT at end, empty buffer, no switch to ASCII as no padding",
                    "001011111010001000"
                    "111110111100010001"
                    "100010100010001000"
                    "000110000100010001"
                    "100010100000011010"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010101101100"
                    "111000100010101011"
                    "010110110001101110"
                    "010110000001101110"
                    "111001010101110110"
                },
        /* 43*/ { -1, -1, -1, { 0, 0, "" }, "\015*>", -1, 0, 16, 18, 1, "1 EDI triplet, ASCII mode",
                    "000011001000111000"
                    "111010101111110001"
                    "100010100010001000"
                    "000110000100010001"
                    "100010100011101000"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011010100110"
                    "011111010100100100"
                    "011000001001100100"
                    "111000011101100111"
                    "101001101110111110"
                },
        /* 44*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015", -1, 0, 16, 18, 1, "1 EDI triplet + singlet, ASCII mode",
                    "000011001000110000"
                    "111010101111111110"
                    "100010100010001000"
                    "000110000100010001"
                    "100010100001100100"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011110101100"
                    "011000011100100011"
                    "101010110001100011"
                    "000110111010101011"
                    "101001000010111001"
                },
        /* 45*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*", -1, 0, 16, 18, 1, "1 EDI triplet + doublet, ASCII mode",
                    "000011001000110000"
                    "111010101111111110"
                    "001010100010001000"
                    "101110000100010001"
                    "100010100001010000"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011011100010"
                    "111101001110101111"
                    "101010110001101100"
                    "101100010011101101"
                    "000010101011111010"
                },
        /* 46*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>", -1, 0, 16, 18, 1, "2 EDI triplets, EDI mode",
                    "111011000000100000"
                    "111010000010110000"
                    "001010111110001000"
                    "101110111100010001"
                    "100010100011001100"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010110101111"
                    "100010111100100100"
                    "000001000000101011"
                    "111000010111100010"
                    "110010111011111011"
                },
        /* 47*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015", -1, 0, 16, 18, 1, "2 EDI triplets + singlet, EDI mode + final ASCII",
                    "111011000000100000"
                    "111010000010110000"
                    "001010111100001000"
                    "101110111111100001"
                    "100010100000101110"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010111100011"
                    "111011011010101001"
                    "011110010100100101"
                    "011111001000100110"
                    "100011100011110000"
                },
        /* 48*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*", -1, 0, 16, 18, 1, "2 EDI triplets + doublet, EDI + final 2 ASCII",
                    "111011000000100000"
                    "111010000010110000"
                    "001010111100000010"
                    "101110111111101011"
                    "100010100000111001"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010110100110"
                    "000010000001101101"
                    "001001011010101001"
                    "101001000000100011"
                    "100011000010110110"
                },
        /* 49*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>", -1, 0, 16, 18, 1, "3 EDI triplets, EDI mode",
                    "111011000000100000"
                    "111010000010110000"
                    "001010000000101111"
                    "101110000010111111"
                    "100010100011011101"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011110100100"
                    "100101000000101000"
                    "110101100111101110"
                    "110110000011100000"
                    "110111100010110011"
                },
        /* 50*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>\015", -1, 0, 16, 18, 1, "3 EDI triplets + singlet, EDI mode + final ASCII singlet",
                    "111011000000100000"
                    "111010000010110000"
                    "001010000000101111"
                    "101110000010111111"
                    "000010100000001001"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "111000011011101000"
                    "110001100000101111"
                    "101111100101100001"
                    "000101100111100111"
                    "011010010010111010"
                },
        /* 51*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>\015*>", -1, 0, 16, 18, 1, "4 EDI triplets, EDI mode",
                    "111011000000100000"
                    "111010000010110000"
                    "001010000000100000"
                    "101110000010110000"
                    "001010111110000011"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "101111110110100011"
                    "100100100001101111"
                    "010001111111101001"
                    "100000101110101011"
                    "110001000001111000"
                },
        /* 52*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>\015*>\015", -1, 0, 16, 18, 1, "4 EDI triplets + singlet, EDI mode + nolatch final ASCII singlet (last data codeword of symbol)",
                    "111011000000100000"
                    "111010000010110000"
                    "001010000000100000"
                    "101110000010110000"
                    "001010000011000011"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "101111101001101110"
                    "111111010000101000"
                    "000011011011101101"
                    "010111001011101001"
                    "101010110001111100"
                },
        /* 53*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>\015*>\015*", -1, 0, 22, 22, 1, "4 EDI triplets + doublet, EDI mode + final 2 ASCII",
                    "1110110000001000000010"
                    "1110100000101100001011"
                    "0000100010000000101111"
                    "0000101011000010111111"
                    "0000100010100010001000"
                    "1110101011000100010001"
                    "1000101000100010001010"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001100001"
                    "1011100001010010100111"
                    "1100001100111101100000"
                    "0110111110101000100011"
                    "1100011011101100100101"
                    "0110100100001000101101"
                    "0010111010100110111000"
                },
        /* 54*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>a", -1, 0, 16, 18, 1, "2 EDI triplets + ASCII singlet, EDI mode + final ASCII",
                    "111011000000100000"
                    "111010000010110000"
                    "001010111101101000"
                    "101110111100100001"
                    "100010100010011001"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011000101111"
                    "101001011011101001"
                    "111010111100101011"
                    "100111010011100101"
                    "011111111010110101"
                },
        /* 55*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>\015*>a", -1, 0, 16, 18, 1, "4 EDI triplets + ASCII singlet, EDI mode + final ASCII",
                    "111011000000100000"
                    "111010000010110000"
                    "001010000000100000"
                    "101110000010110000"
                    "001010011010001000"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "101100100011100101"
                    "010111100000100111"
                    "011011111111100010"
                    "001110101010100111"
                    "110100010001110011"
                },
        /* 56*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>\015*>\015a", -1, 0, 22, 22, 1, "4 EDI triplets + singlet + ASCII, EDI mode + final 2 ASCII",
                    "1110110000001000000010"
                    "1110100000101100001011"
                    "0000100010000000101111"
                    "0000101011000010111111"
                    "0000100110100010001000"
                    "1110100010000100010001"
                    "1000101000100010000110"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101001"
                    "1100100011001011100000"
                    "0000100110101010101011"
                    "0011101011110100100111"
                    "1001111010011000100000"
                    "0001110010110111101110"
                    "1001011011101101111101"
                },
        /* 57*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>\015*a", -1, 0, 22, 22, 1, "3 EDI triplets + doublet + ASCII, EDI mode + final 3 ASCII",
                    "1110110000001000000010"
                    "1110100000101100001011"
                    "0000100010111100000010"
                    "0000101011111111101011"
                    "0110101000100010001000"
                    "0010100001000100010001"
                    "1000101000100010000110"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101101"
                    "1111101100010011101010"
                    "1101101001000001101110"
                    "0110100011111011101000"
                    "1100011001100110101011"
                    "1011110100111010100000"
                    "0001111010001000110010"
                },
        /* 58*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>ABC", -1, 0, 16, 18, 1, "3 EDI triplets + 3 alphas (EDI)",
                    "111011000000100000"
                    "111010000010110000"
                    "001010000000100101"
                    "101110000010111001"
                    "111010111100100000"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "100111111111100110"
                    "111001010000100011"
                    "011001101110101101"
                    "010011011100100111"
                    "000001001101111001"
                },
        /* 59*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>1234567", -1, 0, 22, 22, 1, "3 EDI triplets + 7 digits ending (EDI)",
                    "1110110000001000000010"
                    "1110100000101100001011"
                    "0000100010001000110011"
                    "0000101011000010000011"
                    "0111101111001110001000"
                    "0011101111100000010001"
                    "1000101000100010000000"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001100101"
                    "1011111111111010100100"
                    "1000111100110000100110"
                    "0010100010010111101000"
                    "0101100111000110101001"
                    "0110011101010010101010"
                    "0101111011000100110001"
                },
        /* 60*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>12345678", -1, 0, 22, 22, 1, "3 EDI triplets + 8 digits ending -> DECIMAL",
                    "1110110000001000000010"
                    "1110100000101100001011"
                    "0000100010111110001010"
                    "0000101011111111100100"
                    "1011101101100010001000"
                    "1010100000000100010001"
                    "1000101000100010001110"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101101"
                    "1101011110111101100011"
                    "1100101101000111100010"
                    "1000010100101111100101"
                    "0110001001101100100110"
                    "1101100100110011101111"
                    "1001110001001011110010"
                },
        /* 61*/ { -1, -1, -1, { 0, 0, "" }, "\015*>\015*>\015*>123456789012\015*>", -1, 0, 22, 22, 1, "3 EDI triplets + 12 DECIMAL not ending -> DECIMAL",
                    "1110110000001000000010"
                    "1110100000101100001011"
                    "0000100010111110001010"
                    "0000101011111111100100"
                    "1011101101110110000000"
                    "1010100000110011101110"
                    "0010100011100010001010"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "1011111100010001100011"
                    "0111101001010000100101"
                    "0010110110010110101010"
                    "1100111001111011100010"
                    "1000011000100100101001"
                    "1110010110011011100001"
                    "1100100100000011111100"
                },
        /* 62*/ { -1, -1, -1, { 0, 0, "" }, "123456789012345", -1, 0, 16, 18, 1, "5 DECIMAL triplets, ASCII switch before padding (2)",
                    "111111111111001100"
                    "000110000110010101"
                    "100010110101101111"
                    "000010010110111111"
                    "100010100011110010"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010011100010"
                    "101100110010100101"
                    "111000001001100010"
                    "100010110110101011"
                    "111111011010110001"
                },
        /* 63*/ { -1, -1, -1, { 0, 0, "" }, "1234567890123456", -1, 0, 16, 18, 1, "5 DECIMAL triplets + singlet, switch to ASCII before padding (2)",
                    "111111111111001100"
                    "000110000110010101"
                    "100010110101101111"
                    "000010010110110111"
                    "100010100010100110"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011110100000"
                    "111011001101101000"
                    "101011010111101100"
                    "000101110010100011"
                    "010101000001110111"
                },
        /* 64*/ { -1, -1, -1, { 0, 0, "" }, "12345678901234567", -1, 0, 16, 18, 1, "5 DECIMAL triplets + doublet, switch before ASCII encoding of final char, padding (1)",
                    "111111111111001100"
                    "000110000110010101"
                    "100010110101101111"
                    "000010010110110111"
                    "001110100011101001"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "100000010101101011"
                    "101101010000100010"
                    "011011100001101000"
                    "011100000111101100"
                    "100010110100110100"
                },
        /* 65*/ { -1, -1, -1, { 0, 0, "" }, "123456789012345678", -1, 0, 16, 18, 1, "6 DECIMAL triplets, switch to ASCII, padding (1)",
                    "111111111111001100"
                    "000110000110010101"
                    "100010110101101010"
                    "000010010110100111"
                    "111110100000101101"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "110100010010100001"
                    "010011110101101011"
                    "100010000011100100"
                    "101110000010100101"
                    "011010011110111101"
                },
        /* 66*/ { -1, -1, -1, { 0, 0, "" }, "1234567890123456789", -1, 0, 16, 18, 1, "6 DECIMAL triplets + singlet, switch before ASCII encoding of singlet, no padding",
                    "111111111111001100"
                    "000110000110010101"
                    "100010110101101010"
                    "000010010110100111"
                    "111110001111000011"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "110110100111101001"
                    "000001010011100011"
                    "101011011101101100"
                    "011001101011100100"
                    "101110111011110000"
                },
        /* 67*/ { -1, -1, -1, { 0, 0, "" }, "12345678901234567890", -1, 0, 16, 18, 1, "6 DECIMAL triplets + doublet, switch before ASCII 2-digit encoding of doublet, no padding",
                    "111111111111001100"
                    "000110000110010101"
                    "100010110101101010"
                    "000010010110100111"
                    "111110110110000111"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "110111000010100101"
                    "110000010000100111"
                    "011110101001100011"
                    "111110100110101101"
                    "001000111011111111"
                },
        /* 68*/ { -1, -1, -1, { 0, 0, "" }, "123456789012345678901", -1, 0, 16, 18, 1, "7 DECIMAL triplets, fills to final codeword, no padding",
                    "111111111111001100"
                    "000110000110010101"
                    "100010110101101010"
                    "000010010110100111"
                    "111010101111000101"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000111011011101100"
                    "000000000110101110"
                    "001011010101101011"
                    "111000110101100101"
                    "110001100000110100"
                },
        /* 69*/ { -1, -1, -1, { 0, 0, "" }, "1234567890123456789012", -1, 0, 16, 18, 1, "7 DECIMAL triplets + singlet, fills to final codeword, no padding",
                    "111111111111001100"
                    "000110000110010101"
                    "100010110101101010"
                    "000010010110100111"
                    "111010100010101100"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000111011001101110"
                    "010110011110101111"
                    "101000010001101000"
                    "100111000001100000"
                    "101000001010111101"
                },
        /* 70*/ { -1, -1, -1, { 0, 0, "" }, "123456789012345678901234567890123678901AB", -1, 0, 28, 32, 1, "DECIMAL + 4 bits ASCII switch",
                    "11111111111100110010001101110110"
                    "00011000011001010100000101101010"
                    "10101011101000101111100111100001"
                    "01111000011110100000111011101111"
                    "00101001110001111101000100101000"
                    "10101011101011111100100011100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "00001000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000000000"
                    "01111111111111111111111111111110"
                    "01000000000000000000000000000010"
                    "01111111111111111111111111111110"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010000110111001100000101011"
                    "00011000011011000011101111101001"
                    "11111001011001010000010011100010"
                    "10111001000000011101101001100000"
                    "10111010100111101101110011101010"
                    "10011010100110001011101000100101"
                    "11011010001110110010110100101110"
                    "01011111010001011011010000110111"
                },
        /* 71*/ { -1, -1, -1, { 0, 0, "" }, "123456789012345678901234567890123678901234A", -1, 0, 22, 22, 0, "DECIMAL + single ASCII in last 8 bits; BWIPP different encodation TODO: investigate",
                    "1111111111110011001000"
                    "0001100001100101010000"
                    "1101100110101011101000"
                    "0101101010011100011110"
                    "1011101110011100010010"
                    "1000100011101111111010"
                    "0111100001111001000001"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "1110100010110010101000"
                    "0110010011100000100000"
                    "0100100111101100101110"
                    "1010101110111110101001"
                    "1001001001100100101101"
                    "1111110100001111101111"
                    "0100010100010000111100"
                },
        /* 72*/ { -1, -1, 10, { 0, 0, "" }, "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", -1, 0, 16, 49, 1, "T-48 90",
                    "1111111111001100100011011011010101110100010111110"
                    "0001000110010101000001010101001110001111010000011"
                    "0111000100011001010100000010110100111000111101000"
                    "1011111111001100100011010011010101110100010111110"
                    "0011101111111100110010000110101101010111010001011"
                    "0111000100011001010100000010110100111000111101000"
                    "1110011111101101100001000001001100101010010111010"
                    "0011101111100111100011110011101001101100010110101"
                    "0011110011111001001110110011010010001001010011011"
                    "1000100101011101110111000000111110000010001000010"
                    "0000000000000000000000001000000000000000000000000"
                    "1111111111111111111111111111111111111111111111111"
                    "1000000000000000000000001000000000000000000000001"
                    "1011111111111111111111111111111111111111111111101"
                    "1000000000000000000000001000000000000000000000001"
                    "1011111111111111111111111111111111111111111111101"
                },
        /* 73*/ { -1, -1, 10, { 0, 0, "" }, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", -1, ZINT_ERROR_TOO_LONG, 0, 0, 1, "T-48 89",
                    ""
                },
        /* 74*/ { -1, -1, -1, { 0, 0, "" }, "A123456789012345678901A", -1, 0, 22, 22, 1, "ASCII + 7 DECIMAL triplets + ASCII",
                    "0100111111111111001100"
                    "0010100001000110010101"
                    "1000101101011010101110"
                    "0000100101101001110001"
                    "1011100100100010001000"
                    "1111100010000100010001"
                    "1000101000100010001000"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101110"
                    "0110110011111100100001"
                    "0100111101011000101001"
                    "1110011001111101100000"
                    "0100010001010001101011"
                    "1010001010001011101101"
                    "1101101001100001110101"
                },
        /* 75*/ { -1, -1, -1, { 0, 0, "" }, "A1234567890123456789012A", -1, 0, 22, 22, 1, "ASCII + 7 DECIMAL triplets + singlet + ASCII",
                    "0100111111111111001100"
                    "0010100001000110010101"
                    "1000101101011010101110"
                    "0000100101101001110001"
                    "1011100011010010001000"
                    "1111100011001000010001"
                    "1000101000100010000000"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101000"
                    "1011001000110010100111"
                    "0001000000011010100011"
                    "0100110111001011100110"
                    "0100110111111110101111"
                    "0101101010001000100111"
                    "1110011111011111110100"
                },
        /* 76*/ { -1, -1, -1, { 0, 0, "" }, "A12345678901234567890123A", -1, 0, 22, 22, 1, "ASCII + 7 DECIMAL triplets + doublet + ASCII",
                    "0100111111111111001100"
                    "0010100001000110010101"
                    "1000101101011010101110"
                    "0000100101101001110001"
                    "1011101001010010001000"
                    "1111101001001000010001"
                    "1000101000100010001110"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101011"
                    "0100111101100011100011"
                    "1011010001011001101100"
                    "0110100100111100100101"
                    "0000100001000101101101"
                    "0001111010010111100010"
                    "0001001001000000110100"
                },
        /* 77*/ { -1, -1, -1, { 0, 0, "" }, "ABCDEFGHI123456789012A", -1, 0, 22, 22, 1, "3 C40 triplets + 4 DECIMAL triplets + ASCII 0 padding",
                    "1110110101111001100010"
                    "0110101001100111010100"
                    "1000100101111110001010"
                    "0000101111111111100100"
                    "1011101101110110000100"
                    "1010100000110011100010"
                    "1000101000100010001001"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101001"
                    "1011001000111110100100"
                    "0010110001001001101100"
                    "0101101100110101100011"
                    "1011000110000101101111"
                    "0011001101000011100011"
                    "0100110101010110111011"
                },
        /* 78*/ { -1, -1, -1, { 0, 0, "" }, "ABCDEFGHI12345678", -1, 0, 22, 22, 1, "3 C40 triplets + ASCII 2-digits end-of-data",
                    "1110110101111001100010"
                    "0110101001100111010100"
                    "1000100101111110001010"
                    "0000101111111111100100"
                    "1011101101100010001000"
                    "1010100000000100010001"
                    "1000101000100010001011"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101011"
                    "1100001011100110100001"
                    "0001001101101111100011"
                    "1101011100100001100101"
                    "0001011111001001100101"
                    "1010111111111110101011"
                    "1101101100101010110101"
                },
        /* 79*/ { -1, -1, -1, { 0, 0, "" }, "ABCDEFGH123456789012345678901A", -1, 0, 22, 22, 1, "2 C40 triplets + doublet + ASCII 2-digits ASCII 0 padding",
                    "1110110101111001100010"
                    "0110101001100111010100"
                    "1000100100111110011010"
                    "0000101110111110011111"
                    "1100101101100010011010"
                    "0101101011001110011111"
                    "1100101101100001001011"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0101101100110010100111"
                    "0101100110010101101001"
                    "1011110110000101100001"
                    "1001100010001101100000"
                    "1001001001000101100000"
                    "0010110111111111101111"
                    "1010010000100000110111"
                },
        /* 80*/ { -1, -1, -1, { 0, 0, "" }, "\101\102\103\104\105\106\107\110\111\112\113\200", -1, 0, 22, 22, 1, "3 C40 triplets + doublet + extended ASCII - note extended ASCII not encodable in C40 mode",
                    "1110110101111001100010"
                    "0110101001100111010100"
                    "1000100101100110001011"
                    "0000101111001100101011"
                    "1000101111100010001000"
                    "0001101111000100010001"
                    "1000101000100010000001"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101001"
                    "1001001101110100100010"
                    "1111100010000001101100"
                    "0000001011000000100010"
                    "0010111110100000100000"
                    "1010000101100100100110"
                    "0101111100110010110010"
                },
        /* 81*/ { -1, -1, -1, { 0, 0, "" }, "\200\200", -1, 0, 16, 18, 1, "BYTE",
                    "111011000010001000"
                    "011110001000000000"
                    "100010100010001000"
                    "000110000100010001"
                    "100010100000110111"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011100101011"
                    "001001101101100111"
                    "001011110000101001"
                    "010011001011100111"
                    "110100011101110111"
                },
        /* 82*/ { -1, -1, -1, { 0, 0, "" }, "\200\200\200\200\200\200\200\200", -1, 0, 16, 18, 1, "BYTE (no padding, byte count 0)",
                    "111011000010001000"
                    "011110000000000000"
                    "100010100010001000"
                    "000010000000000000"
                    "100010100010101110"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000000001111100111"
                    "110101101010101101"
                    "000011001111100001"
                    "111101011011100111"
                    "101110100100111000"
                },
        /* 83*/ { -1, -1, -1, { 0, 0, "" }, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066\067\070\071\060\061\062\063", -1, 0, 70, 76, 1, "249 BYTEs + 13 DECIMAL",
                    "1110111111100010001000100011100010001000100010001110001000100010001000111000"
                    "0111101001000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000111000100010001000100011100010001000100010001110001000100010001000111000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000111000100010001000100011100010001000100010001110001000100010001000111000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000111111111100101100"
                    "0000100000000000000000000010000000000000000000001000000000000100011001100101"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000010000000000000000000000000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000010000000000000000000000000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000000000000000000000000000000000000000000000000000000"
                    "0111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "0100000000000000000000000000000000000000000000000000000000000000000000000010"
                    "0111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "1000101101110110001000100010100010001000100010001010001000100010001000101000"
                    "0000101111000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001110001000100010001000111000100010001000100011100010001000100010001110001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000000000000111100011"
                    "0001100001000100010001000110000100010001000100011000010001011011011110101011"
                    "1101100000111100100110000110000010011011010011001000110100111100000010100110"
                    "0000100100111111111001000010100000111100001101011011101011111101001111100110"
                    "0110101100100110110000111110010000101000110001101011010101000100010010100001"
                    "0101111010111101111000110011000001000111111011101100011100011010010111111110"
                    "0100101011011100111101100010011101110101001000101000100100000100010010100100"
                    "0100100011111101111101011110001101010001100011011010001011000000011100100100"
                    "1110101000101010001110101010010011000001011000001001011001011101010011100101"
                    "1010101111110111000110011010110101010001111000101010011010100101100100101011"
                    "1101100111010011110000010010000001101111110111011011101001010001101101100100"
                    "0011101010101010101100111010110100101000011000001001011000100111011011100001"
                    "0000100001100100110111010110110010110100000100011000100101111111110101100001"
                    "1110100010111001100110000110101110000101001110111001010100011110111110100110"
                    "0110100010111000001101100010011011101101111010111011010100111000101010101101"
                    "0010100011101011110111110110111001101001100101011000110001000000111100101011"
                    "0000101101010101000011010110011110110100100111111011100001010010000111100100"
                    "1101111110001011101100010011101010100111101110111101101000010100111000111001"
                },
        /* 84*/ { -1, -1, -1, { 0, 0, "" }, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066\067\070\071\060\061\062\063", -1, 0, 70, 76, 1, "250 BYTEs + 13 DECIMAL",
                    "1110111111000010001000100011100010001000100010001110001000100010001000111000"
                    "0111101010000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000111000100010001000100011100010001000100010001110001000100010001000111000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000111000100010001000100011100010001000100010001110001000100010001000111000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1000101000100010001000100010100010001000100010001010001000100010001111101111"
                    "0000100000000000000000000010000000000000000000001000000000000000000001100001"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000010000000000000000000000000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000010000000000000000000000000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000000000000000000000000000000000000000000000000000000"
                    "0111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "0100000000000000000000000000000000000000000000000000000000000000000000000010"
                    "0111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "1100101100100011011101100010100010001000100010001010001000100010001000101000"
                    "1001100101000011110001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001110001000100010001000111000100010001000100011100010001000100010001110001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100000010101100010"
                    "0001100001000100010001000110000100010001000100011000010001100011101101100000"
                    "1010100001101111100000000010010011001010111100011000101010100011100000101000"
                    "0111101101100100101100101110010111011011000100011000011000100011100011100101"
                    "1011101000111001101101000110000010100011001011011011100001101111110110100100"
                    "0100111100000111001011001011110010000001000011011111001001010100111110110011"
                    "0110100010101111101100111110110100010111010000011000001100110111110110101010"
                    "1011100010111111000000000110001101000111110011011011110100010011110010101010"
                    "0101100001110111111101100110010101110000111111001001111110111100000101101001"
                    "0000100110000111111101000110010110001100010100111000010000001101001111101111"
                    "0011101111110000010101101110010111001100111101001010010100010011000100100011"
                    "0001101011000000100111010010010111001110100111101011100100110010111001101100"
                    "0110100000111010110101100010101100111100111110111000001101001110001111101010"
                    "1001101111111101101000110110000000100001011110101000110110101111101111101111"
                    "1111101110000111111010000110101010100000110100101000001101111101110100101000"
                    "1011100011101001011011001010110001001010111111111011001001101111010110100011"
                    "1111101101101101100101000110010011000011110001101001001011010001000001100100"
                    "1110110001110110001100000011110011001110000101001111000111011001010011111110"
                },
        /* 85*/ { -1, -1, -1, { 0, 0, "" }, "\061\062\063\064\065\066\067\070\071\060\061\062\063\064\065\066\067\070\071\060\061\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066\067\070\071\060\061\062\063", -1, 0, 104, 98, 1, "21 DECIMAL + 501 BYTEs + 13 DECIMAL",
                    "11111111111100110010001101011110101011101011111011111100001000100010011010001000100010001011001000"
                    "00010010011001010100000101110010011100011111011110101100010000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001011001000100010001000111000100010001000100011100010001000100010011010001000100010001011001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001011001000100010001000111000100010001000100011100010001000100010011010001000100010001011001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001011001000100010001000111000100010001000100011100010001000100010011010001000100010001011001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "00000000000000000000000000010000000000000000000000000000000000000000010000000000000000000000000000"
                    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "10001010001000100010001000110000100011111111110010110010001101110110010010001000100010001010001000"
                    "00000010000000000000000000010000000000010001100110010100001111000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010011010001000100010001011001000100010001000111000100010001000100011100010001000100010011010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000010101111100110001001110010100000010101111110110000111111110100110000011"
                    "00010011010001000100010001011010001100101100001111001111110110101000111111011111000101101011001001"
                    "11100110110111110010111110110011111011000100101010110000100011101010110010110111100011100010101000"
                    "00111110111100010011100011010001010100010101100010111001111100011110010001110010011000011010000000"
                    "10100110000000001001011110110001001111100100111010001011111010000001110010100010010110000010011110"
                    "01011010111101101100001000110000111010100100010110010110101101010011110011000001110100110010100101"
                    "10101110101000001111001100110010001001100110000110101100100011010001010000100101111000010010000000"
                    "01100110000011111101000100110011010100101100001010000110111010111000010000110101011100010110001011"
                    "01001010011110111111000101110011000110110111001110101001110110111000110101110010101010111010000010"
                    "00111110101000000010010100110010001010001000000110100000101110111010010111010101011001000010100001"
                    "01100110010110010010111011010110101010110000110110011100010001001101110001011100000001100010100010"
                    "00000110110101100100101101010001110010001101100110000101111101010100010001000100101000010010011010"
                    "01100110011110101001000101110111001010111111111110101101111101111101010100000010000111101010100010"
                    "11011011001011110101010000111111110011010111110111011001011000011010011011000001011110001011011010"
                    "10011010111000000101101111010110110001011101110110010011101001010000010001101100100110111010111011"
                    "00010010111011011110110110110111011010101101110010010111111101000011110110001110011000001010111010"
                    "10101010110001111111010101110110000001011011011010100100100011101011110110101101111111001110100000"
                    "00110010111100010110011101010000101010110110100010001000110111111100110011101111111001001110000101"
                    "10110110011111110110101010010111111001100000100110010000101010100110010110110000010101101010010011"
                    "01101110011000001100101000010101111100010110000110011011011111111110010010110001010011011010010100"
                    "01011110010111000111001101010000010101011000110110111000000100001110010000010011100100101010110001"
                    "10001110110111000010110010010000011110111011000010001001110010010111110111010001100000011110001101"
                    "00100010101001000001101111110011101100101011010010011001100001101000110010111111010000011010000000"
                    "01001010000100010111101011110000010100101101011110011010010011010101010111011111111010000110100000"
                    "11001110100000010100001001010010000101101110100110101100011011001110010101111001100100110010001101"
                    "01110011001111110111010101111010111001011100001111011101001011010000011011001100100000101011000101"
                },
        /* 86*/ { -1, -1, -1, { 0, 0, "" }, "\061\062\063\064\065\066\067\070\071\060\061\062\063\064\065\066\067\070\071\060\061\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066\067\070\071\060\061\062\063\064\065\066\067\070\071\060\061\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066\067\070\071\060\061\062\063", -1, 0, 104, 98, 1, "21 DECIMAL + 251 BYTEs + 21 DECIMAL + 252 BYTEs 13 DECIMAL",
                    "11111111111100110010001101011110101011101011111011111100001000100010011010001000100010001011001000"
                    "00010010011001010100000101110010011100011111011110101000010000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001011001000100010001000111000100010001000100011100010001000100010011010001000100010001011001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "11111111111100110010001101011110101011101011111011111100001000100010011010001000100010001011001000"
                    "00010010011001010100000101110010011100011111011110101000100000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001011001000100010001000111000100010001000100011100010001000100010011010001000100010001011001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "00000000000000000000000000010000000000000000000000000000000000000000010000000000000000000000000000"
                    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010"
                    "01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00000010000000000000000000010000000000000000000010000000000000000000010000000000000000000010000000"
                    "10001110111111110011001000110101110110001000100010100010001000100010010010001000100010001010001000"
                    "00000010010001100101010000110111000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010011010001000100010001011001000100010001000111000100010001000100011100010001000100010011010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110000100010001000100010100010001000100010010010001000100010001010001000"
                    "00010010010001000100010001010001000100010001000110000100010001000100010100010001000100010010010001"
                    "10001010001000100010001000110011000010101111110110110010100010001101110000110101111001011110011111"
                    "00010011010001000100010001011001111110010101010111011100110101000111111101011100100000000011100110"
                    "11001010011011010110001001110100001001000100011010001001011101000111110101001001111100011010010010"
                    "10000010110001101110010110010001110111011000001110100001000101100100010000011011010000010010011000"
                    "00010010010011001101011100010100111100011010001010000110000111001010010111011110111100011110110101"
                    "00000010100100110111111101110000101101010101001110010010110010100100010010111100011011110110001100"
                    "11010110100100100110010110010101010000010100111110111000110001101101110101000010011101011110000010"
                    "01100010011111110001001000010110001001011010001110101011100010001011110111111010011100111110010000"
                    "00101010000111011011000010010111011110110110000010001100110010101111010111000110011110101110000001"
                    "10000010101000010000100101110001011111110001111110001011111110100110010011001100100110100110001110"
                    "01000010001011010010101110010011100101000011101110010111101000100100010100011010101110000110010001"
                    "10000010000001111111101110010100011111101000010010010011101111111001110010100110110010101110101111"
                    "00110010010111000101110010010101110111000100011010000111001110001001010100111001000000010010101001"
                    "10011011111110010110100101011011100111100000011111111000100101010111011100000001011100001011011101"
                    "00010010101111110101101101010001111000011011101010010100001000100101110110110001100111010110101111"
                    "10110010001011100010101000010010110000110101001010100011111010000101010101001111110000010110000010"
                    "10001110100001100111011011010100110000011100111110011110000010111011110101100011000001000010011101"
                    "00010010011011110100110100110001100111101101011110111010111000101010110011100100101001000010000011"
                    "00011010100000000001011100010111110011110011011010010010001000001011110000110010001000100010010001"
                    "00011110011101000010010010110010000001011000100010000011110010101001010111001110011110010010101100"
                    "00111110001100010111011110010111101000000111011010010001101111001101110010110100101010101010001110"
                    "01100010011111000111101001010111100100110000001110110000100001001001010101100010101001111010011001"
                    "01100010100000100010000100010000010111111011110010010001111110110110110100110110010001110110010101"
                    "01010110101110000111010001010011001111110001100110101110101100101101110111100010101111100110000101"
                    "01101010000000111001110100110111110011111001000110101010111111101011110001100001110101000110111101"
                    "01111111101011000101001010011010111100011101111011110000111111000110111010011101101111001111011011"
                },
        /* 87*/ { -1, 3, -1, { 0, 0, "" }, "\351", -1, 0, 16, 18, 1, "é in ISO 8859-1",
                    "100011010101011000"
                    "000110110111010010"
                    "100010100011100110"
                    "001010010110111010"
                    "100010100001001010"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010000100101"
                    "111101010011101001"
                    "101011100101101010"
                    "001010011001100100"
                    "101100110110110101"
                },
        /* 88*/ { -1, 899, -1, { 0, 0, "" }, "\134\134\351\142\154\134\141\150\134", -1, 0, 28, 32, 1, "<bksh>ébl<bksh>ah<bksh>",
                    "10001101010101100010001110110101"
                    "00011011011101001010100101101101"
                    "01011001010101111001100110100110"
                    "11011011011101101110100011101101"
                    "01011001010110011001010101101000"
                    "11011011010010100111011101100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "00001000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000000000"
                    "01111111111111111111111111111110"
                    "01000000000000000000000000000010"
                    "01111111111111111111111111111110"
                    "10001010001000100010001000101000"
                    "00011000010001000100010001100001"
                    "10001010000000110100111111101001"
                    "00011000010100010010010100101010"
                    "01111000001000111111111111101111"
                    "01111011010110101000000101100011"
                    "11111010000111010011000010100010"
                    "10001000010111011010111000100111"
                    "11011011010100010100111000101011"
                    "11011110101110100011111110110011"
                },
        /* 89*/ { GS1_MODE, -1, -1, { 0, 0, "" }, "[90]12[91]1234567890123A", -1, 0, 22, 22, 1, "Step B4",
                    "1110111101100011101110"
                    "1000101100111011000100"
                    "0000101011111001110001"
                    "1110101000001110111111"
                    "0011100100100010001000"
                    "1111100010000100010001"
                    "1000101000100010000111"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001100001"
                    "1001100110011111101011"
                    "1000101110111110100010"
                    "0010010011111111100100"
                    "1110011100011111101010"
                    "0100001001100001101110"
                    "1110110100001101111001"
                },
        /* 90*/ { GS1_MODE, -1, -1, { 0, 0, "" }, "[90]12[91]12345", -1, 0, 16, 18, 1, "Step B5",
                    "111011110110001110"
                    "100010110011101100"
                    "111010000010111101"
                    "010010111011111001"
                    "100010100000001010"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100011001101110"
                    "101011011100101110"
                    "111100010101100000"
                    "011100010001100101"
                    "101000101000110110"
                },
        /* 91*/ { GS1_MODE, -1, -1, { 0, 0, "" }, "[10]AB[90]ABCDEFGHIJKLMNOP[91]ABCDEFGHIJKLMNOPQRSTUVWXYZ", -1, 0, 28, 32, 1, "Step P",
                    "11101110000100010011101101111110"
                    "10001011000010001110001100100110"
                    "01011011100110001010000101101001"
                    "10011010011101010000001111100011"
                    "10011010101101101110000101100001"
                    "10101001100101010101000010100111"
                    "01101001010111100010001100101001"
                    "00001000100011110101101000101010"
                    "00001010100011110001111101101011"
                    "00111011011110000010010011100100"
                    "00001000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000100000"
                    "11111111111111111111111111111111"
                    "00000000000000000000000000000000"
                    "01111111111111111111111111111110"
                    "01000000000000000000000000000010"
                    "01111111111111111111111111111110"
                    "11101011101111010110001000101000"
                    "01101011111111101100010001100001"
                    "10001010000010110100001000100011"
                    "00011000011100110001011011100011"
                    "11001011010100110100000101100011"
                    "10111000010010101001100000100100"
                    "01101001000001010101110001100011"
                    "00011011010110111001110010100100"
                    "01111000010000110101101110100100"
                    "00011110111111111110101011110110"
                },
        /* 92*/ { GS1_MODE, -1, -1, { 0, 0, "" }, "[11]121212", -1, 0, 16, 18, 1, "Start FNC1 and Decimal",
                    "111011000100000101"
                    "110010110011011111"
                    "110010001110001000"
                    "100110001100010001"
                    "100010100001011101"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000100010100101010"
                    "101010101001100101"
                    "110011110100100111"
                    "000111100011101101"
                    "100101110101111101"
                },
        /* 93*/ { -1, -1, 1, { 1, 15, "" }, "ABCDEFGHIJ", -1, 0, 16, 18, 1, "Structured Append (Group mode) 1st symbol, no ECI",
                    "000011111011100101"
                    "111010100101101001"
                    "111010011000101000"
                    "100110110101000000"
                    "010110010011010000"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "111110111110100100"
                    "000001000110100000"
                    "011011101111100000"
                    "010110110000100111"
                    "101110110100110101"
                },
        /* 94*/ { -1, -1, 1, { 2, 15, "" }, "KLMNOPQRST", -1, 0, 16, 18, 1, "Structured Append (Group mode) subsequent symbol, no ECI",
                    "000111111011101001"
                    "110110100101101010"
                    "000010101000111100"
                    "001110110111100000"
                    "011110010101010101"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "100101010000100101"
                    "100010011111100000"
                    "001110111110101011"
                    "010101010000100001"
                    "110100011010111010"
                },
        /* 95*/ { -1, 3, 1, { 1, 15, "" }, "AB", -1, 0, 16, 18, 1, "Structured Append (Group mode) 1st symbol, with ECI",
                    "100011111000000101"
                    "000110100111101101"
                    "010110100010001000"
                    "110110001000100101"
                    "010010010010101011"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "001000110101100110"
                    "110100010011101110"
                    "010010010011100011"
                    "100001000010100000"
                    "000110111011111101"
                },
        /* 96*/ { -1, 3, 1, { 2, 15, "" }, "CDEF", -1, 0, 16, 18, 1, "Structured Append (Group mode) subsequent symbol, with ECI",
                    "000111111001011000"
                    "110110100111010010"
                    "100010100001000100"
                    "001010010101000101"
                    "010010010000110111"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "011001111110101000"
                    "101101010111100000"
                    "111001110100100111"
                    "010010010100100110"
                    "000100100101110110"
                },
        /* 97*/ { -1, -1, 1, { 128, 128, "" }, "ABCDEFGHI", -1, 0, 16, 18, 1, "Structured Append (Extended Group mode)",
                    "100011100011101110"
                    "000010000010010110"
                    "010110111001100010"
                    "100110100111010100"
                    "100010010111101110"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "000011111001100010"
                    "111110000001101110"
                    "011011001000101010"
                    "000001010100101011"
                    "110100001010111101"
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[8192];
    char bwipp_buf[32768];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODEONE, data[i].input_mode, data[i].eci, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (p_ctx->generate) {
            printf("        /*%3d*/ { %s, %d, %d, { %d, %d, \"%s\" }, \"%s\", %d, %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_2,
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)), data[i].length,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            if (ret < ZINT_ERROR) {
                testUtilModulesPrint(symbol, "                    ", "\n");
            } else {
                printf("                    \"\"\n");
            }
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (%s)\n",
                    i, symbol->rows, data[i].expected_rows, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)));
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d (%s)\n",
                    i, symbol->width, data[i].expected_width, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)));

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d (%s)\n",
                    i, ret, width, row, testUtilEscape(data[i].data, length, escaped, sizeof(escaped)));

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, -1, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwipp(i, symbol, -1, data[i].option_2, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf), NULL);
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int option_2;
        struct zint_structapp structapp;
        struct zint_seg segs[3];
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        const char *comment;
        const char *expected;
    };
    /* Figure examples AIM USS Code One (USSCO) Revision March 3, 2000 */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 16, 18, 1, "Standard example",
                    "100011010111100011"
                    "000110110110110111"
                    "010110100010001000"
                    "110110001000101001"
                    "111010001111111011"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "101101111011100110"
                    "100001001111100110"
                    "010110111000101011"
                    "001010010011101001"
                    "010100101110110001"
                },
        /*  1*/ { UNICODE_MODE, -1, { 0, 0, "" }, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 16, 18, 1, "Standard example auto-ECI",
                    "100011010111100011"
                    "000110110110110111"
                    "010110100010001000"
                    "110110001000101001"
                    "111010001111111011"
                    "000010000000100000"
                    "111111111111111111"
                    "000000000000000000"
                    "011111111111111110"
                    "010000000000000010"
                    "011111111111111110"
                    "101101111011100110"
                    "100001001111100110"
                    "010110111000101011"
                    "001010010011101001"
                    "010100101110110001"
                },
        /*  2*/ { UNICODE_MODE, -1, { 0, 0, "" }, { { TU("Ж"), -1, 7 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, 0, 22, 22, 1, "Standard example inverted",
                    "1000110101010110001000"
                    "0001101101110100100010"
                    "1000101110001101011000"
                    "1001101011011111010010"
                    "1000101000111000111000"
                    "0010100101101101110001"
                    "1000101000100010001101"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101111"
                    "1010110111011111100100"
                    "1100011010110110101000"
                    "1111001110101001101101"
                    "1011101010111001100100"
                    "0010011101100011101011"
                    "0011010111000001111101"
                },
        /*  3*/ { UNICODE_MODE, -1, { 0, 0, "" }, { { TU("Ж"), -1, 0 }, { TU("¶"), -1, 0 }, { TU(""), 0, 0 } }, ZINT_WARN_USES_ECI, 22, 22, 1, "Standard example inverted auto-ECI",
                    "1000110101010110001000"
                    "0001101101110100100010"
                    "1000101110001101011000"
                    "1001101011011111010010"
                    "1000101000111000111000"
                    "0010100101101101110001"
                    "1000101000100010001101"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0001000100010001101111"
                    "1010110111011111100100"
                    "1100011010110110101000"
                    "1111001110101001101101"
                    "1011101010111001100100"
                    "0010011101100011101011"
                    "0011010111000001111101"
                },
        /*  4*/ { UNICODE_MODE, -1, { 0, 0, "" }, { { TU("product:Google Pixel 4a - 128 GB of Storage - Black;price:$439.97"), -1, 3 }, { TU("品名:Google 谷歌 Pixel 4a -128 GB的存储空间-黑色;零售价:￥3149.79"), -1, 29 }, { TU("Produkt:Google Pixel 4a - 128 GB Speicher - Schwarz;Preis:444,90 €"), -1, 17 } }, 0, 70, 76, 0, "AIM ITS/04-023:2022 Annex A example; BWIPP different encodation",
                    "1000110101010110001000100011111010110011011010101111000111000010111011110011"
                    "0001101101110100100010010110111110100101111100011011101000110101010010101010"
                    "0111101111011010010111011010001100110000001000101000110001000100001101101011"
                    "0000101011011101100100110010010001001000010000001011010011100011000100100001"
                    "1111100000100110111110011110110100000010000011101001010001000011011100100100"
                    "1100101111101000110111111110010010000100110010101010101001100011100101100001"
                    "0111101011111100101010001110001011100101100010001010011110001111001011101100"
                    "0000101000111101011101101010111100111101001000101011110111101001100111100011"
                    "1111100011010001100110011010011001100010101111001010111110001001010110100111"
                    "1011101010011111111111011110110001010000100110001010001000000000001001101000"
                    "0110100110001000110110001010001000110011001100101001000100101111001011101110"
                    "0101101100000001000001000010110100010010100000001001110010010101000100100110"
                    "1011111010101111011011111011001010111101110010101100111100111011001101111011"
                    "0100100010111101011100010010110110101010100110111010110001001110101011101100"
                    "1101100011101010101010101110001011010101100010001010010101111011000011101101"
                    "1011101010001101000001001110111100011101001000101000110001111101100010101000"
                    "0110100000100100110011100010111100010010100001011010011100010111110100100111"
                    "0010101000101100001101000010101100110001111110111011000001011110101011101110"
                    "0010100110000010010000111110011101011000000101111001010000001000001000101000"
                    "0111100100110110111100101110101110111100011001011011001000010011111001101000"
                    "1110101100110101100110110010101011000011111110101000110010110100101110100010"
                    "1111100111101001000001010010011110001000111111101001011101110000011011100101"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000111000100010001000100011100010001000100010001110001000100010001000111000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "0000100000000000000000000010000000000000000000001000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000010000000000000000000000000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000010000000000000000000000000000000000000000000100000"
                    "1111111111111111111111111111111111111111111111111111111111111111111111111111"
                    "0000000000000000000000000000000000000000000000000000000000000000000000000000"
                    "0111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "0100000000000000000000000000000000000000000000000000000000000000000000000010"
                    "0111111111111111111111111111111111111111111111111111111111111111111111111110"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001110001000100010001000111000100010001000100011100010001000100010001110001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000100010001000101000"
                    "0001100001000100010001000110000100010001000100011000010001000100010001100001"
                    "1000101000100010001000100010100010001000100010001010001000111001101111100110"
                    "0001100001000100010001000110000100010001000100011000010001000110100010100101"
                    "0101101000101000110010110110001110100100110101001000001100000010011101100010"
                    "0100101100111011100101111110111100100011111000111010110101000110001000100111"
                    "1011100010000010011110111110111011110111001100111000101011101100001101100001"
                    "0011111101010101000010110011010000101111001101011101101100000011001101111110"
                    "1001101110000000010100100110001000011000010000001001100010100101011001100000"
                    "1010100110110100010001001110110010011100001101001000000010100010111111100101"
                    "0001101110111011011001101110001100010010110101111000101011101001111000100000"
                    "0100100011110110101101100110010110110100101011001011011000011001000111100011"
                    "1010101111011001001000100110010011010011110100101010010111000101001001100110"
                    "1011100010101101111110010010001011100010010101111001001111010100011111100001"
                    "0110101001011011001101100110110001100100010000101011010010101001110100100110"
                    "1101101000100011101100101110100110011111100111111001010100100101111101100010"
                    "1100100010111011001011100010100100011110001000101011000010100011101011101111"
                    "1110100101000010010101100010010111010111001110011010010101111110100000100011"
                    "0110101011010011001101110110100011011101001011011001000100000001110011101110"
                    "1110110001111000110010010111100110011010101100011101011011100000011001110111"
                },
        /*  5*/ { UNICODE_MODE, -1, { 0, 0, "" }, { { TU("price:$439.97"), -1, 3 }, { TU("零售价:￥3149.79"), -1, 29 }, { TU("Preis:444,90 €"), -1, 17 } }, 0, 40, 42, 1, "AIM ITS/04-023:2022 Annex A example price only",
                    "100011010101011000101100100001110111110110"
                    "000110110111010010001010010100010011101010"
                    "011010011000110010101010001100101110100101"
                    "010010011010110101111001101011110011101101"
                    "100010100010011110001000110011101100101101"
                    "001010001011110111101001000100111010101011"
                    "101110110100111010101010101010110010101101"
                    "110010101110100011011000000100111111100001"
                    "010110100010001001011001011101100110100111"
                    "110110001000100011001001001101101010100100"
                    "001110101000110010111001001011100010101000"
                    "101110111001011101111000000110110101100001"
                    "100011100010001000101100100010001000111000"
                    "000110000100010001001001000100010001100001"
                    "100010100010001000101000100010001000101000"
                    "000010000000000000001000000000000000100000"
                    "111111111111111111111111111111111111111111"
                    "000000000000000000000000000000000000000000"
                    "011111111111111111111111111111111111111110"
                    "010000000000000000000000000000000000000010"
                    "011111111111111111111111111111111111111110"
                    "010000000000000000000000000000000000000010"
                    "011111111111111111111111111111111111111110"
                    "010000000000000000000000000000000000000010"
                    "011111111111111111111111111111111111111110"
                    "000110000100010001001001000100010001100001"
                    "100010100010001000101000100010001000101000"
                    "000111000100010001001101000100010001110001"
                    "100010100010001000101000100010001000101000"
                    "000110000100010001001001000100010001100001"
                    "100010101101011000011000010110000000101110"
                    "000110111010101011001011011010010011100010"
                    "100010000110000101101011111101110001100010"
                    "010110010000111010111000101011000010101010"
                    "111010111101001011001010101000001111100000"
                    "011110101101000001001001011001010000100111"
                    "110110011011111111111000000001111100101100"
                    "110110001001101110101011000011000000101111"
                    "100110111001001100101001010010001011100111"
                    "111011100100101100101111011011100100111101"
                },
        /*  6*/ { DATA_MODE, -1, { 0, 0, "" }, { { TU("\266"), 1, 0 }, { TU("\266"), 1, 7 }, { TU("\266"), 1, 0 } }, 0, 22, 22, 1, "Standard example + extra seg, data mode",
                    "1000110101111000110101"
                    "0001101101101101111101"
                    "1000101000100011100011"
                    "0010100010100110110111"
                    "0101101000100010001110"
                    "1101100010001001011011"
                    "0011101000100010000010"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0111000100010001100011"
                    "1100010110011101101111"
                    "1100000111010111101001"
                    "0010000001001111101110"
                    "1100100101010100100110"
                    "1110101101100111101011"
                    "1001101001111010111100"
                },
        /*  7*/ { UNICODE_MODE, -1, { 1, 15, "" }, { { TU("A"), -1, 3 }, { TU("B"), -1, 4 }, { TU("C"), -1, 5 } }, 0, 22, 22, 1, "Structured Append (Group mode) 1st symbol, with ECI",
                    "1000111110000001010101"
                    "0001101001111011011101"
                    "1000101000100001000101"
                    "0010100010010100101101"
                    "1000101000100001000101"
                    "0010100010011000111101"
                    "1000101000100001000111"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0010001001110100100100"
                    "0010010011001111101011"
                    "1101000100011110101010"
                    "0000000001000101101000"
                    "1101010110111101101011"
                    "0011010110111100101011"
                    "1111101011110000110101"
                },
        /*  8*/ { UNICODE_MODE, -1, { 3, 15, "" }, { { TU("A"), -1, 3 }, { TU("B"), -1, 4 }, { TU("C"), -1, 5 } }, 0, 22, 22, 1, "Structured Append (Group mode) subsequent symbol, with ECI",
                    "0010111110010110001000"
                    "1100101001110100100010"
                    "1000100100010110001000"
                    "0101100010110100100010"
                    "1000100100010110001000"
                    "0110100011110100100010"
                    "1000100100100010001100"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0111010000010001101101"
                    "0001110000100010101101"
                    "0101011110011000101110"
                    "1111110110000110100111"
                    "0100001100101001100010"
                    "1111010111000111100011"
                    "0010100111110110110101"
                },
        /*  9*/ { UNICODE_MODE, -1, { 128, 128, "" }, { { TU("A"), -1, 3 }, { TU("B"), -1, 4 }, { TU("C"), -1, 5 } }, 0, 22, 22, 1, "Structured Append (Extended Group mode)",
                    "1000111000111001011000"
                    "0000100000100111010010"
                    "1000101000010001011000"
                    "0010100101001011010010"
                    "1000101000010001011000"
                    "0010100110001111010010"
                    "1000101000010010000001"
                    "0000100000000000100000"
                    "1111111111111111111111"
                    "0000000000000000000000"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0100000000000000000010"
                    "0111111111111111111110"
                    "0010011101000001101001"
                    "1111101010101110101001"
                    "0111011000100101100010"
                    "1111010000001010101101"
                    "0010001100101010101111"
                    "0000110101011010101000"
                    "0110000110100100110110"
                },
        /* 10*/ { UNICODE_MODE, 9, { 0, 0, "" }, { { TU("A"), -1, 3 }, { TU("B"), -1, 4 }, { TU("C"), -1, 5 } }, ZINT_ERROR_INVALID_OPTION, 0, 0, 1, "Multiple segments not suppoted for Version S",
                    ""
                },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol = NULL;

    char escaped[8192];
    char bwipp_buf[32768];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, BARCODE_CODEONE, data[i].input_mode, -1 /*eci*/,
                            -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/,
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
            printf("        /*%3d*/ { %s, %d, { %d, %d, \"%s\" }, { { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d }, { TU(\"%s\"), %d, %d } }, %s, %d, %d, %d, \"%s\",\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].option_2,
                    data[i].structapp.index, data[i].structapp.count, data[i].structapp.id,
                    testUtilEscape((const char *) data[i].segs[0].source, length, escaped, sizeof(escaped)), data[i].segs[0].length, data[i].segs[0].eci,
                    testUtilEscape((const char *) data[i].segs[1].source, length1, escaped1, sizeof(escaped1)), data[i].segs[1].length, data[i].segs[1].eci,
                    testUtilEscape((const char *) data[i].segs[2].source, length2, escaped2, sizeof(escaped2)), data[i].segs[2].length, data[i].segs[2].eci,
                    testUtilErrorName(data[i].ret), symbol->rows, symbol->width, data[i].bwipp_cmp, data[i].comment);
            if (ret < ZINT_ERROR) {
                testUtilModulesPrint(symbol, "                    ", "\n");
            } else {
                printf("                    \"\"\n");
            }
            printf("                },\n");
        } else {
            if (ret < ZINT_ERROR) {
                int width, row;

                assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n",
                    i, symbol->rows, data[i].expected_rows);
                assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n",
                    i, symbol->width, data[i].expected_width);

                ret = testUtilModulesCmp(symbol, data[i].expected, &width, &row);
                assert_zero(ret, "i:%d testUtilModulesCmp ret %d != 0 width %d row %d\n",
                    i, ret, width, row);

                if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, -1, debug)) {
                    if (!data[i].bwipp_cmp) {
                        if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                    } else {
                        ret = testUtilBwippSegs(i, symbol, -1, data[i].option_2, -1, data[i].segs, seg_count, NULL, bwipp_buf, sizeof(bwipp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwippSegs ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, data[i].expected);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, data[i].expected);
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rt(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int eci;
        int option_2;
        int output_options;
        const char *data;
        int length;
        int ret;
        int expected_eci;
        const char *expected;
        int expected_length;
        int expected_raw_eci;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, -1, -1, "é", -1, 0, 0, "", -1, 0 },
        /*  1*/ { UNICODE_MODE, -1, -1, BARCODE_RAW_TEXT, "é", -1, 0, 0, "\351", -1, 3 },
        /*  2*/ { UNICODE_MODE, -1, -1, -1, "ก", -1, ZINT_WARN_USES_ECI, 13, "", -1, 0 },
        /*  3*/ { UNICODE_MODE, -1, -1, BARCODE_RAW_TEXT, "ก", -1, ZINT_WARN_USES_ECI, 13, "\241", -1, 13 },
        /*  4*/ { DATA_MODE, -1, -1, -1, "\351", -1, 0, 0, "", -1, 0 },
        /*  5*/ { DATA_MODE, -1, -1, BARCODE_RAW_TEXT, "\351", -1, 0, 0, "\351", -1, 3 },
        /*  6*/ { UNICODE_MODE, 26, -1, -1, "é", -1, 0, 26, "", -1, 0 },
        /*  7*/ { UNICODE_MODE, 26, -1, BARCODE_RAW_TEXT, "é", -1, 0, 26, "é", -1, 26 },
        /*  8*/ { UNICODE_MODE, 899, -1, -1, "é", -1, 0, 899, "", -1, 0 },
        /*  9*/ { UNICODE_MODE, 899, -1, BARCODE_RAW_TEXT, "é", -1, 0, 899, "é", -1, 899 },
        /* 10*/ { GS1_MODE, -1, -1, -1, "[01]04912345123459[15]970331[30]128[10]ABC12(", -1, 0, 0, "", -1, 0 },
        /* 11*/ { GS1_MODE, -1, -1, BARCODE_RAW_TEXT, "[01]04912345123459[15]970331[30]128[10]ABC12(", -1, 0, 0, "01049123451234591597033130128\03510ABC12(", -1, 3 },
        /* 12*/ { GS1_MODE | ESCAPE_MODE | GS1PARENS_MODE, -1, -1, BARCODE_RAW_TEXT, "(01)04912345123459(15)970331(30)128(10)ABC12\\(", -1, 0, 0, "01049123451234591597033130128\03510ABC12(", -1, 3 },
        /* 13*/ { UNICODE_MODE, -1, 9, -1, "12345", -1, 0, 0, "", -1, 0 }, /* Version S */
        /* 14*/ { UNICODE_MODE, -1, 9, BARCODE_RAW_TEXT, "12345", -1, 0, 0, "12345", -1, 3 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    int expected_length;

    char escaped[4096];
    char escaped2[4096];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODEONE, data[i].input_mode, data[i].eci,
                                    -1 /*option_1*/, data[i].option_2, -1 /*option_3*/, data[i].output_options,
                                    data[i].data, data[i].length, debug);
        expected_length = data[i].expected_length == -1 ? (int) strlen(data[i].expected) : data[i].expected_length;

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n",
                    i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->eci, data[i].expected_eci, "i:%d eci %d != %d\n",
                        i, symbol->eci, data[i].expected_eci);
            if (symbol->output_options & BARCODE_RAW_TEXT) {
                assert_nonnull(symbol->raw_segs, "i:%d raw_segs NULL\n", i);
                assert_nonnull(symbol->raw_segs[0].source, "i:%d raw_segs[0].source NULL\n", i);
                assert_equal(symbol->raw_segs[0].length, expected_length,
                            "i:%d raw_segs[0].length %d != expected_length %d\n",
                            i, symbol->raw_segs[0].length, expected_length);
                assert_zero(memcmp(symbol->raw_segs[0].source, data[i].expected, expected_length),
                            "i:%d raw_segs[0].source memcmp(%s, %s, %d) != 0\n", i,
                            testUtilEscape((const char *) symbol->raw_segs[0].source, symbol->raw_segs[0].length,
                                            escaped, sizeof(escaped)),
                            testUtilEscape(data[i].expected, expected_length, escaped2, sizeof(escaped2)),
                            expected_length);
                assert_equal(symbol->raw_segs[0].eci, data[i].expected_raw_eci,
                            "i:%d raw_segs[0].eci %d != expected_raw_eci %d\n",
                            i, symbol->raw_segs[0].eci, data[i].expected_raw_eci);
            } else {
                assert_null(symbol->raw_segs, "i:%d raw_segs not NULL\n", i);
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_rt_segs(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int input_mode;
        int output_options;
        struct zint_seg segs[3];
        int ret;

        int expected_rows;
        int expected_width;
        struct zint_seg expected_raw_segs[3];
        int expected_raw_seg_count;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { UNICODE_MODE, -1, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, {0} }, 0, 16, 18, {{0}}, 0 },
        /*  1*/ { UNICODE_MODE, BARCODE_RAW_TEXT, { { TU("¶"), -1, 0 }, { TU("Ж"), -1, 7 }, { TU(""), 0, 0 } }, 0, 16, 18, { { TU("\266"), 1, 3 }, { TU("\266"), 1, 7 }, {0} }, 2 },
        /*  2*/ { UNICODE_MODE, -1, { { TU("éé"), -1, 0 }, { TU("กขฯ"), -1, 0 }, { TU("βββ"), -1, 0 } }, ZINT_WARN_USES_ECI, 28, 32, {{0}}, 0 },
        /*  3*/ { UNICODE_MODE, BARCODE_RAW_TEXT, { { TU("éé"), -1, 0 }, { TU("กขฯ"), -1, 0 }, { TU("βββ"), -1, 0 } }, ZINT_WARN_USES_ECI, 28, 32, { { TU("\351\351"), 2, 3 }, { TU("\241\242\317"), 3, 13 }, { TU("\342\342\342"), 3, 9 } }, 3 },
        /*  4*/ { DATA_MODE, -1, { { TU("¶"), -1, 26 }, { TU("Ж"), -1, 0 }, { TU("\223\137"), -1, 20 } }, 0, 28, 32, {{0}}, 0 },
        /*  5*/ { DATA_MODE, BARCODE_RAW_TEXT, { { TU("¶"), -1, 26 }, { TU("Ж"), -1, 0 }, { TU("\223\137"), -1, 20 } }, 0, 28, 32, { { TU("¶"), 2, 26 }, { TU("\320\226"), 2, 3 }, { TU("\223\137"), 2, 20 } }, 3 },
    };
    const int data_size = ARRAY_SIZE(data);
    int i, j, seg_count, ret;
    struct zint_symbol *symbol = NULL;

    int expected_length;

    char escaped[4096];
    char escaped2[4096];

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilSetSymbol(symbol, BARCODE_CODEONE, data[i].input_mode, -1 /*eci*/,
                            -1 /*option_1*/, -1 /*option_2*/, -1 /*option_3*/, data[i].output_options,
                            NULL, 0, debug);
        for (j = 0, seg_count = 0; j < 3 && data[i].segs[j].length; j++, seg_count++);

        ret = ZBarcode_Encode_Segs(symbol, data[i].segs, seg_count);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode_Segs ret %d != %d (%s)\n",
                    i, ret, data[i].ret, symbol->errtxt);

        assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d (width %d)\n",
                    i, symbol->rows, data[i].expected_rows, symbol->width);
        assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n",
                    i, symbol->width, data[i].expected_width);

        assert_equal(symbol->raw_seg_count, data[i].expected_raw_seg_count, "i:%d symbol->raw_seg_count %d != %d\n",
                    i, symbol->raw_seg_count, data[i].expected_raw_seg_count);
        if (symbol->output_options & BARCODE_RAW_TEXT) {
            assert_nonnull(symbol->raw_segs, "i:%d raw_segs NULL\n", i);
            for (j = 0; j < symbol->raw_seg_count; j++) {
                assert_nonnull(symbol->raw_segs[j].source, "i:%d raw_segs[%d].source NULL\n", i, j);

                expected_length = data[i].expected_raw_segs[j].length;

                assert_equal(symbol->raw_segs[j].length, expected_length,
                            "i:%d raw_segs[%d].length %d != expected_length %d\n",
                            i, j, symbol->raw_segs[j].length, expected_length);
                assert_zero(memcmp(symbol->raw_segs[j].source, data[i].expected_raw_segs[j].source, expected_length),
                            "i:%d raw_segs[%d].source memcmp(%s, %s, %d) != 0\n", i, j,
                            testUtilEscape((const char *) symbol->raw_segs[j].source, expected_length, escaped,
                                            sizeof(escaped)),
                            testUtilEscape((const char *) data[i].expected_raw_segs[j].source, expected_length,
                                            escaped2, sizeof(escaped2)),
                            expected_length);
                assert_equal(symbol->raw_segs[j].eci, data[i].expected_raw_segs[j].eci,
                            "i:%d raw_segs[%d].eci %d != expected_raw_segs.eci %d\n",
                            i, j, symbol->raw_segs[j].eci, data[i].expected_raw_segs[j].eci);
            }
        } else {
            assert_null(symbol->raw_segs, "i:%d raw_segs not NULL\n", i);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_fuzz(const testCtx *const p_ctx) {
    int debug = p_ctx->debug;

    struct item {
        int option_2;
        const char *data;
        int length;
        int ret;
        int bwipp_cmp;
        const char *comment;
    };
    /* s/\/\*[ 0-9]*\*\//\=printf("\/\*%3d*\/", line(".") - line("'<")): */
    static const struct item data[] = {
        /*  0*/ { -1, "3333P33B\035333V3333333333333\0363", -1, 0, 1, "" }, /* #181 Nico Gunkel, OSS-Fuzz */
        /*  1*/ { -1, "{{-06\024755712162106130000000829203983\377", -1, 0, 1, "" }, /* #232 Jan Schrewe, CI-Fuzz, out-of-bounds in is_last_single_ascii() sp + 1 */
        /*  2*/ { -1, "\000\000\000\367\000\000\000\000\000\103\040\000\000\244\137\140\140\000\000\000\000\000\000\000\000\000\005\000\000\000\000\000\165\060\060\060\060\061\060\060\114\114\060\010\102\102\102\102\102\102\102\102\057\102\100\102\057\233\100\102", 60, 0, 1, "" }, /* #300 (#4) Andre Maute (`c1_c40text_cnt()` not accounting for extended ASCII shifts) */
        /*  3*/ { 10, "\153\153\153\060\001\000\134\153\153\015\015\353\362\015\015\015\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\362\362\000", 65, ZINT_ERROR_TOO_LONG, 1, "" }, /* #300 (#8) Andre Maute (`c1_encode()` looping on latch) */
        /*  4*/ { 10, "\015\015\353\362\015\015\015\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\110\015\015\015\015\015\015\015\015\015\015\015\015\015\015\015\362\362\000", 39, 0, 1, "" }, /* #300 (#8 shortened) Andre Maute */
        /*  5*/ { 10, "\153\153\153\153\153\060\001\000\000\134\153\153\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\153\153\153\153\153\153\043\000\000\307\000\147\000\000\000\043\113\153\162\162\215\220", 90, ZINT_ERROR_TOO_LONG, 1, "" }, /* #300 (#12) Andre Maute (too small buffer for Version T) */
    };
    const int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol = NULL;

    char bwipp_buf[32768];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); /* Only do BWIPP test if asked, too slow otherwise */

    testStartSymbol(p_ctx->func_name, &symbol);

    for (i = 0; i < data_size; i++) {

        if (testContinue(p_ctx, i)) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_CODEONE, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, data[i].length, debug);

        ret = ZBarcode_Encode(symbol, TCU(data[i].data), length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            if (do_bwipp && testUtilCanBwipp(i, symbol, -1, data[i].option_2, -1, debug)) {
                if (!data[i].bwipp_cmp) {
                    if (debug & ZINT_DEBUG_TEST_PRINT) printf("i:%d %s not BWIPP compatible (%s)\n", i, testUtilBarcodeName(symbol->symbology), data[i].comment);
                } else {
                    char modules_dump[4096];
                    assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                    ret = testUtilBwipp(i, symbol, -1, data[i].option_2, -1, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf), NULL);
                    assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                    ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, modules_dump);
                    assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                   i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, modules_dump);
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func */
        { "test_large", test_large },
        { "test_input", test_input },
        { "test_encode", test_encode },
        { "test_encode_segs", test_encode_segs },
        { "test_rt", test_rt },
        { "test_rt_segs", test_rt_segs },
        { "test_fuzz", test_fuzz },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}

/* vim: set ts=4 sw=4 et : */
