/*
    libzint - the open source barcode library
    Copyright (C) 2019 - 2021 Robin Stuart <rstuart114@gmail.com>

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

#include "testcommon.h"
#include "test_dmatrix_variant.h"

static void test_large(int index, int debug) {

    struct item {
        int symbology;
        int option_2;
        struct zint_structapp structapp;
        char *pattern;
        int length;
        int ret;
        int expected_rows;
        int expected_width;
    };
    // ISO/IEC 16022:2006 Table 7 and ISO/IEC 21471:2020 (DMRE) Table 7
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "1", 3116, 0, 144, 144 },
        /*  1*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "1", 3117, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  2*/ { BARCODE_DATAMATRIX, -1, { 1, 2, "001001"}, "1", 3108, 0, 144, 144 }, // Structured Append 4 codewords overhead == 8 digits
        /*  3*/ { BARCODE_DATAMATRIX, -1, { 1, 2, "001001"}, "1", 3109, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  4*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "A", 2335, 0, 144, 144 },
        /*  5*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "A", 2336, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  6*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "\200", 1556, 0, 144, 144 }, // Spec says 1555 but 1556 correct as only single byte count of 0 required
        /*  7*/ { BARCODE_DATAMATRIX, -1, { 0, 0, "" }, "\200", 1557, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*  8*/ { BARCODE_HIBC_DM, -1, { 0, 0, "" }, "1", 110, 0, 32, 32 },
        /*  9*/ { BARCODE_HIBC_DM, -1, { 0, 0, "" }, "1", 111, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 10*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "1", 6, 0, 10, 10 },
        /* 11*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "1", 7, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 12*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "A", 3, 0, 10, 10 },
        /* 13*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "A", 4, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 14*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "\200", 1, 0, 10, 10 },
        /* 15*/ { BARCODE_DATAMATRIX, 1, { 0, 0, "" }, "\200", 2, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 16*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "1", 10, 0, 12, 12 },
        /* 17*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "1", 11, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 18*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "A", 6, 0, 12, 12 },
        /* 19*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "A", 7, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 20*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "\200", 3, 0, 12, 12 },
        /* 21*/ { BARCODE_DATAMATRIX, 2, { 0, 0, "" }, "\200", 4, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 22*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "1", 16, 0, 14, 14 },
        /* 23*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "1", 17, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 24*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "A", 10, 0, 14, 14 },
        /* 25*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "A", 11, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 26*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "\200", 6, 0, 14, 14 },
        /* 27*/ { BARCODE_DATAMATRIX, 3, { 0, 0, "" }, "\200", 7, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 28*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "1", 24, 0, 16, 16 },
        /* 29*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "1", 25, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 30*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "A", 16, 0, 16, 16 },
        /* 31*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "A", 17, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 32*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "\200", 10, 0, 16, 16 },
        /* 33*/ { BARCODE_DATAMATRIX, 4, { 0, 0, "" }, "\200", 11, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 34*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "1", 36, 0, 18, 18 },
        /* 35*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "1", 37, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 36*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "A", 25, 0, 18, 18 },
        /* 37*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "A", 26, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 38*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "\200", 16, 0, 18, 18 },
        /* 39*/ { BARCODE_DATAMATRIX, 5, { 0, 0, "" }, "\200", 17, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 40*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "1", 44, 0, 20, 20 },
        /* 41*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "1", 45, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 42*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "A", 31, 0, 20, 20 },
        /* 43*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "A", 32, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 44*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "\200", 20, 0, 20, 20 },
        /* 45*/ { BARCODE_DATAMATRIX, 6, { 0, 0, "" }, "\200", 21, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 46*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "1", 60, 0, 22, 22 },
        /* 47*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "1", 61, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 48*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "A", 43, 0, 22, 22 },
        /* 49*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "A", 44, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 50*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "\200", 28, 0, 22, 22 },
        /* 51*/ { BARCODE_DATAMATRIX, 7, { 0, 0, "" }, "\200", 29, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 52*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "1", 72, 0, 24, 24 },
        /* 53*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "1", 73, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 54*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "A", 52, 0, 24, 24 },
        /* 55*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "A", 53, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 56*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "\200", 34, 0, 24, 24 },
        /* 57*/ { BARCODE_DATAMATRIX, 8, { 0, 0, "" }, "\200", 35, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 58*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "1", 88, 0, 26, 26 },
        /* 59*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "1", 89, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 60*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "A", 64, 0, 26, 26 },
        /* 61*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "A", 65, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 62*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "\200", 42, 0, 26, 26 },
        /* 63*/ { BARCODE_DATAMATRIX, 9, { 0, 0, "" }, "\200", 43, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 64*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "1", 124, 0, 32, 32 },
        /* 65*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "1", 125, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 66*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "A", 91, 0, 32, 32 },
        /* 67*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "A", 92, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 68*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "\200", 60, 0, 32, 32 },
        /* 69*/ { BARCODE_DATAMATRIX, 10, { 0, 0, "" }, "\200", 61, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 70*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "1", 172, 0, 36, 36 },
        /* 71*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "1", 173, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 72*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "A", 127, 0, 36, 36 },
        /* 73*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "A", 128, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 74*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "\200", 84, 0, 36, 36 },
        /* 75*/ { BARCODE_DATAMATRIX, 11, { 0, 0, "" }, "\200", 85, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 76*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "1", 228, 0, 40, 40 },
        /* 77*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "1", 229, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 78*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "A", 169, 0, 40, 40 },
        /* 79*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "A", 170, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 80*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "\200", 112, 0, 40, 40 },
        /* 81*/ { BARCODE_DATAMATRIX, 12, { 0, 0, "" }, "\200", 113, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 82*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "1", 288, 0, 44, 44 },
        /* 83*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "1", 289, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 84*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "A", 214, 0, 44, 44 },
        /* 85*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "A", 215, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 86*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "\200", 142, 0, 44, 44 },
        /* 87*/ { BARCODE_DATAMATRIX, 13, { 0, 0, "" }, "\200", 143, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 88*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "1", 348, 0, 48, 48 },
        /* 89*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "1", 349, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 90*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "A", 259, 0, 48, 48 },
        /* 91*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "A", 260, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 92*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "\200", 172, 0, 48, 48 },
        /* 93*/ { BARCODE_DATAMATRIX, 14, { 0, 0, "" }, "\200", 173, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 94*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "1", 408, 0, 52, 52 },
        /* 95*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "1", 409, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 96*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "A", 304, 0, 52, 52 },
        /* 97*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "A", 305, ZINT_ERROR_TOO_LONG, -1, -1 },
        /* 98*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "\200", 202, 0, 52, 52 },
        /* 99*/ { BARCODE_DATAMATRIX, 15, { 0, 0, "" }, "\200", 203, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*100*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "1", 560, 0, 64, 64 },
        /*101*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "1", 561, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*102*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "A", 418, 0, 64, 64 },
        /*103*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "A", 419, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*104*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "\200", 278, 0, 64, 64 }, // Spec says 277 but 278 correct as only single byte count of 0 required
        /*105*/ { BARCODE_DATAMATRIX, 16, { 0, 0, "" }, "\200", 279, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*106*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "1", 736, 0, 72, 72 },
        /*107*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "1", 737, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*108*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "A", 550, 0, 72, 72 },
        /*109*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "A", 551, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*110*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "\200", 366, 0, 72, 72 }, // Spec says 365 but 366 correct as only single byte count of 0 required
        /*111*/ { BARCODE_DATAMATRIX, 17, { 0, 0, "" }, "\200", 367, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*112*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "1", 912, 0, 80, 80 },
        /*113*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "1", 913, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*114*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "A", 682, 0, 80, 80 },
        /*115*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "A", 683, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*116*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "\200", 454, 0, 80, 80 }, // Spec says 453 but 454 correct as only single byte count of 0 required
        /*117*/ { BARCODE_DATAMATRIX, 18, { 0, 0, "" }, "\200", 455, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*118*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "1", 1152, 0, 88, 88 },
        /*119*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "1", 1153, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*120*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "A", 862, 0, 88, 88 },
        /*121*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "A", 863, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*122*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "\200", 574, 0, 88, 88 }, // Spec says 573 but 574 correct as only single byte count of 0 required
        /*123*/ { BARCODE_DATAMATRIX, 19, { 0, 0, "" }, "\200", 575, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*124*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "1", 1392, 0, 96, 96 },
        /*125*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "1", 1393, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*126*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "A", 1042, 0, 96, 96 },
        /*127*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "A", 1043, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*128*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "\200", 694, 0, 96, 96 }, // Spec says 693 but 694 correct as only single byte count of 0 required
        /*129*/ { BARCODE_DATAMATRIX, 20, { 0, 0, "" }, "\200", 695, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*130*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "1", 1632, 0, 104, 104 },
        /*131*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "1", 1633, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*132*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "A", 1222, 0, 104, 104 },
        /*133*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "A", 1223, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*134*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "\200", 814, 0, 104, 104 }, // Spec says 813 but 814 correct as only single byte count of 0 required
        /*135*/ { BARCODE_DATAMATRIX, 21, { 0, 0, "" }, "\200", 815, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*136*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "1", 2100, 0, 120, 120 },
        /*137*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "1", 2101, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*138*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "A", 1573, 0, 120, 120 },
        /*139*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "A", 1574, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*140*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "\200", 1048, 0, 120, 120 }, // Spec says 1047 but 1048 correct as only single byte count of 0 required
        /*141*/ { BARCODE_DATAMATRIX, 22, { 0, 0, "" }, "\200", 1049, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*142*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "1", 2608, 0, 132, 132 },
        /*143*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "1", 2609, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*144*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "A", 1954, 0, 132, 132 },
        /*145*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "A", 1955, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*146*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "\200", 1302, 0, 132, 132 }, // Spec says 1301 but 1302 correct as only single byte count of 0 required
        /*147*/ { BARCODE_DATAMATRIX, 23, { 0, 0, "" }, "\200", 1303, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*148*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "1", 3116, 0, 144, 144 },
        /*149*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "1", 3117, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*150*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "A", 2335, 0, 144, 144 },
        /*151*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "A", 2336, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*152*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "\200", 1556, 0, 144, 144 }, // Spec says 1555 but 1556 correct as only single byte count of 0 required
        /*153*/ { BARCODE_DATAMATRIX, 24, { 0, 0, "" }, "\200", 1557, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*154*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "1", 10, 0, 8, 18 },
        /*155*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "1", 11, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*156*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "A", 6, 0, 8, 18 },
        /*157*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "A", 7, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*158*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "\200", 3, 0, 8, 18 },
        /*159*/ { BARCODE_DATAMATRIX, 25, { 0, 0, "" }, "\200", 4, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*160*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "1", 20, 0, 8, 32 },
        /*161*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "1", 21, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*162*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "A", 13, 0, 8, 32 },
        /*163*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "A", 14, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*164*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "\200", 8, 0, 8, 32 },
        /*165*/ { BARCODE_DATAMATRIX, 26, { 0, 0, "" }, "\200", 9, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*166*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "1", 32, 0, 12, 26 },
        /*167*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "1", 33, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*168*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "A", 22, 0, 12, 26 },
        /*169*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "A", 23, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*170*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "\200", 14, 0, 12, 26 },
        /*171*/ { BARCODE_DATAMATRIX, 27, { 0, 0, "" }, "\200", 15, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*172*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "1", 44, 0, 12, 36 },
        /*173*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "1", 45, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*174*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "A", 31, 0, 12, 36 },
        /*175*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "A", 32, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*176*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "\200", 20, 0, 12, 36 },
        /*177*/ { BARCODE_DATAMATRIX, 28, { 0, 0, "" }, "\200", 21, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*178*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "1", 64, 0, 16, 36 },
        /*179*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "1", 65, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*180*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "A", 46, 0, 16, 36 },
        /*181*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "A", 47, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*182*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "\200", 30, 0, 16, 36 },
        /*183*/ { BARCODE_DATAMATRIX, 29, { 0, 0, "" }, "\200", 31, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*184*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "1", 98, 0, 16, 48 },
        /*185*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "1", 99, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*186*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "A", 72, 0, 16, 48 },
        /*187*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "A", 73, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*188*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "\200", 47, 0, 16, 48 },
        /*189*/ { BARCODE_DATAMATRIX, 30, { 0, 0, "" }, "\200", 48, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*190*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "1", 36, 0, 8, 48 },
        /*191*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "1", 37, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*192*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "A", 25, 0, 8, 48 },
        /*193*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "A", 26, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*194*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "\200", 16, 0, 8, 48 },
        /*195*/ { BARCODE_DATAMATRIX, 31, { 0, 0, "" }, "\200", 17, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*196*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "1", 48, 0, 8, 64 },
        /*197*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "1", 49, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*198*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "A", 34, 0, 8, 64 },
        /*199*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "A", 35, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*200*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "\200", 22, 0, 8, 64 },
        /*201*/ { BARCODE_DATAMATRIX, 32, { 0, 0, "" }, "\200", 23, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*202*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "1", 64, 0, 8, 80 },
        /*203*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "1", 65, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*204*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "A", 46, 0, 8, 80 },
        /*205*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "A", 47, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*206*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "\200", 30, 0, 8, 80 },
        /*207*/ { BARCODE_DATAMATRIX, 33, { 0, 0, "" }, "\200", 31, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*208*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "1", 76, 0, 8, 96 },
        /*209*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "1", 77, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*210*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "A", 55, 0, 8, 96 },
        /*211*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "A", 56, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*212*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "\200", 36, 0, 8, 96 },
        /*213*/ { BARCODE_DATAMATRIX, 34, { 0, 0, "" }, "\200", 37, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*214*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "1", 98, 0, 8, 120 },
        /*215*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "1", 99, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*216*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "A", 72, 0, 8, 120 },
        /*217*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "A", 73, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*218*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "\200", 47, 0, 8, 120 },
        /*219*/ { BARCODE_DATAMATRIX, 35, { 0, 0, "" }, "\200", 48, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*220*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "1", 126, 0, 8, 144 },
        /*221*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "1", 127, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*222*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "A", 93, 0, 8, 144 },
        /*223*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "A", 94, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*224*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "\200", 61, 0, 8, 144 },
        /*225*/ { BARCODE_DATAMATRIX, 36, { 0, 0, "" }, "\200", 62, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*226*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "1", 86, 0, 12, 64 },
        /*227*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "1", 87, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*228*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "A", 63, 0, 12, 64 },
        /*229*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "A", 64, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*230*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "\200", 41, 0, 12, 64 },
        /*231*/ { BARCODE_DATAMATRIX, 37, { 0, 0, "" }, "\200", 42, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*232*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "1", 128, 0, 12, 88 },
        /*233*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "1", 129, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*234*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "A", 94, 0, 12, 88 },
        /*235*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "A", 95, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*236*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "\200", 62, 0, 12, 88 },
        /*237*/ { BARCODE_DATAMATRIX, 38, { 0, 0, "" }, "\200", 63, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*238*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "1", 124, 0, 16, 64 },
        /*239*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "1", 125, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*240*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "A", 91, 0, 16, 64 },
        /*241*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "A", 92, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*242*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "\200", 60, 0, 16, 64 },
        /*243*/ { BARCODE_DATAMATRIX, 39, { 0, 0, "" }, "\200", 61, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*244*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "1", 88, 0, 20, 36 },
        /*245*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "1", 89, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*246*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "A", 64, 0, 20, 36 },
        /*247*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "A", 65, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*248*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "\200", 42, 0, 20, 36 },
        /*249*/ { BARCODE_DATAMATRIX, 40, { 0, 0, "" }, "\200", 43, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*250*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "1", 112, 0, 20, 44 },
        /*251*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "1", 113, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*252*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "A", 82, 0, 20, 44 },
        /*253*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "A", 83, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*254*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "\200", 54, 0, 20, 44 },
        /*255*/ { BARCODE_DATAMATRIX, 41, { 0, 0, "" }, "\200", 55, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*256*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "1", 168, 0, 20, 64 }, // Spec says 186 but typo
        /*257*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "1", 169, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*258*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "A", 124, 0, 20, 64 },
        /*259*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "A", 125, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*260*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "\200", 82, 0, 20, 64 },
        /*261*/ { BARCODE_DATAMATRIX, 42, { 0, 0, "" }, "\200", 83, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*262*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "1", 144, 0, 22, 48 },
        /*263*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "1", 145, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*264*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "A", 106, 0, 22, 48 },
        /*265*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "A", 107, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*266*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "\200", 70, 0, 22, 48 },
        /*267*/ { BARCODE_DATAMATRIX, 43, { 0, 0, "" }, "\200", 71, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*268*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "1", 160, 0, 24, 48 },
        /*269*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "1", 161, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*270*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "A", 118, 0, 24, 48 },
        /*271*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "A", 119, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*272*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "\200", 78, 0, 24, 48 },
        /*273*/ { BARCODE_DATAMATRIX, 44, { 0, 0, "" }, "\200", 79, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*274*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "1", 216, 0, 24, 64 },
        /*275*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "1", 217, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*276*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "A", 160, 0, 24, 64 },
        /*277*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "A", 161, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*278*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "\200", 106, 0, 24, 64 },
        /*279*/ { BARCODE_DATAMATRIX, 45, { 0, 0, "" }, "\200", 107, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*280*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "1", 140, 0, 26, 40 },
        /*281*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "1", 141, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*282*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "A", 103, 0, 26, 40 },
        /*283*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "A", 104, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*284*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "\200", 68, 0, 26, 40 },
        /*285*/ { BARCODE_DATAMATRIX, 46, { 0, 0, "" }, "\200", 69, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*286*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "1", 180, 0, 26, 48 },
        /*287*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "1", 181, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*288*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "A", 133, 0, 26, 48 },
        /*289*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "A", 134, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*290*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "\200", 88, 0, 26, 48 },
        /*291*/ { BARCODE_DATAMATRIX, 47, { 0, 0, "" }, "\200", 89, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*292*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "1", 236, 0, 26, 64 },
        /*293*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "1", 237, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*294*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "A", 175, 0, 26, 64 },
        /*295*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "A", 176, ZINT_ERROR_TOO_LONG, -1, -1 },
        /*296*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "\200", 116, 0, 26, 64 },
        /*297*/ { BARCODE_DATAMATRIX, 48, { 0, 0, "" }, "\200", 117, ZINT_ERROR_TOO_LONG, -1, -1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char data_buf[3118];

    testStart("test_large");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        testUtilStrCpyRepeat(data_buf, data[i].pattern, data[i].length);
        assert_equal(data[i].length, (int) strlen(data_buf), "i:%d length %d != strlen(data_buf) %d\n", i, data[i].length, (int) strlen(data_buf));

        length = testUtilSetSymbol(symbol, data[i].symbology, -1 /*input_mode*/, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data_buf, data[i].length, debug);
        if (data[i].structapp.count) {
            symbol->structapp = data[i].structapp;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data_buf, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (ret < ZINT_ERROR) {
            assert_equal(symbol->rows, data[i].expected_rows, "i:%d symbol->rows %d != %d\n", i, symbol->rows, data[i].expected_rows);
            assert_equal(symbol->width, data[i].expected_width, "i:%d symbol->width %d != %d\n", i, symbol->width, data[i].expected_width);
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

// Note need ZINT_SANITIZE set for these
static void test_buffer(int index, int debug) {

    struct item {
        int eci;
        int input_mode;
        int output_options;
        char *data;
        int ret;
        char *comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { 16383, UNICODE_MODE, READER_INIT, "1", 0, "" },
        /*  1*/ { 3, UNICODE_MODE, 0, "000106j 05 Galeria A NaÃ§Ã£o0000000000", 0, "From Okapi, consecutive use of upper shift; #176" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_buffer");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, BARCODE_DATAMATRIX, data[i].input_mode, data[i].eci, -1 /*option_1*/, -1, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_options(int index, int debug) {

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
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "1", 0, 10, 10, "" },
        /*  1*/ { BARCODE_DATAMATRIX, -1, 2, -1, -1, -1, { 0, 0, "" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 524: Older Data Matrix standards are no longer supported" },
        /*  2*/ { BARCODE_DATAMATRIX, -1, -1, 1, -1, -1, { 0, 0, "" }, "1", 0, 10, 10, "" },
        /*  3*/ { BARCODE_DATAMATRIX, -1, -1, 2, -1, -1, { 0, 0, "" }, "1", 0, 12, 12, "" },
        /*  4*/ { BARCODE_DATAMATRIX, -1, -1, 48, -1, -1, { 0, 0, "" }, "1", 0, 26, 64, "" },
        /*  5*/ { BARCODE_DATAMATRIX, -1, -1, 49, -1, -1, { 0, 0, "" }, "1", 0, 10, 10, "" }, // Ignored
        /*  6*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "ABCDEFGHIJK", 0, 8, 32, "" },
        /*  7*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_SQUARE, -1, { 0, 0, "" }, "ABCDEFGHIJK", 0, 16, 16, "" },
        /*  8*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU", 0, 32, 32, "" },
        /*  9*/ { BARCODE_DATAMATRIX, -1, -1, -1, DM_DMRE, -1, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU", 0, 20, 44, "" },
        /* 10*/ { BARCODE_DATAMATRIX, -1, -1, -1, 9999, -1, { 0, 0, "" }, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTU", 0, 32, 32, "" }, // Ignored
        /* 11*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, { 0, 0, "" }, "[90]12", 0, 10, 10, "" },
        /* 12*/ { BARCODE_DATAMATRIX, GS1_MODE | GS1PARENS_MODE, -1, -1, -1, -1, { 0, 0, "" }, "(90)12", 0, 10, 10, "" },
        /* 13*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 1, 2, "" }, "1", 0, 12, 12, "" },
        /* 14*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 16, 16, "" }, "1", 0, 12, 12, "" },
        /* 15*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 1, 1, "" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 720: Structured Append count out of range (2-16)" },
        /* 16*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 1, 17, "" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 720: Structured Append count out of range (2-16)" },
        /* 17*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 0, 16, "" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 721: Structured Append index out of range (1-16)" },
        /* 18*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 17, 16, "" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 721: Structured Append index out of range (1-16)" },
        /* 19*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "1001" }, "1", 0, 12, 12, "" },
        /* 20*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "A" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 723: Invalid Structured Append ID (digits only)" },
        /* 21*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "0" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 724: Structured Append ID1 '000' and ID2 '000' out of range (001-254) (ID '000000')" },
        /* 22*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "1" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 725: Structured Append ID1 '000' out of range (001-254) (ID '000001')" },
        /* 23*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "1000" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 726: Structured Append ID2 '000' out of range (001-254) (ID '001000')" },
        /* 24*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "001255" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 726: Structured Append ID2 '255' out of range (001-254) (ID '001255')" },
        /* 25*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "255001" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 725: Structured Append ID1 '255' out of range (001-254) (ID '255001')" },
        /* 26*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "255255" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 724: Structured Append ID1 '255' and ID2 '255' out of range (001-254) (ID '255255')" },
        /* 27*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, { 2, 3, "1234567" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 722: Structured Append ID too long (6 digit maximum)" },
        /* 28*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, READER_INIT, { 2, 3, "1001" }, "1", ZINT_ERROR_INVALID_OPTION, -1, -1, "Error 727: Cannot have Structured Append and Reader Initialisation at the same time" },
        /* 29*/ { BARCODE_DATAMATRIX, ESCAPE_MODE, -1, -1, -1, -1, { 2, 3, "1001" }, "[)>\\R05\\GA\\R\\E", 0, 12, 26, "" }, // Macro05/06 ignored if have Structured Append TODO: error/warning
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    testStart("test_options");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

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

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_reader_init(int index, int generate, int debug) {

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
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, UNICODE_MODE, READER_INIT, "A", 0, 10, 10, "EA 42 81 19 A4 53 21 DF", "TODO: Check this" },
        /*  1*/ { BARCODE_DATAMATRIX, GS1_MODE, READER_INIT, "[91]A", ZINT_ERROR_INVALID_OPTION, 0, 0, "Error 521: Cannot encode in GS1 mode and Reader Initialisation at the same time", "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[1024];

    testStart("test_reader_init");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, -1 /*option_2*/, -1, data[i].output_options, data[i].data, -1, debug);

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
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

static void test_input(int index, int generate, int debug) {

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

        int expected_diff; // Check minimize only (TODO: remove in the not-too-distant future)
    };
    struct item data[] = {
        /*  0*/ { UNICODE_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY80", 0, 0, 18, 18, 1, "(32) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 93 B0 1C 3C 76 FB D4 AB 16 11", "#208", 0 },
        /*  1*/ { UNICODE_MODE, 0, 5, -1, -1, "0466010592130100000k*AGUATY80", 0, 0, 18, 18, 1, "(32) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 93 B0 1C 3C 76 FB D4 AB 16 11", "", 0 },
        /*  2*/ { UNICODE_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY8", 0, 0, 18, 18, 0, "(32) 86 C4 83 87 DE 8F 83 82 82 31 6C E6 07 B7 82 5F D4 3D 08 EB 60 DA B1 82 72 50 A9 5B", "BWIPP different encodation (earlier change to C40)", 0 },
        /*  3*/ { UNICODE_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY80U", 0, 0, 20, 20, 1, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 FE 56 81 76 4F AB 22 B8 6F 0A", "", 0 },
        /*  4*/ { UNICODE_MODE, 0, 5, -1, -1, "0466010592130100000k*AGUATY80U", ZINT_ERROR_TOO_LONG, -1, 0, 0, 0, "Error 522: Input too long for selected symbol size", "", 0 },
        /*  5*/ { UNICODE_MODE, 0, 6, -1, -1, "0466010592130100000k*AGUATY80U", 0, 0, 20, 20, 1, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C EE 08 85 D6 D2 EF 65 FE 56 81 76 4F AB 22 B8 6F 0A", "", 0 },
        /*  6*/ { UNICODE_MODE, 0, -1, -1, -1, "0466010592130100000k*AGUATY80UA", 0, 0, 20, 20, 1, "(40) 86 C4 83 87 DE 8F 83 82 82 31 6C E6 07 B7 82 5F D4 3D 1E 5F FE 81 1E 1B B0 FE E7 54", "", 0 },
        /*  7*/ { UNICODE_MODE, 0, -1, -1, -1, ">*\015>*\015>", 0, 0, 14, 14, 1, "EE 0C A9 0C A9 FE 3F 81 42 B2 11 A8 F9 0A EC C1 1E 41", "X12 symbols_left 3, process_p 1", 0 },
        /*  8*/ { UNICODE_MODE, 0, -1, -1, -1, ">*\015>*\015>*", 0, 0, 14, 14, 1, "EE 0C A9 0C A9 FE 3F 2B 3F 05 D2 10 1B 9A 55 2F 68 C5", "X12 symbols_left 3, process_p 2", 0 },
        /*  9*/ { UNICODE_MODE, 0, -1, -1, -1, ">*\015>*\015>*\015", 0, 0, 14, 14, 1, "EE 0C A9 0C A9 0C A9 FE 1F 30 3F EE 45 C1 1C D7 5F 7E", "X12 symbols_left 1, process_p 0", 0 },
        /* 10*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEF", 0, 0, 12, 12, 1, "E6 59 E9 6D 24 3D 15 EF AA 21 F9 59", "C40 symbols_left 0, process_p 0", 0 },
        /* 11*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFG", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 FE 48 81 8C 7E 09 5E 10 64 BC 5F 4C 91", "C40 symbols_left 3, process_p 1", 0 },
        /* 12*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 FE 48 49 2E 31 00 73 3B 8F 4B 55 93 19", "C40 symbols_left 3, process_p 2", 0 },
        /* 13*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHI", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 80 5F FE 01 DE 20 9F AA C2 FF 8F 08 97", "C40 symbols_left 1, process_p 0", 0 },
        /* 14*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJ", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 80 5F 4B AD 47 09 12 FF 2F 95 CA 5B 4A", "C40 symbols_left 1, process_p 1", 0 },
        /* 15*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJK", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 80 5F FE 4B 4C D8 69 88 60 B9 33 B9 31 E6 BF CA", "C40 symbols_left 3, process_p 2", 0 },
        /* 16*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEF\001G", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 00 3D FE 5D 5A F5 0A 8A 4E 1D 63 07 B9", "C40 symbols_left 1, process_p 0", 0 },
        /* 17*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFG\001", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 02 FE 14 A3 27 63 01 2F B1 94 FE FA", "C40 symbols_left 1, process_p 0", 0 },
        /* 18*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFG\001H", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 02 49 C2 E6 DD 06 89 51 BA 8E 9D 1F", "C40 symbols_left 1, process_p 1", 0 },
        /* 19*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH\001", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 02 81 BD 6D F3 94 FF 82 A6 BF BB F1 4F", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 20*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH\001", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 02 81 FB 93 AE 8B 1C 90 DF FE EB C5 A0 2A 6A 4F", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 21*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH\001I", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 02 4A E1 0D DD BC 56 E4 66 52 E6 AE 02", "C40 symbols_left 3, process_p 2, backtracks", 0 },
        /* 22*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH\001I", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 02 4A 81 93 51 DF C0 0C D3 F9 72 13 17 52 5B 7E", "C40 symbols_left 5, process_p 2, backtracks", 0 },
        /* 23*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHI\001", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 80 5F FE 02 81 47 6C 3E 49 D3 FA 46 47 53 6E E5", "Switches to ASC for last char", 0 },
        /* 24*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHI\001", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F FE 02 81 FB 93 33 E3 4F F7 2D 08 8A BF 64 C3 B0 26", "Switches to ASC for last char", 0 },
        /* 25*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH\001I\001", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 02 4A 02 81 BD 5D C0 B9 09 25 87 3A 09 23 9D C0", "C40 symbols_left 1, process_p 1, backtracks 2", 0 },
        /* 26*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEF+G", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 07 E5 FE 6B 35 71 7F 3D 57 59 46 F7 B9", "C40 symbols_left 1, process_p 0", 0 },
        /* 27*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFG+", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 33 FE 33 F5 97 60 73 48 13 2E E5 74", "C40 symbols_left 1, process_p 0", 0 },
        /* 28*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFG+H", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 33 49 E5 B0 6D 05 FB 36 18 34 86 91", "C40 symbols_left 1, process_p 1", 0 },
        /* 29*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH+", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 2C 81 02 BD 40 CF 3B 06 C2 DF 36 E0 48", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 30*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH+", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 2C 81 FB 93 F6 78 B5 69 0B 83 C6 32 62 1A D2 FF", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 31*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGH+I", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 2C 4A 5E DD 6E E7 92 60 02 32 6B BF 05", "C40 symbols_left 3, process_p 2, backtracks", 0 },
        /* 32*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGH+I", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 2C 4A 81 93 09 2C 69 F5 07 84 5F E4 D5 62 E3 CE", "C40 symbols_left 5, process_p 2, backtracks", 0 },
        /* 33*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHI+", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 80 5F FE 2C 81 F8 BC 8D 12 17 7E 22 27 DE 7F E2", "C40 symbols_left 3, process_p 2, backtracks", 0 },
        /* 34*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHI+", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F FE 2C 81 FB 93 6B 10 E6 0E F9 75 A7 48 A6 F3 08 96", "Switches to ASC for last char", 0 },
        /* 35*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFjG", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 0E 25 FE DA 14 D7 15 47 69 9D 4A 54 6D", "C40 symbols_left 1, process_p 0", 0 },
        /* 36*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGj", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 5B FE B5 F3 24 0A 99 26 D6 CC A8 40", "C40 symbols_left 1, process_p 0", 0 },
        /* 37*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGjH", 0, 0, 14, 14, 1, "E6 59 E9 6D 24 7D 5B 49 63 B6 DE 6F 11 58 DD D6 CB A5", "C40 symbols_left 1, process_p 1", 0 },
        /* 38*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHj", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 6B 81 ED 78 CB 9F 52 EE 52 88 91 67 96", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 39*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHj", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 6B 81 FB 93 BF 72 03 35 09 37 98 FF 39 A7 E3 6D", "C40 symbols_left 1, process_p 1, backtracks", 0 },
        /* 40*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHjI", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 FE 48 49 6B 4A B1 18 E5 B7 FB 88 92 65 CC 38 DB", "C40 symbols_left 3, process_p 2, backtracks", 0 },
        /* 41*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "ABCDEFGHjI", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 FE 48 49 6B 4A 81 93 40 26 DF A9 05 30 01 29 8E DF D2 5C", "C40 symbols_left 5, process_p 2, backtracks", 0 },
        /* 42*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIj", 0, 0, 8, 32, 1, "E6 59 E9 6D 24 80 5F FE 6B 81 17 79 06 42 7E 96 B2 70 79 F8 3C", "Switches to ASC for last char", 0 },
        /* 43*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJÊ", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F FE 4B EB 4B 81 DD D9 F9 C9 C5 38 F3 4B DB 80 92 A7", "Switches to ASC for last 2 chars", 0 },
        /* 44*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJKÊ", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F FE 4B 4C EB 4B 15 17 46 06 70 F3 15 74 45 26 72 2D", "C40 symbols_left 3, process_p 2, backtracks", 0 },
        /* 45*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJKª", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F 93 82 BB B2 FE 11 5C 60 32 A6 DE FC 7B 30 F1 03 56", "C40 symbols_left 1, process_p 0", 0 },
        /* 46*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJKê", 0, 0, 16, 16, 1, "E6 59 E9 6D 24 80 5F 93 82 BB DB FE 78 43 69 3C C2 FE F5 2E 1B 4F B6 04", "C40 symbols_left 1, process_p 0", 0 },
        /* 47*/ { GS1_MODE, 0, -1, -1, -1, "[10]ABCDEFGH[10]ABc", 0, 0, 12, 26, 1, "E8 8C E6 59 E9 6D 24 80 4A A9 8D FE 42 43 64 81 83 B4 8F 6B 95 F6 CE A6 3C 5C 77 86 08 50", "C40 symbols_left 3, process_p 1, backtracks", 0 },
        /* 48*/ { GS1_MODE, 0, -1, -1, GS1_GS_SEPARATOR, "[10]ABCDEFGH[10]ABc", 0, 0, 12, 26, 1, "E8 8C E6 59 E9 6D 24 80 49 B6 0D FE 42 43 64 81 79 E4 20 33 76 5C C7 23 E6 C5 FA 4C FF 88", "C40 symbols_left 3, process_p 1, backtracks", 0 },
        /* 49*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdef", 0, 0, 12, 12, 1, "EF 59 E9 6D 24 E2 CC D9 B4 55 E2 6A", "TEX symbols_left 0, process_p 0", 0 },
        /* 50*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefg", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 FE 68 81 A9 65 CD 3A A2 E9 E0 B7 E1 E5", "TEX symbols_left 3, process_p 1", 0 },
        /* 51*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgh", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 FE 68 69 68 36 28 3C 85 5A E9 D4 49 9A", "TEX symbols_left 3, process_p 2", 0 },
        /* 52*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghi", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 80 5F FE DA BF FA 16 71 15 22 4D E3 F3", "TEX symbols_left 1, process_p 0", 0 },
        /* 53*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdef\001g", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 00 3D FE 86 3B 2F 83 51 99 C0 A1 EC DD", "TEX symbols_left 1, process_p 0", 0 },
        /* 54*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefg\001", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 7D 02 FE CF C2 FD EA DA F8 6C 56 15 9E", "TEX symbols_left 1, process_p 0", 0 },
        /* 55*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefg\001h", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 7D 02 69 7A 9B EB A4 5E DE 99 25 01 8C", "TEX symbols_left 1, process_p 1", 0 },
        /* 56*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgh\001", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 FE 68 69 02 81 EB 84 25 32 6E 1B 5A FB 1D 25 4A", "TEX symbols_left 1, process_p 1, backtracks", 0 },
        /* 57*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "abcdefgh\001", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 FE 68 69 02 81 FB 93 93 FD 1E 3B BA 1D 16 4D 59 41 EC B9", "TEX symbols_left 1, process_p 1, backtracks", 0 },
        /* 58*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgh\001i", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 FE 68 69 02 6A 31 35 48 9B 93 6E 15 BB 02 9D F4", "TEX symbols_left 3, process_p 2, backtracks", 0 },
        /* 59*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "abcdefgh\001i", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 FE 68 69 02 6A 81 93 DE D7 EC 9B 7D 72 9C 68 B8 6E CF 31", "TEX symbols_left 3, process_p 2, backtracks", 0 },
        /* 60*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghi\001", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 80 5F FE 02 81 4D AB 30 86 CD D1 9D F3 15 F5 B1", "Switches to ASC for last char", 0 },
        /* 61*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgh\001i\001", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 FE 68 69 02 6A 02 81 32 55 EC 2E A7 AE 69 41 A6 1F 09 8F", "TEX symbols_left 1, process_p 1, backtracks 2", 0 },
        /* 62*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefJg", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 0E 25 FE 01 75 0D 9C 9C BE 40 88 BF 09", "TEX symbols_left 1, process_p 0", 0 },
        /* 63*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgJ", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 7D 5B FE 6E 92 FE 83 42 F1 0B 0E 43 24", "TEX symbols_left 1, process_p 0", 0 },
        /* 64*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefgJh", 0, 0, 14, 14, 1, "EF 59 E9 6D 24 7D 5B 69 DB CB E8 CD C6 D7 FE 7D 57 36", "TEX symbols_left 1, process_p 1", 0 },
        /* 65*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghJ", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 FE 68 69 4B 81 15 8A 35 57 7F 33 B3 48 01 E0 BD", "TEX symbols_left 1, process_p 1, backtracks", 0 },
        /* 66*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "abcdefghJ", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 FE 68 69 4B 81 FB 93 5B D4 D2 8B EE 85 F2 3E 3F 8E E5 04", "TEX symbols_left 1, process_p 1, backtracks", 0 },
        /* 67*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghJi", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 FE 68 69 4B 6A CF 3B 58 FE 82 46 FC 08 1E 58 03", "TEX symbols_left 3, process_p 2, backtracks", 0 },
        /* 68*/ { UNICODE_MODE, 0, -1, DM_SQUARE, -1, "abcdefghJi", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 FE 68 69 4B 6A 81 93 16 FE 20 2B 29 EA 78 1B DE A1 C6 8C", "TEX symbols_left 3, process_p 2, backtracks", 0 },
        /* 69*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghiJ", 0, 0, 8, 32, 1, "EF 59 E9 6D 24 80 5F FE 4B 81 B3 A5 20 E3 DC F9 74 40 09 30 46", "Switches to ASC for last char", 0 },
        /* 70*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghijkÊ", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 80 5F 93 82 BB DB FE 3E C8 EC 73 58 A7 42 46 10 49 25 99", "TEX symbols_left 1, process_p 0", 0 },
        /* 71*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghijkª", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 80 5F 93 82 BB B2 FE 57 D7 E5 7D 3C 87 4B 13 3B F7 90 CB", "TEX symbols_left 1, process_p 0", 0 },
        /* 72*/ { UNICODE_MODE, 0, -1, -1, -1, "abcdefghijkê", 0, 0, 16, 16, 1, "EF 59 E9 6D 24 80 5F FE 6B 6C EB 6B 59 43 1A B1 96 F4 FF C5 B5 08 AE 2F", "TEX symbols_left 3, process_p 2, backtracks", 0 },
        /* 73*/ { GS1_MODE, 0, -1, -1, -1, "[10]abcdefgh[10]abC", 0, 0, 12, 26, 1, "E8 8C EF 59 E9 6D 24 80 4A A9 8D FE 62 63 44 81 88 DC 73 33 70 A1 83 EA 50 CB 4E 17 90 DB", "TEX symbols left 3, process_p 1, backtracks", 0 },
        /* 74*/ { GS1_MODE, 0, -1, -1, GS1_GS_SEPARATOR, "[10]abcdefgh[10]abC", 0, 0, 12, 26, 1, "E8 8C EF 59 E9 6D 24 80 49 B6 0D FE 62 63 44 81 72 8C DC 6B 93 0B 8A 6F 8A 52 C3 DD 67 03", "TEX symbols left 3, process_p 1, backtracks", 0 },
        /* 75*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>", 0, 0, 12, 12, 1, "EE 00 2B 00 2B 83 3B 0A CE 32 36 65", "X12 symbols_left 0, process_p 0", 0 },
        /* 76*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015", 0, 0, 14, 14, 1, "EE 00 2B 00 2B FE 0E 81 C0 6C BF 37 F6 D6 48 71 E2 38", "Switches to ASC for last char", 0 },
        /* 77*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015*", 0, 0, 14, 14, 1, "EE 00 2B 00 2B FE 0E 2B BD DB 7C 8F 14 46 F1 9F 94 BC", "Switches to ASC for last 2 chars", 0 },
        /* 78*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>", 0, 0, 14, 14, 1, "EE 00 2B 00 2B 00 2B FE BF 81 70 74 1C 65 10 0C 06 38", "X12 symbols_left 1, process_p 0, ASC unlatch at end", 0 },
        /* 79*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>\015", 0, 0, 14, 14, 1, "EE 00 2B 00 2B 00 2B 0E 1C DB D8 26 3E EC CF 9C C3 4A", "X12 symbols_left 1, process_p 1, ASC no latch at end", 0 },
        /* 80*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>\015*", 0, 0, 8, 32, 1, "EE 00 2B 00 2B 00 2B FE 0E 2B 65 37 5F 2F F3 96 BE 9A 03 55 68", "X12 symbols_left 3, process_p 2, ASC last 2 chars", 0 },
        /* 81*/ { UNICODE_MODE, 0, -1, -1, -1, "\015*>\015*>\015*>\015*>", 0, 0, 8, 32, 1, "EE 00 2B 00 2B 00 2B 00 2B FE 6E 95 3A 10 58 4E 96 06 79 09 94", "X12 symbols_left 1, process_p 0, ASC unlatch at end", 0 },
        /* 82*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C", 0, 0, 14, 14, 1, "F0 00 1C 5E 0B 2F C3 81 2D 71 45 13 9B FF A1 B0 0B E2", "EDIFACT symbols_left 1, process_p 0", 0 },
        /* 83*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3", 0, 0, 14, 14, 1, "F0 00 1C 5E 0B 2F C3 34 81 E8 6C 9E CE 12 CB F5 58 3F", "EDIFACT symbols_left 1, process_p 1", 0 },
        /* 84*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3+", 0, 0, 8, 32, 1, "F0 00 1C 5E 0B 2F C3 CE B7 C0 33 C6 81 E1 63 6E 5E B4 27 30 C9", "EDIFACT symbols_left 3, process_p 2", 0 },
        /* 85*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3+D", 0, 0, 8, 32, 1, "F0 00 1C 5E 0B 2F C3 CE B1 1F 4D E1 79 04 2B BC 05 6C 38 73 39", "EDIFACT symbols_left 3, process_p 3", 0 },
        /* 86*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3+D4", 0, 0, 8, 32, 1, "F0 00 1C 5E 0B 2F C3 CE B1 34 F4 EC B3 DC 03 A3 1F B5 86 C3 F7", "EDIFACT symbols_left 0, process_p 0", 0 },
        /* 87*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3+D4=", 0, 0, 16, 16, 1, "F0 00 1C 5E 0B 2F C3 CE B1 34 3E 81 42 96 43 6E 92 0D A9 B1 65 3C CF 9B", "EDIFACT symbols_left 2, process_p 1", 0 },
        /* 88*/ { UNICODE_MODE, 0, -1, -1, -1, "@A1^B2?C3+D4=E", 0, 0, 16, 16, 1, "F0 00 1C 5E 0B 2F C3 CE B1 34 3E 46 AD 8C F2 D8 5D AF F3 65 08 1F E3 A5", "EDIFACT symbols_left 2, process_p 2", 0 },
        /* 89*/ { DATA_MODE, 0, -1, -1, -1, "\377\376", 0, 0, 12, 12, 1, "EB 80 EB 7F 81 6F A8 0F 21 6F 5F 88", "FN4 A7F FN4 A7E, 1 pad", 0 },
        /* 90*/ { DATA_MODE, 0, -1, -1, -1, "\377\376\375", 0, 0, 12, 12, 1, "E7 2C C0 55 E9 67 45 8A D2 7E A9 23", "BAS BFF BFE BFD, no padding", 0 },
        /* 91*/ { DATA_MODE, 3, -1, -1, -1, "\101\102\103\104\300\105\310", 0, 3, 16, 16, 0, "F1 04 42 43 E7 87 5B F1 03 1D 36 81 2C E3 87 24 2D FD 69 9C 87 FA 8A 73", "ECI 4 ASC A41 A42 BAS B43 B44 BC0 B45 BC8; BWIPP different encodation (uses only B256, same no. of codewords)", 0 },
        /* 92*/ { UNICODE_MODE, 26, -1, -1, -1, "ABCDÀEÈ", 0, 26, 12, 26, 1, "F1 1B E7 60 2D C4 5B F1 06 58 B3 C7 21 81 57 ED 3D C0 12 2E 6C 80 58 CC 2C 05 0D 31 FC 2D", "ECI 27 BAS B41 B42 B43 B44 BC3 B80 B45 BC3 B88", 0 },
        /* 93*/ { UNICODE_MODE, 0, -1, -1, -1, "β", ZINT_WARN_USES_ECI, 9, 12, 12, 1, "Warning F1 0A EB 63 81 41 56 DA C0 3D 2D CC", "ECI 10 FN4 A62", 0 },
        /* 94*/ { UNICODE_MODE, 127, -1, -1, -1, "A", 0, 127, 12, 12, 1, "F1 80 01 42 81 14 A2 86 07 F5 27 30", "ECI 128 A41", 0 },
        /* 95*/ { UNICODE_MODE, 16382, -1, -1, -1, "A", 0, 16382, 12, 12, 1, "F1 BF FE 42 81 29 57 AA A0 92 B2 45", "ECI 16383 A41", 0 },
        /* 96*/ { UNICODE_MODE, 810899, -1, -1, -1, "A", 0, 810899, 12, 12, 1, "F1 CC 51 05 42 BB A5 A7 8A C6 6E 0F", "ECI 810900 A41", 0 },
        /* 97*/ { UNICODE_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R05\\GA\\R\\E", 0, 0, 10, 10, 1, "EC 42 81 5D 17 49 F6 B6", "Macro05 A41", 0 },
        /* 98*/ { UNICODE_MODE, 0, -1, -1, -1, "ABCDEFGHIJKLM*", 0, 0, 16, 16, 1, "EE 59 E9 6D 24 80 5F 93 9A FE 4E 2B 09 FF 50 A2 83 BE 32 E1 2F 17 1E F3", "C40 == X12, p_r_6_2_1 true", 0 },
        /* 99*/ { UNICODE_MODE, 0, -1, -1, -1, "\015\015\015\015\015\015\015\015\015a\015\015\015\015\015\015\015", 0, 0, 12, 26, 1, "EE 00 01 00 01 00 01 FE 62 EE 00 01 00 01 FE 0E B5 9A 73 85 83 20 23 2C E0 EC EC BF 71 E0", "a not X12 encodable", 0 },
        /*100*/ { UNICODE_MODE, 0, -1, -1, -1, ".........a.......", 0, 0, 18, 18, 0, "(32) F0 BA EB AE BA EB AE B9 F0 62 2F 2F 2F 2F 2F 2F 2F 81 78 BE 1F 90 B8 89 73 66 DC BD", "a not EDIFACT encodable; BWIPP different encodation (switches to ASCII one dot before)", 0 },
        /*101*/ { GS1_MODE, 0, -1, -1, -1, "[90]........[91]....", 0, 0, 12, 26, 1, "E8 DC 2F 2F 2F 2F 2F 2F 2F 2F E8 DD 2F 2F 2F 2F C6 CC 13 68 0D 9D A9 A5 B8 D5 5A F3 7B 18", "Can't use GS1 EDIFACT if contains FNC1/GS", 0 },
        /*102*/ { GS1_MODE, 0, -1, -1, -1, "[90]........", 0, 0, 8, 32, 1, "E8 DC F0 BA EB AE BA EB AE 81 B1 C0 AB DA A5 92 AF E2 05 DE 56", "Can use GS1 EDIFACT if no FNC1/GS", 0 },
        /*103*/ { GS1_MODE, 0, -1, -1, -1, "[90]ABCDEFGH[91]ABCD", 0, 0, 12, 26, 1, "E8 DC E6 59 E9 6D 24 80 4A AA CE 59 E9 FE 45 81 6A 05 49 36 67 C8 00 DE 35 29 C5 9A 17 EA", "GS1 C40 ok", 0 },
        /*104*/ { GS1_MODE, 0, -1, -1, -1, "[90]ABCD", 0, 0, 14, 14, 1, "E8 DC 42 43 44 45 81 38 98 32 8C 23 4D 87 5A 95 04 A7", "Final ASC unlatch", 0 },
        /*105*/ { UNICODE_MODE, 0, -1, -1, -1, ">*\015>*\015>......", 0, 0, 12, 26, 0, "EE 0C A9 0C A9 FE 3F 2F 2F 2F 2F 2F 2F 81 57 ED 0F 29 C7 9D 5D 64 61 94 14 CD A8 DF 65 8D", "X12 then ASC; BWIPP different encodation", 1 },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[8192];
    char bwipp_buf[32768];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    testStart("test_input");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        symbol->debug = ZINT_DEBUG_TEST; // Needed to get codeword dump in errtxt

        length = testUtilSetSymbol(symbol, BARCODE_DATAMATRIX, data[i].input_mode, data[i].eci, -1 /*option_1*/, data[i].option_2, data[i].option_3, data[i].output_options, data[i].data, -1, debug);

        // Check minimize only? (TODO: remove in the not-too-distant future)
        if (debug & ZINT_DEBUG_TEST_MINIMIZE) { // -d 512
            unsigned char binary[2][2200];
            int inputlen;
            int binlen;
            int variant;
            int binlens[2] = {0};

            if (data[i].ret != 0) {
                ZBarcode_Delete(symbol);
                continue;
            }

            variant = 0;
            inputlen = length;
            binlen = 0;
            ret = dm200encode_variant(symbol, (unsigned char *) data[i].data, binary[0], &inputlen, &binlen, variant);
            assert_equal(ret, data[i].ret, "i:%d dm200encode_variant(%d) ret %d != %d (%s)\n", variant, i, ret, data[i].ret, symbol->errtxt);

            binlens[variant] = binlen;

            variant = 1;
            inputlen = length;
            binlen = 0;
            ret = dm200encode_variant(symbol, (unsigned char *) data[i].data, binary[1], &inputlen, &binlen, variant);
            assert_equal(ret, data[i].ret, "i:%d dm200encode_variant(%d) ret %d != %d (%s)\n", variant, i, ret, data[i].ret, symbol->errtxt);

            binlens[variant] = binlen;

            assert_equal(binlens[0], binlens[1] + data[i].expected_diff, "i:%d binlens[0] %d != %d binlens[1] (%d) + expected_diff (%d)\n",
                        i, binlens[0], binlens[1] + data[i].expected_diff, binlens[1], data[i].expected_diff);

            if (debug & ZINT_DEBUG_TEST_PRINT) {
                if (data[i].expected_diff == 0) {
                    if (memcmp(binary[0], binary[1], binlen) != 0) {
                        printf("i:%d binaries differ\n", i);
                    }
                } else {
                    printf("i:%d diff %d\n", i, data[i].expected_diff);
                }
            }

            ZBarcode_Delete(symbol);
            continue;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d (%s)\n", i, ret, data[i].ret, symbol->errtxt);

        if (generate) {
            printf("        /*%3d*/ { %s, %d, %d, %s, %s, \"%s\", %s, %d, %d, %d, %d, \"%s\", \"%s\", %d },\n",
                    i, testUtilInputModeName(data[i].input_mode), data[i].eci, data[i].option_2, testUtilOption3Name(data[i].option_3),
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
                        char modules_dump[8192];
                        assert_notequal(testUtilModulesDump(symbol, modules_dump, sizeof(modules_dump)), -1, "i:%d testUtilModulesDump == -1\n", i);
                        ret = testUtilBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
                        assert_zero(ret, "i:%d %s testUtilBwipp ret %d != 0\n", i, testUtilBarcodeName(symbol->symbology), ret);

                        ret = testUtilBwippCmp(symbol, bwipp_msg, bwipp_buf, modules_dump);
                        assert_zero(ret, "i:%d %s testUtilBwippCmp %d != 0 %s\n  actual: %s\nexpected: %s\n",
                                       i, testUtilBarcodeName(symbol->symbology), ret, bwipp_msg, bwipp_buf, modules_dump);
                    }
                }
            }
        }

        ZBarcode_Delete(symbol);
    }

    testFinish();
}

static void test_encode(int index, int generate, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int eci;
        int output_options;
        int option_2;
        int option_3;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        int bwipp_cmp;
        char *comment;
        int expected_diff; // Check minimize only (TODO: remove in the not-too-distant future)
        char *expected;
    };
    // Verified manually against ISO/IEC 16022:2006, ISO/IEC 21471:2020, GS1 General Specifications 21.0.1 (GGS), ANSI/HIBC LIC 2.6-2016 (HIBC/LIC) and
    // ANSI/HIBC PAS 1.3-2010 (HIBC/PAS), with noted exceptions
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "1234abcd", 0, 14, 14, 1, "", 0,
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
        /*  1*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "A1B2C3D4E5F6G7H8I9J0K1L2", 0, 18, 18, 1, "16022:2006 Figure 1", 0,
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
        /*  2*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "123456", 0, 10, 10, 1, "16022:2006 Figure O.2", 0,
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
        /*  3*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "30Q324343430794<OQQ", 0, 16, 16, 1, "16022:2006 Figure R.1", 0,
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
        /*  4*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, 32, -1, "A1B2C3D4E5F6G7H8I9J0K1L2", 0, 8, 64, 1, "21471:2020 Figure 1", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1010001010111001101010111011111110001110000000011101010100010101"
                    "1011000000000110110111111110111010110000101001101101001000010110"
                    "1000001000110011110001111011111110001100101010111011111111001111"
                    "1001000111011100101010110001011010010111001000101111101011101100"
                    "1010101011111011100000000001000110101110011111011000101011000111"
                    "1101101100110110111100110000001011100011001001101011001001001000"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /*  5*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, 31, -1, "123456789012345678901234567890123456", 0, 8, 48, 1, "21471:2020 Figure H.3", 0,
                    "101010101010101010101010101010101010101010101010"
                    "110010010100000111110001101001010110100110001011"
                    "110011000111101000101010110111001110011011011010"
                    "110001110010001100001001100011001100110111000111"
                    "111010010010110111101000110001110001000011000000"
                    "100011010010011010111111111101010010100111000001"
                    "101101111110001011010010111010001111110101101110"
                    "111111111111111111111111111111111111111111111111"
                },
        /*  6*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, 29, -1, "30Q324343430794<OQ", 0, 16, 36, 1, "21471:2020 Figure J.1 NOTE: single Q at end, not 2; also not DMRE", 0,
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
        /*  7*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "https://example.com/01/09506000134369", 0, 22, 22, 0, "GGS Figure 2.1.13.1 (and 5.1-9) same; note not GS1; BWIPP different encodation", 0,
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
        /*  8*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]09501101530003[17]150119[10]AB-123", 0, 20, 20, 1, "GGS Figure 2.6.14-3", 0,
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
        /*  9*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]04012345678901[21]ABCDEFG123456789", 0, 20, 20, 1, "GGS Figure 2.6.14-4", 0,
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
        /* 10*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]04012345678901[17]170101[10]ABC123", 0, 20, 20, 1, "GGS Figure 4.15-1 (and 5.1-6)", 0,
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
        /* 11*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, GS1_GS_SEPARATOR, -1, -1, "[01]09504000059101[21]12345678p901[10]1234567p[17]141120[8200]http://www.gs1.org/demo/", 0, 32, 32, 1, "GGS Figure 4.15.1-1 **NOT SAME**, uses 0-padded final TEXT triplet", 0,
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
        /* 12*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]09504000059101[21]12345678p901[10]1234567p[17]141120[8200]http://www.gs1.org/demo/", 0, 32, 32, 1, "GGS Figure 4.15.1-2 (and 4.15.1-3) **NOT SAME**, uses 0-padded final TEXT triplet (as does tec-it)", 0,
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
        /* 13*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]09512345678901[15]170810[21]abcde", 0, 20, 20, 1, "GGS Figure 5.6.2-1", 0,
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
        /* 14*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]00012345678905[17]040115", 0, 12, 26, 1, "GGS Figure 5.6.3.1-1 (left)", 0,
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
        /* 15*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, DM_SQUARE, "[01]00012345678905[17]040115", 0, 18, 18, 1, "GGS Figure 5.6.3.1-1 (right)", 0,
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
        /* 16*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[01]00012345678905[17]180401[21]ABCDEFGHIJKL12345678[91]ABCDEFGHI123456789[92]abcdefghi", 0, 32, 32, 0, "GGS Figure 5.6.3.2-3 (left) **NOT SAME** different encodation; BWIPP different encodation", 0,
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
        /* 17*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, 30, -1, "[01]00012345678905[17]180401[21]ABCDEFGHIJKL12345678[91]abcdefghi", 0, 16, 48, 1, "GGS Figure 5.6.3.2-3 (right) **NOT SAME** different encodation", 0,
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
        /* 18*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, DM_SQUARE, "[00]395011010013000129[403]123+1021JK+0320+12[421]5281500KM", 0, 24, 24, 1, "GGS Figure 6.6.5-6 **NOT SAME** figure has unnecessary FNC1 at end of data", 0,
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
        /* 19*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, -1, -1, "[00]093123450000000012[421]0362770[401]931234518430GR[403]MEL", 0, 24, 24, 1, "GGS Figure 6.6.5-7 **NOT SAME** different encodation", 0,
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
        /* 20*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "A123BJC5D6E71", 0, 16, 16, 0, "HIBC/LIC Figure 3 same; BWIPP different encodation, same no. of codewords", 0,
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
        /* 21*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "A123BJC5D6E71/$$52001510X3", 0, 20, 20, 0, "HIBC/LIC Section 4.3.3 **NOT SAME** different encodation; also figure has weird CRLF after check digit; BWIPP different encodation", 0,
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
        /* 22*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "H123ABC01234567890", 0, 12, 26, 1, "HIBC/LIC Figure C2, same", 0,
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
        /* 23*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, DM_SQUARE, "/ACMRN123456/V200912190833", 0, 20, 20, 1, "HIBC/PAS Section 2.2 Patient Id, same", 0,
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
        /* 24*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R06\\G+/ACMRN123456/V2009121908334\\R\\E", 0, 20, 20, 1, "HIBC/PAS 1.3-2010 Section 2.2 Patient Id Macro, same", 0,
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
        /* 25*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/EO523201", 0, 14, 14, 1, "HIBC/PAS Section 2.2 Purchase Order, same", 0,
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
        /* 26*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, DM_SQUARE, "/EU720060FF0/O523201", 0, 18, 18, 1, "HIBC/PAS Section 2.2 2nd Purchase Order, same", 0,
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
        /* 27*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/EU720060FF0/O523201/Z34H159/M9842431340", 0, 22, 22, 1, "HIBC/PAS Section 2.2 3rd Purchase Order (left), same", 0,
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
        /* 28*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R06\\G+/EU720060FF0/O523201/Z34H159/M9842431340V\\R\\E", 0, 22, 22, 1, "HIBC/PAS Section 2.2 3rd Purchase Order (right), same", 0,
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
        /* 29*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/E+/KN12345", 0, 16, 16, 1, "HIBC/PAS Section 2.2 Asset Tag **NOT SAME** check digit 'A' in figure is for '/KN12345', but actual data is as given here, when check digit is 'J'", 0,
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
        /* 30*/ { BARCODE_HIBC_DM, -1, -1, -1, -1, -1, "/LAH123/NC903", 0, 16, 16, 1, "HIBC/PAS Section 2.2 Surgical Instrument, same", 0,
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
        /* 31*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, 7, -1, "[)>\\R06\\G18VD89536\\G1P8902A\\GS3122A02965\\R\\E", 0, 22, 22, 1, "ANSI MH10.8.17-2017 Figure 4 Macro06 **NOT SAME** 253-state randomising of padding in figure seems incorrect", 0,
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
        /* 32*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R06\\G25S0614141MH80312\\R\\E", 0, 16, 16, 1, "ANSI MH10.8.17-2017 Table B.1 B7", 0,
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
        /* 33*/ { BARCODE_DATAMATRIX, DATA_MODE | ESCAPE_MODE, -1, -1, -1, -1, "[)>\\R05\\G80040614141MH80312\\R\\E", 0, 16, 16, 1, "ANSI MH10.8.17-2017 Table B.1 B8", 0,
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
        /* 34*/ { BARCODE_DATAMATRIX, DATA_MODE, 3, -1, -1, -1, "\101\300", 0, 12, 12, 1, "AÀ", 0,
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
        /* 35*/ { BARCODE_DATAMATRIX, UNICODE_MODE, 26, -1, -1, -1, "AÀ", 0, 14, 14, 1, "AÀ", 0,
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
        /* 36*/ { BARCODE_DATAMATRIX, UNICODE_MODE, -1, -1, -1, DM_SQUARE, "abcdefgh+", 0, 16, 16, 1, "TEX last_shift 2, symbols_left 1, process_p 1", 0,
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
        /* 37*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200", 0, 8, 32, 1, "7 BASE256s, 1 pad", 0,
                    "10101010101010101010101010101010"
                    "10000101000011011000110100100001"
                    "11100111110101001011101110100010"
                    "10111011010100111110010110001011"
                    "11001000110001101000001111000010"
                    "11000010000001111000100101001011"
                    "11010000111100001010011101100100"
                    "11111111111111111111111111111111"
                },
        /* 38*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200", 0, 8, 32, 1, "8 BASE256s, no padding", 0,
                    "10101010101010101010101010101010"
                    "10000101000011011111001101000001"
                    "11010111110101001001011001100010"
                    "11001011010111111010001100100011"
                    "11001000110000101011101100011010"
                    "11000010000001111000010010110011"
                    "11010000110010001001010001111000"
                    "11111111111111111111111111111111"
                },
        /* 39*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, DM_SQUARE, "\200\200\200\200\200\200\200\200\200\200", 0, 16, 16, 1, "8 BASE256s, square, no padding", 0,
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
        /* 40*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200", 0, 16, 16, 1, "9 BASE256s, 1 pad", 0,
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
        /* 41*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200", 0, 22, 22, 1, "22 BASE256s, 6 pads", 0,
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
        /* 42*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, DM_DMRE, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200", 0, 8, 64, 1, "22 BASE256s, no padding", 0,
                    "1010101010101010101010101010101010101010101010101010101010101010"
                    "1000010101100011101010101011101111110100100110011100010011010111"
                    "1101011110001010110000001110001010001011010111001010101101100000"
                    "1100101000110001110100000001100110010100111101111110000010011111"
                    "1000100101001000110110101110011011110110111010101110010111001100"
                    "1100001101011011111101111000110110110101110110111111011010011111"
                    "1101000011001010111101101101110010111100111101001010010011001000"
                    "1111111111111111111111111111111111111111111111111111111111111111"
                },
        /* 43*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066", 0, 64, 64, 1, "249 BASE256s + 6 ASCII (3 double-digits)", 0,
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
        /* 44*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066", 0, 64, 64, 0, "249 BASE256s + 8 ASCII (Sh A80 + 3 double-digits); BWIPP uses 2nd B256 length byte instead of ASCII shift (same no. of codewords)", 0,
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
        /* 45*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\101\102\103\104\105\106\107\110\111\112\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066", 0, 64, 64, 1, "10 ASCII + 251 BASE256s + 6 ASCII", 0,
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
        /* 46*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\101\102\103\104\105\106\107\110\111\112\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066\067\070\071\060\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066", 0, 88, 88, 1, "10 ASCII + 252 BASE256s + 10 ASCII + 253 BASE256 + 6 ASCII", 0,
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
        /* 47*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "\101\102\103\104\105\106\107\110\111\112\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\061\062\063\064\065\066\067\070\071\060\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200\200", 0, 88, 88, 1, "10 ASCII + 252 BASE256s + 10 ASCII + 304 BASE256, no padding", 0,
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
        /* 48*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "@@@@@@@@@_", 0, 8, 32, 0, "EDI; BWIPP uses different encodation, switching to ASC for last 2 chars, not last 3 like Zint", 0,
                    "10101010101010101010101010101010"
                    "10000000001001111001101100001101"
                    "10000000000001001001110011001100"
                    "10000000000110011111100101100001"
                    "11000000110101101100001101111000"
                    "11000001110100111000111101101011"
                    "11000000000000001001000001011010"
                    "11111111111111111111111111111111"
                },
        /* 49*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "@@@@@@@@@@_", 0, 16, 16, 0, "EDI; BWIPP uses different encodation, switching to ASC for last 3 chars, not last 4 like Zint", 0,
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
        /* 50*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "@@@@@@@@@@@_", 0, 16, 16, 0, "EDI; BWIPP uses different encodation, switching to ASC for last 4 chars, not last 1 like Zint", 1,
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
        /* 51*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "@@@@@@@@@@@@_", 0, 16, 16, 0, "EDI; BWIPP uses different encodation, switching to ASC for last 5 chars, not last 2 like Zint", 0,
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
        /* 52*/ { BARCODE_DATAMATRIX, -1, -1, -1, -1, -1, "@@@@@@@@@@@@@_", 0, 12, 26, 0, "EDI; BWIPP uses different encodation, switching to ASC for last 2 chars, not last 3 like Zint", 0,
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
        /* 53*/ { BARCODE_DATAMATRIX, -1, 26, -1, -1, -1, "abcdefghi1234FGHIJKLMNabc@@@@@@@@@é", 0, 24, 24, 0, "Mix of modes TEX ASC C40 ASC EDI BAS; BWIPP uses different encodation", 0,
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
        /* 54*/ { BARCODE_DATAMATRIX, DATA_MODE, -1, -1, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^;<>@[]_`~!||()?{}'ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^;<>@[]_`~!||()?{}'\001\002\003\004\005\006...............\015\015\015\015\015\015\015\015abcdefghijklmnopqrstuvwxyz\015\015\015\015\015\015\015\015...............\001\002\003\004\005\006ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^...............", 0, 132, 132, 0, "Mixed modes (except B256); BWIPP different encodation", 4,
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
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    char escaped[8192];
    char bwipp_buf[32768];
    char bwipp_msg[1024];

    int do_bwipp = (debug & ZINT_DEBUG_TEST_BWIPP) && testUtilHaveGhostscript(); // Only do BWIPP test if asked, too slow otherwise

    testStart("test_encode");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, data[i].eci, -1 /*option_1*/, data[i].option_2, data[i].option_3, data[i].output_options, data[i].data, -1, debug);

        // Check minimize only? (TODO: remove in the not-too-distant future)
        if (debug & ZINT_DEBUG_TEST_MINIMIZE) { // -d 512
            unsigned char binary[2][2200];
            int inputlen;
            int binlen;
            int variant;
            int binlens[2] = {0};

            if (data[i].ret != 0) {
                ZBarcode_Delete(symbol);
                continue;
            }

            variant = 0;
            inputlen = length;
            binlen = 0;
            ret = dm200encode_variant(symbol, (unsigned char *) data[i].data, binary[0], &inputlen, &binlen, variant);
            assert_zero(ret, "i:%d dm200encode_variant(%d) ret %d != 0 (%s)\n", variant, i, ret, symbol->errtxt);

            binlens[variant] = binlen;

            variant = 1;
            inputlen = length;
            binlen = 0;
            ret = dm200encode_variant(symbol, (unsigned char *) data[i].data, binary[1], &inputlen, &binlen, variant);
            assert_equal(ret, data[i].ret, "i:%d dm200encode_variant(%d) ret %d != %d (%s)\n", variant, i, ret, data[i].ret, symbol->errtxt);

            binlens[variant] = binlen;

            assert_equal(binlens[0], binlens[1] + data[i].expected_diff, "i:%d binlens[0] %d != %d binlens[1] (%d) + expected_diff (%d)\n",
                        i, binlens[0], binlens[1] + data[i].expected_diff, binlens[1], data[i].expected_diff);

            if (debug & ZINT_DEBUG_TEST_PRINT) {
                if (data[i].expected_diff == 0) {
                    if (memcmp(binary[0], binary[1], binlen) != 0) {
                        printf("i:%d binaries differ\n", i);
                    }
                } else {
                    printf("i:%d diff %d\n", i, data[i].expected_diff);
                }
            }

            ZBarcode_Delete(symbol);
            continue;
        }

        ret = ZBarcode_Encode(symbol, (unsigned char *) data[i].data, length);
        assert_equal(ret, data[i].ret, "i:%d ZBarcode_Encode ret %d != %d\n", i, ret, data[i].ret);

        if (generate) {
            printf("        /*%3d*/ { %s, %s, %d, %s, %d, %s, \"%s\", %s, %d, %d, %d, \"%s\", %d,\n",
                    i, testUtilBarcodeName(data[i].symbology), testUtilInputModeName(data[i].input_mode), data[i].eci,
                    testUtilOutputOptionsName(data[i].output_options),
                    data[i].option_2, testUtilOption3Name(data[i].option_3),
                    testUtilEscape(data[i].data, length, escaped, sizeof(escaped)),
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
                        ret = testUtilBwipp(i, symbol, -1, data[i].option_2, data[i].option_3, data[i].data, length, NULL, bwipp_buf, sizeof(bwipp_buf));
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

#include <time.h>

#define TEST_PERF_ITER_MILLES   5
#define TEST_PERF_ITERATIONS    (TEST_PERF_ITER_MILLES * 1000)
#define TEST_PERF_TIME(arg)     (((arg) * 1000.0) / CLOCKS_PER_SEC)

// Not a real test, just performance indicator
static void test_perf(int index, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int option_2;
        char *data;
        int ret;

        int expected_rows;
        int expected_width;
        char *comment;
    };
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, -1,
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                    "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK"
                    "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG"
                    "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567"
                    "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde"
                    "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO",
                    0, 96, 96, "960 chars, text/numeric" },
        /*  1*/ { BARCODE_DATAMATRIX, DATA_MODE, -1,
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
        /*  2*/ { BARCODE_DATAMATRIX, -1, -1, "https://example.com/01/09506000134369", 0, 22, 22, "37 chars, text/numeric" },
    };
    int data_size = ARRAY_SIZE(data);
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

        if (index != -1 && i != index) continue;

        diff_create = diff_encode = diff_buffer = diff_buf_inter = diff_print = 0;

        for (j = 0; j < TEST_PERF_ITERATIONS; j++) {
            start = clock();
            symbol = ZBarcode_Create();
            diff_create += clock() - start;
            assert_nonnull(symbol, "Symbol not created\n");

            length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, -1 /*output_options*/, data[i].data, -1, debug);

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
            symbol->output_options &= ~OUT_BUFFER_INTERMEDIATE; // Undo

            start = clock();
            ret = ZBarcode_Print(symbol, 0 /*rotate_angle*/);
            diff_print += clock() - start;
            assert_zero(ret, "i:%d ZBarcode_Print ret %d != 0 (%s)\n", i, ret, symbol->errtxt);
            assert_zero(remove(symbol->outfile), "i:%d remove(%s) != 0\n", i, symbol->outfile);

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
    if (index == -1) {
        printf("%*s: encode % 8gms, buffer % 8gms, buf_inter % 8gms, print % 8gms, create % 8gms\n", comment_max, "totals",
                TEST_PERF_TIME(total_encode), TEST_PERF_TIME(total_buffer), TEST_PERF_TIME(total_buf_inter), TEST_PERF_TIME(total_print), TEST_PERF_TIME(total_create));
    }
}

// Not a real test, just minimization indicator (TODO: remove in the not-too-distant future)
static void test_minimize(int index, int debug) {

    struct item {
        int symbology;
        int input_mode;
        int output_options;
        int option_2;
        char *data;
        int length;
        int ret;

        int expected_diff;
        char *comment;
    };
    // s/\/\*[ 0-9]*\*\//\=printf("\/*%3d*\/", line(".") - line("'<"))
    struct item data[] = {
        /*  0*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200A", -1, 0, 0, "ASC last 1" },
        /*  1*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AA", -1, 0, 0, "ASC 2" },
        /*  2*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAA", -1, 0, 0, "ASC 3" },
        /*  3*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAA", -1, 0, 0, "ASC 4" },
        /*  4*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAA", -1, 0, 0, "ASC 5" },
        /*  5*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAA", -1, 0, 1, "C40 6" },
        /*  6*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAA", -1, 0, 0, "C40" },
        /*  7*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAA", -1, 0, 1, "C40" },
        /*  8*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAA", -1, 0, 2, "C40" },
        /*  9*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAA", -1, 0, 1, "C40 except last ASC" },
        /* 10*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAA", -1, 0, 1, "C40 except last 2 ASC" },
        /* 11*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAAA", -1, 0, 2, "C40 except last ASC" },
        /* 12*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAAAA", -1, 0, 2, "C40 except last 2 ASC" },
        /* 13*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009", -1, 0, 0, "ASC last 1" },
        /* 14*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099", -1, 0, -1, "Switches too early to ASC (EOD), for last 3" },
        /* 15*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999", -1, 0, 0, "" },
        /* 16*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999", -1, 0, 0, "" },
        /* 17*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999", -1, 0, 0, "" },
        /* 18*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999", -1, 0, 0, "" },
        /* 19*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999", -1, 0, 0, "" },
        /* 20*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999", -1, 0, 0, "" },
        /* 21*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999", -1, 0, 0, "" },
        /* 22*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999", -1, 0, 0, "" },
        /* 23*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999999", -1, 0, 0, "" },
        /* 24*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999999", -1, 0, 0, "" },
        /* 25*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999999", -1, 0, 0, "" },
        /* 26*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200a", -1, 0, 0, "ASC last 1" },
        /* 27*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aa", -1, 0, 0, "ASC 2" },
        /* 28*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaa", -1, 0, 0, "ASC 3" },
        /* 29*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaa", -1, 0, 0, "ASC 4" },
        /* 30*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaa", -1, 0, 0, "ASC 5" },
        /* 31*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaa", -1, 0, 1, "TEX 6" },
        /* 32*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa", -1, 0, 0, "TEX" },
        /* 33*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaa", -1, 0, 1, "TEX" },
        /* 34*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaa", -1, 0, 2, "TEX" },
        /* 35*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaa", -1, 0, 1, "TEX except last ASC" },
        /* 36*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaa", -1, 0, 1, "TEX except last 2 ASC" },
        /* 37*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaa", -1, 0, 2, "TEX" },
        /* 38*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaa", -1, 0, 2, "TEX except last ASC" },
        /* 39*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>", -1, 0, 0, "ASC last 1" },
        /* 40*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>", -1, 0, 0, "ASC 2" },
        /* 41*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>", -1, 0, 0, "ASC 3" },
        /* 42*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>", -1, 0, 0, "ASC 4" },
        /* 43*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>", -1, 0, 0, "ASC 5" },
        /* 44*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>", -1, 0, 1, "X12 6" },
        /* 45*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>", -1, 0, 0, "X12 except last 2 ASC" },
        /* 46*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>", -1, 0, 1, "X12" },
        /* 47*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>", -1, 0, 2, "X12" },
        /* 48*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>", -1, 0, 1, "X12 except last 2 ASC" },
        /* 49*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>", -1, 0, 1, "X12 except last 3 ASC" },
        /* 50*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>", -1, 0, 2, "X12" },
        /* 51*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>", -1, 0, 2, "X12 except last ASC" },
        /* 52*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200.", -1, 0, 0, "ASC last 1" },
        /* 53*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200..", -1, 0, 0, "ASC 2" },
        /* 54*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200...", -1, 0, 0, "ASC 3" },
        /* 55*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200....", -1, 0, 0, "ASC 4" },
        /* 56*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200.....", -1, 0, 0, "ASC 5" },
        /* 57*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200......", -1, 0, 0, "ASC 6" },
        /* 58*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200.......", -1, 0, 0, "ASC 7" },
        /* 59*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200........", -1, 0, 1, "EDI 8" },
        /* 60*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200.........", -1, 0, 0, "EDI" },
        /* 61*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200..........", -1, 0, 0, "EDI" },
        /* 62*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200...........", -1, 0, 1, "EDI" },
        /* 63*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200............", -1, 0, 2, "EDI" },
        /* 64*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200.............", -1, 0, 2, "EDI except last ASC" },
        /* 65*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200A\200\200\200", -1, 0, 0, "BAS" },
        /* 66*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AA\200\200\200", -1, 0, 0, "BAS" },
        /* 67*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAA\200\200\200", -1, 0, 0, "BAS" },
        /* 68*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAA\200\200\200", -1, 0, 0, "BAS" },
        /* 69*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAA\200\200\200", -1, 0, 0, "BAS" },
        /* 70*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAA\200\200\200", -1, 0, 0, "BAS" },
        /* 71*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAA\200\200\200", -1, 0, 0, "BAS 7 As, no switch as C40 & X12 cancel each other out" },
        /* 72*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAA\200\200\200", -1, 0, 0, "BAS 8 As, no switch as C40 & X12 cancel each other out" },
        /* 73*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAA\200\200\200", -1, 0, 0, "BAS 9 As, no switch as C40 & X12 cancel each other out" },
        /* 74*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAA\200\200\200", -1, 0, 0, "BAS 10 As, no switch as C40 & X12 cancel each other out" },
        /* 75*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAA\200\200\200", -1, 0, 0, "BAS 11 As, no switch as C40 & X12 cancel each other out" },
        /* 76*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAAA\200\200\200", -1, 0, 0, "BAS 12 As, no switch as C40 & X12 cancel each other out" },
        /* 77*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200AAAAAAAAAAAAA\200\200\200", -1, 0, 0, "BAS 13 As, no switch as C40 & X12 cancel each other out" },
        /* 78*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009\200\200\200", -1, 0, 0, "BAS" },
        /* 79*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099\200\200\200", -1, 0, 0, "BAS" },
        /* 80*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999\200\200\200", -1, 0, 0, "BAS" },
        /* 81*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999\200\200\200", -1, 0, 0, "ASC 4" },
        /* 82*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999\200\200\200", -1, 0, 0, "ASC 4" },
        /* 83*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999\200\200\200", -1, 0, 0, "ASC 6" },
        /* 84*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999\200\200\200", -1, 0, 0, "ASC 6" },
        /* 85*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999\200\200\200", -1, 0, 0, "ASC 8" },
        /* 86*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999\200\200\200", -1, 0, 0, "ASC 8" },
        /* 87*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999\200\200\200", -1, 0, 0, "ASC 10" },
        /* 88*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\20099999999999\200\200\200", -1, 0, 0, "ASC 10" },
        /* 89*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200999999999999\200\200\200", -1, 0, 0, "ASC 12" },
        /* 90*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\2009999999999999\200\200\200", -1, 0, 0, "ASC 12" },
        /* 91*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200a\200\200\200", -1, 0, 0, "BAS" },
        /* 92*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aa\200\200\200", -1, 0, 0, "BAS" },
        /* 93*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaa\200\200\200", -1, 0, 0, "BAS" },
        /* 94*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaa\200\200\200", -1, 0, 0, "BAS" },
        /* 95*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaa\200\200\200", -1, 0, 0, "BAS" },
        /* 96*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaa\200\200\200", -1, 0, 0, "BAS" },
        /* 97*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa\200\200\200", -1, 0, -2, "7 a's, worse - switches to TEX" },
        /* 98*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa\200\200", -1, 0, -2, "7 a's end 2 extended, worse - switches to TEX" },
        /* 99*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa\200", -1, 0, -1, "7 a's end 1 extended, worse - switches to TEX" },
        /*100*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa", -1, 0, 0, "7 a's end 0 extended, switches to TEX but same codeword count" },
        /*101*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200\200aaaaaaa\200\200", -1, 0, -2, "7 a's, worse - switches to TEX" },
        /*102*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200\200\200aaaaaaa\200\200", -1, 0, -2, "7 a's, worse - switches to TEX" },
        /*103*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa\200\200\200\200", -1, 0, -2, "7 a's end 4 extended, worse - switches to TEX" },
        /*104*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaa\200\200\200\200\200", -1, 0, -2, "7 a's end 5 extended, worse - switches to TEX" },
        /*105*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaa\200\200", -1, 0, -2, "8 a's end 2 extended, worse - switches to TEX" },
        /*106*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaa\200\200\200", -1, 0, -2, "8 a's, worse - switches to TEX" },
        /*107*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaa\200\200\200", -1, 0, -1, "9 a's, worse - switches to TEX" },
        /*108*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaa\200\200\200", -1, 0, -1, "10 a's, worse - switches to TEX" },
        /*109*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaa\200\200\200", -1, 0, -1, "11 a's, worse - switches to TEX" },
        /*110*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaa\200\200\200", -1, 0, 0, "12 a's, switches to TEX but same codeword count" },
        /*111*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaa\200\200\200", -1, 0, 0, "13 a's, switches to TEX but same codeword count" },
        /*112*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaa\200\200\200", -1, 0, 0, "14 a's, switches to TEX but same codeword count" },
        /*113*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaa\200\200\200", -1, 0, 1, "15 a's, switches to TEX, better" },
        /*114*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaa\200\200\200", -1, 0, 1, "16 a's, switches to TEX, better" },
        /*115*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaaa\200\200\200", -1, 0, 1, "17 a's, switches to TEX, better" },
        /*116*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaaaa\200\200\200", -1, 0, 2, "18 a's, switches to TEX, better" },
        /*117*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaaaaa\200\200\200", -1, 0, 2, "19 a's, switches to TEX, better" },
        /*118*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaaaaaa\200\200\200", -1, 0, 2, "20 a's, switches to TEX, better" },
        /*119*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaaaaaaaaaaaaaaa\200\200\200", -1, 0, 3, "21 a's, switches to TEX, better" },
        /*120*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200Aaaaaaa\200\200\200", -1, 0, 0, "" },
        /*121*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaAaaa\200\200\200", -1, 0, 0, "" },
        /*122*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaA\200\200\200", -1, 0, 0, "" },
        /*123*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200Aaaaaaaa\200\200\200", -1, 0, -2, "" },
        /*124*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaAaaa\200\200\200", -1, 0, 0, "" },
        /*125*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaAa\200\200\200", -1, 0, 0, "" },
        /*126*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaA\200\200\200", -1, 0, -2, "" },
        /*127*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200aaaaaaaAa\200\200\200", -1, 0, -2, "" },
        /*128*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>\200\200\200", -1, 0, 0, "BAS" },
        /*129*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>\200\200\200", -1, 0, 0, "BAS" },
        /*130*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>\200\200\200", -1, 0, 0, "BAS" },
        /*131*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*132*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*133*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*134*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*135*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*136*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*137*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*138*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*139*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>\200\200\200", -1, 0, 0, "BAS" },
        /*140*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>\200\200\200", -1, 0, 0, "X12 12" },
        /*141*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>\200\200\200", -1, 0, 0, "X12 12" },
        /*142*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>\200\200\200", -1, 0, 0, "X12 12" },
        /*143*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 1, "X12 15" },
        /*144*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 1, "X12 15" },
        /*145*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 1, "X12 15" },
        /*146*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 2, "X12 18" },
        /*147*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 2, "X12 18" },
        /*148*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 2, "X12 18" },
        /*149*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200>>>>>>>>>>>>>>>>>>>>>>\200\200\200", -1, 0, 3, "X12 21" },
        /*150*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\200\200\200|||||||||||||", -1, 0, 0, "Variant 1 last 3 ASC" },
        /*151*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAA|||||||||||AAAAAAAAAAAAAAA", -1, 0, 0, "" },
        /*152*/ { BARCODE_DATAMATRIX, -1, -1, -1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz&,:#-.$/+%*=^ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM" "NOPQRSTUVWXYZ;<>@[]_`~!||()?{}'123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJK" "LMNOPQRSTUVWXYZ12345678912345678912345678912345678900001234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFG" "HIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678901234567" "890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde" "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNO", -1, 0, 0, "960 chars, text/numeric" },
        /*153*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\000\004\a\b\t\n\v\f\r\033\035\036\201\\", 14, 0, 0, "From test_library escape_char_process 0, variant 1 later switch to B256 and changes to ASC for last 2 chars" },
        /*154*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAA*+*0**AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 0, 0, "" },
        /*155*/ { BARCODE_DATAMATRIX, -1, -1, -1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAA", -1, 0, 0, "" },
        /*156*/ { BARCODE_DATAMATRIX, -1, -1, -1, "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037 !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\177", 128, 0, 0, "" },
        /*157*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, "\200\200\200[\200\200[\200\200", -1, 0, 0, "Can't have extended ASCII in GS1 mode so these will never happen" },
        /*158*/ { BARCODE_DATAMATRIX, GS1_MODE, GS1_GS_SEPARATOR, -1, "\200\200\200[\200\200[\200\200", -1, 0, 0, "" },
        /*159*/ { BARCODE_DATAMATRIX, GS1_MODE, -1, -1, "\200\200\200\200[\200\200\200[\200\200", -1, 0, -1, "Stays in ASC after 1st FNC1" },
        /*160*/ { BARCODE_DATAMATRIX, GS1_MODE, GS1_GS_SEPARATOR, -1, "\200\200\200\200[\200\200\200[\200\200", -1, 0, 0, "" },
    };
    int data_size = ARRAY_SIZE(data);
    int i, length, ret;
    struct zint_symbol *symbol;

    unsigned char binary[2][2200];
    int inputlen;
    int binlen;
    int variant;
    int binlens[2] = {0};

    if (!(debug & ZINT_DEBUG_TEST_MINIMIZE)) { // -d 512
        return;
    }

    testStart("test_minimize");

    for (i = 0; i < data_size; i++) {

        if (index != -1 && i != index) continue;

        symbol = ZBarcode_Create();
        assert_nonnull(symbol, "Symbol not created\n");

        length = testUtilSetSymbol(symbol, data[i].symbology, data[i].input_mode, -1 /*eci*/, -1 /*option_1*/, data[i].option_2, -1, data[i].output_options, data[i].data, data[i].length, debug);

        variant = 0;
        inputlen = length;
        binlen = 0;
        ret = dm200encode_variant(symbol, (unsigned char *) data[i].data, binary[0], &inputlen, &binlen, variant);
        assert_equal(ret, data[i].ret, "i:%d dm200encode_variant(%d) ret %d != %d (%s)\n", variant, i, ret, data[i].ret, symbol->errtxt);

        binlens[variant] = binlen;

        variant = 1;
        inputlen = length;
        binlen = 0;
        ret = dm200encode_variant(symbol, (unsigned char *) data[i].data, binary[1], &inputlen, &binlen, variant);
        assert_equal(ret, data[i].ret, "i:%d dm200encode_variant(%d) ret %d != %d (%s)\n", variant, i, ret, data[i].ret, symbol->errtxt);

        binlens[variant] = binlen;

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
}

int main(int argc, char *argv[]) {

    testFunction funcs[] = { /* name, func, has_index, has_generate, has_debug */
        { "test_large", test_large, 1, 0, 1 },
        { "test_buffer", test_buffer, 1, 0, 1 },
        { "test_options", test_options, 1, 0, 1 },
        { "test_reader_init", test_reader_init, 1, 1, 1 },
        { "test_input", test_input, 1, 1, 1 },
        { "test_encode", test_encode, 1, 1, 1 },
        { "test_perf", test_perf, 1, 0, 1 },
        { "test_minimize", test_minimize, 1, 0, 1 },
    };

    testRun(argc, argv, funcs, ARRAY_SIZE(funcs));

    testReport();

    return 0;
}
